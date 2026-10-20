"""survey_rom (first named romscope) - static facts about an N64 ROM; one of
the canon decompilation tools (canon-decompilation-tools/README.md).

    survey_rom.py header   <rom> [<rom> ...]      header fields, libultra version, hashes
    survey_rom.py compiler <rom> [<rom> ...]      compiler fingerprints of the main code segment
        --text START:END   ROM range of code to measure (hex); default: splat create_config's main
    survey_rom.py libsig   <rom> [--seg ROMSTART:ROMEND:VRAM] [--key <symbols.txt> ...]
        libultra/libc names by signature (vendor n64sym, run on a segment slice at its real VRAM),
        each with its signature size and a duplicate flag; with --key (`name = 0xADDR;` lines),
        scored by size bucket. Default segment: splat's entry through main.
        --ref <rom> --refkey <symbols.txt>: verify each name against the same function in a
        reference ROM on the same libultra (VERIFIED / AMBIGUOUS twins / DIFFERS / NO-REF).
    survey_rom.py filetable <rom> --offsets VRAM --ranges VRAM --count N [--out file.tsv]
        a game's file system from two parallel tables (u32 ROM offsets, bit 31 = flag; (start,end)
        pairs), one row per file, with a ROM-coverage summary.
    survey_rom.py unpack <rom> --offsets VRAM --ranges VRAM --count N [--id N ...] [--outdir DIR]
        decompress the file system's files (zlib, and Great Adventure's Konami LZ), each checked
        against the game's own RAM size; writes <id>_<ramstart>.bin.
    survey_rom.py recompress <rom> --offsets VRAM --ranges VRAM --count N [--id N ...]
        which zlib level / memLevel / strategy reproduces each compressed file exactly. (15).
    survey_rom.py textend --raw <file> --vram VRAM
        where an IDO code file's .text ends (its last jr $ra, 16-aligned), and any jal target
        past that boundary.
    survey_rom.py codemap <rom> [--offsets VRAM --ranges VRAM --count N]
        every run of code in the ROM, found by reading every word (code_runs): ROM offset, bytes,
        returns (IDO: one per function). With the file system's tables, each file is read
        DECOMPRESSED, so compressed code is not missed.
    --raw   the input is a plain big-endian code/data file (an unpacked overlay), not a ROM.

Great Adventure's file system: --offsets 0x8002BB94 --ranges 0x80026B00 --count 0xA10.
One tool, extended in place. Instructions are decoded by rabbitizer, never by hand.
Run it from the repository root: python canon-decompilation-tools/survey_rom.py ...

Changelog: first public release (2026-10-20). Earlier history is kept privately.
"""

import collections
import hashlib
import json
import os
import re
import subprocess
import sys
import tempfile

import rabbitizer
import yaml

# Real opcodes, not pseudos: `move` must show which encoding the compiler chose.
rabbitizer.config.pseudos_enablePseudos = False

GPR = rabbitizer.RegGprO32
ZERO, SP, FP = GPR.zero, GPR.sp, GPR.fp
MOVE_OPS = ("or", "addu", "daddu")


RAW = [False]   # --raw: the input is a plain big-endian code/data file (e.g. an unpacked overlay)


def load_rom(path):
    """Return the ROM as big-endian (.z64) bytes, whatever order it is stored in."""
    with open(path, "rb") as f:
        data = f.read()
    if RAW[0]:
        return data, "raw file (--raw)"
    magic = data[:4]
    if magic == b"\x80\x37\x12\x40":
        return data, "z64 (big-endian)"
    if magic == b"\x37\x80\x40\x12":
        return bytes(b for i in range(0, len(data), 2) for b in (data[i + 1], data[i])), "v64 (byteswapped)"
    if magic == b"\x40\x12\x37\x80":
        return bytes(b for i in range(0, len(data), 4) for b in data[i:i + 4][::-1]), "n64 (little-endian)"
    sys.exit(f"survey_rom: {path}: not an N64 ROM (first bytes {magic.hex()})")


