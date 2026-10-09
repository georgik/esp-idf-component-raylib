/*
 * AtomS3R LCD backlight control via the LP5562 LED driver (I2C, addr 0x30).
 *
 * The M5Stack AtomS3R does not drive the LCD backlight from a plain GPIO. The
 * white-backlight LEDs are powered by an LP5562 constant-current LED driver
 * that is reached over I2C (SDA = GPIO45, SCL = GPIO0, 7-bit addr 0x30). Until
 * the LP5562 is enabled and given a PWM set value the panel is dark, so the
 * display never lights up without this module.
 *
 * Register sequence and timing are taken from the working reference firmware
 * (m5stack-atom-s3r-joystick-usb, LP5562Backlight) verbatim.
 */

#include "backlight.h"

#include "esp_log.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "BACKLIGHT";

/* LP5562 I2C address / pins (working example values). */
#define LP5562_I2C_ADDR          0x30
#define LP5562_SDA_GPIO          GPIO_NUM_45
#define LP5562_SCL_GPIO          GPIO_NUM_0
#define LP5562_I2C_FREQ_HZ       400000  /* 400 kHz Fast Mode */

/* LP5562 register addresses. */
#define LP5562_REG_ENABLE        0x00  /* Enable / status            */
#define LP5562_REG_OP_MODE       0x01  /* Operation mode             */
#define LP5562_REG_CONFIG        0x08  /* Configuration              */
#define LP5562_REG_LED_MAP       0x70  /* LED port map               */
#define LP5562_REG_W_PWM         0x0E  /* White-channel PWM set      */
#define LP5562_REG_W_CURRENT     0x0F  /* White-channel current set  */

#define LP5562_MASTER_ENABLE     0x40  /* Master-enable bit          */

static i2c_master_bus_handle_t s_bus = NULL;
static i2c_master_dev_handle_t s_dev = NULL;

static esp_err_t lp5562_write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return i2c_master_transmit(s_dev, buf, sizeof(buf), 100);
}

esp_err_t backlight_init(void)
{
    esp_err_t ret;

    /*
     * The AtomS3R has two physically separate I2C buses:
     *   - the SYS bus (GPIO0 / GPIO45) carrying the LP5562 backlight driver and
     *     the BMI270 IMU, and
     *   - the C-Port / expansion bus (GPIO38 / GPIO39) carrying the StampFly
     *     joystick unit.
     *
     * ESP-IDF's i2c_master driver maps an unset `.i2c_port` to port 0, so the
     * joystick bus and this bus would both try to grab I2C port 0 and the
     * second acquire would fail. We therefore pin each bus to its own
     * peripheral (SOC_I2C_NUM == 2 on the ESP32-S3). This minimal example has
     * no joystick bus, but we still pin to port 0 explicitly to keep the
     * hardware wiring on the SYS peripheral.
     */
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = 0,                 /* SYS I2C peripheral */
        .scl_io_num = LP5562_SCL_GPIO,
        .sda_io_num = LP5562_SDA_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
    };
    ret = i2c_new_master_bus(&bus_cfg, &s_bus);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "failed to create backlight I2C bus on %d/%d",
                 LP5562_SCL_GPIO, LP5562_SDA_GPIO);
        return ret;
    }

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = LP5562_I2C_ADDR,
        .scl_speed_hz = LP5562_I2C_FREQ_HZ,
    };
    ret = i2c_master_bus_add_device(s_bus, &dev_cfg, &s_dev);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "failed to add LP5562 at 0x%02X", LP5562_I2C_ADDR);
        i2c_del_master_bus(s_bus);
        s_bus = NULL;
        return ret;
    }

    /* Reset the driver to a known state. */
    ret = lp5562_write_reg(LP5562_REG_ENABLE, 0x00);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "failed to reset LP5562");
        goto fail;
    }

    /* Enable + brightness, from the working reference firmware. */
    ret = backlight_set_brightness(100);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "failed to enable backlight");
        goto fail;
    }

    ESP_LOGI(TAG, "backlight initialized (LP5562 @ 0x%02X, %d/%d)",
             LP5562_I2C_ADDR, LP5562_SCL_GPIO, LP5562_SDA_GPIO);
    return ESP_OK;

fail:
    i2c_del_master_bus(s_bus);
    s_bus = NULL;
    s_dev = NULL;
    return ret;
}

/*
 * brightness_percent: 0..100. Writes the LP5562 W-channel PWM + current so the
 * backlight is actually driven (not just powered).
 */
esp_err_t backlight_set_brightness(int brightness_percent)
{
    if (brightness_percent < 0) {
        brightness_percent = 0;
    } else if (brightness_percent > 100) {
        brightness_percent = 100;
    }

    /* Step 1: enable internal clock (config). */
    ESP_ERROR_CHECK_WITHOUT_ABORT(lp5562_write_reg(LP5562_REG_CONFIG, 0x01));
    vTaskDelay(pdMS_TO_TICKS(1));

    /* Step 2: enable the chip (master-enable bit). */
    ESP_ERROR_CHECK_WITHOUT_ABORT(lp5562_write_reg(LP5562_REG_ENABLE, LP5562_MASTER_ENABLE));
    vTaskDelay(pdMS_TO_TICKS(1));

    /* Step 3: map all LED ports to the I2C register control. */
    ESP_ERROR_CHECK_WITHOUT_ABORT(lp5562_write_reg(LP5562_REG_LED_MAP, 0x00));
    vTaskDelay(pdMS_TO_TICKS(1));

    /* Step 4: direct PWM control mode. */
    ESP_ERROR_CHECK_WITHOUT_ABORT(lp5562_write_reg(LP5562_REG_OP_MODE, 0x00));
    vTaskDelay(pdMS_TO_TICKS(1));

    /* PWM set (scaled 0..255) and full white-channel current. */
    uint8_t pwm = (uint8_t)((brightness_percent * 255 + 99) / 100);
    ESP_ERROR_CHECK_WITHOUT_ABORT(lp5562_write_reg(LP5562_REG_W_PWM, pwm));
    ESP_ERROR_CHECK_WITHOUT_ABORT(lp5562_write_reg(LP5562_REG_W_CURRENT, 0xFF));

    return ESP_OK;
}

void backlight_on(void)
{
    backlight_set_brightness(100);
}

void backlight_off(void)
{
    if (s_dev != NULL) {
        lp5562_write_reg(LP5562_REG_W_PWM, 0);
    }
}
