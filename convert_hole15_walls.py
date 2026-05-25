#!/usr/bin/env python3
"""Regenerate hole-15 turf walls in TurfWall.cpp from hole-15-turf-wall.csv."""
from __future__ import annotations
import csv, re, sys
from collections import OrderedDict
from pathlib import Path

ROOT = Path(__file__).resolve().parent
TURF_WALL_CPP = ROOT / "TurfWall.cpp"
DEFAULT_CSV = ROOT / "hole-15-turf-wall.csv"
BEGIN = "// BEGIN HOLE15_TURF_WALL_DATA (auto-generated — do not edit by hand)"
END = "// END HOLE15_TURF_WALL_DATA"

def to_world(px: int, py: int) -> tuple[float, float]:
    x = (px - 1) / 819 * 40.0 - 20.0
    z = -((py - 33) / 938 * 55.0 - 27.5)
    return round(x, 3), round(z, 3)

def load_segments(csv_path: Path) -> OrderedDict[str, list[tuple[int, int]]]:
    segments: OrderedDict[str, list[tuple[int, int]]] = OrderedDict()
    with csv_path.open(newline="") as f:
        for row in csv.DictReader(f):
            segments.setdefault(row["object"].strip(), []).append((int(row["x"]), int(row["y"])))
    return segments

def emit_cpp(segments: OrderedDict[str, list[tuple[int, int]]], source: str) -> str:
    labels = ("lower inner berm", "upper outer berm", "left end berm", "island obstacle")
    lines = [f"// Source: {source}", ""]
    names = []
    for i, (obj, pixels) in enumerate(segments.items(), 1):
        seg = f"hole15_turfWall_seg{i}"
        names.append(seg)
        label = labels[i - 1] if i - 1 < len(labels) else obj
        world = [to_world(px, py) for px, py in pixels]
        lines += [f"// {obj!r} — {label} ({len(world)} vertices)", f"static const float {seg}[][2] = {{"]
        lines += [f"    {{ {x:7.3f}f, {z:7.3f}f}}," for x, z in world]
        lines += ["};", f"static const int {seg}Count = {len(world)};", ""]
    lines += ["static void addHole15TurfWallSegments(TurfWall* wall)", "{"]
    lines += [f"    wall->addSegment({s}, {s}Count);" for s in names]
    lines += ["}"]
    return "\n".join(lines)

def main() -> None:
    csv_path = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else DEFAULT_CSV
    segments = load_segments(csv_path)
    text = TURF_WALL_CPP.read_text()
    if BEGIN not in text:
        text = text.replace("// BEGIN HOLE14_TURF_WALL_DATA", BEGIN + "\n" + END + "\n\n// BEGIN HOLE14_TURF_WALL_DATA", 1)
    pat = re.compile(re.escape(BEGIN) + r".*?" + re.escape(END), re.DOTALL)
    TURF_WALL_CPP.write_text(pat.sub(BEGIN + "\n" + emit_cpp(segments, csv_path.name) + "\n" + END, text, 1))

if __name__ == "__main__":
    main()