def header(path):
    rom, order = load_rom(path)
    be32 = lambda o: int.from_bytes(rom[o:o + 4], "big")
    rel = rom[0x0C:0x10]
    libultra = f"{rel[2] // 10}.{rel[2] % 10}{chr(rel[3])}" if rel[2] and 0x41 <= rel[3] <= 0x5A else "unknown"
    with open(path, "rb") as f:
        raw = f.read()
    print(f"== {os.path.basename(path)}")
    print(f"  byte order     {order}, {len(rom)} bytes")
    print(f"  name   (0x20)  {rom[0x20:0x34].decode('ascii', 'replace').rstrip()}")
    print(f"  code   (0x3B)  {rom[0x3B:0x3F].decode('ascii', 'replace')}  revision {rom[0x3F]}")
    print(f"  crc    (0x10)  {be32(0x10):08X} {be32(0x14):08X}")
    print(f"  entry  (0x08)  0x{be32(0x08):08X}")
    print(f"  libultra(0x0C) {rel.hex().upper()} -> {libultra}")
    print(f"  sha1 (as stored) {hashlib.sha1(raw).hexdigest()}")
    print(f"  md5  (as stored) {hashlib.md5(raw).hexdigest()}")


_SPLAT_CFG = {}


def splat_config(path):
    """splat create_config's yaml for this ROM (run once per ROM, in a temp dir)."""
    if path not in _SPLAT_CFG:
        rom, _ = load_rom(path)
        with tempfile.TemporaryDirectory() as tmp:
            z64 = os.path.join(tmp, "rom.z64")
            with open(z64, "wb") as f:
                f.write(rom)
            subprocess.run([sys.executable, "-m", "splat", "create_config", z64], cwd=tmp,
                           check=True, capture_output=True)
            cfg = next(n for n in os.listdir(tmp) if n.endswith(".yaml"))
            with open(os.path.join(tmp, cfg)) as f:
                _SPLAT_CFG[path] = yaml.safe_load(f)
    return _SPLAT_CFG[path]


def code_segment(path):
    """(rom_start, rom_end, vram) of splat's entry through main: the boot-loaded code and data."""
    segs = splat_config(path)["segments"]
    seg_start = lambda s: s[0] if isinstance(s, list) else s.get("start")
    named = {s.get("name"): (i, s) for i, s in enumerate(segs) if isinstance(s, dict)}
    if "main" not in named:
        raise ValueError("splat's create_config found no 'main' segment; pass --seg")
    first = named.get("entry", named["main"])[1]
    i_main = named["main"][0]
    end = next((seg_start(s) for s in segs[i_main + 1:] if seg_start(s) is not None), None)
    if end is None or "vram" not in first:
        raise ValueError("cannot place the entry/main segment; pass --seg")
    return first["start"], end, first["vram"]


def main_text_range(path):
    """ROM range of the main segment's text, as splat's create_config finds it."""
    doc = splat_config(path)
    segs = doc["segments"]
    seg_start = lambda s: s[0] if isinstance(s, list) else s.get("start")
    for i, seg in enumerate(segs):
        if isinstance(seg, dict) and seg.get("name") == "main":
            subs = seg["subsegments"]
            start = subs[0][0] if isinstance(subs[0], list) else subs[0]["start"]
            end = next((s[0] for s in subs if isinstance(s, list) and s[1] in ("data", "rodata")), None)
            if end is None:  # text only: it ends where the next segment starts
                end = next((seg_start(s) for s in segs[i + 1:] if seg_start(s) is not None), None)
            if end is None:
                raise ValueError("splat's main segment has no end")
            return start, end
    raise ValueError("splat's create_config found no 'main' segment; pass --text")


def compiler(path, text=None):
    rom, _ = load_rom(path)
    start, end = text or main_text_range(path)
    n = valid = likely = fp_setup = 0
    moves = dict.fromkeys(MOVE_OPS, 0)
    for off in range(start, end, 4):
        ins = rabbitizer.Instruction(int.from_bytes(rom[off:off + 4], "big"), vram=0x80000000 + off)
        n += 1
        if not ins.isValid():
            continue
        valid += 1
        name = ins.getOpcodeName()
        if ins.isBranchLikely():
            likely += 1
        if name in MOVE_OPS:
            rd, rs, rt = ins.rd, ins.rs, ins.rt
            if rd != ZERO and (rt == ZERO) != (rs == ZERO):
                moves[name] += 1
                if rd == FP and SP in (rs, rt):
                    fp_setup += 1
    total_moves = sum(moves.values()) or 1
    share = {k: v / total_moves for k, v in moves.items()}
    print(f"== {os.path.basename(path)}  text 0x{start:X}-0x{end:X} ({n} words, {valid} valid)")
    print("  move encodings " + "  ".join(f"{k}={moves[k]} ({share[k]:.0%})" for k in MOVE_OPS))
    print(f"  branch-likely  {likely} ({likely * 1000 / max(valid, 1):.1f} per 1000)")
    print(f"  fp = sp setups {fp_setup}")
    # Calibrated on 11 labelled ROMs (docs/CANON_TOOLS.md): IDO encodes move as `or` (98-100%),
    # GCC/KMC as `addu` (100%). 90% leaves margin; the IDO version (5.3 vs 7.1) is not decided here.
    if sum(moves.values()) < 100:
        verdict = "UNDECIDED (fewer than 100 moves measured)"
    elif share["or"] >= 0.90:
        verdict = "IDO (move = or); version not determined by this test"
    elif share["addu"] >= 0.90 or share["daddu"] >= 0.90:
        verdict = "GCC/KMC (move = addu/daddu)"
    else:
        verdict = "MIXED (no encoding reaches 90%: split the range, or linked objects differ)"
    print(f"  verdict        {verdict}")
    return moves, likely, valid, fp_setup


