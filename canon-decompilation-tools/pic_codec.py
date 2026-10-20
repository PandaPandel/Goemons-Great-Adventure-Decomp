"""The game's "PIC" images (X68000 PIC, src/pic.c), decoded and encoded back to the same bytes.

    python canon-decompilation-tools/pic_codec.py list            every PIC sub-file: id, depth, size, mode
    python canon-decompilation-tools/pic_codec.py check [ID]      decode then encode each, compare the bytes
    python canon-decompilation-tools/pic_codec.py events ID       the decisions in one image's stream
    python canon-decompilation-tools/pic_codec.py raws [-v]       the raw sub-files' formats, the unresolved
                                                                  and why (-v: every one and its evidence)

The PTX containers (gPtxParentTable, gPtxOffsetTable in file.c) hold sub-files 0x8000-0x8808;
most are PIC images. A PIC: "PIC", a comment ended by 0x1A (its first four characters, in hex,
may name the transparent colour), a reserved string ended by 0, a byte, the mode (4 bits), 4
bits, the depth (16 bits), width, height (16 bits each), then the body, MSB first:
  - 4/8-bit with type 1: the palette, 16 or 256 entries of 5 bits G, R, B and 1 bit I;
  - 15-bit (picDecodeDirect): run lengths of pixels copied from the line above, each ended by
    a new colour that may chain down the lines below;
  - 16/8/4-bit (picDecodeSkips): skips to the next new colour, each of which may chain down;
    the pixels skipped repeat the last one written, in raster order.
A colour of 15/16 bits comes from a 128-entry ring cache (a 1 bit and the entry) or new (a 0
bit and the colour); an index of 4/8 bits is written as is. A chain continues one line down
per code: 2 straight, 1 left, 3 right, 0 then 1 then 0/1 two left/right, 0 then 0 ends it..

Changelog: first public release (2026-10-20). Earlier history is kept privately.
"""

import collections
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BASEROM = os.path.join(ROOT, "baserom.us.z64")
DECOMPRESSED = os.path.join(ROOT, "baserom.us.decompressed.z64")
MAIN_ROM, MAIN_VRAM = 0x1000, 0x80000400
FILE_ROM_OFFSETS = 0x8002BB94   # gFileRomOffsets: file N's ROM start is [N - 1] (bit 31: compressed)
PTX_PARENT = 0x8002E3DC         # gPtxParentTable: (first sub-file, parent file) pairs, 0-terminated
PTX_OFFSET = 0x8002E49C         # gPtxOffsetTable: sub-file N's offset in its parent is [N - 0x8000]
NO_TRANSPARENT = 0x80000000


def be16(b, o):
    return int.from_bytes(b[o:o + 2], "big")


def be32(b, o):
    return int.from_bytes(b[o:o + 4], "big")


def subfiles(with_parent=False):
    """[(id, bytes)] for every PTX sub-file, by the game's own rule (fileRomSize); with_parent,
    [(id, parent file id, bytes)]."""
    with open(DECOMPRESSED, "rb") as f:
        dec = f.read()
    with open(BASEROM, "rb") as f:
        rom = f.read()

    def main(vram):
        return MAIN_ROM + vram - MAIN_VRAM

    parents = []
    i = 0
    while be16(dec, main(PTX_PARENT) + 2 * i):
        parents.append((be16(dec, main(PTX_PARENT) + 2 * i), be16(dec, main(PTX_PARENT) + 2 * i + 2)))
        i += 2
    out = []
    for sub in range(0x8000, 0x8809):
        k = max(j for j, (first, _) in enumerate(parents) if sub >= first)
        parent = parents[k][1]
        start = be32(dec, main(FILE_ROM_OFFSETS) + 4 * (parent - 1)) & 0x7FFFFFFF
        end = be32(dec, main(FILE_ROM_OFFSETS) + 4 * parent) & 0x7FFFFFFF
        off = be32(dec, main(PTX_OFFSET) + 4 * (sub - 0x8000))
        nxt = be32(dec, main(PTX_OFFSET) + 4 * (sub - 0x8000 + 1)) if sub < 0x8808 else 0
        at = (off & 0xFFFFFF) + start
        if nxt and (k + 1 >= len(parents) or sub + 1 < parents[k + 1][0]):
            size = nxt - off
        else:
            size = end - at
        out.append((sub, parent, rom[at:at + size]) if with_parent else (sub, rom[at:at + size]))
    return out


class BitReader:
    def __init__(self, data):
        self.data, self.pos, self.left, self.byte = data, 0, 0, 0

    def bit(self):
        return self.bits(1)

    def bits(self, n):
        v = 0
        for _ in range(n):
            if self.left == 0:
                self.byte = self.data[self.pos] if self.pos < len(self.data) else 0
                self.pos += 1
                self.left = 8
            self.left -= 1
            v = (v << 1) | ((self.byte >> self.left) & 1)
        return v


class BitWriter:
    def __init__(self):
        self.out, self.byte, self.used = bytearray(), 0, 0

    def bits(self, v, n):
        for k in range(n - 1, -1, -1):
            self.byte = (self.byte << 1) | ((v >> k) & 1)
            self.used += 1
            if self.used == 8:
                self.out.append(self.byte)
                self.byte, self.used = 0, 0

    def flush(self):
        if self.used:
            self.out.append(self.byte << (8 - self.used))
            self.byte, self.used = 0, 0
        return bytes(self.out)


class Cache:
    """The 128-entry colour ring (picCacheClear/New/Get): entries linked prev/next, newest."""

    def __init__(self):
        self.color = [0] * 128
        self.next = [(i + 1) % 128 for i in range(128)]
        self.prev = [(i - 1) % 128 for i in range(128)]
        self.newest = 0

    def new(self, c):
        self.newest = self.next[self.newest]
        self.color[self.newest] = c

    def get(self, i):
        n = self.newest
        if i != n:
            self.prev[self.next[i]] = self.prev[i]
            self.next[self.prev[i]] = self.next[i]
            self.prev[self.next[n]] = i
            self.next[i] = self.next[n]
            self.next[n] = i
            self.prev[i] = n
            self.newest = i
        return self.color[i]


def header(data):
    """(header bytes, depth, width, height, mode, transparent, body offset in bits)."""
    r = BitReader(data)
    r.bits(24)
    trans = NO_TRANSPARENT
    h = r.bits(8)
    if h != 0x1A:
        hexs = bytes([h, r.bits(8), r.bits(8), r.bits(8)])
        try:
            trans = int(hexs.decode("ascii"), 16)
        except ValueError:
            trans = int.from_bytes(hexs, "big")  # the game parses any four bytes as hex digits
        while r.bits(8) != 0x1A:
            pass
    while r.bits(8) != 0:
        pass
    r.bits(8)
    mode = r.bits(4)
    r.bits(4)
    depth = r.bits(16)
    w = r.bits(16)
    hgt = r.bits(16)
    return data[:r.pos], depth, w, hgt, mode, trans


