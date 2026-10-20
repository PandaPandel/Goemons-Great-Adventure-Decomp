"""The game's stage files (StageData.file: rails, collision, geography) as C and back.

    python canon-decompilation-tools/stage_codec.py check            every stage file through C, IDO, link
    python canon-decompilation-tools/stage_codec.py extract OUTDIR   one .c per stage file

A stage file is loaded at segment 3; every pointer in it is a segment-3 address into itself
(the asset survey, results/01_1.txt and others; vram.c and geography.c read it). File 14's StageData names its pieces: the rails (Vec3f[n + 2] each), the collision header
(coll: f32 origin[3]; s16 dims[3]; u16; f32 cell[3]; s32 file; s32 count; grid, lists, tris)
with its CollTri[count], its u16 lists and its u16 grid, and the geography header (geo: the same
origin, dims and cell; s32 count, objCount, file; runs, lists, places, textures; one more f32)
with its u16 lists, its u16 run pairs, its u16 texture list and its GeoPlace[count]. The C is
each piece as an array in file order, the headers pointing at them, every gap an explicit
array of its own bytes; compiled with the game's IDO, linked at 0x03000000, compared.

Changelog: first public release (2026-10-20). Earlier history is kept privately.
"""

import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import model_codec as mc  # noqa: E402

ROOT = mc.ROOT