# the external tools sit beside the repository, in ../vendor (as build_rom.py's do)
VENDOR = os.environ.get("VENDOR", os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                                               "..", "vendor"))
N64SYM_DIR = os.path.join(VENDOR, "n64sym-v0.9a")
SIZE_BUCKETS = ((0, 32), (32, 64), (64, 128), (128, 256), (256, 1 << 30))


def read_key(paths):
    """addr -> set of names, from splat/linker `name = 0xADDR;` lines."""
    key = collections.defaultdict(set)
    pat = re.compile(r"^\s*([A-Za-z_]\w*)\s*=\s*0x([0-9A-Fa-f]+)\s*;")
    for p in paths:
        with open(p, encoding="utf-8", errors="replace") as f:
            for line in f:
                m = pat.match(line)
                if m:
                    key[int(m.group(2), 16)].add(m.group(1))
    return key


LO_USERS = ("addiu", "addi", "daddiu", "ori", "lb", "lbu", "lh", "lhu", "lw", "lwu", "ld", "lwl", "lwr",
            "sb", "sh", "sw", "sd", "swl", "swr", "lwc1", "ldc1", "swc1", "sdc1")


def masked_words(rom, off, nwords, vram):
    """Instruction words with LINK-TIME fields masked, so the same compiled function compares equal
    wherever it was linked: j/jal targets, and a RAM address built by `lui` (0x8000-0x9FFF, KSEG0)
    plus its lo16 user. A hardware address (0xA4xx, KSEG1) is NOT masked: it is what tells the SP and
    SI raw-I/O twins apart, which n64sym confuses."""
    out, hi_reg = [], {}
    for i in range(nwords):
        w = int.from_bytes(rom[off + 4 * i:off + 4 * i + 4], "big")
        ins = rabbitizer.Instruction(w, vram=vram + 4 * i)
        name = ins.getOpcodeName() if ins.isValid() else ""
        if name in ("j", "jal"):
            w &= 0xFC000000
        elif name == "lui":
            imm = w & 0xFFFF
            if 0x8000 <= imm < 0xA000:
                hi_reg[ins.rt] = True
                w &= 0xFFFF0000
            else:
                hi_reg.pop(ins.rt, None)
        elif name in LO_USERS and hi_reg.get(ins.rs):
            w &= 0xFFFF0000
            # Only a GPR destination can clear a tracked HI (lwc1/ldc1 write a float register).
            if name in ("addiu", "addi", "daddiu", "ori", "lb", "lbu", "lh", "lhu", "lw", "lwu", "ld",
                        "lwl", "lwr") and ins.rt != ins.rs:
                hi_reg.pop(ins.rt, None)
        out.append(w)
    return out


