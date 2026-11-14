#!/usr/bin/env bash
set -euo pipefail

# Build every Icebreaker layer using build-icebreaker-map.sh.
# Usage: ./build-icebreaker.sh [zoom]  (default: 4)
ZOOM="${1:-4}"
if [[ ! "$ZOOM" =~ ^[1-5]$ ]]; then
  echo "Usage: $0 [zoom: 1..5]" >&2
  exit 1
fi

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
BUILD_SCRIPT="$SCRIPT_DIR/build-map.sh"
if [[ ! -f "$BUILD_SCRIPT" ]]; then
  echo "Not found: $BUILD_SCRIPT" >&2
  exit 1
fi

layers=(
  00_control_room
  01_engine_room
  02_engine_room_upper
  03_fuel_pumps_lower
  04_fuel_pumps
  05_storage_ecurity
  06_infirmary
  07_helipad
  08_gym-canteen
  09_accommodation_lower
  10_accommodation_mid
  11_accommodation_upper
  12_officers_deck
  13_stairs_blocked
  14_bridge
  15_bridge_roof
)

for i in "${!layers[@]}"; do
  layer="${layers[$i]}"
  printf '\n=== [%d/%d] %s (zoom %s) ===\n' "$((i+1))" "${#layers[@]}" "$layer" "$ZOOM"
  bash "$BUILD_SCRIPT" icebreaker "$layer" "$ZOOM" 256
done

printf '\nAll %d Icebreaker layers built at zoom %s.\n' "${#layers[@]}" "$ZOOM"