def decode(data, log=None):
    """The image's pixels as the stream gives them (raster order; for 15/16-bit the stream's own
    colour values, for 4/8-bit indices), its palette (4/8-bit type 1), and how many bytes the
    stream used. log, a list, gets every decision: ("point", pos, ("cache", i) | ("new", c) |
    ("index", c)), ("chain", [codes])."""
    head, depth, w, h, mode, trans = header(data)
    r = BitReader(data)
    r.pos = len(head)
    pal = []
    if depth in (4, 8) and mode != 4:
        for _ in range(16 if depth == 4 else 256):
            pal.append(r.bits(16))
    cache = Cache()
    total = w * h
    px = [0] * total
    written = [False] * total

    def color():
        if depth >= 15:
            if r.bit():
                i = r.bits(7)
                if log is not None:
                    log.append(("color", ("cache", i)))
                return cache.get(i)
            c = r.bits(depth)
            cache.new(c)
            if log is not None:
                log.append(("color", ("new", c)))
            return c
        c = r.bits(depth)
        if log is not None:
            log.append(("color", ("index", c)))
        return c

    def put(x, y, c):
        if 0 <= x < w and 0 <= y < h:
            px[y * w + x] = c
            written[y * w + x] = True

    def length():
        n = 1
        while r.bit():
            n += 1
        return r.bits(n) + (1 << n) - 1

    def chain(x, y, c):
        codes = []
        while True:
            code = r.bits(2)
            if code == 1:
                x -= 1
            elif code == 3:
                x += 1
            elif code == 0:
                if not r.bit():
                    break
                code = 4 + r.bit()
                x += 2 if code == 5 else -2
            codes.append(code)
            if x >= w:
                break
            y += 1
            if y < h:
                put(x, y, c)
        if log is not None:
            log.append(("chain", codes))

    if depth == 15:
        # picDecodeDirect: each run continues the colour, or the colour a chain painted ahead
        x, y, c = -1, 0, 0
        while True:
            run = length() - 1
            ended = False
            for _ in range(run):
                x += 1
                if x == w:
                    x, y = 0, y + 1
                    if y == h:
                        ended = True
                        break
                if written[y * w + x]:
                    c = px[y * w + x]
                px[y * w + x] = c
            if ended:
                break
            x += 1
            if x == w:
                x, y = 0, y + 1
                if y == h:
                    break
            if log is not None:
                log.append(("point", y * w + x))
            c = color()
            put(x, y, c)
            if r.bit():
                chain(x, y, c)
        return {"head": head, "depth": depth, "w": w, "h": h, "mode": mode, "trans": trans,
                "pal": pal, "px": px, "used": r.pos}
    put(0, 0, 0)
    pos = -1
    while pos < total:
        n = 1
        while r.bit():
            n += 1
        ln = r.bits(n) + (1 << n)
        pos += ln - 1
        if pos >= total:
            break
        if log is not None:
            log.append(("point", pos))
        x, y = pos % w, pos // w
        c = color()
        put(x, y, c)
        if r.bit():
            codes = []
            while True:
                code = r.bits(2)
                if code == 1:
                    x -= 1
                elif code == 3:
                    x += 1
                elif code == 0:
                    if not r.bit():
                        break
                    code = 4 + r.bit()  # 4: two left, 5: two right
                    x += 2 if code == 5 else -2
                y += 1
                codes.append(code)
                put(x, y, c)
            if log is not None:
                log.append(("chain", codes))
    last = 0
    for i in range(total):
        if written[i]:
            last = px[i]
        px[i] = last
    return {"head": head, "depth": depth, "w": w, "h": h, "mode": mode, "trans": trans,
            "pal": pal, "px": px, "used": r.pos}


# The game's encoder, read off its own streams (stats): a chain steps only onto a "change point"
# (a pixel that differs from the one before it in raster order, x = 0 too), unpainted, of the point's
# colour: straight or one left first (both cannot qualify: of two same-colour neighbours only
# the left one is a change point), then one right, then two right; two left NEVER (0 of 400,000
# steps), though the decoder reads it.
CHAIN_ORDER = (2, 1, 3, 5)
STEP = {1: -1, 2: 0, 3: 1, 4: -2, 5: 2}


def encode_direct(img, out, log=None):
    """15-bit (picDecodeDirect): runs that continue the current colour, or switch to a colour a
    chain painted ahead, each ended by a point with a new colour; the same chain rules."""
    w, h, px = img["w"], img["h"], img["px"]
    total = w * h
    change = [i == 0 or px[i] != px[i - 1] for i in range(total)]
    chained = [False] * total
    cache, filled = Cache(), set()

    def length(v):  # picReadLength: n one-bits and a zero (n from 1), then n bits; 2^n - 1 + bits
        n = (v + 1).bit_length() - 1
        out.bits((1 << (n - 1)) - 1, n - 1)
        out.bits(0, 1)
        out.bits(v + 1 - (1 << n), n)

    last, cur = -1, 0
    for pos in range(total):
        if chained[pos]:
            cur = px[pos]
            continue
        if px[pos] == cur:
            continue
        length(pos - last)  # the run (pos - last - 1 pixels) plus one
        last = pos
        c = cur = px[pos]
        if log is not None:
            log.append(("point", pos))
        # this form's encoder counts the cache's starting zeros as entries (0x8001: black from
        # entry 0), searching newest first through the ring (the 16/8/4-bit one does not)
        hit, i = [], cache.newest
        for _ in range(128):
            if cache.color[i] == c:
                hit = [i]
                break
            i = cache.prev[i]
        if hit:
            out.bits(1, 1)
            out.bits(hit[0], 7)
            cache.get(hit[0])
            if log is not None:
                log.append(("color", ("cache", hit[0])))
        else:
            out.bits(0, 1)
            out.bits(c, 15)
            cache.new(c)
            filled.add(cache.newest)
            if log is not None:
                log.append(("color", ("new", c)))
        x, y = pos % w, pos // w
        codes = []
        while y + 1 < h:
            for code in CHAIN_ORDER:
                nx = x + STEP[code]
                q = (y + 1) * w + nx
                if 0 <= nx < w and change[q] and not chained[q] and px[q] == c:
                    break
            else:
                break
            codes.append(code)
            x, y = nx, y + 1
            chained[y * w + x] = True
        if codes:
            if log is not None:
                log.append(("chain", codes))
            out.bits(1, 1)
            for code in codes:
                if code <= 3:
                    out.bits(code, 2)
                else:
                    out.bits(0, 2)
                    out.bits(1, 1)
                    out.bits(code - 4, 1)
            out.bits(0, 3)
        else:
            out.bits(0, 1)
    length(total - last)  # a run past the last pixel ends the image
    return out.flush()


