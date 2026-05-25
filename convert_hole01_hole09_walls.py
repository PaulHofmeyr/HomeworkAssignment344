#!/usr/bin/env python3
"""
Regenerate shared hole-01 / hole-09 turf walls in TurfWall.cpp.

CSV has four separate objects (four walls); holes 1 and 9 share the middle
wall — add TurfWall::createHole01Hole09() once in SceneRoot (not twice).

Usage:
  python3 convert_hole01_hole09_walls.py [path-to.csv]

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
DEFAULT_CSV = ROOT / "hole-1-turf-wall.csv"

PX_MIN_X = 1
PX_WIDTH = 819
PX_MIN_Y = 33
PX_HEIGHT = 938

BEGIN = "// BEGIN HOLE01_HOLE09_TURF_WALL_DATA (auto-generated — do not edit by hand)"
END = "// END HOLE01_HOLE09_TURF_WALL_DATA"


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
    if len(segments) != 4:
        print(f"  warning: expected 4 wall objects, got {len(segments)}")
    return segments


def emit_cpp(segments: OrderedDict[str, list[tuple[int, int]]], source: str) -> str:
    lines: list[str] = [
        f"// Source: {source} (holes 1 & 9 — draw once via createHole01Hole09)",
        "// px 1..820, py 33..971 → 40×55 m map",
        "",
    ]
    seg_names: list[str] = []
    for i, (obj_name, pixels) in enumerate(segments.items(), 1):
        seg = f"hole0109_turfWall_seg{i}"
        seg_names.append(seg)
        world = [to_world(px, py) for px, py in pixels]
        lines.append(f"// {obj_name!r} ({len(world)} vertices)")
        lines.append(f"static const float {seg}[][2] = {{")
        for x, z in world:
            lines.append(f"    {{ {x:7.3f}f, {z:7.3f}f}},")
        lines.append("};")
        lines.append(f"static const int {seg}Count = {len(world)};")
        lines.append("")

    lines.append("static void addHole0109TurfWallSegments(TurfWall* wall)")
    lines.append("{")
    for seg in seg_names:
        lines.append(f"    wall->addSegment({seg}, {seg}Count);")
    lines.append("}")
    return "\n".join(lines)


def ensure_markers(text: str) -> str:
    if BEGIN in text:
        return text
    needle = "// BEGIN HOLE03_TURF_WALL_DATA"
    if needle not in text:
        raise SystemExit("Could not find HOLE03 marker to insert HOLE01/09 block")
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
    print(f"  {len(segments)} wall segment(s) (shared holes 1 & 9):")
    for name, pts in segments.items():
        print(f"    - {name!r}: {len(pts)} vertices")
    print("Wired in SceneRoot.cpp — createHole01Hole09() drawn once.")
    print("Next: make mac && ./minigolf")


if __name__ == "__main__":
    main()
