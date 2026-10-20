"""The game's models, as C and back to the same bytes: the scenery models ("gpol" geography
objects) and the object models (every other model file).

    python canon-decompilation-tools/model_codec.py check [FILE ...]   every model through C, IDO, link
    python canon-decompilation-tools/model_codec.py extract OUTDIR     one geo_NNNN.c per model file
                                                                       (written only when it changes)
    python canon-decompilation-tools/model_codec.py regions FILE       an object model's pieces, in file order

A gpol model file (the asset survey, results/01_1.txt and others) is linked
at segment 3 (0x03000000): a Vtx array from offset 0, then ONE F3DEX2 display list from where
the vertices end to its G_ENDDL, then zeros to 16. Its G_VTX loads cover every vertex; its
textures are G_SETTIMG at the segments 4-10 the object's other files fill (constants here).
The display list's start is read off the data: the first offset (a multiple of 16) from which
the commands run to a G_ENDDL at the end and whose G_VTX loads cover exactly the bytes before
it. The C is a Vtx array and a Gfx array of gbi.h macros (libgfxd, F3DEX2), compiled with the
game's IDO (the project's flags), linked at 0x03000000 and compared with the file.

An object model file (object_files: the files of groups 3, 4, 5 and 0xB that are neither gpol
models, stage files nor texture_codec's pictures; the asset survey's results/08_8, 09_9, 10_10
and 13_13) has no header. It is linked at its group's segment (file.c fileGroup and
gFileGroupTable; fileLoadAttachments relocates a file by group << 24), and its pieces are found
the way the game's readers walk them (regions):
  - display lists: a forward scan for lists that run to a G_ENDDL (one that starts where another
    ends, or with a command that sets a list up), the lists they call (G_DL), and the lists model
    nodes name; a list "found" inside the vertices or lights a list loads starts after them.
    F3DEX2, the game's microcode; file 412 alone ends its lists in F3DEX 1.x's G_ENDDL (0xB8)
    and is written with gbi.h's F3DEX_GBI. An 8-byte G_ENDDL alone is an empty list;
  - the vertices the lists' G_VTX load (Vtx, one array per run), and the lights their G_MOVEMEM
    loads: every one is gbi.h's Lights0 (gdSPDefLights0: an ambient colour and its copy, then a
    zero light), loaded as gsSPLight(&l.a, 3) after gsSPNumLights(2);
  - animations (anim.c AnimHeader and animSeek, polygon.c polDrawAnimModel): word 0 holds the
    frame count (bits 0-7), the root count (8-22) and two counts of words to skip (24-27,
    28-30), word 1 the keys, then the skipped words and the roots. The roots are ModelNode trees
    (object.h; child and sibling are relative indices); each of a node's nine tracks is a
    constant (above 0xA000, less 0xB000) or the offset of its keys, and every track must decode
    (animDecodeKeys' runs) inside the file. The keys are the u8 stream anim.c reads;
  - attachment points (PlActAnim.attach in plgoemon.c and the others, read by mdlAnimNodePos
    in tlb_0089): an animation header whose root "node" starts at the word before it; its
    keys run from the header's keys to that word;
  - attack chains (aniatk.c: a header word with the joints in bits 8-22 and the frames in 0-7,
    the track stream, then the joints, four u16 tracks each: scale, x, y, z);
  - ride.c's RideData (count, radius, then RideBoxes whose MtxF inv is zero in ROM:
    rideInitBoxMtx fills it); vram.c's Coll (laid out CollTri[count], the lists, the grid, the
    header); vanime.c's VAnime (its s, t pairs and its keyframes of VKeys);
  - what the matched C names in these files (code_evidence): the courses vramSetCourse takes
    (Vec3f[kind + 2]: a cardinal spline, object.h Spline) in a set's file (fileload.c
    floadSegToRam; file 11's FileSet), and the vertices it copies out (heapMemCopy) where the
    file's lists load segment 8, the vertex buffer the code fills.
A gap of zeros the compiler would not make is a u8 array named pad; bytes no reader explains
would be a u8 array named unk (none of the 153 has any). The C is every piece in file order,
with its reader's types and field names (vram.c's, ride.c's and vanime.c's structs copied with
their comments), the pointers between pieces as symbols; compiled with the game's IDO and
linked at the file's segment, which build() reads from the C's first line ("linked at").

Which reader writes a file (source): the object reader, unless dl_start claims the file and the
object reader leaves bytes it cannot explain; then the gpol reader. dl_start only sees vertices
then lists, so an object model that starts with vertices would otherwise come out as Vtx.

Changelog: first public release (2026-10-20). Earlier history is kept privately.
"""

import os
import re
import struct
import subprocess
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import nisitenma_archive as arc  # noqa: E402

ROOT = arc.ROOT
SEG3 = 0x03000000
OPS = {0x01, 0x05, 0x06, 0xD7, 0xD9, 0xDF, 0xE3, 0xE6, 0xE7, 0xE8, 0xF0, 0xF2, 0xF3, 0xF5, 0xFC,
       0xFD, 0xFB, 0xFA, 0xE2, 0xE1, 0xF1, 0xF4, 0xF6, 0xF7, 0xF8, 0xF9, 0xEF, 0xED, 0xEE, 0xDE, 0xE4}


def archive():
    """{file id: (group, bytes)} for every non-empty archive file."""
    with open(arc.DECOMPRESSED, "rb") as f:
        dec = f.read()
    rows = {}
    with open(os.path.join(ROOT, "nisitenma.tsv"), encoding="utf-8") as f:
        for line in f.read().splitlines()[1:]:
            p = line.split("\t")
            if p[0].isdigit() and int(p[3], 16):
                rows[int(p[0])] = dec[int(p[2], 16):int(p[2], 16) + int(p[3], 16)]
    return rows


def dl_start(data):
    """The display list's offset in a gpol file, or None if the file is not one."""
    end = len(data)
    while end >= 8 and data[end - 8:end] == bytes(8):
        end -= 8
    if end < 16 or data[end - 8:end] != b"\xdf" + bytes(7):
        return None
    for d in range(16, end - 8, 16):
        ok, covered = True, 0
        for o in range(d, end, 8):
            op = data[o]
            if op not in OPS:
                ok = False
                break
            if op == 0x01:  # G_VTX: n (12 bits at bits 12-19), end index, address
                w0, w1 = struct.unpack_from(">II", data, o)
                n = (w0 >> 12) & 0xFF
                if w1 >> 24 != 3:
                    ok = False
                    break
                covered = max(covered, (w1 & 0xFFFFFF) + 16 * n)
        if ok and covered == d:
            return d
    return None


