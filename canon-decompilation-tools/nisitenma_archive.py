"""The "Nisitenma-Ichigo" ROM file system of Goemon's Great Adventure (USA).

    python canon-decompilation-tools/nisitenma_archive.py survey     measure the stream headers and padding in the baserom
    python canon-decompilation-tools/nisitenma_archive.py unpack     baserom.us.z64 -> baserom.us.decompressed.z64 + nisitenma.tsv
    python canon-decompilation-tools/nisitenma_archive.py pack <decompressed.z64> <out.z64>
                                         a decompressed ROM back to the exact ROM
    python canon-decompilation-tools/nisitenma_archive.py check      unpack, pack, and compare with the baserom

The decompressed ROM: ROM 0x0-0x582530 as-is (header, boot, main, audio), then every non-empty
file decompressed, each at a 16-byte-aligned offset, then the ROM tail. nisitenma.tsv records
where each file went. Packing rules (measured by `survey` over all 2,576 files):
    zlib file = be32(0x80000000 | (4 + len(stream))) + zlib level 9/memLevel 8 stream + 0x00 pad
                to an even length
    LZ file   = konami_lz_pack, the game's own greedy encoder, unpadded; only file 9 (`lz` checks
                it and the 179 music files: 180 of 180 byte-exact)
    raw file  = its bytes (all even-sized)

LoadFile(id, dst) (0x80003E50 in main) reads two tables in main's data:
    0x8002BB84  the 16-byte magic "Nisitenma-Ichigo", then from 0x8002BB94 one u32 ROM offset per
                file id 1..0xA10 (bit 31 = compressed; size = next offset - this one)
    0x80026B00  one (start, end) u32 pair per file: the file's own address range
A compressed file starts with a 4-byte big-endian header: bit 31 set = zlib (1.1.3 in the game;
zlib level 9 / memLevel 8 reproduces every stream exactly), clear = Konami LZ; the low bits are a
length.

Changelog: first public release (2026-10-20). Earlier history is kept privately.
"""

import collections
import glob
import os
import re
import subprocess
import sys
import zlib

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BASEROM = os.path.join(ROOT, "baserom.us.z64")
MAIN_ROM, MAIN_VRAM = 0x1000, 0x80000400
OFFSETS_VRAM, RANGES_VRAM, COUNT = 0x8002BB94, 0x80026B00, 0xA10


def be32(b, o):
    return int.from_bytes(b[o:o + 4], "big")


def entries(rom):
    """(id, rom_start, rom_end, compressed, ram_start, ram_end) for ids 1..COUNT."""
    off = MAIN_ROM + (OFFSETS_VRAM - MAIN_VRAM)
    rng = MAIN_ROM + (RANGES_VRAM - MAIN_VRAM)
    for i in range(COUNT):
        o0, o1 = be32(rom, off + 4 * i), be32(rom, off + 4 * (i + 1))
        yield (i + 1, o0 & 0x7FFFFFFF, o1 & 0x7FFFFFFF, o0 >> 31,
               be32(rom, rng + 8 * i), be32(rom, rng + 8 * i + 4))


def survey():
    rom = open(BASEROM, "rb").read()
    kinds, hdr_rule, pad, pad_bytes, raw_parity, starts = (collections.Counter() for _ in range(6))
    for fid, rs, re_, comp, r0, r1 in entries(rom):
        size = re_ - rs
        starts[rs & 1] += 1
        if size == 0:
            kinds["empty"] += 1
            continue
        if not comp:
            kinds["raw"] += 1
            raw_parity[size & 1] += 1
            continue
        blob = rom[rs:re_]
        h = be32(blob, 0)
        length = h & 0x7FFFFFFF
        if h >> 31:
            kinds["zlib"] += 1
            d = zlib.decompressobj()
            d.decompress(blob[4:])
            stream = len(blob) - 4 - len(d.unused_data)
            hdr_rule["length == 4 + zlib stream" if length == 4 + stream else
                     f"length != 4 + stream ({length} vs {4 + stream})"] += 1
        else:
            kinds["lz"] += 1
            hdr_rule["LZ: length == file size" if length == size else f"LZ: length {length} vs size {size}"] += 1
        p = size - length
        pad[p] += 1
        for b in blob[length:]:
            pad_bytes[b] += 1
    print("kinds      ", dict(kinds))
    print("header     ", dict(hdr_rule))
    print("pad (size - header length)", dict(sorted(pad.items())))
    print("pad byte values", {f"0x{k:02X}": v for k, v in pad_bytes.items()})
    print("raw sizes by parity (0=even)", dict(raw_parity))
    print("file starts by parity (0=even)", dict(starts))


FILES_START, FILES_END = 0x582530, 0xFD7442
DECOMPRESSED = os.path.join(ROOT, "baserom.us.decompressed.z64")
LAYOUT = os.path.join(ROOT, "nisitenma.tsv")


def konami_lz(blob):
    """Konami LZ as the game's decoder (0x80002F40, top-nibble-0 path) implements it:
      00-7F  back-reference: offset ((c<<8)|next) & 0x3FF, length (c>>2)+2, byte-by-byte
      80-9F  (c & 0x1F) literal bytes        A0-BF  end of stream
      C0-DF  next byte repeated (c&0x1F)+2   E0-FE  (c&0x1F)+2 zeros     FF  next+2 zeros
    The 4-byte header is the stream length including itself."""
    h = be32(blob, 0)
    if h >> 28:
        raise ValueError("multi-block LZ (top nibble != 0) is not implemented")
    pos, out = 4, bytearray()
    while pos < h:
        c = blob[pos]
        pos += 1
        if c < 0x80:
            off = ((c << 8) | blob[pos]) & 0x3FF
            pos += 1
            src = len(out) - off
            if off == 0 or src < 0:
                raise ValueError(f"bad back-reference {off} at stream offset {pos}")
            for k in range((c >> 2) + 2):
                out.append(out[src + k])
        elif c < 0xA0:
            out += blob[pos:pos + (c & 0x1F)]
            pos += c & 0x1F
        elif c < 0xC0:
            break
        elif c < 0xE0:
            out += bytes([blob[pos]]) * ((c & 0x1F) + 2)
            pos += 1
        elif c < 0xFF:
            out += bytes((c & 0x1F) + 2)
        else:
            out += bytes(blob[pos] + 2)
            pos += 1
    return bytes(out)


def konami_lz_pack(data, pad=16):
    """Konami LZ as the game's own tool made it (found by the asset survey on
    the 179 music files, permuter/assets/s12): greedy, at each position the longest match of 4
    to 33 bytes within 991 back (1024 - 33), the nearest of equal length; a zero run of 2+ beats
    a match it is not shorter than, a run of another byte of 3+ likewise (at most 33); else a
    literal (runs of 31). One plain block: a u32 header (its length, itself included), the
    tokens, then zeros to pad bytes: 16 in the music region, none in the archive (file 9: the
    same 37,532 tokens as the game's, and its next file starts right after them)."""
    import bisect
    idx = {}
    for q in range(len(data) - 1):
        idx.setdefault(data[q:q + 2], []).append(q)

    def longest(pos, maxd=991, cap=33):
        if pos + 1 >= len(data):
            return 0, 0
        lst = idx.get(data[pos:pos + 2], [])
        j = bisect.bisect_left(lst, pos) - 1
        best = bd = 0
        while j >= 0 and lst[j] >= pos - maxd:
            q = lst[j]
            dd, m = pos - q, 2
            while m < cap and pos + m < len(data) and data[pos + m] == data[pos + m - dd]:
                m += 1
            if m > best:
                best, bd = m, dd
                if m == cap:
                    break
            j -= 1
        return best, bd

    toks, lit, pos, n = bytearray(), bytearray(), 0, len(data)

    def flush():
        nonlocal lit
        while lit:
            toks.append(0x80 | len(lit[:31]))
            toks.extend(lit[:31])
            lit = lit[31:]
    while pos < n:
        ln, dist = longest(pos)
        if ln < 4:
            ln = 0
        b, run = data[pos], 0
        while pos + run < n and data[pos + run] == b and run < 257:
            run += 1
        if b == 0 and run >= 2 and run >= ln:
            flush()
            toks.extend([0xE0 + run - 2] if run <= 32 else [0xFF, run - 2])
            pos += run
        elif b != 0 and run >= 3 and run >= ln:
            run = min(run, 33)
            flush()
            toks.extend([0xC0 + run - 2, b])
            pos += run
        elif ln:
            flush()
            toks.extend([((ln - 2) << 2) | (dist >> 8), dist & 0xFF])
            pos += ln
        else:
            lit.append(b)
            pos += 1
            if len(lit) == 31:
                flush()
    flush()
    body = (len(toks) + 4).to_bytes(4, "big") + bytes(toks)
    return body + bytes(-len(body) % pad)


