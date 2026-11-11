#!/usr/bin/env bash
set -euo pipefail

# Trim assembled Icebreaker PNG layers and record their original positions.
# Run from the folder containing "icebreaker/" or pass a source directory:
#   ./trim-icebreaker-layers.sh
#   ./trim-icebreaker-layers.sh 5
#   ./trim-icebreaker-layers.sh 5 /path/to/icebreaker
# Output: <source-dir>/trimmed-z<zoom>/{layer}-z<zoom>.png, metadata.json

for tool in magick python3; do
  command -v "$tool" >/dev/null 2>&1 || {
    printf 'Missing dependency: %s\n' "$tool" >&2
    exit 1
  }
done

ZOOM="${1:-}"
if [[ -z "$ZOOM" ]]; then
  read -r -p 'Zoom (1-5): ' ZOOM
fi
if [[ ! "$ZOOM" =~ ^[1-5]$ ]]; then
  echo 'Zoom must be a number from 1 to 5.' >&2
  exit 1
fi

if [[ $# -ge 2 ]]; then
  SOURCE_DIR="$2"
elif [[ -d icebreaker ]]; then
  SOURCE_DIR=icebreaker
else
  SOURCE_DIR=.
fi
if [[ ! -d "$SOURCE_DIR" ]]; then
  printf 'Directory does not exist: %s\n' "$SOURCE_DIR" >&2
  exit 1
fi

OUTPUT_DIR="$SOURCE_DIR/trimmed-z$ZOOM"
shopt -s nullglob
files=("$SOURCE_DIR"/*-z"$ZOOM".png)
shopt -u nullglob
if (( ${#files[@]} == 0 )); then
  printf 'No *-z%s.png files found in %s\n' "$ZOOM" "$SOURCE_DIR" >&2
  exit 1
fi

mkdir -p "$OUTPUT_DIR"
manifest=$(mktemp)
trap 'rm -f "$manifest"' EXIT

for source in "${files[@]}"; do
  name=${source##*/}
  layer=${name%-z"$ZOOM".png}
  destination="$OUTPUT_DIR/$name"

  # First read trim geometry with the original image's virtual-canvas offset.
  geometry=$(magick "$source" -trim -format '%wx%h%O' info:)
  if [[ ! "$geometry" =~ ^([0-9]+)x([0-9]+)([+-][0-9]+)([+-][0-9]+)$ ]]; then
    printf 'Unable to parse trim geometry for %s: %s\n' "$name" "$geometry" >&2
    exit 1
  fi
  width=${BASH_REMATCH[1]}
  height=${BASH_REMATCH[2]}
  offset_x=${BASH_REMATCH[3]}
  offset_y=${BASH_REMATCH[4]}

  read -r source_width source_height < <(magick identify -format '%w %h\n' "$source")
  magick "$source" -trim +repage "$destination"

  # Keep the manifest tab-separated; Python writes valid JSON and handles escaping.
  printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
    "$layer" "$name" "$source_width" "$source_height" \
    "$width" "$height" "$offset_x,$offset_y" >> "$manifest"
  printf '%s: %sx%s at (%s, %s) -> %s\n' \
    "$layer" "$width" "$height" "$offset_x" "$offset_y" "$destination"
done

python3 - "$manifest" "$OUTPUT_DIR/metadata.json" "$ZOOM" <<'PY'
import json
import pathlib
import sys

manifest_path, output_path, zoom = sys.argv[1:]
layers = {}
with open(manifest_path, encoding="utf-8") as manifest:
    for line in manifest:
        layer, filename, sw, sh, w, h, offset = line.rstrip("\n").split("\t")
        ox, oy = offset.split(",")
        layers[layer] = {
            "file": filename,
            "sourceWidth": int(sw),
            "sourceHeight": int(sh),
            "width": int(w),
            "height": int(h),
            "offsetX": int(ox),
            "offsetY": int(oy),
        }
result = {"zoom": int(zoom), "layers": layers}
path = pathlib.Path(output_path)
tmp = path.with_suffix(".json.tmp")
tmp.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
tmp.replace(path)
print(f"\nSaved {len(layers)} layers and metadata: {path}")
PY