def to_c(fid, data):
    """The model as C: its Vtx array, its display list, and the zero bytes after it."""
    import pygfxd as g
    d = dl_start(data)
    end = len(data)
    while data[end - 8:end] == bytes(8) and end - 8 >= d + 8 and data[end - 16:end - 8] != b"\xdf" + bytes(7):
        end -= 8
    name = f"geo_{fid:04d}"
    lines = ['#include "ultra64.h"', "", f"Vtx {name}_vtx[] = {{"]
    for o in range(0, d, 16):
        x, y, z, flag, s, t = struct.unpack_from(">hhhHhh", data, o)
        r, gg, b, a = data[o + 12:o + 16]
        lines.append(f"    {{{{{{ {x}, {y}, {z} }}, 0x{flag:X}, {{ {s}, {t} }}, {{ 0x{r:02X}, 0x{gg:02X}, 0x{b:02X}, 0x{a:02X} }}}}}},")
    lines.append("};")

    def macro():
        g.gfxd_puts("    ")
        g.gfxd_macro_dflt()
        g.gfxd_puts(",\n")
        return 0

    def vtx(addr, count):
        if addr >> 24 == 3 and (addr & 0xFFFFFF) < d:
            g.gfxd_printf(f"&{name}_vtx[{(addr & 0xFFFFFF) // 16}]")
            return 1
        return 0

    # one Gfx array per display list: a file may hold several back to back (546: two), and
    # libgfxd stops at each G_ENDDL
    starts, o = [d], d
    while o < end:
        if data[o] == 0xDF and o + 8 < end:
            starts.append(o + 8)
        o += 8
    for k, s in enumerate(starts):
        e = starts[k + 1] if k + 1 < len(starts) else end
        g.gfxd_input_buffer(data[s:e])
        outb = bytes((e - s) * 60 + 4096)
        outbuf = g.gfxd_output_buffer(outb, len(outb))
        g.gfxd_target(g.gfxd_f3dex2)
        g.gfxd_endian(g.GfxdEndian.big, 4)
        g.gfxd_macro_fn(macro)
        g.gfxd_vtx_callback(vtx)
        g.gfxd_execute()
        lines.append("")
        lines.append(f"Gfx {name}_dl{'' if k == 0 else f'_{k}'}[] = {{")
        lines.append(g.gfxd_buffer_to_string(outbuf).rstrip("\n"))
        lines.append("};")
    if end < len(data):
        lines.append("")
        lines.append(f"u8 {name}_pad[{len(data) - end}] = {{ 0 }};")
    return "\n".join(lines) + "\n"


# --- object models ---

def _be32(b, o):
    return struct.unpack_from(">I", b, o)[0]


def _be16(b, o):
    return struct.unpack_from(">H", b, o)[0]


_GROUPS = []


def segment(fid):
    """The segment a file is linked at: its group (file.c fileGroup: gFileGroupTable's (bound,
    group) pairs, a file below the bound has the group)."""
    if not _GROUPS:
        with open(os.path.join(ROOT, "src", "file.c"), encoding="utf-8") as f:
            body = re.search(r"u16 gFileGroupTable\[0x1F0\] = \{(.*?)\};", f.read(), re.S).group(1)
        v = [int(x, 16) for x in re.findall(r"0x[0-9A-F]+", body)]
        _GROUPS.extend((v[i], v[i + 1]) for i in range(0, len(v), 2) if v[i])
    return next((g for bound, g in _GROUPS if fid < bound), 0xFF)


def object_files(files):
    """The object model files: those of groups 3, 4, 5 and 0xB that are neither gpol models,
    stage files (file 14's StageData.file) nor texture_codec's pictures."""
    import stage_codec  # noqa: E402 (it imports this file)
    import texture_codec  # noqa: E402
    stages = set(stage_codec.stage_files(files))
    return sorted(fid for fid, data in files.items() if segment(fid) in (3, 4, 5, 0xB) and not dl_start(data)
                  and fid not in stages and fid not in texture_codec.SPECS)


# the display-list commands each GBI has (gbi.h): F3DEX2, the game's (gspF3DEX2_fifo), and
# F3DEX 1.x (F3DEX_GBI), which only file 412 is written in
_GOOD2 = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDE, 0xDF,
          0xE1, 0xE2, 0xE3, 0xE4, 0xE6, 0xE7, 0xE8, 0xE9, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
          0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD}
GBIS = {
    "f3dex2": {"good": _GOOD2, "end": b"\xdf" + bytes(7), "vtx": 0x01, "dl": 0xDE, "target": "gfxd_f3dex2",
               "first": {0xE7, 0xD9, 0xD7, 0xE3, 0xE2, 0xFC, 0xFD, 0x01, 0xDE, 0xFA, 0xFB, 0xD8, 0xDA, 0xDC, 0xDB},
               "n": lambda w0: (w0 >> 12) & 0xFF,
               "light": lambda w0: w0 >> 24 == 0xDC and w0 & 0xFF == 0x0A},  # G_MOVEMEM, G_MV_LIGHT
    "f3dex": {"good": {0x00, 0x01, 0x03, 0x04, 0x06, *range(0xAF, 0xC0), *range(0xE4, 0x100)} - {0xF1},
              "end": b"\xb8" + bytes(7), "vtx": 0x04, "dl": 0x06, "target": "gfxd_f3dex",
              "first": {0xE7, 0xB6, 0xB7, 0xBA, 0xB9, 0xBB, 0xFC, 0xFD, 0x04, 0x06, 0xFA, 0xFB, 0x01, 0x03, 0xBC},
              "n": lambda w0: (w0 & 0xFFFF) >> 10,
              "light": lambda w0: w0 >> 24 == 0x03 and 0x86 <= (w0 >> 16) & 0xFF <= 0x94 and w0 & 0xFFFF == 0x10},
}


def gbi_of(b):
    """F3DEX2, unless the file's lists end only in F3DEX 1.x's G_ENDDL."""
    ends = {k: sum(1 for o in range(0, len(b) - 7, 8) if b[o:o + 8] == v["end"]) for k, v in GBIS.items()}
    return GBIS["f3dex"] if ends["f3dex"] and not ends["f3dex2"] else GBIS["f3dex2"]


def walk_dl(b, o, seg, gb):
    """The display list at o, to its G_ENDDL (or a G_BRANCH), and the offsets here it loads:
    ({"vtx": {(off, n)}, "dl": {off}, "light": {off}}, end), or None."""
    refs = {"vtx": set(), "dl": set(), "light": set()}
    while o + 8 <= len(b):
        w0, w1 = _be32(b, o), _be32(b, o + 4)
        op = w0 >> 24
        if op not in gb["good"]:
            return None
        here = (w1 >> 24) == seg and (w1 & 0xFFFFFF) < len(b)
        if op == gb["vtx"] and (w1 >> 24) == seg:
            n = gb["n"](w0)
            if (w1 & 0xFFFFFF) + 16 * n > len(b) or w1 & 0xF:
                return None
            refs["vtx"].add((w1 & 0xFFFFFF, n))
        elif op == gb["dl"] and here:
            refs["dl"].add(w1 & 0xFFFFFF)
        elif gb["light"](w0) and here:
            refs["light"].add(w1 & 0xFFFFFF)
        if op == gb["end"][0]:
            return (refs, o + 8) if b[o:o + 8] == gb["end"] else None
        if op == gb["dl"] and (w0 >> 16) & 0xFF == 1:
            return refs, o + 8
        o += 8
    return None