MUSIC_ROM_START, MUSIC_ROM_END, MUSIC_COUNT = 0x80037C9C, 0x80037F68, 179  # audio.c gSndMusicRom*


def lz_check():
    """konami_lz_pack against every Konami-LZ stream in the ROM: the archive's (file 9) and
    the 179 music files (audio.c's gSndMusicRomStart/End), each decoded and encoded back."""
    with open(BASEROM, "rb") as f:
        rom = f.read()
    blobs = []
    for i in range(MUSIC_COUNT):
        s = be32(rom, MAIN_ROM + MUSIC_ROM_START - MAIN_VRAM + 4 * i)
        e = be32(rom, MAIN_ROM + MUSIC_ROM_END - MAIN_VRAM + 4 * i)
        blobs.append((f"music 0x{i + 0x10:X}", rom[s:e]))
    for fid, start, end, comp, _, _ in entries(rom):
        if comp and not be32(rom, start) >> 31:  # bit 31 clear: Konami LZ
            blobs.append((f"file {fid}", rom[start:end]))
    bad = [name for name, blob in blobs
           if konami_lz_pack(konami_lz(blob), 1 if name.startswith("file") else 16) != blob]
    print(f"lz: {len(blobs) - len(bad)} of {len(blobs)} Konami-LZ streams re-encoded byte-exact"
          + (f"; not: {' '.join(bad[:20])}" if bad else ""))
    for name, blob in blobs:
        if name in bad[:3]:
            ours, theirs = lz_tokens(konami_lz_pack(konami_lz(blob))), lz_tokens(blob)
            k = next((i for i in range(min(len(ours), len(theirs))) if ours[i] != theirs[i]), None)
            if k is not None:
                print(f"  {name}: token {k} of {len(theirs)}: game {theirs[k - 2:k + 4]}")
                print(f"  {' ' * len(name)}  {' ' * len(str(k))}    ours {ours[k - 2:k + 4]}")
            else:
                mine = konami_lz_pack(konami_lz(blob))
                print(f"  {name}: the same {len(theirs)} tokens; {len(mine)} bytes against {len(blob)},"
                      f" header {be32(mine, 0):X}/{be32(blob, 0):X}, the game's tail {blob[be32(blob, 0):].hex()}")
    return 0 if not bad else 1


def lz_tokens(blob):
    """(output position, token) per token of a plain Konami-LZ block."""
    h, p, o, res = be32(blob, 0), 4, 0, []
    while p < h:
        c = blob[p]
        if c < 0x80:
            ln = (c >> 2) + 2
            res.append((o, f"ref{ln}/d{((c << 8) | blob[p + 1]) & 0x3FF}"))
            p, o = p + 2, o + ln
        elif c < 0xA0:
            res.append((o, f"lit{c & 0x1F}"))
            p, o = p + 1 + (c & 0x1F), o + (c & 0x1F)
        elif c < 0xE0:
            res.append((o, f"run{(c & 0x1F) + 2}"))
            p, o = p + 2, o + (c & 0x1F) + 2
        elif c < 0xFF:
            res.append((o, f"zero{(c & 0x1F) + 2}"))
            p, o = p + 1, o + (c & 0x1F) + 2
        else:
            res.append((o, f"zerol{blob[p + 1] + 2}"))
            p, o = p + 2, o + blob[p + 1] + 2
    return res


def zlib_pack(data):
    c = zlib.compressobj(9, zlib.DEFLATED, 15, 8, zlib.Z_DEFAULT_STRATEGY)
    stream = c.compress(data) + c.flush()
    out = (0x80000000 | (4 + len(stream))).to_bytes(4, "big") + stream
    return out + b"\0" * (len(out) & 1)


def unpack():
    rom = open(BASEROM, "rb").read()
    work = bytearray(rom[:FILES_START])
    rows = []
    for fid, rs, re_, comp, r0, r1 in entries(rom):
        if re_ == rs:
            rows.append((fid, "empty", 0, 0, rs, 0))
            continue
        blob = rom[rs:re_]
        if not comp:
            kind, data = "raw", blob
        elif be32(blob, 0) >> 31:
            d = zlib.decompressobj()
            kind, data = "zlib", d.decompress(blob[4:])
        else:
            kind, data = "lz", konami_lz(blob)
        work += b"\0" * (-len(work) % 16)
        rows.append((fid, kind, len(work), len(data), rs, re_ - rs))
        work += data
    work += b"\0" * (-len(work) % 16)
    rows.append(("tail", "raw", len(work), len(rom) - FILES_END, FILES_END, len(rom) - FILES_END))
    work += rom[FILES_END:]
    with open(DECOMPRESSED, "wb") as f:
        f.write(work)
    with open(LAYOUT, "w", newline="\n") as f:
        f.write("id\tkind\twork_offset\twork_size\trom_offset\trom_size\n")
        for fid, kind, wo, ws, ro, rsz in rows:
            f.write(f"{fid}\t{kind}\t0x{wo:X}\t0x{ws:X}\t0x{ro:X}\t0x{rsz:X}\n")
    print(f"unpack: {DECOMPRESSED} ({len(work)} bytes), {LAYOUT} ({len(rows)} rows)")


def pack(src, dst):
    work = open(src, "rb").read()
    base = None
    out = bytearray(work[:FILES_START])
    with open(LAYOUT) as f:
        rows = [line.rstrip("\n").split("\t") for line in f][1:]
    for fid, kind, wo, ws, ro, rsz in rows:
        wo, ws, ro, rsz = (int(x, 16) for x in (wo, ws, ro, rsz))
        if len(out) != ro:
            sys.exit(f"pack: file {fid} lands at 0x{len(out):X}, the table says 0x{ro:X}")
        data = work[wo:wo + ws]
        if kind in ("raw", "empty"):
            out += data
        elif kind == "zlib":
            out += zlib_pack(data)
        elif kind == "lz":  # the game's own Konami-LZ encoder, no padding in the archive
            out += konami_lz_pack(data, 1)
        if len(out) != ro + rsz:
            sys.exit(f"pack: file {fid} packs to 0x{len(out) - ro:X} bytes, the ROM has 0x{rsz:X}")
    with open(dst, "wb") as f:
        f.write(out)
    return bytes(out)


def check():
    import hashlib
    unpack()
    tmp = os.path.join(ROOT, "build", "nisitenma_check.z64")
    os.makedirs(os.path.dirname(tmp), exist_ok=True)
    out = pack(DECOMPRESSED, tmp)
    base = open(BASEROM, "rb").read()
    same = out == base
    print(f"check: packed {len(out)} bytes, sha1 {hashlib.sha1(out).hexdigest()} -> "
          + ("MATCH" if same else "DIFFERS"))
    return 0 if same else 1


SHARED_SLOT = (0x801738A0, 0x801738A0 + 0x47000)  # overlays 8, 9 and 10 (gOvlSharedSlot)

# TLB code files kept as splat asm (text + data) instead of a C file with migrated .rodata,
# because the C file's build differed (yaml-diffs). The file's decompiler fixes the cause first.
TLB_ASM = {
    # emptied 2026-10-04: every reason below was rodata splat had left
    # outside its one reader, as in file 135; force-migrate pins it. A file goes back here only
    # with a reason measured after that. The old reasons:
    #   102: func_08000B08's double D_08001468 not migrated (unpaired), among the strings
    #   23, 326, 327: asm-processor, .rodata alignment mod 8
    #   24, 25, 29, 37, 39, 99, 103, 131, 323: build differs (the migrated .rodata's order)
}


def pad_cuts(first, last, vram=0x08000000):
    """The object boundaries a .rodata cut cannot see, from IDA's functions (the exports): a
    function's end, then nothing but zero words, up to a function start that is 16-aligned. IDO
    puts no padding between the functions of one object, so each is a new object, or 16 bytes
    of it a pad row. Prints those that are not a c or pad row's start in the yaml."""
    found = extra = 0
    for fid, e0, s1, aligned, has_cut in _pad_points(first, last, vram):
        if e0 is None:
            found += 1
            print(f"file {fid}: {s1 - vram} bytes of zeros at its start, its first function at 0x{s1:08X}: "
                  f"a pad row there (splat makes a 4-byte function of the first nop)")
        elif not has_cut:
            found += 1
            print(f"file {fid}: function ends 0x{e0:08X}, {s1 - e0} bytes of nops, next starts "
                  f"0x{s1:08X}: no object cut there in the yaml")
        else:
            extra += 1
            print(f"file {fid}: function ends 0x{e0:08X}, nops to the cut at 0x{s1:08X}: "
                  f"{s1 - aligned} bytes past 16-alignment (0x{aligned:08X}): a pad row or an object there")
    print(f"pad-cuts {first}-{last}: {found} boundaries the yaml does not cut; {extra} cuts with more "
          f"padding before them than 16-alignment gives")
    return 0


