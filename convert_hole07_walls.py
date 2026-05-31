#!/usr/bin/env python3
"""
Regenerate hole-07 turf wall vertices in TurfWall.cpp from a map_clicker CSV.

Usage:
  python3 convert_hole07_walls.py "Hole 7 walls.csv"

Coordinate transform (map usable area inside map.png):
  world_x = (pixel_x - 1) / 819 * 40.0 - 20.0
  world_z = -((pixel_y - 33) / 938 * 55.0 - 27.5)

Then: make mac && ./minigolf
"""

from __future__ import annotations

import csv
import re
import sys
from collections import OrderedDict
from pathlib import Path

ROOT = Path(__file__).resolve().parent
TURF_WALL_CPP = ROOT / "TurfWall.cpp"

PX_MIN_X = 1
PX_WIDTH = 819   # px_max_x 820 - px_min_x 1
PX_MIN_Y = 33
PX_HEIGHT = 938  # px_max_y 971 - px_min_y 33

BEGIN = "// BEGIN HOLE07_TURF_WALL_DATA (auto-generated — do not edit by hand)"
END = "// END HOLE07_TURF_WALL_DATA"


def to_world(px: int, py: int) -> tuple[float, float]:
    x = (px - PX_MIN_X) / PX_WIDTH * 40.0 - 20.0
    z = -((py - PX_MIN_Y) / PX_HEIGHT * 55.0 - 27.5)
    return round(x, 3), round(z, 3)


def load_segments(csv_path: Path) -> OrderedDict[str, list[tuple[int, int]]]:
    segments: OrderedDict[str, list[tuple[int, int]]] = OrderedDict()
    with csv_path.open(newline="") as f:
        for row in csv.DictReader(f):
            name = row["object"].strip()
            px, py = int(row["x"]), int(row["y"])
            segments.setdefault(name, []).append((px, py))
    if not segments:
        raise SystemExit(f"No points found in {csv_path}")
    return segments


def emit_cpp(segments: OrderedDict[str, list[tuple[int, int]]]) -> str:
    lines: list[str] = []
    lines.append("// Source CSV → world metres (px 1..820, py 33..971 → 40×55 m map)")
    lines.append("")

    seg_names: list[str] = []
    for i, (obj_name, pixels) in enumerate(segments.items(), 1):
        seg = f"hole07_turfWall_seg{i}"
        seg_names.append(seg)
        world = [to_world(px, py) for px, py in pixels]
        lines.append(f"// {obj_name!r} ({len(world)} vertices)")
        lines.append(f"static const float {seg}[][2] = {{")
        for x, z in world:
            lines.append(f"    {{ {x:7.3f}f, {z:7.3f}f}},")
        lines.append("};")
        lines.append(f"static const int {seg}Count = {len(world)};")
        lines.append("")

    lines.append("static void addHole07TurfWallSegments(TurfWall* wall)")
    lines.append("{")
    for seg in seg_names:
        lines.append(f"    wall->addSegment({seg}, {seg}Count);")
    lines.append("}")
    return "\n".join(lines)


def patch_turf_wall_cpp(block: str) -> None:
    text = TURF_WALL_CPP.read_text()
    if BEGIN not in text or END not in text:
        raise SystemExit(
            f"Markers not found in {TURF_WALL_CPP.name}. "
            f"Expected {BEGIN!r} and {END!r}"
        )
    pattern = re.compile(
        re.escape(BEGIN) + r".*?" + re.escape(END),
        re.DOTALL,
    )
    new_text = pattern.sub(BEGIN + "\n" + block + "\n" + END, text, count=1)
    TURF_WALL_CPP.write_text(new_text)


def main() -> None:
    if len(sys.argv) < 2:
        default = ROOT / "Hole 7 walls.csv"
        csv_path = default if default.is_file() else None
        if csv_path is None:
            raise SystemExit("Usage: python3 convert_hole07_walls.py <path-to.csv>")
    else:
        csv_path = Path(sys.argv[1]).expanduser().resolve()

    if not csv_path.is_file():
        raise SystemExit(f"File not found: {csv_path}")

    pixels_by_obj = load_segments(csv_path)
    block = emit_cpp(pixels_by_obj)

    patch_turf_wall_cpp(block)

    out_csv = ROOT / "hole07_turf_walls.csv"
    out_csv.write_bytes(csv_path.read_bytes())

    print(f"Updated {TURF_WALL_CPP.name}")
    print(f"  {len(pixels_by_obj)} wall segment(s):")
    for name, pts in pixels_by_obj.items():
        print(f"    - {name!r}: {len(pts)} vertices")
    print(f"Copied CSV → {out_csv}")
    print("Next: make mac && ./minigolf")


if __name__ == "__main__":
    main()
