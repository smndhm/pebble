#!/usr/bin/env bash
set -euo pipefail

# Usage: ./scripts/screenshot_at_time.sh HH:MM[:SS] [--keep-forced]
# - Sets forced time via scripts/set-forced-time.sh
# - Builds the app, installs to emulator (basalt), takes a screenshot and saves it under screenshots/
# - By default it disables the forced time after screenshot; pass --keep-forced to retain it.

KEEP_FORCED=0
EMULATOR=basalt

# Parse args: support --emulator|-e and --keep-forced
ARGS=()
while [[ "$#" -gt 0 ]]; do
  case "$1" in
    --keep-forced)
      KEEP_FORCED=1; shift;;
    -e|--emulator)
      EMULATOR="$2"; shift 2;;
    -h|--help)
      echo "Usage: $0 [--emulator EMULATOR] [--keep-forced] HH:MM[:SS]"; exit 0;;
    --)
      shift; break;;
    -* )
      echo "Unknown option: $1"; exit 1;;
    *)
      ARGS+=("$1"); shift;;
  esac
done

if [[ ${#ARGS[@]} -lt 1 ]]; then
  echo "Usage: $0 [--emulator EMULATOR] [--keep-forced] HH:MM[:SS]"; exit 1;
fi

TIME=${ARGS[0]}

SCRIPTDIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$SCRIPTDIR/.." && pwd)"
cd "$ROOT"

echo "Setting forced time to $TIME"
./scripts/set-forced-time.sh "$TIME"

echo "Building app..."
pebble build

echo "Installing to emulator ($EMULATOR)..."
pebble install --emulator "$EMULATOR"

mkdir -p screenshots
FNAME="screenshots/screenshot_${TIME//:/-}_$(date +%Y%m%d_%H%M%S).png"

echo "Taking screenshot (pebble will save to current dir) -> $FNAME"
pebble screenshot --emulator "$EMULATOR"

# Pebble CLI doesn't support --output on all versions. Move the most recent PNG to our name.
LATEST_PNG=$(ls -1t *.png 2>/dev/null | head -n1 || true)
if [[ -z "$LATEST_PNG" ]]; then
  echo "Screenshot not found in current directory. Check pebble CLI output." >&2
else
  mv "$LATEST_PNG" "$FNAME"
fi

if [[ "$KEEP_FORCED" -eq 0 ]]; then
  echo "Disabling forced time"
  ./scripts/set-forced-time.sh off
  pebble build >/dev/null 2>&1 || true
fi

echo "Saved screenshot: $FNAME"