def _pad_points(first, last, vram=0x08000000):
    """pad_cuts' findings as (file, function end, next start, end 16-aligned, the yaml cuts at
    the next start): those with no cut, and those with more padding than 16-alignment gives."""
    import json
    out = []
    rows = {}
    with open(os.path.join(ROOT, "nisitenma.tsv"), encoding="utf-8") as f:
        for line in f.read().splitlines()[1:]:
            p = line.split("\t")
            if p[0].isdigit():
                rows[int(p[0])] = (int(p[2], 16), int(p[3], 16))
    cuts, text_end, seg = {}, {}, None
    with open(os.path.join(ROOT, "goemonsgreatadv.yaml"), encoding="utf-8") as f:
        for line in f:
            m = re.match(r"^  - name:\s*tlb_(\d{4})\s*$", line)
            if m:
                seg = int(m.group(1))
                continue
            if re.match(r"^  - ", line):
                seg = None
            m = (re.match(r"^\s+- \[0x([0-9A-Fa-f]+),\s*(c|pad|asm|data|\.data|\.rodata|rodata)\b", line)
                 or re.match(r"^\s+- \{\s*start:\s*0x([0-9A-Fa-f]+),\s*type:\s*(c|pad|asm|data|\.data|\.rodata|rodata)\b", line))
            if m and seg is not None:
                a = int(m.group(1), 16) - rows[seg][0] + vram
                if m.group(2) in ("c", "pad", "asm"):
                    cuts.setdefault(seg, set()).add(a)
                else:  # the file's .text ends where its first data row starts
                    text_end[seg] = min(text_end.get(seg, a), a)
    with open(DECOMPRESSED, "rb") as f:
        rom = f.read()
    for fid in range(first, last + 1):
        path = os.path.join(IDA_EXPORT, f"{fid:04d}.json")
        if fid not in rows or not os.path.exists(path):
            continue
        with open(path, encoding="utf-8") as f:
            exp = json.load(f)
        spans = []
        for fn in exp["functions"]:
            real = [ea for ea, _, m, _, _ in fn["ins"] if m != "nop"]
            if fn["start"] is not None and real:
                spans.append((min(real), fn["end"]))
            elif fn["start"] is None:  # code IDA left outside every function (file 137's 0x08000020)
                spans += [(ea, ea + 8) for ea, _, m, _, _ in fn["ins"] if m == "jr"]
        spans.sort()
        off = rows[fid][0]
        # zeros before the first function: a pad row at the file's start (e0 None marks it)
        if spans and spans[0][0] > vram and spans[0][0] % 16 == 0 \
                and not rom[off:off + spans[0][0] - vram].strip(b"\0") and spans[0][0] not in cuts.get(fid, ()):
            out.append((fid, None, spans[0][0], vram, False))
        for (s0, e0), (s1, e1) in zip(spans, spans[1:]):
            gap = rom[off + e0 - vram:off + s1 - vram]
            if not (s1 > e0 and s1 % 16 == 0 and not gap.strip(b"\0")) or s1 >= text_end.get(fid, 1 << 32):
                continue  # not padding, or IDA's "function" there is the file's .data
            aligned = (e0 + 15) // 16 * 16
            if s1 not in cuts.get(fid, ()):
                out.append((fid, e0, s1, aligned, False))
            elif aligned < s1 and aligned not in cuts.get(fid, ()):
                out.append((fid, e0, s1, aligned, True))
    return out


ROW = re.compile(r"^      - \[0x([0-9A-Fa-f]+),\s*([\w.]+)(?:,\s*([\w/]+))?\]\s*(#.*)?$")


def _func_addr(folder, name):
    """A function's first .text address, from its .s in asm/matchings/ or asm/nonmatchings/
    (folder: tlb/tlb_NNNN[_K]); None when there is none."""
    for kind in ("matchings", "nonmatchings"):
        p = os.path.join(ROOT, "asm", kind, folder, name + ".s")
        if os.path.exists(p):
            in_text = True
            with open(p, encoding="utf-8") as f:
                for line in f:
                    if line.startswith(".section"):
                        in_text = ".text" in line
                        continue
                    m = re.match(r"^\s*/\* [0-9A-F]+ ([0-9A-F]{8}) [0-9A-F]{8} \*/\s+[a-z]", line) if in_text else None
                    if m:
                        return int(m.group(1), 16)
    return None


def _c_cut(text, folder, addr):
    """The offset in a C file's text where the functions at addr and after begin: the first
    function (a definition, a GLOBAL_ASM pragma, or the #ifdef NON_MATCHING around one) whose
    address is addr or more, with the comments and declarations just above it. Also the offset
    of the first function (the end of the file's preamble). (None, None) if no function is at addr
    or after."""
    lines = text.splitlines(keepends=True)
    anchors, depth, in_ifdef = [], 0, False
    for i, line in enumerate(lines):
        if depth == 0:
            if line.startswith("#ifdef NON_MATCHING"):
                in_ifdef = True
                anchors.append([i, None])
                continue
            if line.startswith("#endif") and in_ifdef:
                in_ifdef = False
                continue
            m = re.match(r'^#pragma GLOBAL_ASM\("asm/nonmatchings/[\w/]+/(\w+)\.s"\)', line)
            if m:
                if in_ifdef:
                    anchors[-1][1] = anchors[-1][1] or m.group(1)
                else:
                    anchors.append([i, m.group(1)])
                continue
            # a definition: its signature closes and "{" follows on the line (the body may start
            # there too), or a signature with no ";" whose "{" comes within 3 lines
            m = (re.match(r"^(?!typedef|extern|static\s+\w+\s+\w+\s*\[)[A-Za-z_][\w \t\*]*?\b(\w+)\s*\([^;{]*\)\s*\{", line)
                 or re.match(r"^(?!typedef|extern|static\s+\w+\s+\w+\s*\[)[A-Za-z_][\w \t\*]*?\b(\w+)\s*\([^;]*$", line))
            if m and "{" in "".join(lines[i:i + 3]):
                if in_ifdef:
                    anchors[-1][1] = anchors[-1][1] or m.group(1)
                else:
                    anchors.append([i, m.group(1)])
        depth += line.count("{") - line.count("}")
    if not anchors:
        return None, None
    j = anchors[0][0]  # the preamble ends above the first function's own comment
    while j > 0 and (not lines[j - 1].strip() or lines[j - 1].lstrip().startswith(("/*", "*", "//"))):
        j -= 1
    first = sum(len(l) for l in lines[:j])
    for k, (i, name) in enumerate(anchors):
        a = _func_addr(folder, name) if name else None
        if a is None:
            sys.exit(f"recut: no address for {name} ({folder})")
        if a >= addr:
            j = i
            # up over the comments, blank lines and declarations that belong to it
            while j > 0 and k > 0 and (not lines[j - 1].strip() or lines[j - 1].lstrip().startswith(("/*", "*", "//"))
                                      or lines[j - 1].rstrip().endswith(";") and not lines[j - 1].startswith("#")
                                      and not lines[j - 1].startswith((" ", "\t", "}"))):
                j -= 1
            return sum(len(l) for l in lines[:j]), first
    return None, first