def stage_files(files):
    """{stage file id: (geo, coll, [(n, points)])} from file 14's StageData records."""
    out = {}
    data = files[14]
    for i in range(len(data) // 0x60):
        f, of, geo, coll = struct.unpack_from(">iiII", data, i * 0x60)
        rails = [struct.unpack_from(">hHI", data, i * 0x60 + 0x10 + 8 * k) for k in range(10)]
        if f and f not in out:
            out[f] = (geo, coll, [(n, p) for n, pad, p in rails if n])
    return out


PIECES = {}  # fid -> [(offset, size, name)] of the last to_c, for check's report


def _piece(fid, off):
    for o, size, name in PIECES.get(fid, ()):
        if o <= off < o + size:
            return f"{name}+0x{off - o:X}"
    return "a gap"


def _f(v):
    if v == 0 and struct.pack(">f", v) != bytes(4):
        return "-0.0f"
    s = format(v, ".9g")
    return (s if any(c in s for c in ".en") else s + ".0") + "f"


def to_c(fid, data, geo, coll, rails):
    name = f"stage_{fid:04d}"
    regions = []  # (offset, size, C text)
    U16_AT = set()  # the labels of the u16 lists, which a following gap can extend

    def u16_text(off, end, label):
        vals = struct.unpack_from(">%dH" % ((end - off) // 2), data, off)
        body = ", ".join(f"0x{v:X}" for v in vals)
        return f"u16 {name}_{label}[{len(vals)}] = {{ {body} }};"

    def u16s(off, end, label):
        U16_AT.add(label)
        regions.append((off, end - off, u16_text(off, end, label)))

    for k, (n, p) in enumerate(rails):
        o = p & 0xFFFFFF
        pts = struct.unpack_from(">%df" % (3 * (n + 2)), data, o)
        body = ", ".join("{ %s, %s, %s }" % tuple(_f(v) for v in pts[3 * i:3 * i + 3]) for i in range(n + 2))
        regions.append((o, 12 * (n + 2), f"f32 {name}_rail{k}[{n + 2}][3] = {{ {body} }};"))
    c = coll & 0xFFFFFF
    cori = struct.unpack_from(">3f3hH3fii3I", data, c)
    count, grid, lists, tris = cori[11], cori[12] & 0xFFFFFF, cori[13] & 0xFFFFFF, cori[14] & 0xFFFFFF
    trows = []
    for i in range(count):
        t = tris + 0x28 * i
        v = struct.unpack_from(">9h", data, t)
        b12 = data[t + 0x12:t + 0x14]
        nd = struct.unpack_from(">4f", data, t + 0x14)
        tail = data[t + 0x24:t + 0x28]
        trows.append("    { { %s }, { 0x%02X, 0x%02X }, { %s }, { 0x%02X, 0x%02X, 0x%02X, 0x%02X } }"
                     % (", ".join(map(str, v)), b12[0], b12[1], ", ".join(_f(x) for x in nd), *tail))
    regions.append((tris, 0x28 * count, f"CollTriRow {name}_tris[{count}] = {{\n" + ",\n".join(trows) + "\n};"))
    dims = cori[3] * cori[4] * cori[5]
    u16s(lists, lists + ((grid - lists) & ~1) if grid > lists else lists, "colllists")
    u16s(grid, grid + 2 * dims, "grid")
    g = geo & 0xFFFFFF
    gh = struct.unpack_from(">3f3hH3fiii4If", data, g)
    gcount, runs, glists, places, texs = gh[10], gh[13] & 0xFFFFFF, gh[14] & 0xFFFFFF, gh[15] & 0xFFFFFF, gh[16] & 0xFFFFFF
    regions_u16 = sorted([(glists, "geolists"), (runs, "runs"), (texs, "textures")])
    ends = [o for o, _ in regions_u16[1:]] + [g]
    for (o, label), e in zip(regions_u16, ends):
        # each list runs to the next piece; the texture list ends at its one 0 terminator, and the
        # zeros after it (padding to 16) are a gap
        e2 = e
        while label == "textures" and e2 - 4 >= o and data[e2 - 4:e2] == bytes(4):
            e2 -= 2
        u16s(o, e2, label)
    prow = []
    for i in range(gcount):
        p = places + 0x18 * i
        idx = struct.unpack_from(">I", data, p)[0]
        pos = struct.unpack_from(">3h", data, p + 4)
        rot = struct.unpack_from(">3H", data, p + 10)
        sca = struct.unpack_from(">3H", data, p + 16)
        prow.append("    { %d, { %s }, { %s }, { %s } }" % (idx, ", ".join(map(str, pos)), ", ".join(map(str, rot)), ", ".join(map(str, sca))))
    regions.append((places, 0x18 * gcount, f"GeoPlaceRow {name}_places[{gcount}] = {{\n" + ",\n".join(prow) + "\n};"))

    def hdr_ptr(pointer, label):
        # null only when the whole pointer is 0: a piece at offset 0 is 0x03000000
        return f"(u32) &{name}_{label}[0]" if pointer else "0"
    regions.append((c, 0x34, f"CollHeaderRow {name}_coll[1] = {{ {{ {{ {', '.join(_f(x) for x in cori[0:3])} }}, "
                    f"{{ {', '.join(map(str, cori[3:6]))} }}, 0x{cori[6]:X}, {{ {', '.join(_f(x) for x in cori[7:10])} }}, "
                    f"{cori[10]}, {count}, {hdr_ptr(cori[12], 'grid')}, {hdr_ptr(cori[13], 'colllists')}, "
                    f"{hdr_ptr(cori[14], 'tris')} }} }};"))
    regions.append((g, 0x40, f"GeoHeaderRow {name}_geo[1] = {{ {{ {{ {', '.join(_f(x) for x in gh[0:3])} }}, "
                    f"{{ {', '.join(map(str, gh[3:6]))} }}, 0x{gh[6]:X}, {{ {', '.join(_f(x) for x in gh[7:10])} }}, "
                    f"{gcount}, {gh[11]}, {gh[12]}, {hdr_ptr(gh[13], 'runs')}, {hdr_ptr(gh[14], 'geolists')}, "
                    f"{hdr_ptr(gh[15], 'places')}, {hdr_ptr(gh[16], 'textures')}, {_f(gh[17])} }} }};"))
    regions.sort()
    out, at, k, pieces = [], 0, 0, []
    for off, size, text in regions:
        if off < at:
            raise ValueError(f"stage {fid}: overlapping pieces at 0x{off:X}")
        if off > at and at % 4 and pieces and pieces[-1][2] in U16_AT:
            # IDO starts every array on a 4-byte boundary: a gap after a u16 list that ends
            # between two words would move everything after it, so the list takes those bytes
            p_off, _, label = pieces[-1]
            at = min(off, (at + 3) & ~3)
            out[-1] = u16_text(p_off, at, label)
            pieces[-1] = (p_off, at - p_off, label)
        if off > at:
            gap = data[at:off]
            out.append(f"u8 {name}_gap{k}[{len(gap)}] = {{ {', '.join(f'0x{b:X}' for b in gap)} }};")
            pieces.append((at, off - at, f"gap{k}"))
            k += 1
        out.append(text)
        pieces.append((off, size, text.split("=")[0].split()[-1].split("[")[0].replace(name + "_", "")))
        at = off + size
    if at < len(data):
        gap = data[at:]
        out.append(f"u8 {name}_gap{k}[{len(gap)}] = {{ {', '.join(f'0x{b:X}' for b in gap)} }};")
    head = ['#include "ultra64.h"', "",
            "typedef struct CollTriRow { s16 v[9]; u8 unk12[2]; f32 nd[4]; u8 tail[4]; } CollTriRow;",
            "typedef struct GeoPlaceRow { u32 object; s16 pos[3]; u16 rot[3]; u16 scale[3]; } GeoPlaceRow;",
            "typedef struct CollHeaderRow { f32 origin[3]; s16 dims[3]; u16 pad; f32 cell[3]; s32 file; s32 count;"
            " u32 grid, lists, tris; } CollHeaderRow;",
            "typedef struct GeoHeaderRow { f32 origin[3]; s16 dims[3]; u16 pad; f32 cell[3]; s32 count, objCount, file;"
            " u32 runs, lists, places, textures; f32 extra; } GeoHeaderRow;", ""]
    # every piece forward-declared, so a header can point at a piece defined after it
    decls = [t.split("=")[0].strip() + ";" for t in out]
    PIECES[fid] = pieces
    return "\n".join(head + ["extern " + d for d in decls] + [""] + out) + "\n"


def main():
    cmd = sys.argv[1:]
    files = mc.archive()
    stages = stage_files(files)
    if cmd and cmd[0] == "check":
        want = [int(x) for x in cmd[1:]] or None
        scratch = os.path.join(ROOT, "permuter", "stage_codec", str(os.getpid()))
        ok, bad = 0, []
        for fid, (geo, coll, rails) in sorted(stages.items()):
            if want and fid not in want:
                continue
            data = files[fid]
            try:
                got = mc.build(to_c(fid, data, geo, coll, rails), scratch)
            except (ValueError, RuntimeError, struct.error) as e:
                bad.append(f"{fid}({str(e)[:40]})")
                continue
            if got == data:
                ok += 1
            else:
                k = next((i for i in range(min(len(got), len(data))) if got[i] != data[i]), None)
                bad.append(f"{fid}@0x{k:X}({_piece(fid, k)})" if k is not None
                           else f"{fid}(len {len(got):X}/{len(data):X})")
        print(f"stages: {ok} of {ok + len(bad)} stage files through C and back byte-exact; not: {' '.join(bad[:20])}")
        return 0 if not bad else 1
    if len(cmd) == 2 and cmd[0] == "extract":
        os.makedirs(cmd[1], exist_ok=True)
        for fid, (geo, coll, rails) in stages.items():
            with open(os.path.join(cmd[1], f"stage_{fid:04d}.c"), "w", encoding="utf-8", newline="\n") as f:
                f.write(to_c(fid, files[fid], geo, coll, rails))
        return 0
    sys.exit(__doc__)


if __name__ == "__main__":
    sys.exit(main())
