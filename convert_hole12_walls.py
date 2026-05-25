#!/usr/bin/env python3
"""
Regenerate hole-12 turf wall vertices in TurfWall.cpp from hole-12-turf-wall.csv.

Each CSV object is one separate wall segment (two walls for hole 12).

Usage:
  python3 convert_hole12_walls.py [path-to.csv]

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
DEFAULT_CSV = ROOT / "hole-12-turf-wall.csv"

PX_MIN_X = 1
PX_WIDTH = 819
PX_MIN_Y = 33
PX_HEIGHT = 938

BEGIN = "// BEGIN HOLE12_TURF_WALL_DATA (auto-generated — do not edit by hand)"
END = "// END HOLE12_TURF_WALL_DATA"


def to_world(px: int, py: int) -> tuple[float, float]:
    x = (px - PX_MIN_X) / PX_WIDTH * 40.0 - 20.0
    z = -((py - PX_MIN_Y) / PX_HEIGHT * 55.0 - 27.5)
    return round(x, 3), round(z, 3)


def load_segments(csv_path: Path) -> OrderedDict[str, list[tuple[int, int]]]:
    segments: OrderedDict[str, list[tuple[int, int]]] = OrderedDict()
    with csv_path.open(newline="") as f:
        for row in csv.DictReader(f):
            name = row["object"].strip()
            segments.setdefault(name, []).append((int(row["x"]), int(row["y"])))
    if not segments:
        raise SystemExit(f"No points found in {csv_path}")
    return segments


def emit_cpp(segments: OrderedDict[str, list[tuple[int, int]]], source: str) -> str:
    lines: list[str] = [
        f"// Source: {source}",
        "// px 1..820, py 33..971 → 40×55 m map",
        "",
    ]
    seg_names: list[str] = []
    for i, (obj_name, pixels) in enumerate(segments.items(), 1):
        seg = f"hole12_turfWall_seg{i}"
        seg_names.append(seg)
        world = [to_world(px, py) for px, py in pixels]
        lines.append(f"// {obj_name!r} ({len(world)} vertices)")
        lines.append(f"static const float {seg}[][2] = {{")
        for x, z in world:
            lines.append(f"    {{ {x:7.3f}f, {z:7.3f}f}},")
        lines.append("};")
        lines.append(f"static const int {seg}Count = {len(world)};")
        lines.append("")

    lines.append("static void addHole12TurfWallSegments(TurfWall* wall)")
    lines.append("{")
    for seg in seg_names:
        lines.append(f"    wall->addSegment({seg}, {seg}Count);")
    lines.append("}")
    return "\n".join(lines)


def ensure_markers(text: str) -> str:
    if BEGIN in text:
        return text
    needle = "// BEGIN HOLE11_TURF_WALL_DATA"
    if needle not in text:
        raise SystemExit("Could not find HOLE11 marker to insert HOLE12 block")
    return text.replace(needle, BEGIN + "\n" + END + "\n\n" + needle, 1)


def patch_turf_wall_cpp(block: str) -> None:
    text = ensure_markers(TURF_WALL_CPP.read_text())
    pattern = re.compile(re.escape(BEGIN) + r".*?" + re.escape(END), re.DOTALL)
    if not pattern.search(text):
        raise SystemExit(f"Markers {BEGIN!r} / {END!r} not found in {TURF_WALL_CPP.name}")
    TURF_WALL_CPP.write_text(
        pattern.sub(BEGIN + "\n" + block + "\n" + END, text, count=1)
    )


def main() -> None:
    csv_path = Path(sys.argv[1]).expanduser().resolve() if len(sys.argv) > 1 else DEFAULT_CSV
    if not csv_path.is_file():
        raise SystemExit(f"File not found: {csv_path}")

    segments = load_segments(csv_path)
    patch_turf_wall_cpp(emit_cpp(segments, csv_path.name))

    print(f"Updated {TURF_WALL_CPP.name}")
    print(f"  {len(segments)} wall segment(s) from {csv_path.name}:")
    for name, pts in segments.items():
        print(f"    - {name!r}: {len(pts)} vertices")
    print("Next: make mac && ./minigolf")


if __name__ == "__main__":
    main()