def recut(fid, write=False, vram=0x08000000):
    """a TLB file's yaml block with pad-cuts' boundaries cut. Each
    boundary with no cut becomes a c row; its object's .data and .rodata rows are cut where its
    functions' reads (IDA) begin, when they read any; padding past 16-alignment becomes a pad
    row; the objects are renumbered tlb_NNNN_K in .text order. write: the block goes into the
    yaml, and each function's C (or GLOBAL_ASM pragma, its path renamed) into its object's C
    file, each new file opening with the old file's preamble."""
    IDA_FILE[0] = fid
    points = _pad_points(fid, fid, vram)
    if not points:
        print(f"recut {fid}: nothing to cut")
        return 0
    off = size = None
    with open(os.path.join(ROOT, "nisitenma.tsv"), encoding="utf-8") as f:
        for line in f.read().splitlines()[1:]:
            p = line.split("\t")
            if p[0] == str(fid):
                off, size = int(p[2], 16), int(p[3], 16)
    with open(DECOMPRESSED, "rb") as f:
        f.seek(off)
        data = f.read(size)
    ypath = os.path.join(ROOT, "goemonsgreatadv.yaml")
    with open(ypath, encoding="utf-8") as f:
        ylines = f.read().splitlines(keepends=True)
    a = next(i for i, l in enumerate(ylines) if l.rstrip() == f"  - name: tlb_{fid:04d}")
    b = next((i for i in range(a + 1, len(ylines)) if ylines[i].startswith("  - ")), len(ylines))
    sub = next(i for i in range(a, b) if ylines[i].rstrip() == "    subsegments:")
    rows = []
    for l in ylines[sub + 1:b]:
        if not l.strip():
            continue
        m = ROW.match(l.rstrip("\n"))
        if not m:
            sys.exit(f"recut {fid}: a row this does not read: {l.strip()}")
        rows.append({"rom": int(m.group(1), 16), "type": m.group(2), "name": m.group(3), "note": m.group(4)})
    v = lambda rom: rom - off + vram  # noqa: E731
    r = lambda va: va - vram + off  # noqa: E731
    cs = sorted((x["rom"], x["name"]) for x in rows if x["type"] == "c")
    text_end = min(x["rom"] for x in rows if x["type"] in ("data", ".data", ".rodata", "rodata"))
    new = []  # (rom, parent object name)
    lead = None  # zeros at the file's start: the first c row moves to the first function
    for _, e0, s1, aligned, has_cut in points:
        if e0 is None:
            lead = s1
            first_row = next(x for x in rows if x["type"] == "c" and x["rom"] == off)
            first_row["rom"] = r(s1)
            rows.append({"rom": off, "type": "pad", "name": None, "note": None})
            cs = [(r(s1) if c == off else c, n) for c, n in cs]
            continue
        if not has_cut:
            new.append((r(s1), [n for c, n in cs if c <= r(s1)][-1]))
        if aligned < s1:
            rows.append({"rom": r(aligned), "type": "pad", "name": None, "note": None})
    # each old object's pieces in order; each piece's reads into its old object's data rows
    pieces = {}
    for c, n in cs:
        pieces[n] = [c] + sorted(rom for rom, p in new if p == n)
    starts = sorted([c for c, _ in cs] + [rom for rom, _ in new])
    nxt = {s: (starts + [text_end])[k + 1] for k, s in enumerate(starts)}
    names = {s: f"tlb/tlb_{fid:04d}_{k}" for k, s in enumerate(starts)}
    if len(starts) == 1:  # one object: it keeps its name (only a leading pad row was added)
        names = {starts[0]: cs[0][1]}
    for n, ps in pieces.items():
        for kind in (("data", ".data"), (".rodata", "rodata")):
            row = next((x for x in rows if x["type"] in kind and x["name"] == n), None)
            if not row:
                continue
            after = sorted(x["rom"] for x in rows if x["rom"] > row["rom"]) + [off + size]
            lo, hi = v(row["rom"]), v(after[0])
            last = None
            for k, p in enumerate(ps):
                reads = [ad for ad, _ in tail_refs(data[p - off:nxt[p] - off], lo, hi, p - off, vram)]
                if k == 0:
                    last = max(reads) if reads else None
                    continue
                if not reads:
                    continue
                cut = min(reads) & ~15
                if last is not None and last >= cut:
                    sys.exit(f"recut {fid}: {names[p]}'s {kind[0]} reads begin at 0x{min(reads):08X}, but "
                             f"the object before it reads 0x{last:08X}")
                rows.append({"rom": r(cut), "type": row["type"], "name": names[p], "note": None, "new": True})
                last = max(reads)
    rename = {n: names[c] for c, n in cs}
    for x in rows:
        if x.get("new"):
            continue  # named for its new object already
        if x["type"] == "c":
            x["name"] = names[x["rom"]]
        elif x["name"] in rename:
            x["name"] = rename[x["name"]]
    rows += [{"rom": s, "type": "c", "name": names[s], "note": None} for s, _ in new]
    seen, block = set(), []
    for x in sorted(rows, key=lambda x: x["rom"]):
        key = (x["rom"], x["type"])
        if key in seen:
            continue
        seen.add(key)
        body = f"0x{x['rom']:X}, {x['type']}" + (f", {x['name']}" if x["name"] else "")
        block.append(f"      - [{body}]" + (f"  {x['note']}" if x["note"] else "") + "\n")
    print("".join(ylines[a:sub + 1] + block), end="")
    if not write:
        return 0
    # the C files: each old file's functions into the files of its pieces
    outs, olds = {}, []
    for c, n in cs:
        old = os.path.join(ROOT, "src", n + ".c")
        olds.append(old)
        with open(old, encoding="utf-8") as f:
            text = f.read()
        if lead is not None and c == r(lead):
            # the 4-byte "function" splat made of the leading zeros: its pragma goes
            at, first = _c_cut(text, n, lead)
            if at is not None and at > first:
                dropped = re.findall(r'GLOBAL_ASM\("[^"]*/(\w+)\.s"\)', text[first:at])
                text = text[:first] + text[at:]
                print(f"recut {fid}: dropped the pragma of {', '.join(dropped)} (leading zeros, now a pad row); "
                      f"remove any symbol_addrs.txt line naming it")
        ps = pieces[n]
        cuts_at, pre = [], None
        for p in ps[1:]:
            at, first = _c_cut(text, n, v(p))
            if at is None:
                sys.exit(f"recut {fid}: no function at or after 0x{v(p):08X} in {n}.c")
            cuts_at.append(at)
            pre = first
        bounds = [0] + cuts_at + [len(text)]
        for k, p in enumerate(ps):
            part = text[bounds[k]:bounds[k + 1]]
            if k:
                part = text[:pre].rstrip("\n") + "\n\n" + part.lstrip("\n")
            part = part.replace(f"asm/nonmatchings/{n}/", f"asm/nonmatchings/{names[p]}/")
            outs[os.path.join(ROOT, "src", names[p] + ".c")] = part.rstrip("\n") + "\n"
    for path, part in outs.items():
        with open(path, "w", encoding="utf-8", newline="\n") as f:
            f.write(part)
    for old in olds:
        if old not in outs:
            os.remove(old)
    ylines[sub + 1:b] = block + ["\n"]
    with open(ypath, "w", encoding="utf-8", newline="\n") as f:
        f.write("".join(ylines))
    print(f"recut {fid}: {len(outs)} C files written, {len([o for o in olds if o not in outs])} removed; "
          f"now delete asm/ and run build_rom.py --split")
    return 0


def tlb_diffs():
    """The TLB code files whose bytes in the last build differ from the baserom's. Each file is
    read where the build put it (its segment's _ROM_START/_ROM_END in the map), so a file that
    changed size does not make every file after it differ; those files are named apart."""
    rows = []
    with open(os.path.join(ROOT, "nisitenma.tsv"), encoding="utf-8") as f:
        for line in f.read().splitlines()[1:]:
            p = line.split("\t")
            if p[0].isdigit() and p[1] != "empty" and 16 <= int(p[0]) <= 327:
                rows.append((int(p[0]), int(p[2], 16), int(p[3], 16)))
    with open(DECOMPRESSED, "rb") as f:
        want = f.read()
    with open(os.path.join(ROOT, "build", "goemonsgreatadv.decompressed.z64"), "rb") as f:
        got = f.read()
    built = {}
    with open(os.path.join(ROOT, "build", "goemonsgreatadv.map"), encoding="utf-8", errors="replace") as f:
        for m in re.finditer(r"^\s+0x([0-9a-f]+)\s+tlb_(\d{4})_ROM_(START|END) = ", f.read(), re.M):
            built.setdefault(int(m.group(2)), {})[m.group(3)] = int(m.group(1), 16)
    bad, resized = [], []
    for fid, off, size in rows:
        b = built.get(fid, {})
        start, end = b.get("START", off), b.get("END", off + size)
        if end - start != size:
            resized.append(fid)
        if want[off:off + size] != got[start:end]:
            bad.append(fid)
    print(f"yaml-diffs: {len(bad)} TLB files differ: {bad}")
    if resized:
        print(f"yaml-diffs: {len(resized)} of them changed size (the files after them only moved): {resized}")
    return 0


def overlay_symbols():
    """{ovl_00NN: [(start, size)]} for the shared-slot overlays' functions and data, from the
    built ELF (objdump -t)."""
    objdump = os.path.join(os.path.dirname(ROOT), "vendor", "mips64", "bin", "mips64-elf-objdump.exe")
    elf = os.path.join(ROOT, "build", "goemonsgreatadv.elf")
    syms = collections.defaultdict(list)
    for line in subprocess.run([objdump, "-t", elf], capture_output=True, text=True, check=True).stdout.splitlines():
        p = line.split()
        if len(p) == 6 and p[2] in ("F", "O") and p[3] in (".ovl_0008", ".ovl_0009", ".ovl_0010"):
            syms[p[3][1:]].append((int(p[0], 16), max(int(p[4], 16), 1)))
    return syms


def slot_refs(code):
    """Addresses in the shared slot that a TLB file's code builds with lui + addiu/load/store
    (code at 0x08000000 cannot jal to 0x80000000: every call out is a lui/addiu + jalr)."""
    hi, refs = {}, []
    for i in range(0, len(code) - 3, 4):
        w = be32(code, i)
        op, rs, rt, imm = w >> 26, (w >> 21) & 31, (w >> 16) & 31, w & 0xFFFF
        if w == 0x03E00008:
            hi.clear()
        elif op == 0x0F:
            hi[rt] = imm << 16
        elif rs in hi and (op == 0x09 or 0x20 <= op <= 0x2F or op in (0x31, 0x35, 0x39, 0x3D)):
            addr = (hi[rs] + (imm - 0x10000 if imm & 0x8000 else imm)) & 0xFFFFFFFF
            if SHARED_SLOT[0] <= addr < SHARED_SLOT[1]:
                refs.append(addr)
            if op == 0x09 and rt == rs:
                del hi[rs]
    return refs