def _called(b, seg, dls, gb):
    todo = [t for e, r in dls.values() for t in r["dl"]]
    while todo:
        t = todo.pop()
        if t in dls or t % 8:
            continue
        r = walk_dl(b, t, seg, gb)
        if r:
            dls[t] = (r[1], r[0])
            todo.extend(r[0]["dl"])


def find_dls(b, seg, gb):
    """{start: (end, refs)}: a list at each 8-aligned offset that starts one (or follows another
    list's G_ENDDL) and runs to a G_ENDDL, then the lists they call."""
    dls = {}
    o, after_end = 0, False
    while o + 8 <= len(b):
        r = walk_dl(b, o, seg, gb) if (b[o] in gb["first"] or (after_end and b[o])) else None
        if r and (r[1] - o >= 16 or after_end):
            dls[o] = (r[1], r[0])
            o, after_end = r[1], True
            continue
        o, after_end = o + 8, False
    _called(b, seg, dls, gb)
    return dls


def track_end(b, keys, pos, frames):
    """Where a track's keys end (anim.c animDecodeKeys' runs) after frames keys; None if they run
    off the file. A count of 0 runs to the end."""
    o, left = keys + pos, frames
    while left > 0:
        if o >= len(b):
            return None
        op = b[o]
        o += 1
        if op < 0x78:
            o += 1 if op >= 0x60 else 0
            n = op & 0x1F
            o += n if n else left
            left -= n if n else left
        elif op < 0xF0:
            ln = (op - 0x78) & 0x1F
            o += 2 if op >= 0xD8 else 1
            left -= (ln + 2) if ln else left
        else:
            n = op & 0xF
            o += 2 * (n if n else left)
            left -= n if n else left
    return o if o <= len(b) else None


def node_tree(b, root, seg, dls, gb):
    """The ModelNodes reachable from root (polygon.c polDrawModelNode: child, then sibling), or
    None if one is not one: its dl is 0, a display list here (or a list only it names, added to
    dls), or an address in another segment (model.c masks the top nibble)."""
    seen, stack, new = set(), [root], {}
    while stack:
        o = stack.pop()
        if o in seen:
            continue
        if o < 0 or o % 4 or o + 0x18 > len(b):
            return None
        dl = _be32(b, o)
        t = dl & 0xFFFFFF
        if dl and (dl >> 24) & 0xF == seg and t not in dls and t not in new:
            r = walk_dl(b, t, seg, gb) if t % 8 == 0 and t < len(b) and b[t] else None
            if not r:
                return None
            new[t] = (r[1], r[0])
        seen.add(o)
        c, sib = struct.unpack_from(">bb", b, o + 4)
        if c:
            stack.append(o + c * 0x18)
        if sib:
            stack.append(o + sib * 0x18)
    dls.update(new)
    return seen


def find_anims(b, seg, dls, taken, gb):
    """{offset: animation}: anim.c's headers and polDrawAnimModel's roots, each kept only if its
    trees are ModelNodes and every track decodes inside the file; and the attachment points."""
    out = {}
    for o in range(0, len(b) - 12, 4):
        if o in taken:
            continue
        w0, w1 = _be32(b, o), _be32(b, o + 4)
        frames, n = w0 & 0xFF, (w0 >> 8) & 0x7FFF
        a, bb = (w0 >> 28) & 7, (w0 >> 24) & 0xF
        if frames == 0 or n == 0 or n > 32 or (w1 and ((w1 >> 24) != seg or (w1 & 0xFFFFFF) >= len(b))):
            continue
        p = o + 8 + 4 * (a + bb)
        if p + 4 * n > len(b):
            continue
        roots = [_be32(b, p + 4 * i) for i in range(n)]
        r0 = roots[0] & 0xFFFFFF
        if w1 and roots[0] >> 24 == seg and r0 == o - 4 and all(r == 0 for r in roots[1:]):
            # an attachment point: its root "node" is the word before the header and the header
            # itself; its keys run from the header's keys to that word
            k0, e = w1 & 0xFFFFFF, r0
            while e > k0 and b[e - 1] == 0:
                e -= 1
            if k0 <= r0 and not set(range(r0, p + 4 * n)) & taken:
                out[r0] = {"end": p + 4 * n, "frames": frames, "keys": k0, "roots": [r0], "nodes": set(),
                           "spans": [(k0, e)] if k0 < e else [], "attach": o}
            continue
        if not all((r >> 24) == seg and (r & 0xFFFFFF) + 0x18 <= len(b) for r in roots):
            continue
        nodes, trial = set(), dict(dls)
        for r in roots:
            t = node_tree(b, r & 0xFFFFFF, seg, trial, gb)
            if t is None:
                nodes = None
                break
            nodes |= t
        if not nodes:
            continue
        spans, ok = [], True
        for no in nodes:
            for k in range(9):
                v = _be16(b, no + 6 + 2 * k) & 0xBFFF
                if v > 0xA000:
                    continue
                e = track_end(b, w1 & 0xFFFFFF, v & 0x7FFF, 1 if v & 0x8000 else frames) if w1 else None
                if e is None:
                    ok = False
                    break
                spans.append(((w1 & 0xFFFFFF) + (v & 0x7FFF), e))
            if not ok:
                break
        if not ok or (set(range(o, p + 4 * n)) | {x for no in nodes for x in range(no, no + 0x18)}) & taken:
            continue
        dls.update(trial)
        out[o] = {"end": p + 4 * n, "frames": frames, "keys": (w1 & 0xFFFFFF) if w1 else None,
                  "roots": [r & 0xFFFFFF for r in roots], "nodes": nodes, "spans": spans}
    return out


class Region:
    """A piece of an object model: [start, end), its kind, its alignment in IDO's .data."""

    def __init__(self, start, end, kind, align=4, **kw):
        self.start, self.end, self.kind, self.align = start, end, kind, align
        self.__dict__.update(kw)

    def __repr__(self):
        return f"{self.kind}@{self.start:X}-{self.end:X}"


def _runs(cells, step):
    out, i = [], 0
    while i < len(cells):
        j = i
        while j + 1 < len(cells) and cells[j + 1] == cells[j] + step:
            j += 1
        out.append((cells[i], cells[j] + step))
        i = j + 1
    return out


def _dl_regions(dls, extra_vtx=()):
    out = []
    vtx, lights = set(extra_vtx), set()
    for s, (e, refs) in dls.items():
        vtx |= refs["vtx"]
        lights |= refs["light"]
    for s, e in _runs(sorted({a + 16 * k for a, n in vtx for k in range(n)}), 16):
        out.append(Region(s, e, "vtx", 8))
    starts = sorted(dls)
    for k, s in enumerate(starts):  # a list another one calls into ends where that one starts
        out.append(Region(s, min([dls[s][0]] + starts[k + 1:k + 2]), "dl", 8))
    for a in sorted(lights):  # every one a Lights0 (24 bytes): to_c checks its bytes
        out.append(Region(a, a + 24, "light", 8))
    return out


_EVIDENCE = {}