def encode(img, log=None):
    """The bytes for img (decode's dict), by the candidate encoder: a point wherever the pixel
    differs from the one before it in raster order and no chain has painted it; each point's
    colour from the cache when there; a chain from each point through the change points below
    it, in CHAIN_ORDER. log gets the same decisions decode logs."""
    depth, w, h, px = img["depth"], img["w"], img["h"], img["px"]
    out = BitWriter()
    for b in img["head"]:
        out.bits(b, 8)
    for v in img["pal"]:
        out.bits(v, 16)
    if depth == 15:
        return encode_direct(img, out, log)
    total = w * h
    change = [i == 0 or px[i] != px[i - 1] for i in range(total)]  # raster order: x = 0 too
    painted = [False] * total
    cache, filled = Cache(), set()
    last = -1
    fill = 0
    for pos in range(total):
        if painted[pos]:
            fill = px[pos]
            continue
        if px[pos] == fill:  # pixel 0 is written 0 first (put(0, 0, 0)): a point only if not 0
            continue
        d = pos - last + 1  # the stream's length: pos advances by length - 1
        n = d.bit_length() - 1
        out.bits((1 << (n - 1)) - 1, n - 1)
        out.bits(0, 1)
        out.bits(d - (1 << n), n)
        last = pos
        c = px[pos]
        if log is not None:
            log.append(("point", pos))
        if depth >= 15:
            # the encoder's cache starts empty: the decoder's zeroed entries are never a hit
            # (0x8024, 0x80A8: colour 0 written new while entries still held the initial 0)
            hit = [i for i in range(128) if cache.color[i] == c and i in filled]
            if hit:
                i = hit[0]
                out.bits(1, 1)
                out.bits(i, 7)
                cache.get(i)
                if log is not None:
                    log.append(("color", ("cache", i)))
            else:
                out.bits(0, 1)
                out.bits(c, depth)
                cache.new(c)
                filled.add(cache.newest)
                if log is not None:
                    log.append(("color", ("new", c)))
        else:
            out.bits(c, depth)
            if log is not None:
                log.append(("color", ("index", c)))
        painted[pos] = True
        fill = c
        x, y = pos % w, pos // w
        codes = []
        while y + 1 < h:
            for code in CHAIN_ORDER:
                nx = x + STEP[code]
                q = (y + 1) * w + nx
                if 0 <= nx < w and change[q] and not painted[q] and px[q] == c:
                    break
            else:
                break
            codes.append(code)
            x, y = nx, y + 1
            painted[y * w + x] = True
        if codes:
            if log is not None:
                log.append(("chain", codes))
            out.bits(1, 1)
            for code in codes:
                if code <= 3:
                    out.bits(code, 2)
                else:
                    out.bits(0, 2)
                    out.bits(1, 1)
                    out.bits(code - 4, 1)
            out.bits(0, 3)
        else:
            out.bits(0, 1)
    # the end: a length past the last pixel
    d = total - last + 1
    n = d.bit_length() - 1
    out.bits((1 << (n - 1)) - 1, n - 1)
    out.bits(0, 1)
    out.bits(d - (1 << n), n)
    return out.flush()


def pics():
    return [(sub, data) for sub, data in subfiles() if data[:3] == b"PIC"]


FILE_ATTACH = 0x8003514C  # gFileAttachTable (file.c): per file, (sub-file, address) pairs
GEOOBJ_FILES = range(398, 412)  # the GeoObj tables (StageData.objFile): u32 dl; u8 kind, unk5; u16 files[11]
FILE_SETS = 11  # archive file 11: FileSet[326], s16 file[4]; floadGetSetRam puts file[j] in segs[j]
# A frame's segs[i] is segment i + 3 (polSetSegments, polygon.c): a file at slot j of a FileSet
# row, or at slot k of a GeoObj row, is read by its row's models at segment 3 + j (3 + k).

# Frame animations (fade.c fadeAnimStart/fadeAnimStep): the code's AnimFrame table ({u32 addr;
# s16 duration;}, addr 0 ends it) names frames in a file; each in turn goes to segs[3], segment 6,
# of a model whose display list loads segment 6. Each row: the code's call, the table's code
# segment and VRAM (read from the ROM), the frames' file, the model's file, the model display
# list's offset (None: every segment-6 load in the file, which must agree). Read off the matched
# C, the tables and loads confirmed in IDA (ida_dump_funcs data:/function dumps), 2026-10-16.
ANIM_LINKS = (
    ("itex.c itexSparkle: gItexSparkleAnim, file 0x123, dl D_3000490; itex() puts the frames in"
     " segs[3] of a model with set 0x123's segments", 6, 0x8010C290, 769, 769, None),
    ("itex.c itexPuff*: gItexPuffAnim, file 0x123, gItexTemplate's dl 0x030003A0", 6, 0x8010C2F8, 769, 769, None),
    ("itex.c itexGlowBlue: gItexGlowAnim, file 0x121 (set 0x121's 767)", 6, 0x8010C360, 767, 769, None),
    ("itex.c itexFlash: gItexFlashAnim, file 0x121", 6, 0x8010C3E8, 767, 769, None),
    ("itex.c itexSparkleYellow/Blue: gItexBombAnim, file 0x11F (765)", 6, 0x8010C470, 765, 769, None),
    ("itex.c itexTimedYellow: gItexTimedYellowAnim, file 0x125 (771)", 6, 0x8010C4F8, 771, 769, None),
    ("tlb_0032_1.c clr5DemoEfDoucyuProc: D_08008D74, demoPolSetDL(D_3000580, set 0xCC (669))",
     32, 0x08008D74, 669, 669, 0x580),
    ("tlb_0032_1.c the flame: D_08008D8C, demoPolSetDL(D_3000770, set 0xCC)", 32, 0x08008D8C, 669, 669, 0x770),
    ("diveef.c plMoya: gDiveRippleTexAnim, file 0x306, dl2 D_30016C0, set 0x128 (774)",
     9, 0x801A72BC, 774, 774, 0x16C0),
    ("tlb_0327_1.c Pentani: D_08016F58, bossSetPolDL(D_30151B0, set 0xF0 (708, 701))",
     327, 0x08016F58, 708, 708, 0x151B0),
    ("tlb_0164.c Etsk_hipeye_main: hipeyeTexAnim, eneNewAnime(D_3000824), kind 0x21E set 0x1E (484)",
     164, 0x080016A0, 484, 484, None),
    ("tlb_0023_1.c Omitchan: D_08008DBC, demoAnimeSet(..., set 0xC7 (664))", 23, 0x08008DBC, 664, 664, None),
    ("tlb_0109.c shopChaBaba_proc: shopAnimChaBaba, eneNewAnime(D_3000244), kind 0x19F set 0x98"
     " (614, 578), frames in eneGetFile(o, 1)", 109, 0x08006A10, 578, 614, None),
)

