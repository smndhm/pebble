#!/usr/bin/env bash
set -euo pipefail

# Usage: ./scripts/assemble_sequence.sh <sequence_dir> [--fps N] [--gif-fps N] [--scale WxH]
# Example: ./scripts/assemble_sequence.sh screenshots/sequence_basalt_20251207_003207 --fps 25 --gif-fps 12 --scale 320x320
# Produces: <sequence_dir>/<basename>.mp4 and <sequence_dir>/<basename>.gif

if [[ ${1-} == "" ]]; then
  echo "Usage: $0 <sequence_dir> [--fps N] [--scale WxH]"
  exit 1
fi

SEQ_DIR="$1"
shift
FPS=25
GIF_FPS=""
SCALE=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --fps)
      FPS="$2"; shift 2;;
    --gif-fps)
      GIF_FPS="$2"; shift 2;;
    --scale)
      SCALE="$2"; shift 2;;
    -h|--help)
      echo "Usage: $0 <sequence_dir> [--fps N] [--scale WxH]"; exit 0;;
    *)
      echo "Unknown arg: $1"; exit 1;;
  esac
done

if [[ ! -d "$SEQ_DIR" ]]; then
  echo "Directory not found: $SEQ_DIR"; exit 1
fi

# Find png files in lexicographic order
# We'll create a temporary workdir with symlinks named frame00001.png ... to handle arbitrary names
TMP=$(mktemp -d)
count=0
for f in $(ls -1v "$SEQ_DIR"/*.png 2>/dev/null || true); do
  if [[ -f "$f" ]]; then
    count=$((count+1))
    ln -s "$PWD/$f" "$TMP/$(printf "frame%05d.png" "$count")"
  fi
done

if (( count == 0 )); then
  echo "No PNG files found in $SEQ_DIR"; rm -rf "$TMP"; exit 1
fi

BASE=$(basename "$SEQ_DIR")
OUT_MP4="$SEQ_DIR/${BASE}.mp4"
OUT_GIF="$SEQ_DIR/${BASE}.gif"

# Build scale filter if requested (parse WIDTHxHEIGHT)
SCALE_FILTER=""
if [[ -n "$SCALE" ]]; then
  if [[ ! "$SCALE" =~ ^([0-9]+)x([0-9]+)$ ]]; then
    echo "--scale must be WIDTHxHEIGHT"; rm -rf "$TMP"; exit 1
  fi
  WIDTH=${BASH_REMATCH[1]}
  HEIGHT=${BASH_REMATCH[2]}
  # scale while preserving aspect, then pad to exact size
  SCALE_FILTER=",scale='min($WIDTH,iw)':'min($HEIGHT,ih)':force_original_aspect_ratio=decrease,pad=${WIDTH}:${HEIGHT}:(ow-iw)/2:(oh-ih)/2"
fi

# Create MP4 (h264) using libx264
ffmpeg -y -framerate "$FPS" -i "$TMP/frame%05d.png" -c:v libx264 -pix_fmt yuv420p -vf "fps=$FPS${SCALE_FILTER}" -movflags +faststart "$OUT_MP4"

# Create GIF by reusing the MP4 as source (preserves timing from video)
PALETTE="$TMP/palette.png"
# Determine which FPS to use for GIF (allow independent control)
if [[ -z "$GIF_FPS" ]]; then
  GIF_FPS="$FPS"
fi

# generate a temporary video with exact FPS and scale filter applied (same as OUT_MP4)
TMP_VIDEO="$TMP/tmp_video.mp4"
ffmpeg -y -framerate "$FPS" -i "$TMP/frame%05d.png" -c:v libx264 -pix_fmt yuv420p -vf "fps=$FPS${SCALE_FILTER}" -movflags +faststart "$TMP_VIDEO"

# Use the video as source for palettegen and gif creation, using GIF_FPS to sample frames
ffmpeg -y -i "$TMP_VIDEO" -vf "fps=$GIF_FPS${SCALE_FILTER},palettegen" -y "$PALETTE"
ffmpeg -y -i "$TMP_VIDEO" -i "$PALETTE" -filter_complex "fps=$GIF_FPS${SCALE_FILTER}[x];[x][1:v]paletteuse" -y "$OUT_GIF"

# Cleanup
rm -rf "$TMP"

echo "Created: $OUT_MP4"
echo "Created: $OUT_GIF"
