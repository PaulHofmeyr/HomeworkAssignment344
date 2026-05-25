#!/usr/bin/env python3
"""
Regenerate hole-02 turf wall vertices in TurfWall.cpp from hole2-turf-walls.csv.

Hole 2 layout (118 clicks):
  - points 1–45: outer berm (one segment)
  - points 46–66, 67–84, 85–100, 101–118: four separate inner walls
    (must not share one polygon or they connect in the mesh)

Usage:
  python3 convert_hole02_walls.py [path-to.csv]

Then: make mac && ./minigolf
"""

from __future__ import annotations

import csv
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
TURF_WALL_CPP = ROOT / "TurfWall.cpp"
DEFAULT_CSV = ROOT / "hole2-turf-walls.csv"

PX_MIN_X = 1
PX_WIDTH = 819
PX_MIN_Y = 33
PX_HEIGHT = 938

BEGIN = "// BEGIN HOLE02_TURF_WALL_DATA (auto-generated — do not edit by hand)"
END = "// END HOLE02_TURF_WALL_DATA"

# 0-based indices where each new closed wall starts (after outer at 45)
HOLE02_SEGMENT_STARTS = (0, 45, 66, 84, 100)


def to_world(px: int, py: int) -> tuple[float, float]:
    x = (px - PX_MIN_X) / PX_WIDTH * 40.0 - 20.0
    z = -((py - PX_MIN_Y) / PX_HEIGHT * 55.0 - 27.5)
    return round(x, 3), round(z, 3)


def load_pixels(csv_path: Path) -> list[tuple[int, int]]:
    pixels: list[tuple[int, int]] = []
    with csv_path.open(newline="") as f:
        for row in csv.DictReader(f):
            pixels.append((int(row["x"]), int(row["y"])))
    if len(pixels) < 3:
        raise SystemExit(f"Need at least 3 points in {csv_path}")
    return pixels


def split_hole02(pixels: list[tuple[int, int]]) -> list[list[tuple[int, int]]]:
    """Five segments: outer ring + four disconnected inner berms."""
    n = len(pixels)
    starts = list(HOLE02_SEGMENT_STARTS) + [n]
    segments: list[list[tuple[int, int]]] = []
    for i in range(len(starts) - 1):
        seg = pixels[starts[i] : starts[i + 1]]
        if len(seg) >= 3:
            segments.append(seg)
        elif seg:
            print(f"  warning: skipped {len(seg)} point(s) at index {starts[i]}")
    return segments


def emit_cpp(segments: list[list[tuple[int, int]]], source: str) -> str:
    lines: list[str] = [
        f"// Source: {source}",
        "// px 1..820, py 33..971 → 40×55 m map",
        "",
    ]
    seg_names: list[str] = []
    for i, pixels in enumerate(segments, 1):
        seg = f"hole02_turfWall_seg{i}"
        seg_names.append(seg)
        world = [to_world(px, py) for px, py in pixels]
        lines.append(f"// segment {i} ({len(world)} vertices)")
        lines.append(f"static const float {seg}[][2] = {{")
        for x, z in world:
            lines.append(f"    {{ {x:7.3f}f, {z:7.3f}f}},")
        lines.append("};")
        lines.append(f"static const int {seg}Count = {len(world)};")
        lines.append("")

    lines.append("static void addHole02TurfWallSegments(TurfWall* wall)")
    lines.append("{")
    for seg in seg_names:
        lines.append(f"    wall->addSegment({seg}, {seg}Count);")
    lines.append("}")
    return "\n".join(lines)


def ensure_markers(text: str) -> str:
    if BEGIN in text:
        return text
    needle = "// BEGIN HOLE07_TURF_WALL_DATA"
    if needle not in text:
        raise SystemExit("Could not find HOLE07 marker to insert HOLE02 block")
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

    pixels = load_pixels(csv_path)
    if len(pixels) != 118:
        print(f"  warning: expected 118 points, got {len(pixels)}")
    segments = split_hole02(pixels)
    block = emit_cpp(segments, csv_path.name)
    patch_turf_wall_cpp(block)

    print(f"Updated {TURF_WALL_CPP.name}")
    print(f"  {len(segments)} wall segment(s) from {csv_path.name}:")
    for i, seg in enumerate(segments, 1):
        print(f"    - segment {i}: {len(seg)} vertices")
    print("Next: make mac && ./minigolf")


if __name__ == "__main__":
    main()
