#!/usr/bin/env bash
set -euo pipefail

# Usage: ./trim-map.sh <map> <zoom> [source_directory]
# Example: ./trim-map.sh icebreaker 5
#          ./trim-map.sh icebreaker 5 /path/to/icebreaker
# Input:   <source_directory>/*-z<zoom>.png
# Output:  <source_directory>/trimmed-z<zoom>/{layer}.png
#          <source_directory>/trimmed-z<zoom>/metadata.json

if (( $# < 2 || $# > 3 )); then
    printf 'Usage: %s <map> <zoom> [source_directory]\n' "$0" >&2
    exit 1
fi

MAP="$1"
ZOOM="$2"
SOURCE_DIR="${3:-$MAP}"

if [[ ! "$MAP" =~ ^[a-zA-Z0-9_-]+$ ]]; then
    printf 'Invalid map name: %s\n' "$MAP" >&2
    exit 1
fi

if [[ ! "$ZOOM" =~ ^[1-6]$ ]]; then
    echo 'Zoom must be 1..6' >&2
    exit 1
fi

if [[ ! -d "$SOURCE_DIR" ]]; then
    printf 'Directory does not exist: %s\n' "$SOURCE_DIR" >&2
    exit 1
fi

for tool in magick python3; do
    command -v "$tool" >/dev/null 2>&1 || {
        printf 'Missing dependency: %s\n' "$tool" >&2
        exit 1
    }
done

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
    output_name="${layer}.png"
    destination="$OUTPUT_DIR/$output_name"

    # Trim geometry includes offsets relative to the original canvas.
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

    # +repage removes ImageMagick's virtual-canvas offset from the saved PNG.
    magick "$source" -trim +repage "$destination"

    printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
        "$layer" "$output_name" "$source_width" "$source_height" \
        "$width" "$height" "$offset_x" "$offset_y" >> "$manifest"

    printf '%s: %sx%s at (%s, %s) -> %s\n' \
        "$layer" "$width" "$height" "$offset_x" "$offset_y" "$destination"
done

python3 - "$manifest" "$OUTPUT_DIR/metadata.json" "$ZOOM" <<'PY'
import json
from pathlib import Path
import sys

manifest_path, output_path, zoom = sys.argv[1:]
output = Path(output_path)

# Retain hand-edited layer names and the preferred initial layer on regeneration.
previous = json.loads(output.read_text(encoding="utf-8")) if output.is_file() else {}
previous_layers = previous.get("layers", {})

layers = {}
with open(manifest_path, encoding="utf-8") as manifest:
    for line in manifest:
        layer_id, filename, sw, sh, w, h, ox, oy = line.rstrip("\n").split("\t")
        old = previous_layers.get(layer_id, {})
        layer = {
            "file": filename,
            "offsetX": int(ox),
            "offsetY": int(oy),
            "width": int(w),
            "height": int(h),
            "sourceWidth": int(sw),
            "sourceHeight": int(sh),
        }
        if "name" in old:
            layer["name"] = old["name"]
        layers[layer_id] = layer

# Keep other top-level metadata fields, if any.
result = {**previous, "zoom": int(zoom), "layers": layers}
default_layer = previous.get("defaultLayer")
if default_layer not in layers:
    result["defaultLayer"] = next(iter(layers))

# Atomic replacement of metadata, so partial JSON is not left behind.
tmp = output.with_suffix(".json.tmp")
tmp.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
tmp.replace(output)
print(f"\nSaved {len(layers)} layers and metadata: {output}")
PY
