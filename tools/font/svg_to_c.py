#!/usr/bin/env python3
"""Parse SVG digit files and generate src/c/digit_paths.h with GPath point arrays.

Usage: svg_to_c.py <watchface_dir>
  Reads  <watchface_dir>/resources/svg/0.svg … 9.svg
  Writes <watchface_dir>/src/c/digit_paths.h
"""

import os, re, argparse
import xml.etree.ElementTree as ET

TOL = 0.4  # bezier flatness tolerance in SVG units (sub-pixel at target sizes)


def _subdiv(p0, p1, p2, p3, result, tol2, depth=0):
    if depth > 14:
        result.append(p3); return
    cx, cy = p3[0]-p0[0], p3[1]-p0[1]
    d1 = abs((p1[0]-p0[0])*cy - (p1[1]-p0[1])*cx)
    d2 = abs((p2[0]-p0[0])*cy - (p2[1]-p0[1])*cx)
    chord2 = cx*cx + cy*cy + 1
    if (d1+d2)**2 <= tol2*chord2:
        result.append(p3); return
    m01  = ((p0[0]+p1[0])*.5, (p0[1]+p1[1])*.5)
    m12  = ((p1[0]+p2[0])*.5, (p1[1]+p2[1])*.5)
    m23  = ((p2[0]+p3[0])*.5, (p2[1]+p3[1])*.5)
    m012 = ((m01[0]+m12[0])*.5, (m01[1]+m12[1])*.5)
    m123 = ((m12[0]+m23[0])*.5, (m12[1]+m23[1])*.5)
    mid  = ((m012[0]+m123[0])*.5, (m012[1]+m123[1])*.5)
    _subdiv(p0, m01, m012, mid,  result, tol2, depth+1)
    _subdiv(mid, m123, m23, p3,  result, tol2, depth+1)


def flatten_cubic(p0, p1, p2, p3):
    result = []
    _subdiv(p0, p1, p2, p3, result, (TOL*16)**2)
    return result


def _tokenize(d):
    return re.findall(r'[MmCcHhVvLlZz]|[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?', d)


def parse_points(d):
    tokens = _tokenize(d)
    cmds, cur_cmd, cur_args = [], None, []
    for tok in tokens:
        if re.fullmatch(r'[A-Za-z]', tok):
            if cur_cmd: cmds.append((cur_cmd, cur_args))
            cur_cmd, cur_args = tok, []
        else:
            cur_args.append(float(tok))
    if cur_cmd: cmds.append((cur_cmd, cur_args))

    pts, pos, start = [], (0.0, 0.0), (0.0, 0.0)

    for cmd, args in cmds:
        if cmd == 'M':
            for i in range(0, len(args), 2):
                p = (args[i], args[i+1]); pts.append(p); pos = p
                if i == 0: start = p
        elif cmd == 'm':
            for i in range(0, len(args), 2):
                p = (pos[0]+args[i], pos[1]+args[i+1]); pts.append(p); pos = p
                if i == 0: start = p
        elif cmd == 'H':
            for x in args: pos = (x, pos[1]); pts.append(pos)
        elif cmd == 'h':
            for dx in args: pos = (pos[0]+dx, pos[1]); pts.append(pos)
        elif cmd == 'V':
            for y in args: pos = (pos[0], y); pts.append(pos)
        elif cmd == 'v':
            for dy in args: pos = (pos[0], pos[1]+dy); pts.append(pos)
        elif cmd == 'L':
            for i in range(0, len(args), 2):
                pos = (args[i], args[i+1]); pts.append(pos)
        elif cmd == 'l':
            for i in range(0, len(args), 2):
                pos = (pos[0]+args[i], pos[1]+args[i+1]); pts.append(pos)
        elif cmd == 'C':
            for i in range(0, len(args), 6):
                p1 = (args[i], args[i+1]); p2 = (args[i+2], args[i+3]); p3 = (args[i+4], args[i+5])
                for fp in flatten_cubic(pos, p1, p2, p3): pts.append(fp)
                pos = p3
        elif cmd == 'c':
            for i in range(0, len(args), 6):
                p1 = (pos[0]+args[i],   pos[1]+args[i+1])
                p2 = (pos[0]+args[i+2], pos[1]+args[i+3])
                p3 = (pos[0]+args[i+4], pos[1]+args[i+5])
                for fp in flatten_cubic(pos, p1, p2, p3): pts.append(fp)
                pos = p3
        elif cmd in ('Z', 'z'):
            pos = start

    if not pts: return []
    # Drop closing point if it duplicates start
    if abs(pts[-1][0]-pts[0][0]) < 0.5 and abs(pts[-1][1]-pts[0][1]) < 0.5:
        pts = pts[:-1]
    # Dedup consecutive identical points
    out = [pts[0]]
    for p in pts[1:]:
        if abs(p[0]-out[-1][0]) > 0.3 or abs(p[1]-out[-1][1]) > 0.3:
            out.append(p)
    return [(round(x), round(y)) for x, y in out]


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('watchface_dir', help='Root of the watchface (contains resources/svg/ and src/c/)')
    args = parser.parse_args()

    watchface_dir = os.path.abspath(args.watchface_dir)
    svg_dir = os.path.join(watchface_dir, "resources", "svg")
    out_h   = os.path.join(watchface_dir, "src", "c", "digit_paths.h")

    all_pts = []
    for d in range(10):
        svg_path = os.path.join(svg_dir, f"{d}.svg")
        tree = ET.parse(svg_path)
        root = tree.getroot()
        ns = re.match(r'\{[^}]+\}', root.tag)
        ns = ns.group(0) if ns else ''
        path_el = next(root.iter(f'{ns}path'))
        pts = parse_points(path_el.get('d', ''))
        all_pts.append(pts)
        print(f"  digit {d}: {len(pts)} pts")

    max_pts = max(len(p) for p in all_pts)
    print(f"  max: {max_pts} pts")

    lines = [
        "#pragma once",
        "#include <pebble.h>",
        "",
        "// Digit path points in SVG space: x in [0,1000], y in [0,750].",
        "// Draw-time scaling: pt.x * rect.w / 1000 + origin.x  (same for y/750)",
        "// Generated by svg_to_c.py -- do not edit.",
        "",
    ]

    for d, pts in enumerate(all_pts):
        n = len(pts)
        # 4 points per line for readability
        rows = []
        for i in range(0, n, 4):
            chunk = pts[i:i+4]
            rows.append("  " + ", ".join(f"{{{x},{y}}}" for x,y in chunk))
        body = ",\n".join(rows)
        lines.append(f"static const GPoint kDigitPts{d}[{n}] = {{")
        lines.append(body)
        lines.append("};")
        lines.append("")

    lines += [
        "typedef struct { uint16_t n; const GPoint *pts; } DigitPath;",
        "",
        "static const DigitPath kDigitPaths[10] = {",
    ]
    for d, pts in enumerate(all_pts):
        lines.append(f"  {{ {len(pts)}, kDigitPts{d} }},")
    lines += [
        "};",
        "",
        f"#define DIGIT_PATH_MAX_PTS  {max_pts + 8}",
    ]

    with open(out_h, "w") as f:
        f.write("\n".join(lines) + "\n")
    print(f"  written: {out_h}")


if __name__ == "__main__":
    main()
