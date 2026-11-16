#!/usr/bin/env bash
set -euo pipefail

# Build every Icebreaker layer using build-icebreaker-map.sh.
# Usage: ./build-the-lab.sh [zoom]  (default: 4)
ZOOM="${1:-4}"
if [[ ! "$ZOOM" =~ ^[2-6]$ ]]; then
  echo "Usage: $0 [zoom: 2..6]" >&2
  exit 1
fi

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
BUILD_SCRIPT="$SCRIPT_DIR/build-map.sh"
if [[ ! -f "$BUILD_SCRIPT" ]]; then
  echo "Not found: $BUILD_SCRIPT" >&2
  exit 1
fi

layers=(
  1st
  2nd
  technical
)

for i in "${!layers[@]}"; do
  layer="${layers[$i]}"
  printf '\n=== [%d/%d] %s (zoom %s) ===\n' "$((i+1))" "${#layers[@]}" "$layer" "$ZOOM"
  bash "$BUILD_SCRIPT" labs_v4 "$layer" "$ZOOM" 175
done

printf '\nAll %d Icebreaker layers built at zoom %s.\n' "${#layers[@]}" "$ZOOM"