def libsig(path, seg=None, keys=(), ref=None, refkeys=()):
    exe = os.path.join(N64SYM_DIR, "n64sym.exe")
    if not os.path.isfile(exe):
        raise ValueError(f"{exe} missing: fetch n64sym v0.9a (github.com/shygoo/n64sym)")
    with open(os.path.join(N64SYM_DIR, "web", "signatures.json")) as f:
        sizes = collections.defaultdict(set)
        for entry in json.load(f):
            sizes[entry[0]].add(entry[1])
    rom, _ = load_rom(path)
    start, end, vram = seg or code_segment(path)
    with tempfile.TemporaryDirectory() as tmp:
        sl = os.path.join(tmp, "seg.bin")
        with open(sl, "wb") as f:
            f.write(rom[start:end])
        out = subprocess.run([exe, sl, "-s", "-h", f"0x{vram:08X}"], cwd=tmp,
                             capture_output=True, text=True, check=True).stdout
    hits = []
    for line in out.splitlines():
        m = re.match(r"^([0-9A-Fa-f]{8})\s+(\S+)\s*$", line.strip())
        if m:
            hits.append((int(m.group(1), 16), m.group(2)))
    count = collections.Counter(n for _, n in hits)
    key = read_key(keys) if keys else None
    # The verifier: the same-named function in a reference ROM on the SAME libultra, whose names a
    # human decomp verified. VERIFIED = every masked word identical.
    refname = None
    if ref:
        rrom, _ = load_rom(ref)
        rstart, rend, rvram = code_segment(ref)
        refname = collections.defaultdict(list)
        rkey = read_key(refkeys)
        for a, names in rkey.items():
            for n in names:
                refname[n].append(a)
        # The function's REAL length in the reference build: distance to the next key symbol. The
        # signature size can be longer than this build's function (n64sym lists __ull_rem as 100 B
        # where the next function starts 60 B later), and a window that runs into the NEXT function
        # turns a true match into DIFFERS.
        raddrs = sorted(rkey)
        import bisect
        def ref_len(a, fallback):
            i = bisect.bisect_right(raddrs, a)
            return min(raddrs[i] - a, fallback) if i < len(raddrs) and fallback else fallback
        # TWINS. Masking RAM addresses is what lets code compare across games, and it also makes two
        # functions that differ ONLY in which RAM variable they touch identical: osViGetCurrent- vs
        # osViGetNextFramebuffer, __osSiRelAccess vs __osPiRelAccess. Measured on Smash's self-test:
        # both "verified" under the wrong name. So every named reference function is indexed by its
        # masked code, and a match that more than one DIFFERENTLY-NAMED function shares is AMBIGUOUS.
        twin = collections.defaultdict(set)
        for i, a in enumerate(raddrs):
            if not (rvram <= a < rvram + (rend - rstart)) or i + 1 >= len(raddrs):
                continue
            n = min(raddrs[i + 1] - a, 4096) // 4
            if n and rstart + (a - rvram) + 4 * n <= rend:
                twin[tuple(masked_words(rrom, rstart + (a - rvram), n, a))].add(
                    "/".join(sorted(rkey[a])))
    vstats = collections.Counter()
    print(f"== {os.path.basename(path)}  segment ROM 0x{start:X}-0x{end:X} at 0x{vram:08X}: {len(hits)} hits")
    stats = {b: collections.Counter() for b in SIZE_BUCKETS}
    for addr, name in hits:
        size = max(sizes.get(name, {0}))
        dup = count[name] > 1
        verdict = ""
        if refname is not None:
            toff = start + (addr - vram)
            cands = [a for a in refname.get(name, ()) if rvram <= a and rstart + (a - rvram) + size <= rend]
            if size == 0 or not cands:
                verdict = "NO-REF"
            elif toff < start or toff + size > end:
                verdict = "OUT-OF-SEGMENT"
            else:
                best = None
                for a in cands:
                    nw = ref_len(a, size) // 4
                    mine = masked_words(rom, toff, nw, addr)
                    d = sum(x != y for x, y in zip(mine, masked_words(rrom, rstart + (a - rvram), nw, a)))
                    if best is None or d < best[0]:
                        best = (d, nw, tuple(mine))
                if best[0]:
                    verdict = f"DIFFERS({best[0]}/{best[1]})"
                elif len(twin.get(best[2], ())) > 1:
                    verdict = "AMBIGUOUS(" + "|".join(sorted(twin[best[2]])) + ")"
                else:
                    verdict = "VERIFIED"
            vstats[verdict.split("(")[0]] += 1
        status = ""
        if key is not None:
            status = "exact" if name in key.get(addr, ()) else ("renamed" if addr in key else "not-in-key")
            b = next(b for b in SIZE_BUCKETS if b[0] <= size < b[1])
            stats[b][status] += 1
            stats[b]["dup" if dup else "unique"] += 1
            if dup and status == "exact":
                stats[b]["dup-exact"] += 1
        names_here = ",".join(sorted(key.get(addr, ()))) if key is not None and status == "renamed" else ""
        print(f"  {addr:08X}  {name:<28} {size:>6}B{'  DUP' if dup else ''}"
              f"{'  ' + verdict if verdict else ''}"
              f"{'  ' + status if status else ''}{'  key=' + names_here if names_here else ''}")
    if refname is not None:
        print(f"  --- against {os.path.basename(ref)}: " +
              ", ".join(f"{k} {v}" for k, v in sorted(vstats.items())))
    if key is not None:
        print("  --- scored against the key, by signature size ---")
        print("  size        hits  exact  renamed  not-in-key  exact%   (dup hits, of which exact)")
        for b in SIZE_BUCKETS:
            s = stats[b]
            n = s["exact"] + s["renamed"] + s["not-in-key"]
            if n:
                label = f"{b[0]}-{b[1] - 1}B" if b[1] < (1 << 30) else f">={b[0]}B"
                print(f"  {label:<10} {n:>5} {s['exact']:>6} {s['renamed']:>8} {s['not-in-key']:>11}"
                      f"  {100 * s['exact'] / n:5.1f}%   ({s['dup']}, {s['dup-exact']})")