# Textures the code loads itself (gDPLoadTextureBlock and its kin, built in the code): (file, its
# address, G_IM_FMT, G_IM_SIZ, width, height, the code). Each read off the matched C; the address,
# the render tile's format and line and the load's size confirmed in IDA, 2026-10-16. A CI
# texture's TLUT is the one the model's display list loads after it (found by the tracer).
CODE_TEXTURES = (
    (571, 0x03000CD0, 2, 0, 32, 32, "tlb_0071.c bloodwWaterGfx, namiWaterGfx (kinds 0x9F-0xA0, set 0x6F (571, 538))"),
    (572, 0x03002850, 2, 0, 32, 32, "tlb_0072.c kamaHotwaterDL (kinds 0xA1-0xA5, set 0x70 (572, 540))"),
    (625, 0x03000150, 2, 0, 64, 32, "tlb_0109.c shopAwaDraw (shopAwa_proc, kind 0x1B7, set 0xA3 (625))"),
    (632, 0x03000270, 2, 0, 64, 32, "tlb_0139.c kawaKawaDraw (kawaKawaProc, kind 0x1BD, set 0xAA (632))"),
    (633, 0x03000450, 2, 0, 64, 32, "tlb_0139.c kawaNamiDraw (kawaNamiProc, kind 0x1BE, set 0xAB (633))"),
    (634, 0x030000D0, 2, 0, 64, 32, "tlb_0139.c kawaKaikiDraw (kawaKaikiProc, kind 0x1BF, set 0xAC (634))"),
    (680, 0x04000000, 2, 0, 64, 32, "tlb_0139.c kawaKirikawa1Draw (kind 0x1D3, set 0xE2 (692, 680): segment 4)"),
    (680, 0x04000420, 2, 0, 64, 32, "tlb_0139.c kawaDerokawa1Draw (kind 0x1D7, set 0xE5 (695, 680))"),
    (680, 0x04000840, 4, 1, 32, 64, "tlb_0139.c kawaDerokawa2Draw (kind 0x1D8, set 0xE5 (695, 680))"),
    (538, 0x04000000, 4, 1, 32, 32, "tlb_0068.c waterDrawTexture (waterRideProc, kinds 0x93-0x95, set 0x6A (566, 538))"),
    (538, 0x04000800, 4, 1, 32, 64, "tlb_0071.c bloodwWaveGfx (kinds 0x9F-0xA0, set 0x6F (571, 538))"),
    (494, 0x03002C50, 4, 1, 32, 32, "tlb_0039.c func_08002F64 for \"water\" func_080031C0 (kind 0x19, set 0x26 (494))"),
    (654, 0x03021310, 4, 0, 64, 32, "tlb_0019_1.c gdemoWaterProc (demoPolSetDL, set 0xBD (654))"),
    (684, 0x03009D60, 4, 1, 64, 32, "tlb_0124.c sekisyo4IzumiDl (sekisyo4IzumiProc, kind 0x157, set 0xDA (684))"),
    (710, 0x030014B0, 4, 0, 16, 16, "bpause.c bpauseDrawIconI4 (set 0xF2 (710))"),
    (715, 0x03024970, 4, 0, 32, 64, "tlb_0326_1.c func_080121DC for \"Taki\" (bossSetPolDL, set 0xF5 (715))"),
    (723, 0x03024A90, 4, 0, 32, 32, "tlb_0325_6.c \"SKY\" func_08013448 (bossSetPolDL, set 0xFB (723)): the near clouds"),
    (723, 0x03024C90, 4, 0, 32, 32, "tlb_0325_6.c \"SKY\": the far clouds"),
    (739, 0x0303FA80, 4, 0, 288, 224, "pakfiler.c bg_proc: pakFilerDrawI4Tiles 0x120 x 0xE0 (set 0x109 (739));"
     " tlb_0018_5.c titleTbackProc"),
    (740, 0x03001BD0, 4, 0, 32, 65, "tlb_0018_7.c pselePscurProc: five 32x13 4-bit sprites, the players' marks"
     " (D_3001BD0 + port * 208) and the cursor (D_3001F10), drawn by pseleDrawCursorSprite through"
     " titleDrawImage4b (G_IM_FMT_I: D_0800BE28 = 4) and as CI4 with the code's titleCursorPalettes"),
    (741, 0x03000B30, 4, 0, 256, 232, "tlb_0018_1.c diaryDrawTile4b: tiles of the 256-wide 4-bit image at"
     " D_3000B30 (diarySegLoad, set 0x10B (741)) in D_0800BC7C's format, 4 (G_IM_FMT_I); 232 rows fill it"),
    (742, 0x0301D870, 4, 0, 48, 131, "tlb_0018_8.c subtGoemonProc kind 0: Goemon's shadow, titleDrawImage4b (I),"
     " set 0x10C (742)"),
    (742, 0x03030190, 4, 0, 160, 128, "tlb_0018_8.c subtGoemonProc kind 1: the shadow"),
    (742, 0x03040B10, 4, 0, 144, 128, "tlb_0018_8.c subtGoemonProc kind 2: the shadow"),
    (742, 0x03051B50, 4, 0, 144, 128, "tlb_0018_8.c subtGoemonProc kind 3: the shadow"),
    (742, 0x0306C550, 4, 0, 256, 19, "tlb_0018_8.c subtNomakiProc kind 0: the area's name, titleDrawImage4b (I)"),
    (742, 0x0306CED0, 4, 0, 272, 20, "tlb_0018_8.c subtNomakiProc kind 1"),
    (742, 0x0306D970, 4, 0, 208, 16, "tlb_0018_8.c subtNomakiProc kind 2"),
    (742, 0x0306DFF0, 4, 0, 272, 36, "tlb_0018_8.c subtNomakiProc kind 3"),
    (742, 0x0306F310, 4, 0, 224, 20, "tlb_0018_8.c subtNomakiProc kind 4"),
    (744, 0x0300A2C0, 4, 0, 16, 11, "gauge.c gaugeDrawJoinPanel panel 1 (segment 3: set 0x10E's file (744))"),
    (744, 0x0300A320, 4, 0, 16, 16, "gauge.c gaugeDrawJoinPanel panel 2"),
    (751, 0x03002780, 4, 1, 8, 256, "tlb_0016_0.c mapRoadProc (floadSegToRam(tex, 0x113, 0): 751): I8 8 x 256"),
    (751, 0x03002F80, 4, 1, 8, 256, "tlb_0016_0.c mapRoadProc, road kind 0"),
    (751, 0x03003780, 4, 1, 8, 256, "tlb_0016_0.c mapRoadProc, road kind 2"),
    (776, 0x0301FF30, 4, 1, 32, 64, "tlb_0017_1.c effShowerLoadTex (tlb_0017_2's \"shower\", set 0x13A (776))"),
)

# The staff roll (tlb_0180.c): staffPageLines, 12 pages of StaffLine {u32 tex; u16 file; s32 w, h;
# s32 unk[2]; s32 time;} (time 0 ends a page, tex 0 is a pause), drawn by the "dtex" task:
# staffPageDrawI4Tiles (I4) unless staffLineStep's rgba (staffPageProc: page 11 only) asks for
# staffDrawRgba16Tiles. (code file, table VRAM, pages, the first RGBA16 page)
STAFF_PAGES = (180, 0x08001FE4, 12, 11)
FMT_NAMES = {0: "RGBA", 1: "YUV", 2: "CI", 3: "IA", 4: "I"}

