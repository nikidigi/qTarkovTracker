#!/usr/bin/env bash
set -euo pipefail

# Usage: ./build-map.sh <map> <layer> <zoom> [tile_size]
# Example: ./build-map.sh icebreaker 06_infirmary 5 256

if (( $# < 3 || $# > 4 )); then
    echo "Usage: $0 <map> <layer> <zoom> [tile_size]" >&2
    exit 1
fi

MAP="$1"
LAYER="$2"
ZOOM="$3"
TILE_SIZE="${4:-256}"

BASE_URL="https://assets.tarkov.dev/maps/$MAP"

for value in "$MAP" "$LAYER"; do
    if [[ ! "$value" =~ ^[a-zA-Z0-9_-]+$ ]]; then
        echo "Invalid map or layer name: $value" >&2
        exit 1
    fi
done

if [[ ! "$ZOOM" =~ ^[1-6]$ ]]; then
    echo "Zoom must be 1..6" >&2
    exit 1
fi

if [[ ! "$TILE_SIZE" =~ ^[0-9]+$ ]] || (( TILE_SIZE < 1 )); then
    echo "Tile size must be a positive integer" >&2
    exit 1
fi

for tool in curl magick; do
    command -v "$tool" >/dev/null || {
        echo "Missing command: $tool" >&2
        exit 1
    }
done

SIDE=$((1 << ZOOM))

WORKDIR="$MAP/$LAYER/z$ZOOM"
TILE_DIR="$WORKDIR/tiles"
OUTPUT="$MAP/${LAYER}-z${ZOOM}.png"
PLACEHOLDER="$WORKDIR/empty.png"

mkdir -p "$TILE_DIR"

magick -size "${TILE_SIZE}x${TILE_SIZE}" xc:none "$PLACEHOLDER"

count=0
missing=0
inputs=()

for ((y=0; y<SIDE; y++)); do
    for ((x=0; x<SIDE; x++)); do
        tile="$TILE_DIR/${x}_${y}.png"

        # Validate cached tile
        if [[ -s "$tile" ]] && ! magick identify "$tile" >/dev/null 2>&1; then
            rm -f "$tile"
        fi

        if [[ ! -s "$tile" ]]; then
            url="$BASE_URL/$LAYER/$ZOOM/$x/$y.png"
            tmp="${tile}.part"

            if curl --fail --location --silent --show-error \
                --retry 2 \
                --connect-timeout 10 \
                --max-time 30 \
                --output "$tmp" "$url" \
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

    printf 'Row %d/%d; found: %d; missing: %d\n' \
        "$((y+1))" "$SIDE" "$count" "$missing"
done

if (( count == 0 )); then
    echo "No tiles found. Check map, layer and URL." >&2
    exit 1
fi

magick montage "${inputs[@]}" \
    -tile "${SIDE}x${SIDE}" \
    -geometry "${TILE_SIZE}x${TILE_SIZE}+0+0" \
    -background none \
    "$OUTPUT"

echo "Saved: $OUTPUT ($((SIDE * TILE_SIZE))x$((SIDE * TILE_SIZE)) pixels)"
echo "Found $count tiles; $missing empty cells."