def filetable(path, offsets, ranges, count, out=None):
    rom, _ = load_rom(path)
    start, end, vram = code_segment(path)
    to_rom = lambda va: start + (va - vram)
    be32 = lambda o: int.from_bytes(rom[o:o + 4], "big")
    for va in (offsets, ranges):
        if not (start <= to_rom(va) < end):
            raise ValueError(f"0x{va:08X} is outside splat's entry/main segment")
    rows, kinds, flags, prev = [], collections.Counter(), collections.Counter(), None
    order_breaks = []
    for i in range(count):
        fid = i + 1
        o0, o1 = be32(to_rom(offsets) + 4 * i), be32(to_rom(offsets) + 4 * (i + 1))
        flag, rs, re_ = o0 >> 31, o0 & 0x7FFFFFFF, o1 & 0x7FFFFFFF
        size = re_ - rs
        r0, r1 = be32(to_rom(ranges) + 8 * i), be32(to_rom(ranges) + 8 * i + 4)
        hdr = rom[rs:rs + 4].hex().upper() if size > 0 and rs + 4 <= len(rom) else "-"
        if size == 0:
            kind = "empty"
        elif 0x01000000 <= r0 < 0x10000000:
            kind = f"seg{r0 >> 24:X}"
        elif 0x80000000 <= r0 < 0x80800000:
            kind = "ram"
        else:
            kind = "other"
        if size < 0 or (prev is not None and rs < prev):
            order_breaks.append(fid)
        prev = rs
        kinds[kind] += 1
        flags[flag] += 1
        rows.append((fid, rs, re_, size, flag, hdr, r0, r1, r1 - r0, kind))
    lines = ["id\trom_start\trom_end\tsize\tflag\thdr\tram_start\tram_end\tram_size\tkind"]
    lines += [f"{f}\t0x{a:06X}\t0x{b:06X}\t{s}\t{fl}\t{h}\t0x{c:08X}\t0x{d:08X}\t{e}\t{k}"
              for f, a, b, s, fl, h, c, d, e, k in rows]
    if out:
        with open(out, "w", newline="\n") as f:
            f.write("\n".join(lines) + "\n")
    else:
        print("\n".join(lines))
    nonempty = [r for r in rows if r[3] > 0]
    print(f"== {os.path.basename(path)}  {count} files from offsets 0x{offsets:08X} / ranges 0x{ranges:08X}")
    print("  kinds   " + "  ".join(f"{k}={v}" for k, v in sorted(kinds.items())))
    print(f"  flag    bit31 set={flags[1]}  clear={flags[0]}")
    if nonempty:
        print(f"  ROM     0x{min(r[1] for r in nonempty):06X}-0x{max(r[2] for r in nonempty):06X}, "
              f"{sum(r[3] for r in nonempty)} bytes in {len(nonempty)} non-empty files")
    print(f"  order   {len(order_breaks)} out-of-order or negative-size entries"
          + (f" (first ids: {order_breaks[:10]})" if order_breaks else ""))
    ram = [r for r in rows if r[9] == "ram"]
    if ram:
        print(f"  ram     {len(ram)} files link to RAM, lowest 0x{min(r[6] for r in ram):08X}, "
              f"highest end 0x{max(r[7] for r in ram):08X}")


def file_entries(rom, path, offsets, ranges, count):
    """(id, rom_start, rom_end, flag, ram_start, ram_end) per file, from the two parallel tables."""
    start, end, vram = code_segment(path)
    to_rom = lambda va: start + (va - vram)
    be32 = lambda o: int.from_bytes(rom[o:o + 4], "big")
    for i in range(count):
        o0, o1 = be32(to_rom(offsets) + 4 * i), be32(to_rom(offsets) + 4 * (i + 1))
        yield (i + 1, o0 & 0x7FFFFFFF, o1 & 0x7FFFFFFF, o0 >> 31,
               be32(to_rom(ranges) + 8 * i), be32(to_rom(ranges) + 8 * i + 4))


