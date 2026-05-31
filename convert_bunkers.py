#!/usr/bin/env python3
"""Inject sand-bunker polygon data into CourseLayout.cpp from bunkers.csv."""
from __future__ import annotations

import csv
import math
import re
import sys
from collections import OrderedDict, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent
CPP = ROOT / "CourseLayout.cpp"
DEFAULT_CSV = ROOT / "bunkers.csv"
BEGIN = "// BEGIN BUNKER_DATA (auto-generated — do not edit by hand)"
END = "// END BUNKER_DATA"
CLUSTER_JUMP_PX = 35


def to_world(px: int, py: int) -> tuple[float, float]:
    x = (px - 1) / 819 * 40.0 - 20.0
    z = -((py - 33) / 938 * 55.0 - 27.5)
    return round(x, 3), round(z, 3)


def split_clusters(pixels: list[tuple[int, int]]) -> list[list[tuple[int, int]]]:
    if not pixels:
        return []
    clusters: list[list[tuple[int, int]]] = [[pixels[0]]]
    for px, py in pixels[1:]:
        lx, ly = clusters[-1][-1]
        if math.hypot(px - lx, py - ly) > CLUSTER_JUMP_PX:
            clusters.append([])
        clusters[-1].append((px, py))
    return [c for c in clusters if len(c) >= 3]


def hole_from_object(name: str) -> int:
    m = re.search(r"bunker\s+(\d+)", name, re.I)
    if not m:
        raise ValueError(f"Cannot parse hole from {name!r}")
    return int(m.group(1)) - 1


def load_by_hole(csv_path: Path) -> dict[int, list[list[tuple[float, float]]]]:
    raw: OrderedDict[str, list[tuple[int, int]]] = OrderedDict()
    with csv_path.open(newline="") as f:
        for row in csv.DictReader(f):
            raw.setdefault(row["object"].strip(), []).append((int(row["x"]), int(row["y"])))

    by_hole: dict[int, list[list[tuple[float, float]]]] = defaultdict(list)
    for obj, pixels in raw.items():
        hole = hole_from_object(obj)
        clusters = split_clusters(pixels)
        if obj.lower() == "bunker 11" and len(clusters) >= 2:
            by_hole[10].append([to_world(px, py) for px, py in clusters[0]])
            by_hole[11].append([to_world(px, py) for px, py in clusters[1]])
            continue
        for cluster in clusters:
            by_hole[hole].append([to_world(px, py) for px, py in cluster])
    return dict(by_hole)


def emit_poly(name: str, world: list[tuple[float, float]]) -> list[str]:
    lines = [f"static const float {name}[][2] = {{"]
    lines += [f"    {{ {x:7.3f}f, {z:7.3f}f}}," for x, z in world]
    lines += ["};", f"static const int {name}Count = {len(world)};"]
    return lines


def emit_cpp(by_hole: dict[int, list[list[tuple[float, float]]]], source: str) -> str:
    lines = [f"// Source: {source}", ""]
    poly_names: dict[int, list[str]] = defaultdict(list)

    for hole in range(18):
        for j, world in enumerate(by_hole.get(hole, [])):
            name = f"bunker_h{hole + 1:02d}_p{j}"
            poly_names[hole].append(name)
            lines += emit_poly(name, world)
            lines.append("")

    lines += ["static void uploadHoleBunkers(BatchedFlat out[18])", "{"]
    lines += [
        "    constexpr float Y_BUNKER = 0.0065f;",
        "    constexpr float COL_BUNKER_R = 0.96f, COL_BUNKER_G = 0.90f, COL_BUNKER_B = 0.72f;",
    ]
    for hole in range(18):
        lines.append(f"    {{ // hole {hole + 1:02d}")
        lines.append("        std::vector<float> buf;")
        for name in poly_names[hole]:
            lines.append(
                f"        {{ auto v = polyVerts({name}, {name}Count, "
                f"COL_BUNKER_R, COL_BUNKER_G, COL_BUNKER_B, Y_BUNKER); "
                f"buf.insert(buf.end(), v.begin(), v.end()); }}"
            )
        lines.append(f"        out[{hole}].upload(buf);")
        lines.append("    }")
    lines.append("}")
    return "\n".join(lines)


def main() -> None:
    csv_path = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else DEFAULT_CSV
    by_hole = load_by_hole(csv_path)
    text = CPP.read_text()
    if BEGIN not in text:
        raise SystemExit(f"Missing {BEGIN} marker in CourseLayout.cpp")
    pat = re.compile(re.escape(BEGIN) + r".*?" + re.escape(END), re.DOTALL)
    CPP.write_text(pat.sub(BEGIN + "\n" + emit_cpp(by_hole, csv_path.name) + "\n" + END, text, 1))
    for h in sorted(by_hole):
        print(f"  hole {h + 1:2d}: {len(by_hole[h])} bunker(s)")


if __name__ == "__main__":
    main()
