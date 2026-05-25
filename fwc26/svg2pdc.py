#!/usr/bin/env python3
"""
Convert an SVG file to PDC (Pebble Draw Command) resources.

Handles:
  - Multiple <path> elements (layers)
  - Compound paths (multiple M…Z subpaths per <path>)
  - Holes via winding direction (nonzero rule) or alternation (evenodd rule)
  - SVG fill="none" paths are skipped; stroked-only paths become open PDC paths

PDC binary layout (little-endian):
  File header  : "PDCI" (4) + image_size uint32 (4)
  Image data   : version uint8 (1) + reserved uint8 (1)
                 + viewbox_w int16 (2) + viewbox_h int16 (2)
                 + command_list
  Command list : num_commands uint16 (2) + commands
  PATH command : type uint8=1 (1) + flags uint8=0 (1)
                 + stroke_color uint8 (1) + stroke_width uint8 (1)
                 + fill_color uint8 (1)
                 + path_open uint16 (2) + num_points uint16 (2)
                 + points: n × (int16 x, int16 y)

Pebble GColor8: bits 7-6 = alpha (11=opaque), 5-4 = R, 3-2 = G, 1-0 = B
  0xFF = opaque white, 0xC0 = opaque black, 0x00 = transparent
"""

import os
import re
import struct
import sys
import xml.etree.ElementTree as ET

PLATFORMS = [
    ("std",   48),   # aplite/basalt/diorite/flint  → 64×48 px per digit
    ("chalk", 60),   # chalk                        → 80×60 px per digit
    ("emery", 68),   # emery                        → 91×68 px per digit
]

OUT = os.path.join(os.path.dirname(__file__), "resources/digits")

COLOR_FILL  = 0xFF  # opaque white — outer/filled areas
COLOR_HOLE  = 0xC0  # opaque black — holes (subtract), assumes black background
COLOR_CLEAR = 0x00  # transparent


# ---------------------------------------------------------------------------
# Bezier flattening (De Casteljau)
# ---------------------------------------------------------------------------

def _subdiv(p0, p1, p2, p3, result, tol2, depth=0):
    if depth > 14:
        result.append(p3)
        return

    def cross2(ax, ay, bx, by):
        return ax * by - ay * bx

    cx, cy = p3[0] - p0[0], p3[1] - p0[1]
    d1 = abs(cross2(p1[0]-p0[0], p1[1]-p0[1], cx, cy))
    d2 = abs(cross2(p2[0]-p0[0], p2[1]-p0[1], cx, cy))
    chord2 = cx*cx + cy*cy + 1
    if (d1 + d2) ** 2 <= tol2 * chord2:
        result.append(p3)
        return
    m01  = ((p0[0]+p1[0])*.5, (p0[1]+p1[1])*.5)
    m12  = ((p1[0]+p2[0])*.5, (p1[1]+p2[1])*.5)
    m23  = ((p2[0]+p3[0])*.5, (p2[1]+p3[1])*.5)
    m012 = ((m01[0]+m12[0])*.5, (m01[1]+m12[1])*.5)
    m123 = ((m12[0]+m23[0])*.5, (m12[1]+m23[1])*.5)
    mid  = ((m012[0]+m123[0])*.5, (m012[1]+m123[1])*.5)
    _subdiv(p0, m01, m012, mid,  result, tol2, depth+1)
    _subdiv(mid, m123, m23, p3,  result, tol2, depth+1)


def flatten_cubic(p0, p1, p2, p3, tol=0.4):
    result = []
    _subdiv(p0, p1, p2, p3, result, (tol * 16) ** 2)
    return result


# ---------------------------------------------------------------------------
# SVG path parsing → list of subpaths
# ---------------------------------------------------------------------------

def _tokenize(d):
    return re.findall(
        r'[MmCcHhVvLlZz]|[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?', d
    )