def konami_lz(blob):
    """Great Adventure's LZ (sub_80002F40's top-nibble-0 path), as IDA's Hex-Rays
    decompilation of that function and of its byte-advance sub_800034D4 reads. The 4-byte BE header
    H is the stream length INCLUDING itself; decoding stops when the position reaches H.
      00-7F  back-reference: offset ((c<<8)|next) & 0x3FF, length (c>>2)+2, byte-by-byte (overlap ok)
      80-9F  (c & 0x1F) literal bytes        A0-BF  end of stream
      C0-DF  next byte repeated (c&0x1F)+2   E0-FE  (c&0x1F)+2 zeros     FF  next+2 zeros
    The multi-block path (top nibble != 0) is not implemented: no file in this ROM uses it."""
    h = int.from_bytes(blob[:4], "big")
    if h >> 28:
        raise ValueError("multi-block LZ (top nibble != 0) is not implemented")
    pos, out = 4, bytearray()
    while pos < h:
        c = blob[pos]
        pos += 1
        if c < 0x80:
            off = ((c << 8) | blob[pos]) & 0x3FF
            pos += 1
            if off == 0 or off > len(out):
                raise ValueError(f"back-reference offset {off} with {len(out)} bytes out, at stream {pos}")
            src = len(out) - off
            for k in range((c >> 2) + 2):
                out.append(out[src + k])
        elif c < 0xA0:
            n = c & 0x1F
            out += blob[pos:pos + n]
            pos += n
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


def file_data(blob, flag):
    """(bytes, how) of one file-system file as the game loads it: stored raw (flag clear), zlib
    (header bit 31) or Konami LZ; bytes None if it does not decompress."""
    import zlib
    if not flag:
        return blob, "raw"
    if int.from_bytes(blob[:4], "big") >> 31:
        try:
            d = zlib.decompressobj()
            data = d.decompress(blob[4:])
            return (data, "zlib") if d.eof else (None, "zlib-truncated")
        except zlib.error as e:
            return None, f"zlib-error({e})"
    try:
        return konami_lz(blob), "konami-lz"
    except (ValueError, IndexError) as e:
        return None, f"konami-lz-error({e})"


def unpack(path, offsets, ranges, count, ids=None, outdir=None):
    rom, _ = load_rom(path)
    tally = collections.Counter()
    for fid, rs, re_, flag, r0, r1 in file_entries(rom, path, offsets, ranges, count):
        if ids and fid not in ids:
            continue
        if re_ <= rs:
            continue
        want = r1 - r0
        data, how = file_data(rom[rs:re_], flag)
        if data is None:
            verdict = "FAILED"
        elif len(data) == want:
            verdict = "OK"
        else:
            verdict = "SHORT" if len(data) < want else "LONG"
        tally[(how.split("(")[0], verdict)] += 1
        if ids or verdict != "OK":
            print(f"  file {fid:4d}  ROM 0x{rs:06X}-0x{re_:06X}  {how:<18} "
                  f"{'-' if data is None else len(data):>8} vs range {want:>8}  {verdict}")
        if outdir and data is not None:
            os.makedirs(outdir, exist_ok=True)
            with open(os.path.join(outdir, f"{fid:04d}_{r0:08X}.bin"), "wb") as f:
                f.write(data)
    print(f"== {os.path.basename(path)}  unpack: " +
          ", ".join(f"{h}/{v} {n}" for (h, v), n in sorted(tally.items())))


