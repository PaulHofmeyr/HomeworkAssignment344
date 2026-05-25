#!/usr/bin/env python3
"""
Regenerate hole-04 turf wall vertices in TurfWall.cpp from hole-4-turf-wall.csv.

Trace order on the map:
  - points 1–34 then 51–123: large outer berm (one closed ring, skips the inner loop)
  - points 35–50: small inner oval (separate segment — must not connect to outer)

Usage:
  python3 convert_hole04_walls.py [path-to.csv]

Then: make mac && ./minigolf
"""

from __future__ import annotations

import csv
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
TURF_WALL_CPP = ROOT / "TurfWall.cpp"
DEFAULT_CSV = ROOT / "hole-4-turf-wall.csv"

PX_MIN_X = 1
PX_WIDTH = 819
PX_MIN_Y = 33
PX_HEIGHT = 938

BEGIN = "// BEGIN HOLE04_TURF_WALL_DATA (auto-generated — do not edit by hand)"
END = "// END HOLE04_TURF_WALL_DATA"

# 1-based CSV point numbers (inclusive start, exclusive end for inner)
OUTER_A_END = 34          # points 1–34
INNER_START = 35
INNER_END = 50            # points 35–50
OUTER_B_START = 51        # points 51–123


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


def split_hole04(pixels: list[tuple[int, int]]) -> list[list[tuple[int, int]]]:
    n = len(pixels)
    if n < OUTER_B_START:
        raise SystemExit(f"Expected at least {OUTER_B_START} points, got {n}")

    # Outer: 1–34 and 51–end; inner: 35–50 (inclusive)
    outer = pixels[:OUTER_A_END] + pixels[OUTER_B_START - 1 :]
    inner = pixels[INNER_START - 1 : INNER_END]
    segments: list[list[tuple[int, int]]] = []
    if len(outer) >= 3:
        segments.append(outer)
    if len(inner) >= 3:
        segments.append(inner)
    return segments


def emit_cpp(segments: list[list[tuple[int, int]]], source: str) -> str:
    labels = ("outer berm", "inner oval")
    lines: list[str] = [
        f"// Source: {source}",
        "// px 1..820, py 33..971 → 40×55 m map",
        "",
    ]
    seg_names: list[str] = []
    for i, pixels in enumerate(segments, 1):
        seg = f"hole04_turfWall_seg{i}"
        seg_names.append(seg)
        label = labels[i - 1] if i - 1 < len(labels) else f"segment {i}"
        world = [to_world(px, py) for px, py in pixels]
        lines.append(f"// {label} ({len(world)} vertices)")
        lines.append(f"static const float {seg}[][2] = {{")
        for x, z in world:
            lines.append(f"    {{ {x:7.3f}f, {z:7.3f}f}},")
        lines.append("};")
        lines.append(f"static const int {seg}Count = {len(world)};")
        lines.append("")

    lines.append("static void addHole04TurfWallSegments(TurfWall* wall)")
    lines.append("{")
    for seg in seg_names:
        lines.append(f"    wall->addSegment({seg}, {seg}Count);")
    lines.append("}")
    return "\n".join(lines)


def ensure_markers(text: str) -> str:
    if BEGIN in text:
        return text
    needle = "// BEGIN HOLE02_TURF_WALL_DATA"
    if needle not in text:
        raise SystemExit("Could not find HOLE02 marker to insert HOLE04 block")
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
    segments = split_hole04(pixels)
    patch_turf_wall_cpp(emit_cpp(segments, csv_path.name))

    print(f"Updated {TURF_WALL_CPP.name}")
    print(f"  {len(segments)} wall segment(s) from {csv_path.name}:")
    for i, seg in enumerate(segments, 1):
        print(f"    - segment {i}: {len(seg)} vertices")
    print("Next: make mac && ./minigolf")


if __name__ == "__main__":
    main()