def parse_subpaths(d):
    """
    Parse SVG path `d` attribute into a list of subpaths.
    Each subpath is a dict with keys:
      'points'  – list of (float x, float y) in SVG user units
      'closed'  – bool
    """
    tokens = _tokenize(d)
    cmds = []
    cur_cmd, cur_args = None, []
    for tok in tokens:
        if re.fullmatch(r'[A-Za-z]', tok):
            if cur_cmd is not None:
                cmds.append((cur_cmd, cur_args))
            cur_cmd, cur_args = tok, []
        else:
            cur_args.append(float(tok))
    if cur_cmd is not None:
        cmds.append((cur_cmd, cur_args))

    subpaths = []
    points = []
    closed = False
    pos = (0.0, 0.0)
    start = (0.0, 0.0)

    def new_sub(first):
        nonlocal points, closed, start
        if points:
            subpaths.append({'points': points, 'closed': closed})
        points = [first]
        closed = False
        start = first

    for cmd, args in cmds:
        if cmd == 'M':
            for i in range(0, len(args), 2):
                pt = (args[i], args[i+1])
                if i == 0:
                    new_sub(pt)
                else:
                    points.append(pt)
                pos = pt
        elif cmd == 'm':
            for i in range(0, len(args), 2):
                pt = (pos[0]+args[i], pos[1]+args[i+1])
                if i == 0:
                    new_sub(pt)
                else:
                    points.append(pt)
                pos = pt
        elif cmd == 'L':
            for i in range(0, len(args), 2):
                pos = (args[i], args[i+1])
                points.append(pos)
        elif cmd == 'l':
            for i in range(0, len(args), 2):
                pos = (pos[0]+args[i], pos[1]+args[i+1])
                points.append(pos)
        elif cmd == 'H':
            for x in args:
                pos = (x, pos[1])
                points.append(pos)
        elif cmd == 'h':
            for dx in args:
                pos = (pos[0]+dx, pos[1])
                points.append(pos)
        elif cmd == 'V':
            for y in args:
                pos = (pos[0], y)
                points.append(pos)
        elif cmd == 'v':
            for dy in args:
                pos = (pos[0], pos[1]+dy)
                points.append(pos)
        elif cmd == 'C':
            for i in range(0, len(args), 6):
                p1 = (args[i],   args[i+1])
                p2 = (args[i+2], args[i+3])
                p3 = (args[i+4], args[i+5])
                for fp in flatten_cubic(pos, p1, p2, p3):
                    points.append(fp)
                pos = p3
        elif cmd == 'c':
            for i in range(0, len(args), 6):
                p1 = (pos[0]+args[i],   pos[1]+args[i+1])
                p2 = (pos[0]+args[i+2], pos[1]+args[i+3])
                p3 = (pos[0]+args[i+4], pos[1]+args[i+5])
                for fp in flatten_cubic(pos, p1, p2, p3):
                    points.append(fp)
                pos = p3
        elif cmd == 'S':
            # Smooth cubic (reflect previous control point)
            prev_p2 = pos
            for i in range(0, len(args), 4):
                p1 = (2*pos[0]-prev_p2[0], 2*pos[1]-prev_p2[1])
                p2 = (args[i],   args[i+1])
                p3 = (args[i+2], args[i+3])
                for fp in flatten_cubic(pos, p1, p2, p3):
                    points.append(fp)
                prev_p2 = p2
                pos = p3
        elif cmd == 'Q':
            for i in range(0, len(args), 4):
                q  = (args[i],   args[i+1])
                p3 = (args[i+2], args[i+3])
                cp1 = (pos[0]+2/3*(q[0]-pos[0]), pos[1]+2/3*(q[1]-pos[1]))
                cp2 = (p3[0] +2/3*(q[0]-p3[0]),  p3[1] +2/3*(q[1]-p3[1]))
                for fp in flatten_cubic(pos, cp1, cp2, p3):
                    points.append(fp)
                pos = p3
        elif cmd in ('Z', 'z'):
            closed = True
            pos = start

    if points:
        subpaths.append({'points': points, 'closed': closed})

    return subpaths


# ---------------------------------------------------------------------------
# Winding / signed area (determines outer vs hole for nonzero fill rule)
# ---------------------------------------------------------------------------

def signed_area(points):
    """Shoelace formula. In screen coords (Y-down): CW → positive."""
    n = len(points)
    area = sum(
        points[i][0] * points[(i+1) % n][1] - points[(i+1) % n][0] * points[i][1]
        for i in range(n)
    )
    return area / 2


# ---------------------------------------------------------------------------
# Color helpers
# ---------------------------------------------------------------------------

_CSS_COLORS = {
    'black':   (0, 0, 0),
    'white':   (255, 255, 255),
    'red':     (255, 0, 0),
    'green':   (0, 128, 0),
    'blue':    (0, 0, 255),
    'yellow':  (255, 255, 0),
    'cyan':    (0, 255, 255),
    'magenta': (255, 0, 255),
}


def css_to_pebble(css):
    """Convert CSS color string to Pebble GColor8 (8-bit)."""
    if not css or css in ('none', 'transparent'):
        return None  # no fill

    css = css.strip().lower()
    if css in _CSS_COLORS:
        r, g, b = _CSS_COLORS[css]
    elif css.startswith('#'):
        h = css[1:]
        if len(h) == 3:
            h = h[0]*2 + h[1]*2 + h[2]*2
        r = int(h[0:2], 16)
        g = int(h[2:4], 16)
        b = int(h[4:6], 16)
    else:
        return COLOR_FILL  # fallback

    r2 = round(r / 255 * 3)
    g2 = round(g / 255 * 3)
    b2 = round(b / 255 * 3)
    return 0xC0 | (r2 << 4) | (g2 << 2) | b2


# ---------------------------------------------------------------------------
# Main conversion
# ---------------------------------------------------------------------------