def recompress(path, offsets, ranges, count, ids=None):
    """Can each zlib file be recompressed to the ROM's exact bytes?."""
    import zlib
    rom, _ = load_rom(path)
    settings = [(lv, ml, st) for lv in range(1, 10) for ml in range(1, 10)
                for st in (zlib.Z_DEFAULT_STRATEGY, zlib.Z_FILTERED)]
    by_setting, none, total, first_hit = collections.Counter(), [], 0, collections.Counter()
    for fid, rs, re_, flag, r0, r1 in file_entries(rom, path, offsets, ranges, count):
        if (ids and fid not in ids) or re_ <= rs or not flag:
            continue
        blob = rom[rs:re_]
        if not int.from_bytes(blob[:4], "big") >> 31:
            continue
        d = zlib.decompressobj()
        data = d.decompress(blob[4:])
        stream = blob[4:len(blob) - len(d.unused_data)]
        total += 1
        # Level 9 / memLevel 8 / default reproduced every sampled file: try it
        # first, and sweep every setting only for a file it does not reproduce.
        c = zlib.compressobj(9, zlib.DEFLATED, 15, 8, zlib.Z_DEFAULT_STRATEGY)
        if c.compress(data) + c.flush() == stream:
            hits = [(9, 8, zlib.Z_DEFAULT_STRATEGY)]
        else:
            hits = []
            for lv, ml, st in settings:
                c = zlib.compressobj(lv, zlib.DEFLATED, 15, ml, st)
                if c.compress(data) + c.flush() == stream:
                    hits.append((lv, ml, st))
        for h in hits:
            by_setting[h] += 1
        if hits:
            first_hit[hits[0]] += 1
        else:
            none.append(fid)
    name = {zlib.Z_DEFAULT_STRATEGY: "default", zlib.Z_FILTERED: "filtered"}
    print(f"== {os.path.basename(path)}  recompress: {total} zlib files, Python zlib {zlib.ZLIB_RUNTIME_VERSION}")
    print(f"  reproduced by at least one setting: {total - len(none)}   by none: {len(none)}"
          + (f" (first ids: {none[:15]})" if none else ""))
    for (lv, ml, st), n in by_setting.most_common(8):
        print(f"  level {lv} memLevel {ml} {name[st]:<8} reproduces {n}")


def textend(path, vram):
    """Where an IDO code file's .text ends: after the LAST `jr $ra` and its delay slot (IDO's text
    is functions only; jump tables live in .rodata). Checked: every `jal` target that lands inside
    this file must lie before the boundary."""
    data, _ = load_rom(path)
    n = len(data) // 4
    last_ret, jal_targets = None, []
    for i in range(n):
        w = int.from_bytes(data[4 * i:4 * i + 4], "big")
        if w == 0x03E00008:
            last_ret = i
        elif w >> 26 == 3:  # jal
            jal_targets.append(((vram + 4 * i) & 0xF0000000) | ((w & 0x03FFFFFF) << 2))
    if last_ret is None:
        raise ValueError("no `jr $ra` in the file")
    end = 4 * (last_ret + 2)
    aligned = (end + 15) // 16 * 16
    pad = data[end:aligned]
    inside = [t for t in jal_targets if vram <= t < vram + len(data)]
    after = sorted({t for t in inside if t >= vram + end})
    print(f"== {os.path.basename(path)} at 0x{vram:08X}: {len(data)} bytes")
    print(f"  last jr $ra at +0x{4 * last_ret:X} (0x{vram + 4 * last_ret:08X}); text ends +0x{end:X}"
          f"; 16-aligned +0x{aligned:X} (0x{vram + aligned:08X}); pad {pad.hex() or '-'}")
    print(f"  jal targets inside the file: {len(inside)}; landing AFTER the boundary: {len(after)}"
          + (f"  <-- text runs longer: {[hex(t) for t in after[:8]]}" if after else "  (consistent)"))
    return aligned


JR_RA = (0x03E00008).to_bytes(4, "big")


def code_runs(data, vram=0x80000000):
    """Where the code is in a byte string, found by reading it, not from any config. Each
    `jr $ra` (with a valid delay slot) and the valid instructions before it, back to an invalid
    word or the end of the run before, is code; leading zero words are trimmed (no function
    starts with a nop); a run that starts where the last one ended joins it, so one file's text
    reads as one run. [(start, end, returns)], end just past the last delay slot.

    A function that never returns (no `jr $ra`) is counted only when another function's return
    follows it in the same run. A lone 8-byte run may be data that happens to hold the word
    03E00008: codemap and report_progress mark runs under 16 bytes."""
    n = len(data) // 4
    known = {}

    def ok(i):
        if i not in known:
            w = int.from_bytes(data[4 * i:4 * i + 4], "big")
            known[i] = rabbitizer.Instruction(w, vram=vram + 4 * i).isValid()
        return known[i]

    runs, pos = [], 0
    while True:
        j = data.find(JR_RA, pos)
        if j < 0:
            return runs
        if j % 4:
            pos = j + 1
            continue
        i = j // 4
        pos = j + 4
        if i + 1 >= n or not ok(i + 1):
            continue
        floor = runs[-1][1] // 4 if runs else 0
        k = i - 1
        while k >= floor and ok(k):
            k -= 1
        start = k + 1
        while start < i and data[4 * start:4 * start + 4] == bytes(4):
            start += 1
        if runs and start == floor:
            runs[-1] = (runs[-1][0], 4 * (i + 2), runs[-1][2] + 1)
        else:
            runs.append((4 * start, 4 * (i + 2), 1))
        pos = 4 * (i + 2)