def _archive_file(fid):
    """One archive file's decompressed bytes (its nisitenma.tsv row)."""
    with open(os.path.join(ROOT, "nisitenma.tsv"), encoding="utf-8") as f:
        for line in f.read().splitlines()[1:]:
            p = line.split("\t")
            if p[0] == str(fid):
                with open(arc.DECOMPRESSED, "rb") as g:
                    g.seek(int(p[2], 16))
                    return g.read(int(p[3], 16))
    return b""


def code_evidence():
    """What the matched C (src/, src/tlb/) says lies in these files: {"course": {(file, offset):
    points}, "copy": {(offset, size)}}.
      vramSetCourse(n, kind, (Vec3f *) floadSegToRam((u32) SYM, set, k)): a course of kind
        points, kind + 2 Vec3f (a cardinal spline), in file 11's FileSet[set].file[k];
      heapMemCopy(dst, <SYM in a file>, size): vertices the code copies out of a file.
    SYM is a D_3xxxxxx name or a name undefined_syms.txt gives a segment-3 address (the names
    the segment addresses were given)."""
    if _EVIDENCE:
        return _EVIDENCE
    sets = _archive_file(11)
    course, copy = {}, set()
    num = r"(0x[0-9A-Fa-f]+|\d+)"
    with open(os.path.join(ROOT, "undefined_syms.txt"), encoding="utf-8") as f:
        named = {m.group(1): int(m.group(2), 16)
                 for m in re.finditer(r"^(\w+)\s*=\s*(0x[0-9A-Fa-f]+);", f.read(), re.M)}

    def seg3(name):
        """A symbol's segment-3 address, or None."""
        a = int(name[2:], 16) if re.fullmatch(r"D_[0-9A-F]+", name) else named.get(name)
        return a if a is not None and a >> 24 == 3 else None

    for d in ("src", os.path.join("src", "tlb")):
        for n in sorted(os.listdir(os.path.join(ROOT, d))):
            if not n.endswith(".c"):
                continue
            with open(os.path.join(ROOT, d, n), encoding="utf-8") as f:
                text = f.read()
            calls = [(m.group(1), m.group(2), m.group(3), m.group(4)) for m in re.finditer(
                r"vramSetCourse\(\s*[^,()]+,\s*%s,\s*\(Vec3f \*\)\s*floadSegToRam\(\(u32\)\s*(\w+),"
                r"\s*%s,\s*(\d+)\)\)" % (num, num), text)]
            # the same through a local (tlb_0221: p = floadSegToRam(...); vramSetCourse(1, 0x31, p);)
            calls += [(m.group(5), m.group(2), m.group(3), m.group(4)) for m in re.finditer(
                r"\b(\w+) = (?:\([^()]*\)\s*)?floadSegToRam\(\(u32\)\s*(\w+),\s*%s,\s*(\d+)\);\s*"
                r"vramSetCourse\(\s*[^,()]+,\s*%s,\s*(?:\([^()]*\)\s*)?\1\)" % (num, num), text)]
            for kind, sym, st, k in calls:
                addr = seg3(sym)
                if addr is None:
                    continue
                kind, st, k = int(kind, 0), int(st, 0), int(k)
                course[(struct.unpack_from(">h", sets, st * 8 + 2 * k)[0], addr & 0xFFFFFF)] = kind + 2
            for m in re.finditer(r"heapMemCopy\(([^;]*?),\s*%s\);" % num, text):
                addr = next((a for a in map(seg3, re.findall(r"\b[A-Za-z_]\w*", m.group(1))) if a is not None), None)
                if addr is not None:
                    copy.add((addr & 0xFFFFFF, int(m.group(2), 0)))
    _EVIDENCE.update(course=course, copy=copy)
    return _EVIDENCE


def _finite(v, lim=1e7):
    return v == v and abs(v) < lim