def svg_to_pdc_commands(svg_path, target_h):
    """
    Returns (commands, pdc_w, pdc_h) where commands is a list of dicts:
      { 'fill': uint8, 'stroke': uint8, 'stroke_width': uint8,
        'open': bool, 'points': [(int16, int16), ...] }
    """
    tree = ET.parse(svg_path)
    root = tree.getroot()

    vb = root.get('viewBox')
    if vb:
        _, _, vbox_w, vbox_h = map(float, vb.split())
    else:
        vbox_w = float(root.get('width',  100))
        vbox_h = float(root.get('height', 100))

    scale   = target_h / vbox_h
    pdc_w   = round(vbox_w * scale)
    pdc_h   = target_h

    def tp(x, y):
        return (round(x * scale), round(y * scale))

    def dedup(pts):
        out = [pts[0]] if pts else []
        for p in pts[1:]:
            if p != out[-1]:
                out.append(p)
        return out

    tag_ns  = re.match(r'\{[^}]+\}', root.tag)
    tag_ns  = tag_ns.group(0) if tag_ns else ''

    commands = []

    for path_el in root.iter(f'{tag_ns}path'):
        d = path_el.get('d', '')
        if not d:
            continue

        fill_css   = path_el.get('fill',   'black')
        stroke_css = path_el.get('stroke', 'none')
        fill_rule  = path_el.get('fill-rule', 'nonzero')

        # Also check style attribute
        style = path_el.get('style', '')
        for prop in style.split(';'):
            prop = prop.strip()
            if prop.startswith('fill:'):
                fill_css = prop[5:].strip()
            elif prop.startswith('stroke:'):
                stroke_css = prop[7:].strip()
            elif prop.startswith('fill-rule:'):
                fill_rule = prop[10:].strip()

        base_fill   = css_to_pebble(fill_css)
        base_stroke = css_to_pebble(stroke_css) or 0x00

        subpaths = parse_subpaths(d)

        # For nonzero rule: determine outer winding from the subpath with largest area.
        # Subpaths with opposite winding are holes.
        scaled_subpaths = []
        for sp in subpaths:
            raw = sp['points']
            if len(raw) < 2:
                continue
            pts = dedup([tp(x, y) for x, y in raw])
            if len(pts) < 2:
                continue
            scaled_subpaths.append((pts, sp['closed']))

        if not scaled_subpaths:
            continue

        if fill_rule == 'nonzero' and len(scaled_subpaths) > 1:
            areas  = [signed_area(pts) for pts, _ in scaled_subpaths]
            outer_area = max(areas, key=abs)
            outer_sign = 1 if outer_area >= 0 else -1
        else:
            outer_sign = None  # unused for evenodd or single subpath

        for i, (scaled, closed) in enumerate(scaled_subpaths):
            # Determine fill color per subpath
            if base_fill is None:
                fill_color = COLOR_CLEAR
            elif fill_rule == 'evenodd':
                fill_color = base_fill if i % 2 == 0 else COLOR_HOLE
            elif outer_sign is None:
                # Single subpath: always outer fill regardless of winding direction
                fill_color = base_fill
            else:
                area = signed_area(scaled)
                this_sign = 1 if area >= 0 else -1
                fill_color = base_fill if this_sign == outer_sign else COLOR_HOLE

            commands.append({
                'fill':         fill_color,
                'stroke':       base_stroke,
                'stroke_width': 0,
                'open':         not closed,
                'points':       scaled,
            })

    return commands, pdc_w, pdc_h


def make_pdc(commands, vbox_w, vbox_h):
    cmd_data = b''
    for c in commands:
        pts = c['points']
        n   = len(pts)
        cmd_data += struct.pack(
            "<BBBBBHH",
            1,                  # type = PATH
            0,                  # flags
            c['stroke'],        # stroke_color
            c['stroke_width'],  # stroke_width
            c['fill'],          # fill_color
            1 if c['open'] else 0,  # path_open
            n,
        )
        for x, y in pts:
            cmd_data += struct.pack("<hh", x, y)

    cmd_list   = struct.pack("<H", len(commands)) + cmd_data
    image_data = struct.pack("<BBhh", 1, 0, vbox_w, vbox_h) + cmd_list
    return b"PDCI" + struct.pack("<I", len(image_data)) + image_data


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <file.svg> [out_dir]")
        sys.exit(1)

    svg_path = sys.argv[1]
    out_dir  = sys.argv[2] if len(sys.argv) > 2 else OUT
    os.makedirs(out_dir, exist_ok=True)

    base = os.path.splitext(os.path.basename(svg_path))[0]

    for plat, cap_h in PLATFORMS:
        cmds, w, h = svg_to_pdc_commands(svg_path, cap_h)
        pdc = make_pdc(cmds, w, h)

        out_path = os.path.join(out_dir, f"{plat}_{base}.pdc")
        with open(out_path, "wb") as f:
            f.write(pdc)

        summary = ', '.join(
            f"0x{c['fill']:02X}({'open' if c['open'] else 'closed'},{len(c['points'])}pts)"
            for c in cmds
        )
        print(f"[{plat}] {w}×{h}px  {len(cmds)} cmd(s): {summary}  → {out_path} ({len(pdc)}B)")


if __name__ == "__main__":
    main()
