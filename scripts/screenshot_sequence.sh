#!/usr/bin/env bash
set -euo pipefail

# Usage: ./scripts/screenshot_sequence.sh --emulator EMU --start HH:MM[:SS] --end HH:MM[:SS]
# This script does NOT wait for a wall-clock start time. Instead it forces the displayed
# time inside the app for each second between --start and --end (inclusive). For each
# second it regenerates `src/c/forced_time.c`, rebuilds and reinstalls the app, then
# takes a screenshot. This ensures no waiting is necessary and the displayed time is forced.

EMULATOR=basalt
START=""
END=""
TIME=""
KEEP_FORCED=0
WIPE_EVERY=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --emulator|-e)
      EMULATOR="$2"; shift 2;;
    --start)
      START="$2"; shift 2;;
    --end)
      END="$2"; shift 2;;
    --time|-t)
      TIME="$2"; shift 2;;
    --keep-forced)
      KEEP_FORCED=1; shift;;
    --wipe-every)
      WIPE_EVERY="$2"; shift 2;;
    -h|--help)
      echo "Usage: $0 --emulator EMU --start HH:MM[:SS] --end HH:MM[:SS] [--time HH:MM[:SS]] [--keep-forced] [--wipe-every N]"; exit 0;;
    *)
      echo "Unknown arg: $1"; exit 1;;
  esac
done

if [[ -z "$START" || -z "$END" ]]; then
  echo "--start and --end are required"; exit 1
fi

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT_DIR"

# Preflight: ensure pebble CLI is present and SDK is installed to avoid automatic
# SDK download during runs (which can prompt or fail). If the SDK isn't found,
# exit with a helpful message.
if ! command -v pebble >/dev/null 2>&1; then
  echo "Error: 'pebble' CLI not found in PATH. Install pebble-tool before running this script." >&2
  exit 1
fi

# Check for SDK installation directory used by pebble-tool (best-effort)
if [[ ! -d "$HOME/.pebble-sdk/SDKs/current" ]] && [[ -z "${PEBBLE_SDK_PATH-}" ]]; then
  echo "Error: Pebble SDK not found (no ~/.pebble-sdk/SDKs/current and PEBBLE_SDK_PATH unset)." >&2
  echo "Install the SDK or set PEBBLE_SDK_PATH to avoid automatic SDK download by the CLI." >&2
  exit 1
fi