# The message window's faces (face.c, overlay 6): faceMakePic takes face k's picture at
# (gFacePicTable[k] & 0xFFFFFF) + fileRamAddr(gFaceFileTable[k]), and msgfrm.c frmFaceDrawAt
# loads it with gDPLoadTextureBlock RGBA 16b 32x32. Both tables and both functions confirmed in
# IDA (0006_800C7B10: data:, hilo: and the function dumps), 2026-10-17. (code segment, the file
# table's VRAM, the picture table's VRAM, faces, G_IM_FMT, G_IM_SIZ, width, height)
FACE_TABLE = (6, 0x8010AB30, 0x8010ABEC, 47, 0, 2, 32, 32)

# The raw sub-files nothing loads. Searched (2026-10-17) for each one's sub-file id, its parent
# file's and its own file's id, and its address at every segment: every value IDA resolves in all
# 316 code exports, every word of every code segment, every display list's G_SETTIMG in every
# archive file, the frame, file-set (11), kind (12) and stage tables; none loads these. So each
# format is read off the sub-file itself: a CI texture carries its TLUT right after its pixels
# (16 RGBA5551 colours, each with its alpha bit set), a texture with none is I (as the textures
# loaded beside it in the same file are); the width is the row stride at which its pixels repeat
# best (_row_stride, measured each run and required to equal it); the height every row that fits,
# zeros after (_layout). (sub-file, G_IM_FMT, G_IM_SIZ, width, height, the basis)
UNLOADED = (
    (0x847C, 2, 0, 32, 64, "file 715 (set 0xF5, the Taki stage) +1B250, just before 0x847D (CI4 32x32 and"
     " its TLUT, which 715's display lists load): 1024 bytes of 4-bit pixels (two equal 32x32 halves),"
     " then 16 RGBA5551 colours, every one with its alpha bit"),
    (0x84E9, 4, 0, 80, 143, "file 739 (set 0x109, the title) +514B0, among PICs nothing loads either: 4-bit,"
     " no TLUT, a silhouette, as the I4 pictures loaded from the same file (0x84E7) and the title card's"
     " I4 shadows (0x8515, 0x8517, 0x8519, 0x851B); 143 rows of 80, then 8 zero bytes"),
    (0x84EA, 4, 0, 64, 144, "file 739 +52B10: 4-bit, no TLUT, a silhouette (as 0x84E9)"),
    (0x84EB, 4, 0, 144, 140, "file 739 +53D10: 4-bit, no TLUT, a silhouette (as 0x84E9)"),
    (0x84EF, 4, 0, 64, 48, "file 739 +6B020, just before the title's button captions 0x84F0-0x84F1: 4-bit,"
     " no TLUT, lines of text glyphs"),
    (0x851D, 4, 0, 256, 108, "file 742 (set 0x10C, the title card) +68F50, just before the area names"
     " 0x851E-0x8522 that tlb_0018_8.c subtNomakiProc draws as I4: 4-bit, no TLUT, lines of Japanese text"),
    (0x855D, 4, 0, 16, 256, "file 738, segment 4 of the map's area sets 0x114-0x118 (tlb_0016_0.c"
     " mapAreas[].file): in each 8-byte row, pixels 0-7 are the I8 8x256 road texture 0x854D's (file 751,"
     " tlb_0016_0.c mapRoadProc) to 4 bits, row for row, and pixels 8-15 are 0 or 1"),
    (0x856D, 4, 0, 16, 256, "file 738 (as 0x855D, the same bytes): 0x854D's road texture to 4 bits"),
    (0x856E, 4, 0, 16, 256, "file 738 (as 0x855D): 0x854E's road texture to 4 bits"),
    (0x856F, 4, 0, 16, 256, "file 738 (as 0x855D): 0x854F's road texture to 4 bits"),
)