IDA_EXPORT = os.path.join(os.path.dirname(ROOT), "ida", "goemons_great_adventure", "export")
IDA_FILE = [None]      # the archive file the callers are working on: its IDA export answers
USE_IDA = [True]       # False: the old word decode (rodata-objects --compare, and no export)
_IDA_REFS = {}


def _ida_refs(fid):
    """[(instruction address, data target, opcode)] of a TLB file, from its IDA export."""
    if fid not in _IDA_REFS:
        path = os.path.join(IDA_EXPORT, f"{fid:04d}.json")
        refs = None
        if os.path.exists(path):
            import json
            with open(path, encoding="utf-8") as f:
                exp = json.load(f)
            refs = sorted((ea, t, op) for fn in exp["functions"] for ea, op, _m, _o, t in fn["ins"]
                          if t is not None)
        _IDA_REFS[fid] = refs
    return _IDA_REFS[fid]


def tail_refs(code, lo, hi, base=0, vram=0x08000000):
    """(address, opcode) of every instruction in code (the file's bytes from offset base) that
    reaches [lo, hi): from IDA's resolved data references when IDA_FILE names a file with an
    export, else from a decode of the lui + addiu/load/store words."""
    refs = _ida_refs(IDA_FILE[0]) if USE_IDA[0] and IDA_FILE[0] is not None else None
    if refs is not None:
        a, b = vram + base, vram + base + len(code)
        return [(t, op) for ea, t, op in refs if a <= ea < b and lo <= t < hi]
    regs, refs = {}, []
    for i in range(0, len(code) - 3, 4):
        w = be32(code, i)
        op, rs, rt, imm = w >> 26, (w >> 21) & 31, (w >> 16) & 31, w & 0xFFFF
        if w == 0x03E00008:
            regs.clear()
        elif op == 0x0F:
            regs[rt] = imm << 16
        elif rs in regs and (op == 0x09 or 0x20 <= op <= 0x2F or op in (0x31, 0x35, 0x39, 0x3D)):
            addr = (regs[rs] + (imm - 0x10000 if imm & 0x8000 else imm)) & 0xFFFFFFFF
            if lo <= addr < hi:
                refs.append((addr, op))
            if op == 0x09 and rt == rs:
                del regs[rs]
    return refs


def _is_string(data, o):
    """A NUL-terminated string of printable ASCII, tab/CR/LF or EUC-JP characters (the game's
    encoding: two bytes A1-FE, or 8E and a half-width kana A1-DF) at o, NUL-padded to a word
    (IDO's .rodata). A display list's E7000000 is not one, nor a double's bytes (3F91DF46...)."""
    n = data.find(b"\0", o)
    if n <= o or any(data[n:(n + 4) & ~3]):
        return False
    s, i = data[o:n], 0
    while i < len(s):
        b = s[i]
        if b in (9, 10, 13) or 0x20 <= b < 0x7F:
            i += 1
        elif i + 1 < len(s) and ((0xA1 <= b <= 0xFE and 0xA1 <= s[i + 1] <= 0xFE)
                                 or (b == 0x8E and 0xA1 <= s[i + 1] <= 0xDF)):
            i += 2
        else:
            return False
    return True


def rodata_start(data, text, vram=0x08000000, units=False):
    """Where a TLB file's .rodata begins (.text, .data, .rodata, each 16-aligned, linked in that
    order). IDO 7.1 puts EVERY string literal in .rodata, those in data initializers too, in
    source order, then the late rodata: floats and doubles, jump tables (measured: permuter/rodata_probe). So .rodata starts at the lowest string the code or the data
    points at; with no string, at the lowest late item: a float/double loaded (lwc1/ldc1) and
    never stored, or a jump table (lw'd words into .text that are not function starts).
    None: neither (no .rodata). units=True asks instead whether the file was linked from more
    than one object (a string after a late item: file 21's "goecreate\\n" after its doubles)."""
    end = vram + len(data)
    refs = tail_refs(data[:text], vram + text, end, 0, vram)
    stored = {a for a, op in refs if 0x28 <= op <= 0x2F or op in (0x39, 0x3D)}
    late = {a for a, op in refs if op in (0x31, 0x35) and a not in stored}
    starts = {vram} | {vram + i + 8 for i in range(0, text - 7, 4) if be32(data, i) == 0x03E00008}
    for a, op in refs:
        if op != 0x23 or a in stored:
            continue
        o = a - vram
        ws = [be32(data, o + 4 * k) for k in range(2) if o + 4 * k + 4 <= len(data)]
        if len(ws) == 2 and all(vram <= x < vram + text and x % 4 == 0 and x not in starts for x in ws):
            late.add(a)
    pointed = {a for a, op in refs if op == 0x09}  # the code takes these addresses
    pointed |= {be32(data, o) for o in range(text, len(data) - 3, 4)}  # the data points at these
    # three characters at least: a round float (40.0f, 0x42200000) reads as the string "B "
    strings = {a for a in pointed if vram + text <= a < end and a % 4 == 0 and a not in stored
               and _is_string(data, a - vram) and data.find(b"\0", a - vram) - (a - vram) >= 3}
    if not strings:
        return (min(late) if late else None) if not units else False
    if units:  # several objects: a string after the first late item opens the next one's .rodata
        lo = min(late) if late else end
        return any(_is_string(data, o) and data.find(b"\0", o) - o >= 3
                   for o in range(lo - vram, len(data) - 3, 4))
    o = min(strings) - vram
    while o - 4 >= text and data[o - 1] == 0:  # back over the strings no one points at (a
        q = o - 1                              # compiled-out PRINTF's format leads file 37's)
        while q > text and data[q - 1] == 0:
            q -= 1
        while q > text and (data[q - 1] in (9, 10, 13) or data[q - 1] >= 0x20):
            q -= 1
        if q == o or q % 4 or not _is_string(data, q) or data.find(b"\0", q) - q < 3:
            break  # (file 102: the floats 11.0, 12.0 above its strings read as "A0", "A@")
        o = q
    if o % 16:  # the section is 16-aligned: zero padding before it goes with it, else the
        down = o & ~15  # first string was not the first (a string-looking word above it)
        o = down if down >= text and not any(data[down:o]) else (o + 15) & ~15
    return vram + o


def rodata_objects(data, text, ro, vram=0x08000000):
    """The file offsets where each linked object's .rodata begins (the first is ro): an item
    after a late item (a float/double loaded and never stored, a jump table) opens the next.
    A table of numbers read with lw is not late: it is const data, in .rodata in source order."""
    refs = tail_refs(data[:text], vram + text, vram + len(data), 0, vram)
    stored = {a for a, op in refs if 0x28 <= op <= 0x2F or op in (0x39, 0x3D)}
    late, words = set(), set()  # late items, and every word they cover (a double two, a table all)
    for a, op in refs:
        o = a - vram
        if op == 0x23 and not (o + 4 <= len(data) and vram <= be32(data, o) < vram + text):
            continue  # lw of a number, not of a code address: const data, not a jump table
        if op in (0x31, 0x35, 0x23) and a not in stored and o >= ro:
            late.add(o)
            words |= {o, o + 4} if op == 0x35 else {o}
            while op == 0x23 and o + 4 < len(data) and vram <= be32(data, o + 4) < vram + text:
                o += 4
                words.add(o)  # a jump table's next entry
    cands, seen = [], False
    for p in range(ro, len(data) - 3, 4):
        if p in late:
            seen = True
        elif p in words or not any(data[p:p + 4]):
            continue  # a late item's own word, or padding
        elif seen and ((data[p - 1] == 0 or p - 4 in words) and _is_string(data, p)
                       and data.find(b"\0", p) - p >= 3  # "sea" right after file 16's last float
                       or p in {a - vram for a, op in refs if op == 0x23}):  # a const table read with lw
            cands.append(p & ~15 if not any(data[p & ~15:p]) else p)
            seen = False
    # a const table sits in .rodata in source order, among the strings: a string after one is
    # no cut. A real cut has no function reading .rodata on both sides of it (an object's
    # functions read only its own), and is 16-aligned.
    reads = []
    starts_f = sorted({0} | {i + 8 for i in range(0, text - 7, 4) if be32(data, i) == 0x03E00008})
    for k, s in enumerate(starts_f):
        e = starts_f[k + 1] if k + 1 < len(starts_f) else text
        reads.append({a - vram for a, op in tail_refs(data[s:e], vram + ro, vram + len(data), s, vram)})
    starts = [ro]
    for c in cands:
        if c % 16 == 0 and not any(min(r) < c <= max(r) for r in reads if r and min(r) >= starts[-1]):
            starts.append(c)
    return starts