def find_colls(b, seg, free):
    """vram.c's Coll (vramLoadColl): its CollTri[count], its lists (u16 triangle numbers, each
    list ending in 0), its grid (a u16 list start per cell, 0xFFFF none), then the header."""
    out = []
    for o in range(0, len(b) - 0x34 + 1, 4):
        if not all(free[o:o + 0x34]):
            continue
        g, l, t = struct.unpack_from(">III", b, o + 0x28)
        cnt = struct.unpack_from(">i", b, o + 0x24)[0]
        if (g >> 24, l >> 24, t >> 24) != (seg, seg, seg) or not 0 < cnt < 20000:
            continue
        g, l, t = g & 0xFFFFFF, l & 0xFFFFFF, t & 0xFFFFFF
        dims = struct.unpack_from(">3h", b, o + 0xC)
        cells = dims[0] * dims[1] * dims[2]
        if not (0 < cells < 200000 and t % 4 == 0 and t + 0x28 * cnt <= l <= g and g + 2 * cells <= len(b)
                and all(free[t:g + 2 * cells])):
            continue
        lists = struct.unpack_from(">%dH" % ((g - l) // 2), b, l)
        if any(x >= cnt for x in lists) or any(x != 0xFFFF and x >= len(lists) for x in struct.unpack_from(">%dH" % cells, b, g)):
            continue
        new = [Region(o, o + 0x34, "coll", 4, count=cnt), Region(t, t + 0x28 * cnt, "tris", 4)]
        if g % 4:  # an array starts on 4 bytes: the grid is then the lists' tail
            new.append(Region(l, g + 2 * cells, "lists", 4))
        else:
            new += [Region(l, g, "lists", 4), Region(g, g + 2 * cells, "grid", 4)]
        for r in new:
            free[r.start:r.end] = bytes(r.end - r.start)
        out += new
    return out


def find_atks(b, seg, free):
    """aniatk.c's attack chains: the header (aniAtkJointCount, aniAtkFrameCount, aniAtkTrack
    Stream, aniAtkJoints), the joints (four u16 tracks each), the stream (aniAtkWalkTrack: the
    same runs as anim.c's keys)."""
    out = []
    for o in range(0, len(b) - 12 + 1, 4):
        if not all(free[o:o + 12]):
            continue
        w0, w1, w2 = struct.unpack_from(">III", b, o)
        frames, n = w0 & 0xFF, (w0 >> 8) & 0x7FFF
        if not (frames and 1 <= n <= 64 and (w0 >> 23) == 0 and w1 >> 24 == seg and w2 >> 24 == seg):
            continue
        s, j = w1 & 0xFFFFFF, w2 & 0xFFFFFF
        if s >= len(b) or j % 4 or j + 8 * n > len(b) or not all(free[j:j + 8 * n]):
            continue
        end = s
        for k in range(4 * n):
            v = _be16(b, j + 2 * k) & 0xBFFF
            e = None if v > 0xA000 else track_end(b, s, v & 0x7FFF, 1 if v & 0x8000 else frames)
            if v <= 0xA000 and e is None:
                end = None
                break
            end = max(end, e or end)
        if end is None or (end > s and not all(free[s:end])):
            continue
        new = [Region(o, o + 12, "atk", 4, frames=frames, joints=n), Region(j, j + 8 * n, "joints", 4)]
        new += [Region(s, end, "stream", 4)] if end > s else []
        for r in new:
            free[r.start:r.end] = bytes(r.end - r.start)
        out += new
    return out


def find_vanimes(b, seg, free):
    """vanime.c's VAnime (vanimeMakeVtx): count, frames, the s, t pairs, up to five keyframes of
    VKeys (s16 pos[3], s8 col[3]; 10 bytes)."""
    out = []
    for o in range(0, len(b) - 0x1C + 1, 4):
        if not all(free[o:o + 0x1C]):
            continue
        count, frames = struct.unpack_from(">hh", b, o)
        tcs, *keys = struct.unpack_from(">6I", b, o + 4)
        used = [k for k in keys if k]
        if not (1 <= count <= 4096 and 1 <= frames <= 5) or any(p >> 24 != seg for p in [tcs] + used) or len(used) < frames:
            continue
        pieces = sorted({(tcs & 0xFFFFFF, 4 * count, "tcs")} | {(k & 0xFFFFFF, 10 * count, "vkeys") for k in used})
        if any(p % 4 or p + n > len(b) or not all(free[p:p + n]) for p, n, _ in pieces):
            continue
        new = [Region(o, o + 0x1C, "vanime", 4)] + [Region(p, p + n, kind, 4) for p, n, kind in pieces]
        for r in new:
            free[r.start:r.end] = bytes(r.end - r.start)
        out += new
    return out


def find_rides(b, free):
    """ride.c's RideData (rideCreate): s16 count, f32 radius, u8 unk8[4], then count RideBoxes
    (u16 flags 0-7, f32 half[3], pos[3], rot[3], and an MtxF inv that is zero in ROM)."""
    out, o = [], 0
    while o + 12 + 0x68 <= len(b):
        cnt = struct.unpack_from(">h", b, o)[0]
        end = o + 12 + 0x68 * cnt
        ok = 1 <= cnt <= 64 and end <= len(b) and all(free[o:end]) and b[o + 2:o + 4] == b"\0\0"
        rad = struct.unpack_from(">f", b, o + 4)[0]
        ok = ok and _finite(rad) and rad > 0
        for k in range(cnt if ok else 0):
            bo = o + 12 + 0x68 * k
            if (_be16(b, bo) > 7 or b[bo + 2:bo + 4] != b"\0\0" or any(b[bo + 0x28:bo + 0x68])
                    or not all(_finite(x) for x in struct.unpack_from(">9f", b, bo + 4))):
                ok = False
                break
        if ok:
            out.append(Region(o, end, "ride", 4, count=cnt))
            free[o:end] = bytes(end - o)
            o = end
        else:
            o += 4
    return out


def _partition(fid, b, extra_vtx):
    seg, gb = segment(fid), gbi_of(b)
    dls = find_dls(b, seg, gb)
    # what the lists load (Lights0, Vtx) outranks a list the scan found inside it (the light's
    # or the vertices' bytes read as commands): that list starts after them
    for _ in range(4):
        called = {t for e, r in dls.values() for t in r["dl"]}
        loaded = [r for r in _dl_regions(dls) if r.kind in ("vtx", "light")]
        moved = False
        for s in sorted(dls):
            hit = None if s in called else next((r for r in loaded if r.start <= s < r.end), None)
            if hit:
                e = dls.pop(s)[0]
                s2 = (hit.end + 7) & ~7
                r2 = walk_dl(b, s2, seg, gb) if s2 < e and s2 not in dls else None
                if r2:
                    dls[s2] = (r2[1], r2[0])
                moved = True
        if not moved:
            break
    taken = {i for r in _dl_regions(dls, extra_vtx) for i in range(r.start, r.end)}
    anims = find_anims(b, seg, dls, taken, gb)
    _called(b, seg, dls, gb)
    out, keys = _dl_regions(dls, extra_vtx), {}
    for o, an in sorted(anims.items()):
        out.append(Region(o, an["end"], "attach" if "attach" in an else "anim", 4, info=an))
        if an["keys"] is not None and an["spans"]:
            keys[an["keys"]] = max(keys.get(an["keys"], an["keys"]), max(e for s, e in an["spans"]))
    out += [Region(s, e, "nodes", 4) for s, e in _runs(sorted({no for an in anims.values() for no in an["nodes"]}), 0x18)]
    out += [Region(k0, e, "keys", 4) for k0, e in sorted(keys.items())]
    owner, bad, kept = {}, [], []
    free = bytearray(b"\1" * len(b))

    def claim(rs):
        for r in rs:
            if any(i in owner for i in range(r.start, r.end)):
                bad.append(r)
                continue
            for i in range(r.start, r.end):
                owner[i] = r
            kept.append(r)
            free[r.start:r.end] = bytes(r.end - r.start)
    claim(out)
    for find in (lambda: find_colls(b, seg, free), lambda: find_atks(b, seg, free),
                 lambda: find_vanimes(b, seg, free), lambda: find_rides(b, free)):
        claim(find())
    for (f, off), pts in code_evidence()["course"].items():
        if f == fid and off + 12 * pts <= len(b) and all(free[off:off + 12 * pts]):
            claim([Region(off, off + 12 * pts, "course", 4)])
    for o in range(0, len(b) - 7, 8):  # an empty display list: a G_ENDDL alone
        if b[o:o + 8] == gb["end"] and all(free[o:o + 8]):
            claim([Region(o, o + 8, "dl", 8)])
    kept.sort(key=lambda r: r.start)
    full, at = [], 0
    for r in kept + [Region(len(b), len(b), "end")]:
        if r.start > at:
            full.append(Region(at, r.start, "unk" if any(b[at:r.start]) else "pad", 1))
        if r.kind != "end":
            full.append(r)
        at = max(at, r.end)
    return seg, full, bad


def regions(fid, b):
    """An object model's pieces in file order (Region: kind, start, end; the bytes no reader
    explains "unk", zeros "pad"), and the pieces that overlapped others: (segment, pieces, bad).
    Vertices the code copies out (code_evidence) are Vtx when the file's lists load segment 8
    (the buffer the code fills) and nothing else here explains their bytes."""
    seg, full, bad = _partition(fid, b, ())
    unk = [r for r in full if r.kind == "unk"]
    gb = gbi_of(b)
    v8 = 0
    for s, (e, refs) in find_dls(b, seg, gb).items():
        for o in range(s, e, 8):
            if b[o] == gb["vtx"] and b[o + 4] == 0x08:
                v8 = max(v8, (_be32(b, o + 4) & 0xFFFFFF) + 16 * gb["n"](_be32(b, o)))
    extra = set()
    for off, size in code_evidence()["copy"]:
        run = next((r for r in unk if r.start == off), None)
        if v8 and run and size % 16 == 0 and off % 8 == 0 and v8 <= min(size, run.end - off):
            extra.add((off, min(size, run.end - off) // 16))
    return _partition(fid, b, extra) if extra else (seg, full, bad)


# the structs of the files that read them, copied with their comments
_TYPES = {
    "coll": ["/* vram.c's */",
             "typedef struct CollTri {",
             "    /* 0x00 */ s16 v[3]; /* a corner */",
             "    /* 0x06 */ s16 e1[3]; /* and the two edges from it */",
             "    /* 0x0C */ s16 e2[3];",
             "    /* 0x12 */ u8 unk12[2];",
             "    /* 0x14 */ f32 n[3]; /* the normal; x above 10: not made yet */",
             "    /* 0x20 */ f32 d; /* the plane's offset */",
             "    /* 0x24 */ u8 kind;",
             "    /* 0x25 */ u8 zone; /* the course the ground belongs to (0: none) */",
             "    /* 0x26 */ u8 unk26[2];",
             "} CollTri;",
             "typedef struct Coll {",
             "    /* 0x00 */ f32 origin[3];",
             "    /* 0x0C */ s16 dims[3]; /* cells */",
             "    /* 0x12 */ u8 unk12[2];",
             "    /* 0x14 */ f32 cell[3]; /* a cell's size */",
             "    /* 0x20 */ s32 file;",
             "    /* 0x24 */ s32 count; /* triangles */",
             "    /* 0x28 */ u16 *grid; /* a list's start for each cell (0xFFFF: none) */",
             "    /* 0x2C */ u16 *lists; /* triangle numbers, each list ending in 0 */",
             "    /* 0x30 */ CollTri *tris;",
             "} Coll;", ""],
    "ride": ["/* ride.c's */",
             "typedef struct MtxF {",
             "    f32 m[4][4];",
             "} MtxF;",
             "typedef struct RideBox {",
             "    /* 0x00 */ u16 flags; /* 1, 2, 4: its x, y or z faces are open */",
             "    /* 0x04 */ f32 half[3];",
             "    /* 0x10 */ f32 pos[3];",
             "    /* 0x1C */ f32 rot[3];",
             "    /* 0x28 */ MtxF inv;",
             "} RideBox;", ""],
    "vanime": ["/* vanime.c's */",
               "typedef struct VAnime {",
               "    /* 0x00 */ s16 count; /* vertices */",
               "    /* 0x02 */ s16 frames; /* keyframes */",
               "    /* 0x04 */ u32 tcs; /* each vertex's s, t (s16 each) */",
               "    /* 0x08 */ u32 keys[5]; /* each keyframe's vertices (VKey) */",
               "} VAnime;",
               "typedef struct VKey {",
               "    /* 0x0 */ s16 pos[3];",
               "    /* 0x6 */ s8 col[3];",
               "} VKey;", ""],
}
_STEP = {"vtx": 16, "dl": 8, "nodes": 0x18, "keys": 1, "anim": 4, "attach": 4, "tris": 0x28, "lists": 2,
         "grid": 2, "atk": 4, "joints": 8, "stream": 1, "tcs": 4, "vkeys": 10, "course": 12}


def _f(v):
    if v == 0 and struct.pack(">f", v) != bytes(4):
        return "-0.0f"
    s = format(v, ".9g")
    return (s if any(c in s for c in ".en") else s + ".0") + "f"


def object_to_c(fid, b):
    """An object model as C: every piece in file order with its reader's type, the pointers
    between pieces as symbols, the gaps IDO's alignment does not make as u8 arrays."""
    import pygfxd as g
    seg, regs, bad = regions(fid, b)
    if bad:
        raise ValueError(f"{fid}: overlapping pieces {bad[:3]}")
    gb = gbi_of(b)
    content = [r for r in regs if r.kind not in ("pad", "unk")]

    def name(r):
        return f"mdl_{fid:04d}_{r.kind}_{r.start:04X}"

    def ref(addr):
        """A segmented address here as C: a symbol, an element, or a struct's address."""
        if addr >> 24 != seg:
            return None
        r = next((r for r in content if r.start <= (addr & 0xFFFFFF) < r.end), None)
        if r is None:
            return None
        d = (addr & 0xFFFFFF) - r.start
        if r.kind == "light":
            return f"&{name(r)}.a" if d == 0 else None
        if r.kind in ("coll", "ride", "vanime"):
            return f"&{name(r)}" if d == 0 else None
        if d == 0 and r.kind not in ("vtx", "nodes", "anim", "attach", "joints", "course"):
            return name(r)
        step = _STEP.get(r.kind)
        return f"&{name(r)}[{d // step}]" if step and d % step == 0 else None

    def ptr(v):
        s = ref(v)
        if s is None:
            raise ValueError(f"{fid}: 0x{v:08X} is not the start of a piece")
        return s

    def word(v):
        """A u32 that may point here: (u32) and the symbol, keeping a top-nibble flag."""
        s = ref(v & 0x0FFFFFFF) if (v & 0x0FFFFFFF) >> 24 == seg else None
        if s is None:
            return f"0x{v:08X}" if v else "0"
        return f"(u32) {s}" + (f" + 0x{v & 0xF0000000:08X}" if v & 0xF0000000 else "")

    def fls(o, n):
        return ", ".join(_f(x) for x in struct.unpack_from(">%df" % n, b, o))

    def rows(items, per=1, indent="    "):
        return "{\n" + "".join(indent + ", ".join(items[i:i + per]) + ",\n" for i in range(0, len(items), per)) + "}"

    def gfx(r):
        def macro():
            g.gfxd_puts("    ")
            g.gfxd_macro_dflt()
            g.gfxd_puts(",\n")
            return 0

        def sym(addr, *_):
            s = ref(addr)
            if s:
                g.gfxd_printf(s)
            return 1 if s else 0
        g.gfxd_input_buffer(b[r.start:r.end])
        outb = bytes((r.end - r.start) * 80 + 4096)
        ob = g.gfxd_output_buffer(outb, len(outb))
        g.gfxd_target(getattr(g, gb["target"]))
        g.gfxd_endian(g.GfxdEndian.big, 4)
        g.gfxd_macro_fn(macro)
        g.gfxd_vtx_callback(sym)
        g.gfxd_dl_callback(sym)
        g.gfxd_light_callback(sym)
        g.gfxd_execute()
        return "{\n" + g.gfxd_buffer_to_string(ob).rstrip("\n") + "\n}"

    texts = {}
    for r in content:
        n, o, size = name(r), r.start, r.end - r.start
        if r.kind in ("keys", "stream"):
            texts[r] = (f"u8 {n}[{size}]", rows([f"0x{x:02X}" for x in b[o:r.end]], 16))
        elif r.kind == "nodes":
            items = []
            for p in range(o, r.end, 0x18):
                c, s = struct.unpack_from(">bb", b, p + 4)
                t = [f"0x{x:04X}" for x in struct.unpack_from(">9H", b, p + 6)]
                items.append(f"{{ {word(_be32(b, p))}, {c}, {s}, {{ {', '.join(t[0:3])} }}, {{ {', '.join(t[3:6])} }}, "
                             f"{{ {', '.join(t[6:9])} }} }}")
            texts[r] = (f"ModelNode {n}[{size // 0x18}]", rows(items))
        elif r.kind in ("anim", "attach"):
            an = r.info
            h = an.get("attach", o)
            vals = [word(_be32(b, p)) for p in range(o, r.end, 4)]
            roots = len(an["roots"])
            vals[(h - o) // 4] = (f"0x{_be32(b, h):08X} /* {an['frames']} frames"
                                  + ("" if r.kind == "attach" else f", {roots} root{'s' if roots > 1 else ''}") + " */")
            texts[r] = (f"u32 {n}[{size // 4}]", "{ " + ", ".join(vals) + " }")
        elif r.kind == "vtx":
            items = []
            for p in range(o, r.end, 16):
                x, y, z, flag, s, t = struct.unpack_from(">hhhHhh", b, p)
                items.append(f"{{{{{{ {x}, {y}, {z} }}, 0x{flag:X}, {{ {s}, {t} }}, "
                             f"{{ {', '.join(f'0x{c:02X}' for c in b[p + 12:p + 16])} }}}}}}")
            texts[r] = (f"Vtx {n}[{size // 16}]", rows(items))
        elif r.kind == "light":
            x = b[o:r.end]
            if x[0:3] != x[4:7] or x[3] or x[7] or any(x[8:24]):
                raise ValueError(f"{fid}: {r} is not a Lights0")
            texts[r] = (f"Lights0 {n}", f"gdSPDefLights0(0x{x[0]:02X}, 0x{x[1]:02X}, 0x{x[2]:02X})")
        elif r.kind == "dl":
            texts[r] = (f"Gfx {n}[]", gfx(r))
        elif r.kind == "coll":
            texts[r] = (f"Coll {n}", f"{{ {{ {fls(o, 3)} }}, {{ {', '.join(map(str, struct.unpack_from('>3h', b, o + 0xC)))} }}, "
                                    f"{{ 0x{b[o + 0x12]:02X}, 0x{b[o + 0x13]:02X} }}, {{ {fls(o + 0x14, 3)} }}, "
                                    f"{struct.unpack_from('>i', b, o + 0x20)[0]}, {r.count}, {ptr(_be32(b, o + 0x28))}, "
                                    f"{ptr(_be32(b, o + 0x2C))}, {ptr(_be32(b, o + 0x30))} }}")
        elif r.kind == "tris":
            items = []
            for p in range(o, r.end, 0x28):
                v = struct.unpack_from(">9h", b, p)
                items.append(f"{{ {{ {v[0]}, {v[1]}, {v[2]} }}, {{ {v[3]}, {v[4]}, {v[5]} }}, {{ {v[6]}, {v[7]}, {v[8]} }}, "
                             f"{{ 0x{b[p + 0x12]:02X}, 0x{b[p + 0x13]:02X} }}, {{ {fls(p + 0x14, 3)} }}, {fls(p + 0x20, 1)}, "
                             f"{b[p + 0x24]}, {b[p + 0x25]}, {{ 0x{b[p + 0x26]:02X}, 0x{b[p + 0x27]:02X} }} }}")
            texts[r] = (f"CollTri {n}[{size // 0x28}]", rows(items))
        elif r.kind in ("lists", "grid"):
            texts[r] = (f"u16 {n}[{size // 2}]", rows([f"0x{x:X}" for x in struct.unpack_from(">%dH" % (size // 2), b, o)], 16))
        elif r.kind == "atk":
            texts[r] = (f"u32 {n}[3]", f"{{ 0x{_be32(b, o):08X} /* {r.joints} joints, {r.frames} frames */, "
                                       f"(u32) {ptr(_be32(b, o + 4))}, (u32) {ptr(_be32(b, o + 8))} }}")
        elif r.kind == "joints":
            texts[r] = (f"u16 {n}[{size // 8}][4]", rows(["{ " + ", ".join(f"0x{x:04X}" for x in struct.unpack_from(">4H", b, p)) + " }"
                                                          for p in range(o, r.end, 8)]))
        elif r.kind == "ride":
            boxes = [f"{{ 0x{_be16(b, p):X}, {{ {fls(p + 4, 3)} }}, {{ {fls(p + 0x10, 3)} }}, {{ {fls(p + 0x1C, 3)} }}, {{ {{ {{ 0 }} }} }} }}"
                     for p in range(o + 12, r.end, 0x68)]
            texts[r] = (f"struct {{ s16 count; f32 radius; u8 unk8[4]; RideBox boxes[{r.count}]; }} {n}",
                        f"{{\n    {r.count}, {fls(o + 4, 1)}, {{ {', '.join(f'0x{x:02X}' for x in b[o + 8:o + 12])} }},\n    "
                        + rows(boxes, indent="        ")[:-1] + "    }\n}")
        elif r.kind == "course":
            texts[r] = (f"Vec3f {n}[{size // 12}]", rows([f"{{ {fls(p, 3)} }}" for p in range(o, r.end, 12)]))
        elif r.kind == "vanime":
            cnt, frames = struct.unpack_from(">hh", b, o)
            keys = ", ".join(f"(u32) {ptr(k)}" if k else "0" for k in struct.unpack_from(">5I", b, o + 8))
            texts[r] = (f"VAnime {n}", f"{{ {cnt}, {frames}, (u32) {ptr(_be32(b, o + 4))}, {{ {keys} }} }}")
        elif r.kind == "tcs":
            v = struct.unpack_from(">%dh" % (size // 2), b, o)
            texts[r] = (f"s16 {n}[{size // 4}][2]", rows([f"{{ {v[i]}, {v[i + 1]} }}" for i in range(0, len(v), 2)], 6))
        elif r.kind == "vkeys":
            items = []
            for p in range(o, r.end, 10):
                k = struct.unpack_from(">3h3b", b, p)
                if b[p + 9]:
                    raise ValueError(f"{fid}: the VKey at 0x{p:X} has a pad byte")
                items.append(f"{{ {{ {k[0]}, {k[1]}, {k[2]} }}, {{ {k[3]}, {k[4]}, {k[5]} }} }}")
            texts[r] = (f"VKey {n}[{size // 10}]", rows(items, 3))
        else:
            raise ValueError(f"{fid}: no C for {r}")
    # placement: IDO starts every array on 4 bytes (Vtx, Gfx, Lights0 on 8); a gap it does not
    # make is an array of its own bytes
    at, order = 0, []

    def gap(s, e):
        data = b[s:e]
        kind = "unk" if any(data) else "pad"
        order.append((f"u8 mdl_{fid:04d}_{kind}_{s:04X}[{e - s}]",
                      "{ 0 }" if kind == "pad" else "{ " + ", ".join(f"0x{x:02X}" for x in data) + " }", kind))
    for r in [r for r in regs if r.kind not in ("pad", "unk")] + [Region(len(b), len(b), "end")]:
        p4 = (at + 3) & ~3
        if r.start > at and any(b[at:min(p4, r.start)]):
            raise ValueError(f"{fid}: bytes at 0x{at:X} where the compiler pads")
        if r.kind == "end":
            last = len(b)
            while last > at and b[last - 1] == 0:  # the build pads the file's tail with zeros
                last -= 1
            if last > at:
                gap(p4, (last + 3) & ~3)
            break
        if r.start % r.align:
            raise ValueError(f"{fid}: {r} is not on {r.align} bytes")
        if r.start > at and (((at + r.align - 1) & ~(r.align - 1)) != r.start or any(b[at:r.start])):
            gap(p4, r.start)
        order.append(texts[r] + (r.kind,))
        at = r.end
    head = [f"/* Archive file {fid} ({fid:#x}), linked at 0x{seg << 24:08X} (file.c gFileGroupTable: group {seg}). */"]
    if gb is GBIS["f3dex"]:
        head += ["/* Its display lists are in the F3DEX 1.x GBI (gbi.h's F3DEX_GBI), not the game's F3DEX2. */",
                 "#undef F3DEX_GBI_2", "#define F3DEX_GBI"]
    head += ['#include "ultra64.h"', '#include "object.h"', ""]
    kinds = {kind for _, _, kind in order}
    for key, used in (("coll", {"coll", "tris"}), ("ride", {"ride"}), ("vanime", {"vanime", "vkeys"})):
        if kinds & used:
            head += _TYPES[key]
    # every piece declared first, so a pointer can name a piece defined after it (a RideData's
    # struct has no tag, and nothing points at one)
    decls = [f"extern {d};" for d, _, kind in order if kind not in ("pad", "unk", "ride")]
    return "\n".join(head + decls + [""] + [f"{d} = {init};\n" for d, init, _ in order])


def object_typed(fid, data):
    """True when the object reader puts every byte of the file in a piece its reader walks: no
    "unk", no overlapping pieces."""
    seg, regs, bad = regions(fid, data)
    return not bad and not any(r.kind == "unk" for r in regs)


def source(fid, data):
    """A model file's C: the object reader's, unless the file is one dl_start claims and the
    object reader leaves bytes unexplained; then the gpol reader's. dl_start takes vertices and
    then lists to a G_ENDDL, so an object model that starts that way (file 769: an animation
    header among the vertices; file 766: small lists among them) came out as Vtx."""
    if dl_start(data) and not object_typed(fid, data):
        return to_c(fid, data)
    return object_to_c(fid, data)


def build(c_text, scratch, base=SEG3):
    """The bytes the C gives: compiled with the project's IDO and flags, linked at base (segment
    3 for a model; an object model's first line names its own segment, "linked at 0x0N000000",
    and that wins), .data then .rodata (a string literal's place)."""
    m = re.match(r"/\*[^\n]*linked at (0x[0-9A-F]{8})", c_text)
    if m:
        base = int(m.group(1), 16)
    os.makedirs(scratch, exist_ok=True)
    src, obj = os.path.join(scratch, "model.c"), os.path.join(scratch, "model.o")
    elf, out = os.path.join(scratch, "model.elf"), os.path.join(scratch, "model.bin")
    for tries in range(20):
        try:
            with open(src, "w", encoding="utf-8", newline="\n") as f:
                f.write(c_text)
            break
        except PermissionError:
            # Windows: something still holds the last run's model.c (a scanner); wait it out
            if tries == 19:
                raise
            time.sleep(0.5)
    mf =os.path.join(os.path.dirname(os.path.abspath(__file__)), "match_function.py")
    r = subprocess.run([sys.executable, mf, "compile", src, "-o", obj], cwd=ROOT, capture_output=True, text=True)
    if r.returncode:
        raise RuntimeError(f"compile: {r.stdout[-400:]}{r.stderr[-400:]}")
    script = os.path.join(scratch, "link.ld")
    with open(script, "w", newline="\n") as f:
        f.write(f"SECTIONS {{ . = 0x{base:X}; .data : {{ *(.data) *(.rodata) }} /DISCARD/ : {{ *(*) }} }}\n")
    import build_rom  # the build's binutils (tools/, or BINUTILS)
    subprocess.run([build_rom.LD, "-EB", "-m", "elf32ebmip", "--no-check-sections",
                    "-T", script, "-e", "0", "-o", elf, obj], check=True, capture_output=True)
    subprocess.run([build_rom.OBJCOPY, "-O", "binary", "-j", ".data", elf, out],
                   check=True, capture_output=True)
    with open(out, "rb") as f:
        return f.read()


def _check(fid, data, make, scratch):
    """None if the C gives the file's bytes, else where it first differs."""
    try:
        got = build(make(fid, data), scratch)
    except RuntimeError:
        return f"{fid}(compile)"
    except ValueError as e:
        return f"{fid}({str(e)[:60]})"
    got = got + bytes(len(data) - len(got)) if len(got) < len(data) else got
    if got == data:
        return None
    k = next((i for i in range(min(len(got), len(data))) if got[i] != data[i]), None)
    return f"{fid}@0x{k:X}" if k is not None else f"{fid}(len)"


def main():
    cmd = sys.argv[1:]
    files = archive()
    if cmd and cmd[0] == "check":
        want = [int(x) for x in cmd[1:]] or None
        scratch = os.path.join(ROOT, "permuter", "model_codec", str(os.getpid()))  # one per run
        claimed = [fid for fid, data in sorted(files.items()) if (want is None or fid in want) and dl_start(data)]
        models = [fid for fid in claimed if not object_typed(fid, files[fid])]
        objects = [fid for fid in object_files(files) if want is None or fid in want]
        print(f"dl_start's files: {len(claimed)}; the object reader explains every byte of "
              f"{len(claimed) - len(models)}, written as object models; {len(models)} as gpol")
        objects = sorted(set(objects) | (set(claimed) - set(models)))
        bad = [x for x in (_check(fid, files[fid], to_c, scratch) for fid in models) if x]
        print(f"models: {len(models) - len(bad)} of {len(models)} gpol files through C and back byte-exact;"
              f" not: {' '.join(bad[:20])}")
        bad2, unk = [], []
        for fid in objects:
            x = _check(fid, files[fid], object_to_c, scratch)
            if x:
                bad2.append(x)
            elif any(r.kind == "unk" for r in regions(fid, files[fid])[1]):
                unk.append(str(fid))
        print(f"object models: {len(objects) - len(bad2)} of {len(objects)} files through C and back byte-exact,"
              f" {len(objects) - len(bad2) - len(unk)} with every byte in a piece its reader walks;"
              f" bytes no reader explains: {' '.join(unk[:20])}; not: {' '.join(bad2[:20])}")
        return 0 if not bad and not bad2 else 1
    if len(cmd) == 2 and cmd[0] == "extract":
        os.makedirs(cmd[1], exist_ok=True)
        objects = set(object_files(files))
        for fid, data in sorted(files.items()):
            if dl_start(data) or fid in objects:
                path, text = os.path.join(cmd[1], f"geo_{fid:04d}.c"), source(fid, data)
                if os.path.exists(path):  # written only when it changes: the build's compiles stay cached
                    with open(path, encoding="utf-8") as f:
                        if f.read() == text:
                            continue
                with open(path, "w", encoding="utf-8", newline="\n") as f:
                    f.write(text)
        return 0
    if len(cmd) == 2 and cmd[0] == "regions":
        seg, regs, bad = regions(int(cmd[1]), files[int(cmd[1])])
        print(f"segment {seg:X}: " + " ".join(map(repr, regs)) + (f"\noverlapping: {bad}" if bad else ""))
        return 0
    sys.exit(__doc__)


if __name__ == "__main__":
    sys.exit(main())
