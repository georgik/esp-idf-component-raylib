# CI/CD Workflows for ESP-IDF Raylib Component

This directory contains GitHub Actions workflows for building, testing, and validating the raylib component for ESP-IDF.

## Workflows

### 1. Upload Raylib Component (`upload-raylib-component.yml`)

**Triggers:**
- Manual dispatch (workflow_dispatch)
- Push to version tags (v*)
- GitHub release published

**Purpose:** Uploads the raylib component to the Espressif Component Registry.

**Features:**
- Automatically extracts version from git tags (e.g., `v5.6.0~0`)
- Updates `idf_component.yml` with the correct version
- Uploads component with all dependencies and examples
- Recursive submodule checkout (includes raylib upstream)

**Requirements:**
- `IDF_COMPONENT_API_TOKEN` secret must be configured
  - Get from: https://components.espressif.com/settings/api-keys

**Manual Usage:**
1. Go to Actions tab → "Upload Raylib Component"
2. Click "Run workflow"
3. Enter version (e.g., `5.6.0~1`)
4. Component will be available at: https://components.espressif.com/components/georgik/raylib

**Automatic Usage:**
- Create a git tag: `git tag v5.6.0~1 && git push origin v5.6.0~1`
- Or create a GitHub release

### 2. Build Raylib Example (`build-example.yml`)

**Triggers:**
- Manual dispatch (workflow_dispatch)
- Push to main branch
- Pull requests to main branch

**Purpose:** Builds the raylib hello example for multiple ESP32 boards and creates merged binary artifacts.

**Boards tested:**
- ESP32-S3-BOX-3 (esp32s3)
- M5Stack Core S3 (esp32s3)
- ESP32-P4 Function EV Board (esp32p4)

**Artifacts:** Merged binary files ready for flashing: `hello-{board}.bin`

**Parameters (manual dispatch):**
- `board`: Target board (default: 'all')
- `prefix`: Binary name prefix (default: 'hello')
- `flash_size`: Flash storage size (default: '4MB')
- `use_upstream_raylib`: Update raylib submodule to latest main (default: false)

### 3. Test Raylib Example (`test-example.yml`)

**Triggers:**
- Manual dispatch (workflow_dispatch)
- Automatically after successful build workflow

**Purpose:** Runs Wokwi simulation, captures screenshots, and validates the application behavior.

**Features:**
- Simulates ESP32 hardware using Wokwi
- Captures screenshot at 5 seconds (during color test sequence)
- Automatically flips screenshot vertically (raylib coordinate system correction)
- Uploads simulation logs and screenshots as artifacts
- Fails if "Rebooting..." is detected (indicating crash)

**Requirements:**
- `WOKWI_CLI_TOKEN` secret must be configured in GitHub repository settings

**Artifacts:**
- `screenshot-{board}.png` - Display screenshot
- `wokwi-logs-{board}.txt` - Serial output logs

## Setting Up CI for Your Fork

### Required Secrets

Add these secrets to your GitHub repository (Settings → Secrets and variables → Actions):

1. **IDF_COMPONENT_API_TOKEN** (required for component upload)
   - Get from: https://components.espressif.com/settings/api-keys
   - Allows uploading components to ESP Component Registry

2. **WOKWI_CLI_TOKEN** (required for simulation testing)
   - Get your token from https://wokwi.com/dashboard/ci
   - Required for Wokwi simulation

### Running Workflows Manually

1. Go to "Actions" tab in your GitHub repository
2. Select the workflow you want to run
3. Click "Run workflow"
4. Adjust parameters if needed
5. Click "Run workflow" button

## Local Testing

You can test the build locally using the same commands:

```bash
cd raylib/examples/hello
idf.py @boards/esp-box-3.cfg build
```

For Wokwi testing (requires Wokwi CLI):
```bash
wokwi-cli \
  --timeout 12000 \
  --timeout-exit-code 0 \
  --screenshot-part "esp" \
  --screenshot-time 5000 \
  --elf "build.esp-box-3/hello-esp-box-3.bin" \
  --screenshot-file "screenshot.png" \
  --serial-log-file "wokwi-logs.txt" \
  "raylib/examples/hello/boards/esp-box-3"
```

## Continuous Integration Flow

**Development Workflow:**
```
[Push/PR] → Build Example
              ↓ (on success)
           Test Example
```

**Release Workflow:**
```
[Tag v*] or [Release] → Upload Component to ESP Registry
```

## Adding New Boards

To add a new board to CI:

1. Add board configuration to `raylib/examples/hello/boards/{board-name}.cfg`
2. Create Wokwi diagram in `raylib/examples/hello/boards/{board-name}/diagram.json`
3. Update matrix in both workflow files:
   - `build-example.yml` (line 34)
   - `test-example.yml` (line 21)
4. Add target mapping in `build-example.yml` (lines 45-52)

## Troubleshooting

**Build fails:**
- Check ESP-IDF version compatibility (v6.0 required)
- Verify board configuration files exist
- Check component dependencies in `idf_component.yml`

**Wokwi simulation fails:**
- Verify `WOKWI_CLI_TOKEN` is set
- Check `diagram.json` is valid
- Review `wokwi-logs-{board}.txt` artifact for errors

**Screenshot issues:**
- Ensure simulation runs long enough (5+ seconds)
- Check if raylib initialization completed
- Verify LCD display is configured in Wokwi diagram