def asm_func_starts(off, vram=0x08000000, fid=None):
    """The function starts (file offsets) splat found in a TLB file: from its asm/<ROM>.s while
    it is asm, or from its functions' .s files once it has C files (asm/nonmatchings/tlb/
    tlb_NNNN[_K]/): true starts, where a raw jr $ra scan also counts a leaf switch's cases."""
    texts = []
    path = os.path.join(ROOT, "asm", f"{off:X}.s")
    if os.path.exists(path):
        with open(path, encoding="utf-8") as f:
            texts.append(f.read())
    base = os.path.join(ROOT, "asm", "nonmatchings", "tlb")
    if fid is not None and os.path.isdir(base):
        for d in os.listdir(base):
            if re.match(r"^tlb_%04d(_\d+)?$" % fid, d):
                for n in os.listdir(os.path.join(base, d)):
                    if n.startswith("func_") and n.endswith(".s"):
                        with open(os.path.join(base, d, n), encoding="utf-8") as f:
                            texts.append(f.read())
    starts = sorted({int(m.group(1), 16) - vram for t in texts for m in
                     re.finditer(r"^glabel \w+\n(?:[^\n]*\n)*?\s*/\* [0-9A-F]+ ([0-9A-F]{8}) ", t, re.M)})
    return starts or None


def tlb_objects(data, text, ro, vram=0x08000000, starts=None):
    """Each linked object of a TLB file as (text start, data start, rodata start), file offsets:
    .rodata cut by rodata_objects; .text cut at the first function reading the next object's
    .rodata, back over functions that read none to a 16-aligned start; .data cut at the lowest
    address the object's functions read, 16-aligned. None: the cuts contradict each other, and
    OBJECTS_WHY[0] says how."""
    OBJECTS_WHY[0] = ""

    def fail(why):
        OBJECTS_WHY[0] = why
        return None
    rstarts = rodata_objects(data, text, ro, vram)
    if len(rstarts) == 1:
        return [(0, text, ro)]
    fstarts = starts or sorted({0} | {i + 8 for i in range(0, text - 7, 4) if be32(data, i) == 0x03E00008})
    fends = fstarts[1:] + [text]
    rb = rstarts + [len(data)]
    obj = []
    for s, e in zip(fstarts, fends):
        ks = {next(k for k in range(len(rstarts)) if rb[k] <= a - vram < rb[k + 1])
              for a, op in tail_refs(data[s:e], vram + ro, vram + len(data), s, vram)}
        if len(ks) > 1:
            return fail(f"the function at +0x{s:X} reads the .rodata of objects {sorted(ks)}")
        obj.append(ks.pop() if ks else None)
    tcuts = [0]
    for k in range(1, len(rstarts)):
        firsts = [i for i, o in enumerate(obj) if o == k]
        if not firsts:
            return fail(f"no function reads object {k}'s .rodata (+0x{rstarts[k]:X})")
        late = [fstarts[i] for i in range(firsts[0], len(obj)) if obj[i] is not None and obj[i] < k]
        if late:
            return fail(f"object {k} starts at +0x{fstarts[firsts[0]]:X}, but the function at +0x{late[0]:X} "
                        f"after it reads an earlier object's .rodata")
        i = firsts[0]
        while i > 0 and obj[i - 1] is None and fstarts[i] % 16:
            i -= 1
        if fstarts[i] % 16 or fstarts[i] <= tcuts[-1]:
            return fail(f"object {k}'s .text cut +0x{fstarts[i]:X} is not 16-aligned or not after the last")
        tcuts.append(fstarts[i])
    dcuts = [text]
    tb = tcuts + [text]
    for k in range(1, len(rstarts)):
        # reads below the last cut are an earlier object's data, reached through an extern
        mine = [a - vram for a, op in tail_refs(data[tb[k]:tb[k + 1]], vram + text, vram + ro, tb[k], vram)
                if a - vram >= dcuts[-1]]
        d = (min(mine) & ~15) if mine else None
        later = [a - vram for a, op in tail_refs(data[tb[k]:text], vram + text, vram + ro, tb[k], vram)
                 if a - vram >= dcuts[-1]]
        d = d if d is not None else ((min(later) & ~15) if later else ro)
        if d < dcuts[-1]:
            return fail(f"object {k}'s .data cut +0x{d:X} is before object {k - 1}'s (+0x{dcuts[-1]:X}): "
                        f"its functions read data below the last cut")
        dcuts.append(d)
    return list(zip(tcuts, dcuts, rstarts))


OBJECTS_WHY = [""]  # why tlb_objects last gave up


def force_migrate(fid, write=False):
    """the rodata blocks of TLB file fid that splat left as their own
    GLOBAL_ASM (outside every function) although exactly one of the file's functions reads
    each: a symbol_addrs.txt line per block with function_owner:<reader>, so the next split
    moves each into its reader's .s (as IDO laid it out: that function's late rodata)."""
    srcs = sorted(glob.glob(os.path.join(ROOT, "src", "tlb", f"tlb_{fid:04d}*.c")))
    lines, alone = [], []
    for src in srcs:
        stem = os.path.splitext(os.path.basename(src))[0]
        folder = os.path.join(ROOT, "asm", "nonmatchings", "tlb", stem)
        with open(src, encoding="utf-8") as f:
            blocks = re.findall(r'GLOBAL_ASM\("[^"]*/(D_([0-9A-F]{8})_tlb_\d{4})\.s"\)', f.read())
        funcs = {}
        for p in glob.glob(os.path.join(folder, "*.s")):
            if not os.path.basename(p).startswith("D_"):
                with open(p, encoding="utf-8") as f:
                    funcs[os.path.basename(p)[:-2]] = f.read()
        # which function's .s already holds each migrated rodata symbol (by address)
        owned = sorted((int(a, 16), fn) for fn, text in funcs.items()
                       for a in re.findall(r"^dlabel \w+_([0-9A-F]{8})_tlb_\d{4}\s*$", text, re.M))
        for name, addr in blocks:
            readers = [fn for fn, text in funcs.items() if re.search(r"\b%s\b" % name, text)]
            if len(readers) == 1:
                lines.append(f"{name} = 0x{addr}; // segment:tlb_{fid:04d} function_owner:{readers[0]}")
                continue
            a = int(addr, 16)
            before = [fn for x, fn in owned if x < a][-1:]
            after = [fn for x, fn in owned if x > a][:1]
            if not readers and before and before == after:
                # read by no function, but inside one function's rodata: part of it. Unread,
                # splat sees .word, not .float, and keeps it out of the late rodata: typed f32
                # when every word reads as an ordinary float
                words = []
                p = os.path.join(folder, f"{name}.s")
                if os.path.exists(p):
                    with open(p, encoding="utf-8") as f:
                        words = [int(w, 16) for w in re.findall(r"\.word 0x([0-9A-Fa-f]{8})\b", f.read())]
                floats = words and all(w & 0x7FFFFFFF == 0 or 0x30000000 <= w & 0x7FFFFFFF < 0x4B000000 for w in words)
                typed = f" type:f32 size:0x{4 * len(words):X}" if floats else ""
                lines.append(f"{name} = 0x{addr}; // segment:tlb_{fid:04d} function_owner:{before[0]}{typed}")
            else:
                alone.append(f"{name} ({len(readers)} readers)")
    print("\n".join(lines))
    print(f"force-migrate {fid}: {len(lines)} blocks with one reader; left as they are: "
          + (", ".join(alone) or "none"))
    if write and lines:
        path = os.path.join(ROOT, "symbol_addrs.txt")
        with open(path, encoding="utf-8") as f:
            have = f.read()
        new = [l for l in lines if l.split(" =")[0] + " =" not in have]
        with open(path, "a", encoding="utf-8", newline="\n") as f:
            f.write(f"// file {fid}: rodata splat left outside its one reader\n"
                    + "".join(l + "\n" for l in new))
        print(f"force-migrate {fid}: {len(new)} lines added to symbol_addrs.txt; delete asm/ and re-split")
    return 0