# parse START and END as HH:MM[:SS] and compute seconds-of-day
parse_hms() {
  local in="$1"
  if [[ ! "$in" =~ ^([0-9]{1,2}):([0-9]{2})(:([0-9]{2}))?$ ]]; then
    return 1
  fi
  local HH=${BASH_REMATCH[1]}
  local MM=${BASH_REMATCH[2]}
  local SS=${BASH_REMATCH[4]:-0}
  HH=$((10#$HH)); MM=$((10#$MM)); SS=$((10#$SS))
  if (( HH < 0 || HH > 23 || MM < 0 || MM > 59 || SS < 0 || SS > 59 )); then
    return 1
  fi
  echo $((HH*3600 + MM*60 + SS))
  return 0
}

start_sec=$(parse_hms "$START" ) || { echo "Failed to parse --start. Use HH:MM or HH:MM:SS"; exit 1; }
end_sec=$(parse_hms "$END" ) || { echo "Failed to parse --end. Use HH:MM or HH:MM:SS"; exit 1; }

if (( end_sec <= start_sec )); then
  echo "End time must be after start time"; exit 1
fi

OUT_DIR="screenshots/sequence_${EMULATOR}_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$OUT_DIR"

# validate WIPE_EVERY is a non-negative integer
if ! [[ "$WIPE_EVERY" =~ ^[0-9]+$ ]]; then
  echo "Error: --wipe-every must be a non-negative integer" >&2
  exit 1
fi

# wipe helper: reset/kill emulator processes or use pebble CLI wipe/kill
do_wipe() {
  local emu="$1"
  echo "Wiping/resetting emulator (requested)..." 
  if command -v pebble >/dev/null 2>&1; then
    if pebble kill --help >/dev/null 2>&1; then
      pebble kill --force || echo "pebble kill failed"
    elif pebble wipe --help >/dev/null 2>&1; then
      pebble wipe --everything || echo "pebble wipe failed"
    else
      pkill -f qemu-system-arm 2>/dev/null || true
      pkill -f pebble-emulator 2>/dev/null || true
      pkill -f pebble-sim 2>/dev/null || true
      pkill -f qemu 2>/dev/null || true
      sleep 1
    fi
  else
    pkill -f qemu-system-arm 2>/dev/null || true
    pkill -f pebble-emulator 2>/dev/null || true
    pkill -f pebble-sim 2>/dev/null || true
    pkill -f qemu 2>/dev/null || true
    sleep 1
  fi
  # give the emulator a moment to restart if it does
  sleep 1
}

# Iterate through each second between start and end, force that displayed time,
# rebuild & reinstall the app, then take a screenshot. This avoids waiting on wall-clock.
count=0
for (( t = start_sec; t <= end_sec; t++ )); do
  hh=$(( (t/3600) % 24 ))
  mm=$(( (t%3600) / 60 ))
  ss=$(( t % 60 ))
  time_str=$(printf "%02d:%02d:%02d" "$hh" "$mm" "$ss")

  echo "Forcing displayed time: $time_str  (step $((count+1)))"
  ./scripts/set-forced-time.sh "$time_str"

  echo "Building..."
  pebble build

  echo "Installing to emulator: $EMULATOR"
  # Try install, and if it fails try a few times. On repeated failures run a wipe.
  install_with_retries() {
    local emu="$1"
    local tries=0
    local max=3
    while [[ $tries -lt $max ]]; do
      if pebble install --emulator "$emu"; then
        return 0
      fi
      tries=$((tries+1))
      echo "Install failed (attempt $tries/$max). Retrying..."
      sleep 1
    done
    echo "Install failed after $max attempts. Attempting to reset emulator $emu and retrying..."
    # attempt to stop the emulator via pebble CLI if available (prefer kill)
    if command -v pebble >/dev/null 2>&1; then
      if pebble kill --help >/dev/null 2>&1; then
        pebble kill --force || echo "pebble kill failed"
      elif pebble wipe --help >/dev/null 2>&1; then
        # fallback to wipe if kill not available
        pebble wipe --everything || echo "pebble wipe failed"
      else
        echo "pebble (kill/wipe) not supported by this CLI. Trying fallback process reset..."
        pkill -f qemu-system-arm 2>/dev/null || true
        pkill -f pebble-emulator 2>/dev/null || true
        pkill -f pebble-sim 2>/dev/null || true
        pkill -f qemu 2>/dev/null || true
        sleep 1
      fi
    else
      echo "pebble CLI not found; cannot call kill/wipe. Trying process reset..."
      pkill -f qemu-system-arm 2>/dev/null || true
      pkill -f pebble-emulator 2>/dev/null || true
      pkill -f pebble-sim 2>/dev/null || true
      pkill -f qemu 2>/dev/null || true
      sleep 1
    fi
    if pebble install --emulator "$emu"; then
      return 0
    fi
    echo "Install still failed after wipe. Skipping this step." >&2
    return 1
  }

  if ! install_with_retries "$EMULATOR"; then
    echo "Skipping screenshot for $time_str due to install failures" >&2
    count=$((count+1))
    continue
  fi

  ts=$(date +%Y%m%d_%H%M%S)
  # take screenshot and detect produced PNG
  pre_list=$(mktemp)
  ls -1t *.png 2>/dev/null > "$pre_list" || true
  pebble screenshot --emulator "$EMULATOR" || true
  found=""
  tries=0
  max_tries=6
  while [[ -z "$found" && $tries -lt $max_tries ]]; do
    sleep 0.25
    for f in $(ls -1t *.png 2>/dev/null || true); do
      if ! grep -xFq "$f" "$pre_list" 2>/dev/null; then
        found="$f"; break
      fi
    done
    tries=$((tries+1))
  done
  rm -f "$pre_list"

  if [[ -n "$found" ]]; then
    dst="$OUT_DIR/screenshot_${EMULATOR}_${time_str//:/-}_${ts}_$count.png"
    mv "$found" "$dst"
    echo "Saved $dst"
  else
    echo "No screenshot file produced for $time_str" >&2
  fi

  count=$((count+1))

  # Optional: wipe/reset emulator every WIPE_EVERY screenshots (if >0)
  if [[ $WIPE_EVERY -gt 0 && $((count % WIPE_EVERY)) -eq 0 ]]; then
    echo "Reached $count screenshots; performing emulator wipe/reset as requested (every $WIPE_EVERY)."
    do_wipe "$EMULATOR"
  fi
done

if [[ "$KEEP_FORCED" -eq 0 && -n "$TIME" ]]; then
  echo "Disabling forced time"
  ./scripts/set-forced-time.sh off
  pebble build >/dev/null 2>&1 || true
fi

echo "Done. Sequence saved in $OUT_DIR"