def _row_stride(data, siz, n, most=512, rows=16):
    """The row width, in pixels, at which a 4- or 8-bit texture's first n bytes repeat best: the
    stride s (2 to `most` pixels, at least `rows` rows) with the least mean |p[i] - p[i + s]| over
    its pixel values (a CI texture's indices). A stride of whole vertical repeats of the picture
    scores too (0x856F's dashes repeat every 32 rows of 16: 512 pixels, 8 rows), hence 16 rows."""
    import operator
    px = [v for b in data[:n] for v in (b >> 4, b & 15)] if siz == 0 else list(data[:n])
    best = None
    for s in range(2, min(most, len(px) // rows) + 1):
        v = sum(map(abs, map(operator.sub, px[s:], px[:-s]))) / (len(px) - s)
        if best is None or v < best[0]:
            best = (v, s)
    return best[1]


def _code_segment(seg, files, dec):
    """(bytes, VRAM) of a code segment: main, an overlay (archive 6, 8-10) or a TLB file."""
    if seg == "main":
        return dec[MAIN_ROM:], MAIN_VRAM
    return files[seg], {6: 0x800C7B10, 8: 0x801738A0, 9: 0x801738A0, 10: 0x801738A0}.get(seg, 0x08000000)


def _walk_loads(data, o, lo, size):
    """The texture loads a display list makes from the G_SETTIMG at o (to its G_ENDDL or G_DL,
    48 commands at most) of [lo, lo + size): [("tex", offset, fmt, siz, w, h) | ("tlut", offset,
    count)]. A texture is the render tile's G_SETTILE and G_SETTILESIZE over the TMEM a
    G_LOADBLOCK filled from the image, the block exactly the texture's bytes; a TLUT is a
    G_LOADTLUT, before or after its texture."""
    timg, tiles, loads, out = None, {}, [], []
    for k in range(o, min(len(data) - 7, o + 8 * 48), 8):
        w0, w1 = be32(data, k), be32(data, k + 4)
        op = w0 >> 24
        if op == 0xFD:
            timg = (w1, (4, 8, 16, 32)[(w0 >> 19) & 3])
        elif op == 0xF5:
            tiles[(w1 >> 24) & 7] = ((w0 >> 21) & 7, (w0 >> 19) & 3, w0 & 0x1FF)
        elif op == 0xF3 and timg is not None and (w1 >> 24) & 7 in tiles:
            loads.append((tiles[(w1 >> 24) & 7][2], timg[0], (((w1 >> 12) & 0xFFF) + 1) * timg[1] // 8))
        elif op == 0xF0 and timg is not None:
            if lo <= timg[0] < lo + size:
                out.append(("tlut", timg[0] - lo, ((w1 >> 14) & 0x3FF) + 1))
            timg = None
        elif op == 0xF2 and (w1 >> 24) & 7 != 7 and (w1 >> 24) & 7 in tiles:
            fmt, siz, tmem = tiles[(w1 >> 24) & 7]
            src = [(t, n) for m, t, n in loads if m == tmem]
            if src and lo <= src[-1][0] < lo + size:
                w = ((((w1 >> 12) & 0xFFF) - ((w0 >> 12) & 0xFFF)) >> 2) + 1
                h = (((w1 & 0xFFF) - (w0 & 0xFFF)) >> 2) + 1
                if src[-1][1] == w * h * (4, 8, 16, 32)[siz] // 8:
                    out.append(("tex", src[-1][0] - lo, fmt, siz, w, h))
        elif op in (0xDE, 0xDF) and k != o:
            break
    return out


def _settimgs(data):
    """{address: [offsets]} of every 8-aligned G_SETTIMG in a file (a display list's command)."""
    out = collections.defaultdict(list)
    for o in range(0, len(data) - 7, 8):
        if data[o] == 0xFD:
            out[be32(data, o + 4)].append(o)
    return out


def _region_bytes(r):
    return r[2] * 2 if r[0] == "tlut" else r[4] * r[5] * (4, 8, 16, 32)[r[3]] // 8


def _layout(regions, data):
    """(fmt, siz, w, h) when the regions fill the sub-file as n64_to_png writes it: one texture
    at 0, a CI texture's 16 or 256 TLUT entries right after it, then zeros to the end; else a
    reason."""
    texs = sorted({r for r in regions if r[0] == "tex"})
    tluts = sorted({r for r in regions if r[0] == "tlut"})
    if not texs:
        return "no texture load (TLUT only)" if tluts else "no load"
    if len(texs) > 1:
        return "the sources disagree: " + ", ".join(str(t[1:]) for t in texs)
    _, off, fmt, siz, w, h = texs[0]
    if off:
        return f"the texture at +{off:X}, not at 0"
    end = _region_bytes(texs[0])
    if fmt == 2:
        want = ("tlut", end, 16 if siz == 0 else 256)
        if tluts != [want]:
            return f"the CI texture's TLUT is not one load of {want[2]} right after it: {tluts}"
        end += _region_bytes(want)
    elif tluts:
        return f"a TLUT load in a non-CI texture: {tluts}"
    if end > len(data):
        return f"the texture runs {end - len(data)} bytes past the sub-file"
    if any(data[end:]):
        return f"{len(data) - end} bytes after the texture are not zero"
    return (fmt, siz, w, h)


def raw_texture_evidence():
    """{raw sub-file: ((fmt, siz, width, height) or the reason it has none, [sources])}, for
    every raw (non-PIC) PTX sub-file. The sub-file is attached (gFileAttachTable) to an address
    in a file; its format comes from:
      - the display lists of every file that maps that file (the file itself at its group's
        segment, the FileSet rows at 3 + slot, the GeoObj rows at 3 + slot) that load it;
      - ANIM_LINKS: a code table of frames shown through segment 6 of a model;
      - CODE_TEXTURES, STAFF_PAGES and FACE_TABLE: the code's own loads;
      - a twin: the same bytes as a sub-file those loads give a format;
      - UNLOADED: the sub-file's own bytes, for those nothing loads (its width measured).
    Every source must agree, and the layout must fill the sub-file (_layout)."""
    import struct
    import model_codec as mc
    with open(DECOMPRESSED, "rb") as f:
        dec = f.read()

    def main(vram):
        return MAIN_ROM + vram - MAIN_VRAM
    files = mc.archive()
    subs = {s: d for s, d in subfiles() if d[:3] != b"PIC"}
    attach = collections.defaultdict(list)  # sub-file -> [(file, address)]
    at_addr = {}  # (file, address) -> sub-file
    for fid in range(1, 0xA11):
        p = be32(dec, main(FILE_ATTACH) + 4 * fid)
        while p and be32(dec, main(p)):
            sub, dest = be32(dec, main(p)), be32(dec, main(p) + 4)
            attach[sub].append((fid, dest))
            at_addr[(fid, dest)] = sub
            p += 8
    maps = collections.defaultdict(set)  # file -> {(model file, segment)}
    fs = files[FILE_SETS]
    for o in range(0, len(fs) - 7, 8):
        row = struct.unpack_from(">4h", fs, o)
        for j, a in enumerate(row):
            if a > 0:
                maps[a].update((m, 3 + j) for m in row if m > 0)
    for gf in GEOOBJ_FILES:
        data = files.get(gf, b"")
        for o in range(0, len(data) - 0x1B, 0x1C):
            ids = struct.unpack_from(">11H", data, o + 6)
            if ids[0]:
                for k in range(11):
                    if ids[k]:
                        maps[ids[k]].add((ids[0], 3 + k))
    timgs = {}
    regions = collections.defaultdict(list)  # sub-file -> [(region, source)]

    def dl_loads(model, lo, size, dl=None):
        """Loads of [lo, lo + size) by model's display lists (from dl to its G_ENDDL, or all)."""
        data = files.get(model, b"")
        if model not in timgs:
            timgs[model] = _settimgs(data)
        out = []
        for addr, offs in timgs[model].items():
            if lo <= addr < lo + size:
                for o in offs:
                    if dl is None or (dl <= o and not any(data[k] == 0xDF for k in range(dl, o, 8))):
                        out += [(r, o) for r in _walk_loads(data, o, lo, size)]
        return out
    for sub, data in subs.items():
        for fid, dest in attach.get(sub, []):
            for model, seg in sorted({(fid, dest >> 24)} | maps.get(fid, set())):
                lo = (seg << 24) | (dest & 0xFFFFFF)
                for r, o in dl_loads(model, lo, len(data)):
                    regions[sub].append((r, f"display list: file {model} +{o:X} reads {lo:08X} (segment {seg})"))
    for what, seg, vram, frames, model, dl in ANIM_LINKS:
        code, base = _code_segment(seg, files, dec)
        k = vram - base
        while be32(code, k):
            sub = at_addr.get((frames, be32(code, k)))
            if sub in subs:
                for r, o in dl_loads(model, 0x06000000, len(subs[sub]), dl):
                    regions[sub].append((r, f"frame animation: {what}; file {model} +{o:X} loads segment 6"))
            k += 8
    for fid, addr, fmt, siz, w, h, what in CODE_TEXTURES:
        sub = at_addr.get((fid, addr))
        if sub in subs:
            regions[sub].append((("tex", 0, fmt, siz, w, h), f"code: {what}"))
    seg, vram, pages, rgba_from = STAFF_PAGES
    code, base = _code_segment(seg, files, dec)
    for page in range(pages):
        k = be32(code, vram - base + 4 * page) - base
        while True:
            tex, fid, w, h, t = be32(code, k), be32(code, k + 4) >> 16, be32(code, k + 8), be32(code, k + 12), \
                be32(code, k + 24)
            if t == 0:
                break
            sub = at_addr.get((fid, tex)) if tex else None
            if sub in subs:
                fmt, siz = (0, 2) if page >= rgba_from else (4, 0)
                regions[sub].append((("tex", 0, fmt, siz, w, h),
                                     f"code: tlb_0180.c staff roll page {page} (staffPageLines), file {fid}"))
            k += 28
    seg, files_at, pics_at, faces, fmt, siz, w, h = FACE_TABLE
    code, base = _code_segment(seg, files, dec)
    for k in range(faces):
        fid, pic = be32(code, files_at - base + 4 * k), be32(code, pics_at - base + 4 * k)
        sub = at_addr.get((fid, pic))
        if sub in subs:
            regions[sub].append((("tex", 0, fmt, siz, w, h),
                                 f"code: face.c faceMakePic, face {k} (gFaceFileTable: file {fid}, gFacePicTable:"
                                 f" {pic:08X}), drawn by msgfrm.c frmFaceDrawAt (gDPLoadTextureBlock RGBA16 32x32)"))
    # twins: a raw sub-file with no load, byte for byte one whose loads give its format
    loaded = {sub: _layout([r for r, _ in rs], subs[sub]) for sub, rs in regions.items()}
    for sub, data in sorted(subs.items()):
        if isinstance(loaded.get(sub), tuple):
            continue
        twin = next((o for o, v in sorted(loaded.items()) if isinstance(v, tuple) and subs[o] == data), None)
        if twin is not None:
            v = loaded[twin]
            for r in sorted({r for r, _ in regions[twin]}):
                regions[sub].append((r, f"twin: the same {len(data)} bytes as 0x{twin:04X}"
                                        f" ({FMT_NAMES[v[0]]}{(4, 8, 16, 32)[v[1]]} {v[2]}x{v[3]}:"
                                        f" {sorted({s for _, s in regions[twin]})[0]})"))
    strides = {}
    for sub, fmt, siz, w, h, basis in UNLOADED:
        if sub not in subs:
            continue
        n = w * h * (4, 8, 16, 32)[siz] // 8
        strides[sub] = (_row_stride(subs[sub], siz, n), w)
        regions[sub].append((("tex", 0, fmt, siz, w, h), f"no load: {basis}; its pixels repeat best at"
                                                         f" {strides[sub][0]} a row"))
        if fmt == 2:
            regions[sub].append((("tlut", n, 16 if siz == 0 else 256), f"no load: {basis}"))
    out = {}
    for sub, data in sorted(subs.items()):
        rs = regions.get(sub, [])
        v = _layout([r for r, _ in rs], data)
        if sub in strides and strides[sub][0] != strides[sub][1]:
            v = f"UNLOADED says {strides[sub][1]} a row, but its pixels repeat best at {strides[sub][0]}"
        out[sub] = (v, sorted({src for _, src in rs}))
    return out


def raw_textures():
    """{raw sub-file: (fmt, siz, width, height)} for every raw PTX sub-file whose format the
    evidence gives (raw_texture_evidence)."""
    return {sub: v for sub, (v, _) in raw_texture_evidence().items() if isinstance(v, tuple)}


def struct_unpack(fmt, data, o):
    import struct
    return struct.unpack_from(fmt, data, o)


def _c5(v):
    return (v << 3) | (v >> 2)  # 5 bits to 8, and back by >> 3: exact both ways


def to_png(data):
    """A PIC sub-file as PNG bytes: 15/16-bit as RGBA (as the game shows them: RGBA5551, the
    16-bit I bit as alpha), 4/8-bit as indexed (the palette as the game reads it; a mode-4 image
    has none: grey levels). The header bytes, which hold the comment and the transparent
    colour, and the sub-file's size go in a tEXt chunk, so to_pic gives the same bytes back."""
    import io
    import png
    img = decode(data)
    w, h, px, depth = img["w"], img["h"], img["px"], img["depth"]
    meta = {"PIC-header": img["head"].hex(), "PIC-size": str(len(data))}

    def rgba(f):  # RGBA5551
        return (_c5(f >> 11), _c5((f >> 6) & 31), _c5((f >> 1) & 31), 255 if f & 1 else 0)

    if depth == 15:
        rows = [[v for x in range(w) for v in rgba(((px[y * w + x] & 0x1F) << 11)
                | (((px[y * w + x] >> 10) & 0x1F) << 6) | (((px[y * w + x] >> 5) & 0x1F) << 1) | 1)]
                for y in range(h)]
        writer = png.Writer(w, h, greyscale=False, alpha=True)
    elif depth == 16:
        rows = [[v for x in range(w) for v in rgba(((px[y * w + x] & 0xFFC0) >> 5)
                | ((px[y * w + x] & 0x3E) << 10) | (px[y * w + x] & 1))] for y in range(h)]
        writer = png.Writer(w, h, greyscale=False, alpha=True)
    elif img["pal"]:
        pal = [rgba((((v >> 6) & 31) << 11) | (((v >> 11) & 31) << 6) | (((v >> 1) & 31) << 1) | (v & 1))
               for v in img["pal"]]
        rows = [[px[y * w + x] for x in range(w)] for y in range(h)]
        writer = png.Writer(w, h, palette=pal, bitdepth=depth)
    else:
        rows = [[px[y * w + x] for x in range(w)] for y in range(h)]
        writer = png.Writer(w, h, greyscale=True, bitdepth=depth)
    buf = io.BytesIO()
    writer.write(buf, rows)
    raw = bytearray(buf.getvalue())
    # tEXt chunks before IEND
    import struct
    import zlib
    chunks = b""
    for k, v in meta.items():
        body = k.encode() + b"\0" + v.encode()
        chunks += struct.pack(">I", len(body)) + b"tEXt" + body + struct.pack(">I", zlib.crc32(b"tEXt" + body))
    return bytes(raw[:-12]) + chunks + bytes(raw[-12:])


def to_pic(png_bytes):
    """The PIC sub-file a to_png PNG came from: pixels and palette back to the stream's values,
    encoded by the game's encoder, zero-padded to the sub-file's size."""
    import png
    r = png.Reader(bytes=png_bytes)
    w, h, rows, info = r.read()
    rows = [list(row) for row in rows]
    meta = {}
    for ctype, body in png.Reader(bytes=png_bytes).chunks():
        if ctype == b"tEXt":
            k, v = body.split(b"\0", 1)
            meta[k.decode()] = v.decode()
    head = bytes.fromhex(meta["PIC-header"])
    _, depth, _, _, mode, trans = header(head + bytes(16))

    def f5551(r8, g8, b8, a8):
        return ((r8 >> 3) << 11) | ((g8 >> 3) << 6) | ((b8 >> 3) << 1) | (1 if a8 else 0)

    pal = []
    if depth in (15, 16):
        px = []
        for row in rows:
            for x in range(w):
                f = f5551(*row[4 * x:4 * x + 4])
                if depth == 15:  # back to the stream's X68000 order
                    px.append(((f >> 6 & 31) << 10) | ((f >> 1 & 31) << 5) | (f >> 11))
                else:  # decode's ((c & 0xFFC0) >> 5) | ((c & 0x3E) << 10) | (c & 1), inverted
                    px.append(((f & 0x7FE) << 5) | ((f >> 11) << 1) | (f & 1))
    else:
        px = [v for row in rows for v in row[:w]]
        if "palette" in info:
            for (r8, g8, b8, *a) in info["palette"]:
                f = f5551(r8, g8, b8, a[0] if a else 255)
                pal.append((((f >> 6) & 31) << 11) | (((f >> 11) & 31) << 6) | (((f >> 1) & 31) << 1) | (f & 1))
    img = {"head": head, "depth": depth, "w": w, "h": h, "mode": mode, "trans": trans,
           "pal": pal, "px": px}
    out = encode(img)
    return out + bytes(int(meta["PIC-size"]) - len(out))


def rebuild(data):
    """A PIC sub-file from its decoded image: the encoded stream, then the sub-file's tail as
    the encoder left it (zero padding, measured: ptx)."""
    img = decode(data)
    out = encode(img)
    return out + bytes(len(data) - len(out)) if len(out) <= len(data) else out


def ptx_check():
    """Every PTX sub-file rebuilt (a PIC re-encoded from its pixels, anything else as is) and
    compared with the ROM: the stream plus zero padding to the next sub-file."""
    ok, bad, other, tails = 0, [], 0, collections.Counter()
    for sub, data in subfiles():
        if data[:3] != b"PIC":
            other += 1
            continue
        img = decode(data)
        enc = encode(img)
        tail = data[len(enc):]
        tails[(len(tail), any(tail))] += 1
        if rebuild(data) == data:
            ok += 1
        else:
            bad.append(f"0x{sub:04X}")
    print(f"ptx: {ok} PIC sub-files rebuilt byte-exact with zero padding, {len(bad)} not"
          f" ({' '.join(bad[:20])}); {other} sub-files are not PICs (raw textures)")
    print("tails (bytes after the stream, nonzero?): "
          + ", ".join(f"{n}{'!' if nz else ''}x{c}" for (n, nz), c in sorted(tails.items())))
    return 0 if not bad else 1


def chain_stats():
    """How the game's encoder chains, from its own streams (skips form): at each step of each
    chain (and at each point it did not chain from), which of the five offsets below hold the
    point's colour, unpainted so far, and which differ from their left neighbour; and what the
    game took. Printed as counts of (candidates, taken)."""
    from collections import Counter
    seen = Counter()
    for sub, data in pics():
        if header(data)[1] == 15:
            continue
        log = []
        img = decode(data, log)
        w, h, px = img["w"], img["h"], img["px"]
        painted = [False] * (w * h)
        painted[0] = True
        pos = None
        for e in log:
            if e[0] == "point":
                pos = e[1]
                painted[pos] = True
                c = px[pos]
                x, y = pos % w, pos // w
                took = []
                continue
            if e[0] == "color":
                continue
            # a chain: replay each step, noting the candidates the game saw
            for code in e[1]:
                cands = []
                for k in (1, 2, 3, 4, 5):
                    nx = x + STEP[k]
                    if 0 <= nx < w and y + 1 < h:
                        q = (y + 1) * w + nx
                        if px[q] == c and not painted[q]:
                            left = nx == 0 or px[q - 1] != c
                            cands.append(f"{k}{'' if left else '*'}")
                seen[(" ".join(cands), code)] += 1
                x += STEP[code]
                y += 1
                if 0 <= x < w and y < h:
                    painted[y * w + x] = True
    for (cands, code), n in seen.most_common(60):
        print(f"{n:6d}  candidates [{cands}] took {code}")
    return 0


def main():
    cmd = sys.argv[1:]
    if cmd == ["list"]:
        for sub, data in pics():
            head, depth, w, h, mode, trans = header(data)
            print(f"0x{sub:04X} {len(data):6d} bytes  {depth:2d}-bit {w}x{h} mode {mode}"
                  + ("" if trans == NO_TRANSPARENT else f" transparent 0x{trans:04X}"))
        return 0
    if cmd == ["stats"]:
        return chain_stats()
    if cmd == ["ptx"]:
        return ptx_check()
    if cmd and cmd[0] == "raws":  # raws [-v]: the evidence for each raw sub-file
        ev = raw_texture_evidence()
        subs = dict(subfiles())
        ok = {s: v for s, v in ev.items() if isinstance(v[0], tuple)}
        kinds = collections.Counter()
        for s, (v, srcs) in ok.items():
            kinds["+".join(sorted({x.split(":")[0] for x in srcs}))] += 1
        print(f"raws: {len(ok)} of {len(ev)} raw sub-files have a format from the evidence, each filling its"
              f" sub-file (by source: {', '.join(f'{k} {n}' for k, n in sorted(kinds.items()))});"
              f" {len(ev) - len(ok)} unresolved")
        for s, (v, srcs) in sorted(ev.items()):
            if not isinstance(v, tuple):
                print(f"  0x{s:04X} ({len(subs[s])} bytes) unresolved: {v}")
            elif cmd[1:] == ["-v"]:
                print(f"  0x{s:04X} ({len(subs[s])} bytes) {FMT_NAMES[v[0]]}{(4, 8, 16, 32)[v[1]]} {v[2]}x{v[3]}")
                for src in srcs:
                    print(f"      {src}")
        return 0
    if cmd == ["png"]:  # every PIC through PNG and back
        ok, bad = 0, []
        for sub, data in pics():
            if to_pic(to_png(data)) == data:
                ok += 1
            else:
                bad.append(f"0x{sub:04X}")
        print(f"png: {ok} PIC sub-files back from PNG byte-exact, {len(bad)} not {' '.join(bad[:20])}")
        return 0 if not bad else 1
    if cmd and cmd[0] == "check":
        want = int(cmd[1], 0) if len(cmd) > 1 else None
        ok = bad = 0
        first = None
        skipped, fails = 0, []
        for sub, data in pics():
            if want is not None and sub != want:
                continue
            img = decode(data)
            got = encode(img)
            if got == data[:len(got)] and len(got) >= img["used"] - 1:
                ok += 1
            else:
                bad += 1
                fails.append(f"0x{sub:04X}/{img['depth']}")
                if first is None:
                    k = next((i for i in range(min(len(got), len(data))) if got[i] != data[i]), min(len(got), len(data)))
                    first = f"0x{sub:04X}: first byte differs at 0x{k:X} of 0x{img['used']:X} used (encoded 0x{len(got):X})"
        print(f"check: {ok} PIC images byte-exact, {bad} not, {skipped} 15-bit not tried"
              + (f"; {first}" if first else ""))
        if fails:
            print("not: " + " ".join(fails[:60]))
        return 0 if bad == 0 else 1
    if len(cmd) == 2 and cmd[0] == "events":
        want = int(cmd[1], 0)
        data = dict(pics())[want]
        log, mine = [], []
        img = decode(data, log)
        encode(img, mine)
        k = next((i for i in range(min(len(log), len(mine))) if log[i] != mine[i]), None)
        print(f"0x{want:04X}: {img['depth']}-bit {img['w']}x{img['h']}; {len(log)} decisions in the game's"
              f" stream, {len(mine)} in ours; first difference at {k}")
        if k is not None:
            w = img["w"]
            for i in range(max(0, k - 6), min(len(log), k + 6)):
                e = log[i]
                xy = f" ({e[1] % w},{e[1] // w})" if e[0] == "point" else ""
                print(f"  {'>' if i == k else ' '} game: {e}{xy}   ours: {mine[i] if i < len(mine) else '-'}")
        return 0
    sys.exit(__doc__)


if __name__ == "__main__":
    sys.exit(main())