def tlb_units(fid, vram=0x08000000):
    """a TLB file linked from several objects, cut into them. Each object's
    .text, .data and .rodata are 16-aligned and linked in object order (A.text B.text A.data
    B.data A.rodata B.rodata); an object's .rodata is its strings then its late items, so a
    string after a late item opens the next object's .rodata (IDO_NOTES 287). Each function
    goes to the object whose .rodata it reads; the .text cut is where those objects change,
    the .data cut where the data the next object's functions read begins. Prints the cuts and
    whatever contradicts them (a function reading two objects' .rodata, an unaligned cut)."""
    IDA_FILE[0] = fid  # IDA's references for this file
    row = None
    with open(os.path.join(ROOT, "nisitenma.tsv"), encoding="utf-8") as f:
        for line in f.read().splitlines()[1:]:
            p = line.split("\t")
            if p[0] == str(fid):
                row = (int(p[2], 16), int(p[3], 16))
    with open(DECOMPRESSED, "rb") as f:
        f.seek(row[0])
        data = f.read(row[1])
    rets = [i for i in range(0, len(data) - 3, 4) if data[i:i + 4] == b"\x03\xe0\x00\x08"]
    text = (rets[-1] + 8 + 15) // 16 * 16
    ro = rodata_start(data, text, vram) - vram
    # the functions, from the split's asm (asm/<ROM>.s: each glabel and its first address)
    with open(os.path.join(ROOT, "asm", f"{row[0]:X}.s"), encoding="utf-8") as f:
        funcs = [(int(m.group(2), 16) - vram, m.group(1)) for m in
                 re.finditer(r"^glabel (\w+)\n(?:[^\n]*\n)*?\s*/\* [0-9A-F]+ ([0-9A-F]{8}) ", f.read(), re.M)]
    starts = rodata_objects(data, text, ro, vram)
    bounds = starts + [len(data)]

    def obj_of(o):
        return next(k for k in range(len(starts)) if bounds[k] <= o < bounds[k + 1])
    own, notes = [], []
    for k, (start, name) in enumerate(funcs):
        end = funcs[k + 1][0] if k + 1 < len(funcs) else text
        mine = {obj_of(a - vram) for a, op in tail_refs(data[start:end], vram + ro, vram + len(data), start, vram)}
        if len(mine) > 1:
            notes.append(f"{name} reads .rodata of objects {sorted(mine)}")
        own.append(min(mine) if mine else None)
    # each object's first function: the first that reads its .rodata, after the last of the one before
    cuts, last = [0], 0
    for k in range(1, len(starts)):
        firsts = [i for i, o in enumerate(own) if o == k]
        if not firsts:
            notes.append(f"object {k}: no function reads its .rodata")
            cuts.append(None)
            continue
        i = firsts[0]
        # back over functions that read no .rodata, to the object's 16-aligned .text start
        while i > last + 1 and own[i - 1] is None and funcs[i][0] % 16:
            i -= 1
        if any(o is not None and o < k for o in own[i:]):
            notes.append(f"object {k}: a function after its first reads an earlier object's .rodata")
        if funcs[i][0] % 16:
            notes.append(f"object {k}: its first function {funcs[i][1]} is not 16-aligned")
        cuts.append(funcs[i][0])
        last = i
    print(f"file {fid}: text 0x{text:X}, data 0x{text:X}-0x{ro:X}, rodata 0x{ro:X}-0x{len(data):X}; "
          f"{len(starts)} objects by .rodata")
    for k in range(len(starts)):
        t = f"0x{cuts[k]:X}" if cuts[k] is not None else "?"
        print(f"  object {k}: text from {t}, rodata 0x{bounds[k]:X}-0x{bounds[k + 1]:X}, "
              f"{sum(1 for o in own if o == k)} functions read it")
    for n in notes:
        print(f"  ! {n}")
    objs = tlb_objects(data, text, ro, vram, asm_func_starts(row[0], vram, fid))
    print(f"  yaml writer: {len(objs)} objects, accepted" if objs else
          f"  yaml writer: REFUSED: {OBJECTS_WHY[0]}")
    if fid in TLB_ASM:
        print(f"  TLB_ASM (kept asm by hand): {TLB_ASM[fid]}")
    return 0


def resident(refs, syms):
    """The overlay with a symbol starting at the most of these references: the one in the slot
    while the file runs. None if no reference is a symbol's start. (Inside-a-symbol does not
    tell them apart: every slot address is inside some function of overlays 9 and 10 both.)"""
    starts = {ovl: {s for s, n in lst} for ovl, lst in syms.items()}
    score = {ovl: sum(a in st for a in refs) for ovl, st in starts.items()}
    best = max(score, key=score.get) if score else None
    return (best if best and score[best] else None), score


def code_yaml(first, last, vram=0x08000000, write=False, count=False):
    """the archive's TLB code files (Great Adventure: 16-327, each IDO-compiled
    and linked at 0x08000000, run through the TLB by fileMapTlb) as splat code segments, printed
    as yaml: one segment per non-empty file from its place in the decompressed ROM (nisitenma.tsv)
    to the next file's, all at the same VRAM under one exclusive_ram_id; asm from its start,
    data from where its .text ends (after the last `jr $ra` and its delay slot, 16-aligned: the
    rule of survey_rom.py textend). Then the bin segment for the files after them."""
    rows = []
    with open(os.path.join(ROOT, "nisitenma.tsv"), encoding="utf-8") as f:
        for line in f.read().splitlines()[1:]:
            p = line.split("\t")
            if p[0].isdigit() and p[1] != "empty":
                rows.append((int(p[0]), int(p[2], 16), int(p[3], 16)))
    rows.sort(key=lambda r: r[1])
    with open(DECOMPRESSED, "rb") as f:
        rom = f.read()
    out = []
    nfuncs, ntext = 0, 0
    syms, slots, multis = overlay_symbols(), collections.Counter(), []
    # a file that already has its C file(s) keeps its block as it is: its C is written against it
    with open(os.path.join(ROOT, "goemonsgreatadv.yaml"), encoding="utf-8") as f:
        current = f.read()
    kept = {int(m.group(1)): m.group(0).rstrip("\n") for m in
            re.finditer(r"^  - name: tlb_(\d{4})\n(?:    .*\n|      .*\n)*", current, re.M)
            if ", c, tlb/" in m.group(0)}
    for k, (fid, off, size) in enumerate(rows):
        IDA_FILE[0] = fid  # IDA's references for this file
        if not first <= fid <= last:
            continue
        if fid in kept and not count and fid not in TLB_ASM:
            out.append(kept[fid] + "\n")
            nxt = next((r for r in rows[k + 1:] if r[0] > last), None)
            if (fid == last or (rows[k + 1:] and rows[k + 1][0] > last)) and nxt:
                out.append(f"  - name: files_{nxt[0]:04d}\n    type: bin\n    start: 0x{nxt[1]:X}\n")
            continue
        data = rom[off:off + size]
        rets = [i for i in range(0, len(data) - 3, 4) if data[i:i + 4] == b"\x03\xe0\x00\x08"]
        if not rets:
            sys.exit(f"yaml: file {fid} has no jr $ra")
        text = (rets[-1] + 8 + 15) // 16 * 16
        nfuncs += len(rets)  # IDO: one jr $ra per function (early returns branch to the epilogue)
        ntext += text
        # The slot's overlay while this file runs: splat binds the file's references into the
        # slot to it (prioritized_segments), not to whichever overlay it searches first.
        ovl, score = resident(slot_refs(data[:text]), syms)
        slots[ovl] += 1
        ro = rodata_start(data, text, vram)
        if count:
            slots["rodata" if ro else "no rodata"] += 1
            if count == "rodata":
                print(f"file {fid}: text 0x{text:X} size 0x{size:X} rodata "
                      + (f"0x{ro - vram:X}" if ro else "-"))
            continue
        # one C file per TLB file (src/tlb/tlb_NNNN.c, its functions as GLOBAL_ASM until each
        # matches); its .data as splat data (asm) until it is C; its .rodata migrated into the
        # functions that use it (IDO's late rodata), so each function matches with its own
        name = f"tlb/tlb_{fid:04d}"
        ro_off = ro - vram if ro else size
        # several objects in one file: one C file cannot give their .rodata order (and
        # asm-processor refuses the second object's doubles), and a jump table left in a .rodata
        # asm file cannot reach its cases' .L labels in another object: the file stays splat asm
        # (text, data) until it is split into its objects
        objs = tlb_objects(data, text, ro - vram, vram, asm_func_starts(off, vram, fid)) if ro else [(0, text, size)]
        if objs and len(objs) > 1 and fid not in TLB_ASM:
            # several objects: one C file each (src/tlb/tlb_NNNN_K.c), all their .text first,
            # then their .data, then their .rodata, as the linker laid them out
            head = (f"  - name: tlb_{fid:04d}\n    type: code\n    start: 0x{off:X}\n"
                    f"    vram: 0x{vram:08X}\n    exclusive_ram_id: tlb_{vram:08X}\n"
                    + (f"    prioritized_segments: [{ovl}]\n" if ovl else "") + "    subsegments:\n")
            tb = [o[0] for o in objs] + [text]
            db = [o[1] for o in objs] + [objs[0][2]]
            lines = [f"      - [0x{off + o[0]:X}, c, {name}_{j}]\n" for j, o in enumerate(objs)]
            lines += [f"      - [0x{off + db[j]:X}, data, {name}_{j}]\n" for j in range(len(objs)) if db[j] < db[j + 1]]
            lines += [f"      - [0x{off + o[2]:X}, .rodata, {name}_{j}]\n" for j, o in enumerate(objs)]
            out.append(head + "".join(lines))
            nxt = next((r for r in rows[k + 1:] if r[0] > last), None)
            if (fid == last or (rows[k + 1:] and rows[k + 1][0] > last)) and nxt:
                out.append(f"  - name: files_{nxt[0]:04d}\n    type: bin\n    start: 0x{nxt[1]:X}\n")
            continue
        multi = objs is None
        if multi or fid in TLB_ASM:
            multis.append(fid)
            out.append(f"  - name: tlb_{fid:04d}\n    type: code\n    start: 0x{off:X}\n"
                       f"    vram: 0x{vram:08X}\n    exclusive_ram_id: tlb_{vram:08X}\n"
                       + (f"    prioritized_segments: [{ovl}]\n" if ovl else "") + "    subsegments:\n"
                       f"      - [0x{off:X}, asm]\n" + (f"      - [0x{off + text:X}, data]\n" if text < size else ""))
            nxt = next((r for r in rows[k + 1:] if r[0] > last), None)
            if (fid == last or (rows[k + 1:] and rows[k + 1][0] > last)) and nxt:
                out.append(f"  - name: files_{nxt[0]:04d}\n    type: bin\n    start: 0x{nxt[1]:X}\n")
            continue
        out.append(f"  - name: tlb_{fid:04d}\n    type: code\n    start: 0x{off:X}\n"
                   f"    vram: 0x{vram:08X}\n    exclusive_ram_id: tlb_{vram:08X}\n"
                   + (f"    prioritized_segments: [{ovl}]\n" if ovl else "") + "    subsegments:\n"
                   f"      - [0x{off:X}, c, {name}]\n"
                   + (f"      - [0x{off + text:X}, data, {name}]\n" if text < ro_off else "")
                   + (f"      - [0x{off + ro_off:X}, .rodata, {name}]\n" if ro else ""))
        nxt = next((r for r in rows[k + 1:] if r[0] > last), None)
        if fid == last or (rows[k + 1:] and rows[k + 1][0] > last):
            if nxt:
                out.append(f"  - name: files_{nxt[0]:04d}\n    type: bin\n    start: 0x{nxt[1]:X}\n")
    if count:
        print(f"files {first}-{last}: {nfuncs} jr $ra (about that many functions) in 0x{ntext:X} bytes of .text;"
              f" slot overlay per file: {dict(slots)}")
        return 0
    block = "\n".join(out)
    if not write:
        print(block)
        return 0
    path = os.path.join(ROOT, "goemonsgreatadv.yaml")
    with open(path, encoding="utf-8") as f:
        text = f.read()
    end = text.rstrip().rfind("\n  - [0x")  # the end marker, after the last segment
    if end < 0:
        sys.exit("yaml: no end marker in the yaml; nothing written")
    start = end + 1
    if "name: tlb_" in text:
        # the first generation (asm + data per file) is replaced; never a block a person has
        # edited since (a C file already named, data already C)
        start = text.find("  - name: tlb_")
        old = text[start:end + 1]
        if re.search(r"- \[0x[0-9A-F]+, (?!asm\]|data\]|(c|data|\.rodata|rodata), tlb/tlb_\d{4}(_rodata|_\d+)?\])", old):
            sys.exit("yaml: the tlb_ segments are no longer the generated ones; nothing written")
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(text[:start] + block + "\n" + text[end + 1:])
    print(f"yaml: {sum(1 for l in out if 'name: tlb_' in l)} code segments written before the end marker;"
          f" {len(multis)} of several objects (.rodata kept as asm): {multis}")
    return 0