def codemap(path, offsets=None, ranges=None, count=None):
    rom, _ = load_rom(path)
    places = []  # (label, rom offset, bytes read, how)
    if offsets is None:
        places.append(("rom", 0, rom, "as stored"))
        if not RAW[0]:
            print("  note: a compressed file's code is not visible here; give --offsets/--ranges/--count "
                  "to read every file-system file decompressed")
    else:
        covered = []
        for fid, rs, re_, flag, r0, r1 in file_entries(rom, path, offsets, ranges, count):
            if re_ <= rs:
                continue
            data, how = file_data(rom[rs:re_], flag)
            covered.append((rs, re_))
            if data is None:
                print(f"  file {fid}: NOT READ ({how}): it may hold code")
                continue
            places.append((f"file {fid}", rs, data, how))
        at = 0
        for rs, re_ in sorted(covered) + [(len(rom), len(rom))]:
            if rs > at:
                places.append((f"rom 0x{at:X}", at, rom[at:rs], "outside the file system"))
            at = max(at, re_)
    total = funcs = 0
    for label, off, data, how in places:
        for s, e, r in code_runs(data):
            total += e - s
            funcs += r
            tiny = "  (under 16 bytes: may be data)" if e - s < 16 else ""
            print(f"  {label:<14} +0x{s:06X}-0x{e:06X}  {e - s:>8} bytes  {r:>5} returns  [{how}]{tiny}")
    print(f"== {os.path.basename(path)}  codemap: {total} bytes of code in the runs above, {funcs} returns")
    return total, funcs


def main(argv):
    if len(argv) < 2 or argv[0] not in ("header", "compiler", "libsig", "filetable", "unpack",
                                        "recompress", "textend", "codemap"):
        sys.exit(__doc__)
    mode, args = argv[0], argv[1:]
    if "--raw" in args:
        RAW[0] = True
        args.remove("--raw")
    text = seg = None
    keys = []
    if "--text" in args:
        i = args.index("--text")
        a, b = args[i + 1].split(":")
        text = (int(a, 16), int(b, 16))
        del args[i:i + 2]
    if "--seg" in args:
        i = args.index("--seg")
        seg = tuple(int(x, 16) for x in args[i + 1].split(":"))
        del args[i:i + 2]
    while "--key" in args:
        i = args.index("--key")
        keys.append(args[i + 1])
        del args[i:i + 2]
    opt = {}
    for flag in ("--offsets", "--ranges", "--count", "--out", "--outdir", "--vram"):
        if flag in args:
            i = args.index(flag)
            opt[flag] = args[i + 1]
            del args[i:i + 2]
    ids = set()
    while "--id" in args:
        i = args.index("--id")
        ids.add(int(args[i + 1], 0))
        del args[i:i + 2]
    ref, refkeys = None, []
    if "--ref" in args:
        i = args.index("--ref")
        ref = args[i + 1]
        del args[i:i + 2]
    while "--refkey" in args:
        i = args.index("--refkey")
        refkeys.append(args[i + 1])
        del args[i:i + 2]
    failed = 0
    for rom in args:
        try:
            if mode == "header":
                header(rom)
            elif mode == "compiler":
                compiler(rom, text)
            elif mode == "libsig":
                libsig(rom, seg, keys, ref, refkeys)
            elif mode == "filetable":
                filetable(rom, int(opt["--offsets"], 16), int(opt["--ranges"], 16),
                          int(opt["--count"], 0), opt.get("--out"))
            elif mode == "unpack":
                unpack(rom, int(opt["--offsets"], 16), int(opt["--ranges"], 16),
                       int(opt["--count"], 0), ids, opt.get("--outdir"))
            elif mode == "recompress":
                recompress(rom, int(opt["--offsets"], 16), int(opt["--ranges"], 16),
                           int(opt["--count"], 0), ids)
            elif mode == "codemap":
                fs = "--offsets" in opt
                codemap(rom, int(opt["--offsets"], 16) if fs else None, int(opt["--ranges"], 16) if fs else None,
                        int(opt["--count"], 0) if fs else None)
            else:
                textend(rom, int(opt["--vram"], 16))
        except (ValueError, subprocess.CalledProcessError, StopIteration) as e:
            print(f"== {os.path.basename(rom)}  FAILED: {e}")
            failed += 1
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main(sys.argv[1:])
