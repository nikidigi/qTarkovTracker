#!/usr/bin/env bash
set -euo pipefail

# Usage: ./build-icebreaker-map.sh [layer] [zoom]
# Example: ./build-icebreaker-map.sh 06_infirmary 5
LAYER="${1:-06_infirmary}"
ZOOM="${2:-5}"
TILE_SIZE=256
BASE_URL="https://assets.tarkov.dev/maps/icebreaker"

if [[ ! "$LAYER" =~ ^[a-zA-Z0-9_-]+$ ]]; then
  echo "Invalid layer name" >&2; exit 1
fi
if [[ ! "$ZOOM" =~ ^[1-5]$ ]]; then
  echo "Zoom must be 1..5" >&2; exit 1
fi
for tool in curl magick; do
  command -v "$tool" >/dev/null || { echo "Missing command: $tool" >&2; exit 1; }
done

SIDE=$((1 << ZOOM))
WORKDIR="icebreaker/${LAYER}/z${ZOOM}"
TILE_DIR="$WORKDIR/tiles"
mkdir -p "$TILE_DIR"
OUTPUT="icebreaker/${LAYER}-z${ZOOM}.png"
PLACEHOLDER="$WORKDIR/empty.png"
magick -size "${TILE_SIZE}x${TILE_SIZE}" xc:none "$PLACEHOLDER"

count=0
missing=0
inputs=()
for ((y=0; y<SIDE; y++)); do
  for ((x=0; x<SIDE; x++)); do
    tile="$TILE_DIR/${x}_${y}.png"
    if [[ ! -s "$tile" ]]; then
      url="$BASE_URL/$LAYER/$ZOOM/$x/$y.png"
      tmp="${tile}.part"
      if curl --fail --location --silent --show-error --retry 2 \
          --connect-timeout 10 --max-time 30 --output "$tmp" "$url" \
          && magick identify "$tmp" >/dev/null 2>&1; then
        mv "$tmp" "$tile"
      else
        rm -f "$tmp"
      fi
    fi
    if [[ -s "$tile" ]]; then
      inputs+=("$tile")
      ((count+=1))
    else
      inputs+=("$PLACEHOLDER")
      ((missing+=1))
    fi
  done
  printf 'Row %d/%d; downloaded/available: %d; missing: %d\n' "$((y+1))" "$SIDE" "$count" "$missing"
done

if ((count == 0)); then
  echo "No tiles found. Check layer and URL." >&2
  exit 1
fi

magick montage "${inputs[@]}" \
  -tile "${SIDE}x${SIDE}" \
  -geometry "${TILE_SIZE}x${TILE_SIZE}+0+0" \
  -background none \
  "$OUTPUT"

echo "Saved: $OUTPUT ($((${SIDE} * TILE_SIZE))x$((${SIDE} * TILE_SIZE)) pixels)"
echo "Found $count tiles; $missing empty cells."