if __name__ == "__main__":
    cmd = sys.argv[1:]
    if len(cmd) in (2, 3) and cmd[0] == "recut":
        sys.exit(recut(int(cmd[1]), write=cmd[2:] == ["--write"]))
    if len(cmd) == 3 and cmd[0] == "pad-cuts":
        sys.exit(pad_cuts(int(cmd[1]), int(cmd[2])))
    if cmd == ["yaml-diffs"]:
        sys.exit(tlb_diffs())
    if cmd == ["lz"]:
        sys.exit(lz_check())
    if len(cmd) == 2 and cmd[0] == "units":
        sys.exit(tlb_units(int(cmd[1])))
    if len(cmd) == 2 and cmd[0] == "refs-compare":
        # every (instruction, target) the old decode and IDA find in one file's .text, and which
        # side lacks it
        fid = int(cmd[1])
        with open(DECOMPRESSED, "rb") as f:
            rom_bytes = f.read()
        with open(LAYOUT, encoding="utf-8") as f:
            off, size = next((int(p[2], 16), int(p[3], 16)) for p in
                             (line.split("\t") for line in f.read().splitlines()[1:]) if p[0] == str(fid))
        blob = rom_bytes[off:off + size]
        rets = [i for i in range(0, len(blob) - 3, 4) if blob[i:i + 4] == b"\x03\xe0\x00\x08"]
        text = (rets[-1] + 8 + 15) // 16 * 16
        dec = set()
        regs = {}
        for i in range(0, text, 4):  # the old decode, per instruction, to name each side's misses
            w = be32(blob, i)
            op, rs, rt, imm = w >> 26, (w >> 21) & 31, (w >> 16) & 31, w & 0xFFFF
            if w == 0x03E00008:
                regs.clear()
            elif op == 0x0F:
                regs[rt] = imm << 16
            elif rs in regs and (op == 0x09 or 0x20 <= op <= 0x2F or op in (0x31, 0x35, 0x39, 0x3D)):
                a = (regs[rs] + (imm - 0x10000 if imm & 0x8000 else imm)) & 0xFFFFFFFF
                if 0x08000000 + text <= a < 0x08000000 + size:
                    dec.add((0x08000000 + i, a))
                if op == 0x09 and rt == rs:
                    del regs[rs]
        ida = {(ea, t) for ea, t, op in (_ida_refs(fid) or []) if 0x08000000 + text <= t < 0x08000000 + size}
        for ea, t in sorted(dec - ida):
            print(f"  decode only: {ea:08X} -> {t:08X}")
        for ea, t in sorted(ida - dec):
            print(f"  IDA only:    {ea:08X} -> {t:08X}")
        print(f"refs-compare {fid}: {len(dec & ida)} both, {len(dec - ida)} decode only, {len(ida - dec)} IDA only")
        sys.exit(0)
    if len(cmd) in (3, 4) and cmd[0] == "rodata-objects":
        # --compare: the old word decode and IDA's references side by side; only differences printed
        compare = cmd[3:] == ["--compare"]
        with open(DECOMPRESSED, "rb") as f:
            rom_bytes = f.read()
        with open(LAYOUT, encoding="utf-8") as f:
            layout = {int(p[0]): (int(p[2], 16), int(p[3], 16)) for p in
                      (line.split("\t") for line in f.read().splitlines()[1:]) if p[0].isdigit()}
        same = differ = 0
        for fid in range(int(cmd[1]), int(cmd[2]) + 1):
            off, size = layout.get(fid, (0, 0))
            blob = rom_bytes[off:off + size]
            rets = [i for i in range(0, len(blob) - 3, 4) if blob[i:i + 4] == b"\x03\xe0\x00\x08"]
            if not size or not rets:
                continue
            IDA_FILE[0] = fid
            text = (rets[-1] + 8 + 15) // 16 * 16
            answers = []
            for use in ([False, True] if compare else [True]):
                USE_IDA[0] = use
                ro = rodata_start(blob, text)
                starts = rodata_objects(blob, text, ro - 0x08000000) if ro else [size]
                answers.append(f"{len(starts)} " + " ".join(f"{s:X}" for s in starts))
            USE_IDA[0] = True
            if not compare:
                print(f"{fid} {answers[0]}")
            elif answers[0] == answers[1]:
                same += 1
            else:
                differ += 1
                print(f"{fid} DIFFERS: decode {answers[0]} | IDA {answers[1]}"
                      + ("" if _ida_refs(fid) is not None else " (no IDA export)"))
        if compare:
            print(f"rodata-objects --compare: {same} files the same, {differ} different")
        sys.exit(0)
    if len(cmd) in (2, 3) and cmd[0] == "force-migrate":
        sys.exit(force_migrate(int(cmd[1]), write=cmd[2:] == ["--write"]))
    if len(cmd) in (3, 4) and cmd[0] == "yaml":
        sys.exit(code_yaml(int(cmd[1]), int(cmd[2]), write=cmd[3:] == ["--write"],
                           count={"--count": True, "--rodata": "rodata"}.get((cmd[3:] or [""])[0], False)))
    if cmd == ["survey"]:
        survey()
    elif cmd == ["unpack"]:
        unpack()
    elif len(cmd) == 3 and cmd[0] == "pack":
        pack(cmd[1], cmd[2])
    elif cmd == ["check"]:
        sys.exit(check())
    else:
        sys.exit(__doc__)
