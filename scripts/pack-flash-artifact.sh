#!/usr/bin/env bash
#
# pack-flash-artifact.sh — ship only what espbrew needs to flash a build.
#
# WHY
#   ESP-IDF's build/ directory is huge (~150 MB) and almost all of it is
#   irrelevant to flashing:
#     - component object files, .d files, compiled libs  (esp-idf/**)
#     - CMake internals: build.ninja, CMakeCache.txt, cmake_install.cmake
#     - ELF / map debug symbols: *.elf, *.map
#     - IDE index: compile_commands.json
#     - generated templates: kconfigs*.in, ldgen_libraries*, flash_args*.in
#     - metadata: project_description.json, hints.yml, flasher_args.json*
#   espbrew reads NONE of that. It only reads flash_args plus the images that
#   flash_args lists. Uploading the whole build/ therefore bloats the GitHub
#   artifact ~400x for zero benefit.
#
# WHAT THIS DOES
#   Parses build/flash_args the same way espbrew does
#   (internal/project/espidf.go: parseFlashFiles -> resolveBuildPath) and copies
#   ONLY:
#     - flash_args            (the authoritative flash plan: offsets + image list)
#     - every file it lists   (bootloader/bootloader.bin,
#                              partition_table/partition-table.bin,
#                              <project>.bin, and any extra data-partition image
#                              such as storage.bin from fatfs_create_spiflash_image)
#   into <out>, preserving each file's build/ sub-path so espbrew's
#   resolveBuildPath() finds them at the exact same relative location.
#
#   Because we copy exactly the files espbrew reads, the artifact stays in lock
#   step with espbrew even if a future partition table adds an extra image — we
#   do NOT hard-code the three standard files.
#
# USAGE
#   scripts/pack-flash-artifact.sh [BUILD_DIR] [OUT_DIR]
#     BUILD_DIR defaults to "build"
#     OUT_DIR   defaults to "build-flash"
#
# EXIT
#   0 on success. Non-zero if build/flash_args is missing. A referenced image
#   that is absent on disk is reported on stderr and skipped (it would be
#   skipped by espbrew too).
set -euo pipefail

BUILD_DIR="${1:-build}"
OUT_DIR="${2:-build-flash}"

if [ ! -f "$BUILD_DIR/flash_args" ]; then
  echo "pack-flash-artifact: error: $BUILD_DIR/flash_args not found — run 'idf.py build' first." >&2
  exit 1
fi

rm -rf "$OUT_DIR"
mkdir -p "$OUT_DIR"

# The flash plan itself is always needed.
cp "$BUILD_DIR/flash_args" "$OUT_DIR/flash_args"

copied=0
skipped=0

# Read flash_args line by line. Default IFS splits off the leading offset token
# from the rest of the line; image paths never contain spaces, so nothing is
# lost by letting read collapse the separating whitespace.
while read -r off rest || [ -n "${off:-}" ]; do
  # Skip the header line ("--flash-mode dio --flash-freq 40m --flash-size 2MB"),
  # blank lines, and comments.
  case "$off" in
    ""|--*) continue ;;
  esac
  case "$rest" in
    \#*) continue ;;
  esac
  if [ -z "${rest:-}" ]; then
    continue
  fi

  if [ ! -f "$BUILD_DIR/$rest" ]; then
    echo "pack-flash-artifact: warning: flash_args references missing $rest — skipping" >&2
    skipped=$((skipped + 1))
    continue
  fi

  dest="$OUT_DIR/$rest"
  mkdir -p "$(dirname "$dest")"
  cp "$BUILD_DIR/$rest" "$dest"
  copied=$((copied + 1))
done < "$BUILD_DIR/flash_args"

total_before="$(du -sh "$BUILD_DIR" 2>/dev/null | cut -f1)"
total_after="$(du -sh "$OUT_DIR" 2>/dev/null | cut -f1)"
echo "pack-flash-artifact: built $OUT_DIR with $copied image(s) + flash_args"
if [ "$skipped" -ne 0 ]; then
  echo "pack-flash-artifact: skipped $skipped missing image(s) (see warnings above)." >&2
fi
echo "pack-flash-artifact: $total_before -> $total_after"
