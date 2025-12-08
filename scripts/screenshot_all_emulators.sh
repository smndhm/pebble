#!/usr/bin/env bash
set -euo pipefail

# Usage: ./scripts/screenshot_all_emulators.sh HH:MM[:SS] [--keep-forced] [emulator...]
# If no emulators are provided, defaults to: basalt chalk diorite emery flint aplite

KEEP_FORCED=0

# Parse optional flags (only --keep-forced). The next non-option argument must be TIME.
TIME=""
while [[ $# -gt 0 ]]; do
  case "$1" in
    --keep-forced)
      KEEP_FORCED=1; shift;;
    -h|--help)
      echo "Usage: $0 [--keep-forced] HH:MM[:SS]"; exit 0;;
    --)
      shift; break;;
    -* )
      echo "Unknown option: $1"; exit 1;;
    *)
      TIME="$1"; shift; break;;
  esac
done

if [[ -z "$TIME" ]]; then
  echo "Usage: $0 [--keep-forced] HH:MM[:SS]"; exit 1
fi

# Always target emulators (excluding emery and flint for now)
EMULATORS=(basalt chalk diorite aplite)

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT_DIR"

echo "Setting forced time to $TIME"
./scripts/set-forced-time.sh "$TIME"

echo "Building once for all emulators..."
pebble build

mkdir -p screenshots

for EMU in "${EMULATORS[@]}"; do
  echo "--- Installing to emulator: $EMU ---"
  pebble install --emulator "$EMU"
  # Give emulator a short moment to start
  sleep 1

  echo "Taking screenshot on $EMU..."
  # Record existing PNGs so we can detect the new one
  pre_list=$(mktemp)
  ls -1t *.png 2>/dev/null > "$pre_list" || true

  found=""
  tries=0
  max_tries=6
  while [[ -z "$found" && $tries -lt $max_tries ]]; do
    pebble screenshot --emulator "$EMU" || true
    sleep 0.5
    # look for the newest PNG not in pre_list
    for f in $(ls -1t *.png 2>/dev/null || true); do
      if ! grep -xFq "$f" "$pre_list" 2>/dev/null; then
        found="$f"
        break
      fi
    done
    tries=$((tries+1))
  done

  rm -f "$pre_list"

  if [[ -n "$found" ]]; then
    DST="screenshots/screenshot_${EMU}_${TIME//:/-}_$(date +%Y%m%d_%H%M%S).png"
    mv "$found" "$DST"
    echo "Saved $DST"
  else
    echo "No screenshot found after pebble screenshot for $EMU" >&2
  fi
done

if [[ "$KEEP_FORCED" -eq 0 ]]; then
  echo "Disabling forced time"
  ./scripts/set-forced-time.sh off
  pebble build >/dev/null 2>&1 || true
fi

echo "Done. Screenshots are in ./screenshots/"
