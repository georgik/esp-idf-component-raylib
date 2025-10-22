# CI/CD Implementation Summary

## What Was Added

### GitHub Actions Workflows

Three automated workflows were implemented based on the esp-idf-component-SDL reference:

1. **`upload-raylib-component.yml`** - Component Registry upload
   - Uploads component to Espressif Component Registry
   - Triggered on: version tags (v*), releases, manual dispatch
   - Automatically updates version in idf_component.yml
   - Makes component available via `idf.py add-dependency georgik/raylib`

2. **`build-example.yml`** - Build automation
   - Builds hello example for multiple boards (ESP32-S3-BOX-3, M5Stack Core S3, ESP32-P4)
   - Triggered on: push to main, pull requests, manual dispatch
   - Creates merged binary artifacts ready for flashing
   - Optional: Update raylib submodule to latest upstream

3. **`test-example.yml`** - Simulation and testing
   - Runs Wokwi simulation with hardware emulation
   - Captures screenshots at 5 seconds (during color test)
   - Automatically flips screenshots (raylib coordinate system)
   - Uploads logs and screenshots as artifacts
   - Triggered automatically after successful build

### Documentation

- **`.github/README.md`**: Comprehensive CI/CD documentation
  - Workflow descriptions
  - Setup instructions
  - Required secrets
  - Troubleshooting guide
  - How to add new boards

- **Updated main `README.md`**: Added CI/CD section with links

## Key Differences from SDL Component

### Raylib-Specific Adaptations

1. **Screenshot Flipping**
   - Added ImageMagick step to flip screenshots vertically
   - Reason: Raylib coordinate system has Y-axis inverted compared to LCD

2. **Screenshot Timing**
   - Reduced to 5 seconds (from 10s in SDL example)
   - Matches the hello example's color test sequence timing

3. **Binary Naming**
   - Default prefix: `hello` (vs `snow` in SDL)
   - Matches the raylib example name

4. **ESP-IDF Version**
   - Pinned to `v6.0` (vs `latest` in SDL)
   - Ensures compatibility with current component setup

## Next Steps

### Required Actions Before CI Works

1. **Configure GitHub Secrets**
   - Go to repository Settings → Secrets and variables → Actions
   - Add `IDF_COMPONENT_API_TOKEN` (required for publishing)
     - Get from: https://components.espressif.com/settings/api-keys
   - Add `WOKWI_CLI_TOKEN` (required for testing)
     - Get from: https://wokwi.com/dashboard/ci

2. **Test Workflows Manually**
   - Go to Actions tab
   - Run "Build Raylib Example" workflow
   - Verify it completes successfully
   - Check artifacts are uploaded

3. **Verify Wokwi Simulation**
   - Ensure diagram.json exists for all test boards
   - Check screenshot quality and timing
   - Review serial logs for errors

### Optional Enhancements

1. **Add More Boards**
   - ESP32-C3 LCD Kit
   - ESP32-C6 DevKit
   - Update matrix in all three workflows

2. **Performance Testing**
   - Add FPS measurement to serial logs
   - Parse and report in CI

3. **Coverage Badges**
   - Add build status badges to README
   - Example: `![Build](https://github.com/user/repo/actions/workflows/build-example.yml/badge.svg)`

4. **Automated Releases**
   - Create GitHub releases on tags
   - Attach all board binaries

## Testing the CI Locally

Before pushing, you can test the build step locally:

```bash
# Navigate to example
cd raylib/examples/hello

# Build for ESP32-S3-BOX-3
idf.py @boards/esp-box-3.cfg build

# Create merged binary
cd build.esp-box-3
esptool.py --chip esp32s3 merge_bin -o hello-esp-box-3.bin "@flash_args"
```

Test Wokwi simulation (requires Wokwi CLI and token):

```bash
export WOKWI_CLI_TOKEN="your-token-here"

wokwi-cli \
  --timeout 12000 \
  --timeout-exit-code 0 \
  --screenshot-part "esp" \
  --screenshot-time 5000 \
  --elf "build.esp-box-3/hello-esp-box-3.bin" \
  --screenshot-file "screenshot.png" \
  --serial-log-file "wokwi-logs.txt" \
  "boards/esp-box-3"

# Flip screenshot to match raylib coordinate system
convert screenshot.png -flip screenshot.png
```

## Maintenance

### Updating Workflows

When adding new features or boards:
1. Update all three workflow files (maintain consistency)
2. Test manually via workflow_dispatch first
3. Document changes in `.github/README.md`

### Monitoring

Check the Actions tab regularly for:
- Build failures
- Simulation crashes
- Screenshot regressions

## References

- Original SDL component CI: `/Volumes/ssdt5/Users/georgik/projects/esp-idf-component-SDL/.github/workflows/`
- Wokwi CLI docs: https://docs.wokwi.com/wokwi-ci/getting-started
- GitHub Actions docs: https://docs.github.com/en/actions
