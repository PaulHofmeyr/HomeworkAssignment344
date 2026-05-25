#!/usr/bin/env python3
"""
Regenerate path geometry in CourseLayout.cpp from Path_coords.csv.

Outer + inner polylines form a corridor (path strip); inner marks the side
where there is no path.  Standalone loops (right / middle path) are filled;
upper path uses upper path inner as exclusion.

  world_x = (pixel_x - 1) / 819 * 40.0 - 20.0
  world_z = -((pixel_y - 33) / 938 * 55.0 - 27.5)

Usage:
  python3 convert_path_coords.py
  make mac && ./minigolf
"""

from __future__ import annotations

import csv
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent
LAYOUT_CPP = ROOT / "CourseLayout.cpp"
CSV_PATH = ROOT / "Path_coords.csv"

PX_MIN_X, PX_WIDTH = 1, 819
PX_MIN_Y, PX_HEIGHT = 33, 938

BEGIN = "// BEGIN PATH_DATA (auto-generated — do not edit by hand)"
END = "// END PATH_DATA"


def to_world(px: int, py: int) -> tuple[float, float]:
    x = (px - PX_MIN_X) / PX_WIDTH * 40.0 - 20.0
    z = -((py - PX_MIN_Y) / PX_HEIGHT * 55.0 - 27.5)
    return round(x, 3), round(z)


def dist(a: tuple[float, float], b: tuple[float, float]) -> float:
    return ((a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2) ** 0.5


def closest_idx(poly: list[tuple[float, float]], pt: tuple[float, float]) -> int:
    return min(range(len(poly)), key=lambda i: dist(poly[i], pt))


def outer_arc_for_inner(
    outer: list[tuple[float, float]], inner: list[tuple[float, float]]
) -> list[int]:
    """Return outer vertex indices for the bank parallel to inner (longer arc)."""
    i0 = closest_idx(outer, inner[0])
    i1 = closest_idx(outer, inner[-1])
    n = len(outer)

    def arc(a: int, b: int) -> list[int]:
        if a <= b:
            return list(range(a, b + 1))
        return list(range(a, n)) + list(range(0, b + 1))

    fwd = arc(i0, i1)
    bwd = arc(i1, i0)
    return fwd if len(fwd) >= len(bwd) else bwd


def corridor_ring(
    outer: list[tuple[float, float]], inner: list[tuple[float, float]]
) -> list[tuple[float, float]]:
    idx = outer_arc_for_inner(outer, inner)
    return [outer[i] for i in idx] + list(reversed(inner))


def closed_ring(pts: list[tuple[float, float]], thresh: float = 6.0) -> list[tuple[float, float]]:
    if len(pts) < 3:
        return pts
    if dist(pts[0], pts[-1]) > thresh:
        return pts + [pts[0]]
    return pts


def load_csv() -> dict[str, list[tuple[int, int]]]:
    segs: dict[str, list[tuple[int, int]]] = {}
    with CSV_PATH.open(newline="") as f:
        for row in csv.DictReader(f):
            name = row["object"].strip()
            segs.setdefault(name, []).append((int(row["x"]), int(row["y"])))
    return segs


def emit_ring(name: str, ring: list[tuple[float, float]], comment: str) -> list[str]:
    lines = [f"// {comment} ({len(ring)} vertices)"]
    lines.append(f"static const float {name}[][2] = {{")
    for x, z in ring:
        lines.append(f"    {{ {x:7.3f}f, {z:7.3f}f}},")
    lines.append("};")
    lines.append(f"static const int {name}Count = {len(ring)};")
    lines.append("")
    return lines


def build_block(segs: dict[str, list[tuple[int, int]]]) -> str:
    paths = [to_world(px, py) for px, py in segs["paths"]]
    inner = [to_world(px, py) for px, py in segs["path inner"]]
    right = closed_ring([to_world(px, py) for px, py in segs["right path"]])
    middle = closed_ring([to_world(px, py) for px, py in segs["middle path"]])
    upper = [to_world(px, py) for px, py in segs["upper path"]]
    upper_in = [to_world(px, py) for px, py in segs["upper path inner"]]

    rings: list[tuple[str, list[tuple[float, float]], str]] = [
        (
            "path_main_corridor",
            corridor_ring(paths, inner),
            "paths + path inner (main course walkway)",
        ),
        ("path_right", right, "right path"),
        ("path_middle", middle, "middle path (gazebo loop)"),
        (
            "path_upper_corridor",
            corridor_ring(upper, upper_in),
            "upper path + upper path inner",
        ),
    ]

    lines: list[str] = [
        "// Source: Path_coords.csv → world metres (px 1..820, py 33..971)",
        "",
    ]
    names: list[str] = []
    for var, ring, comment in rings:
        if len(ring) < 3:
            raise SystemExit(f"{var}: need at least 3 vertices, got {len(ring)}")
        lines.extend(emit_ring(var, ring, comment))
        names.append(var)

    return "\n".join(lines)


def patch_layout(block: str) -> None:
    text = LAYOUT_CPP.read_text()
    if BEGIN not in text:
        raise SystemExit(f"Marker {BEGIN!r} not found — run initial insert first")
    text = re.sub(
        rf"{re.escape(BEGIN)}.*?{re.escape(END)}",
        f"{BEGIN}\n{block}\n{END}",
        text,
        count=1,
        flags=re.DOTALL,
    )
    # Remove legacy single road_outline if still present
    text = re.sub(
        r"// road:.*?\nstatic const float road_outline\[\]\[2\] = \{.*?\};\n"
        r"static const int road_outlineCount = \d+;\n\n",
        "",
        text,
        count=1,
        flags=re.DOTALL,
    )
    LAYOUT_CPP.write_text(text)


def ensure_markers() -> None:
    text = LAYOUT_CPP.read_text()
    if BEGIN in text:
        return
    # Insert markers before first rockbed (after dams)
    marker = "\n" + BEGIN + "\n" + END + "\n\n"
    needle = "static const float rockbed01_outline"
    if needle not in text:
        raise SystemExit("Could not find insertion point for PATH_DATA")
    text = text.replace(needle, marker + needle, 1)
    LAYOUT_CPP.write_text(text)


def main() -> None:
    if not CSV_PATH.is_file():
        raise SystemExit(f"Missing {CSV_PATH}")
    segs = load_csv()
    for key in ("paths", "path inner", "right path", "middle path", "upper path", "upper path inner"):
        if key not in segs:
            raise SystemExit(f"Missing segment {key!r} in CSV")
    ensure_markers()
    block = build_block(segs)
    patch_layout(block)
    print(f"Patched {LAYOUT_CPP.name}")
    for line in block.splitlines():
        if "Count" in line and "static" in line:
            print(f"  {line.strip()}")


if __name__ == "__main__":
    main()
