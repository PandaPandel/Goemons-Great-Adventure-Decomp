"""zlib 1.1.3's inflate side, from upstream's source, compiled as the game compiled it.

    python canon-decompilation-tools/build_zlib.py build                compile lib/zlib's files into build/zlib/
    python canon-decompilation-tools/build_zlib.py map                  where each object's .text is in main's code, and
                                              its .data/.rodata/.bss
    python canon-decompilation-tools/build_zlib.py diff ROM file        where an object departs from the ROM

(Not zlib.py: that would shadow Python's zlib, which canon-decompilation-tools/nisitenma_archive.py
packs the ROM with.)

The game links the inflate half of zlib 1.1.3; the ROM holds "inflate 1.1.3 Copyright 1995-1998
Mark Adler". lib/zlib is upstream (https://github.com/madler/zlib) at v1.1.3, unedited.

The flags are the game's own (canon-decompilation-tools/build_rom.py: IDO 7.1 through asm-processor, -O2 -mips2) less
-signed: zlib's char is IDO's default, unsigned. inflateInit2_'s `version[0] != ZLIB_VERSION[0]`
loads both with lbu in the ROM; -signed makes them lb, the only difference in the eight files.
Plus -DNO_ERRNO_H. zutil.h's errno is never read by the inflate code, and ultralib's IDO headers
have no errno.h; include/stddef.h is the one header they lack that zlib does read. Each file is
compiled from an LF copy in build/zlib/src: cfe reads a CRLF line's `\\` continuation as a
syntax error, and a Windows checkout may have CRLF.

canon-decompilation-tools/build_rom.py calls link_inputs() for the yaml's `[rom, lib, libz, <file>]` entries. It archives
those objects into build/lib/libz.a and writes build/zlib_syms.txt, the addresses of what they
call outside zlib that symbol_addrs.txt does not name. They call calloc and free (the game's,
in romdecode.c, at the addresses this file reads back from the ROM) and memcpy (libultra's).

Changelog: first public release (2026-10-20). Earlier history is kept privately.
"""
import os
import re
import struct
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import build_rom as build  # noqa: E402  (ROOT, the compile command, binutils)
import build_libultra as ultralib  # noqa: E402  (the ELF reader and the ROM comparison)

ROOT = build.ROOT
ZLIB = os.path.join(ROOT, "lib", "zlib")
SRC = os.path.join(ROOT, "build", "zlib", "src")
OUT = "build/zlib"
FILES = ("inflate", "infblock", "infutil", "zutil", "adler32", "inftrees", "infcodes", "inffast")
ARCHIVE = "build/lib/libz.a"  # the yaml's lib_path is build/lib
SYMS = "build/zlib_syms.txt"


def stage():
    """LF copies of lib/zlib's sources and headers in build/zlib/src, rewritten only on a change."""
    os.makedirs(SRC, exist_ok=True)
    for n in os.listdir(ZLIB):
        if not n.endswith((".c", ".h")):
            continue
        with open(os.path.join(ZLIB, n), "rb") as f:
            data = f.read().replace(b"\r\n", b"\n")
        out = os.path.join(SRC, n)
        if os.path.exists(out):
            with open(out, "rb") as f:
                if f.read() == data:
                    continue
        with open(out, "wb") as f:
            f.write(data)


def obj_path(name):
    return f"{OUT}/{name}.o"


def compile_one(name):
    stage()
    src = os.path.relpath(os.path.join(SRC, name + ".c"), ROOT).replace("\\", "/")
    obj = obj_path(name)
    if not build.stale(obj, [src, "include/stddef.h"]):
        return True
    os.makedirs(os.path.join(ROOT, OUT), exist_ok=True)
    cmd = [a for a in build.compile_c(src, obj, ["-DNO_ERRNO_H"]) if a != "-signed"]
    r = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True, env=build.ENV)
    return r.returncode == 0 and os.path.exists(os.path.join(ROOT, obj))


def cmd_build():
    failed = [n for n in FILES if not compile_one(n)]
    print(f"zlib 1.1.3: {len(FILES) - len(failed)} compiled, {len(failed)} failed")
    for n in failed:
        print("  FAILED", n)


def elf(name):
    return ultralib.read_elf(os.path.join(ROOT, obj_path(name)))


def cmd_map():
    """Each object's place in main's code (16-aligned, relocated fields masked) and where its
    data sections are (ultralib.bases: from its %hi/%lo pairs, or its bytes in main's data)."""
    with open(ultralib.BASEROM, "rb") as f:
        rom = f.read()
    n = (ultralib.MAIN_DATA - ultralib.MAIN_ROM) // 4
    code = struct.unpack_from(f">{n}I", rom, ultralib.MAIN_ROM)
    for name in FILES:
        if not compile_one(name):
            print(f"{name}: FAILED to compile")
            continue
        e = elf(name)
        words, masks = ultralib.masked_words(e["text"], e["relocs"])
        hits = [ultralib.MAIN_ROM + i * 4 for i in range(0, n - len(words) + 1, 4)
                if all((code[i + k] ^ w) & m == 0 for k, (w, m) in enumerate(zip(words, masks)))]
        if len(hits) != 1:
            print(f"{name}: {len(e['text']):X} bytes of text, {len(hits)} matches")
            continue
        b = ultralib.bases(e, rom, hits[0])
        secs = " ".join(f"{s[1:]}={e['sizes'][s]:X}@{b.get(s, 0):08X}" for s in ultralib.DATA_SECS
                        if e["sizes"].get(s))
        print(f"{hits[0]:X} {hits[0] + len(e['text']):X} {name} {secs or '-'}"
              + ("" if ultralib.placed(e, b) else " UNPLACED"))


def cmd_diff(at, name):
    """Why an object does not match at a ROM offset: the sizes, and the words that differ."""
    if not compile_one(name):
        sys.exit(f"FAILED: zlib {name}")
    with open(ultralib.BASEROM, "rb") as f:
        rom = f.read()
    e = elf(name)
    words, masks = ultralib.masked_words(e["text"], e["relocs"])
    org = struct.unpack_from(f">{len(words)}I", rom, at)
    bad = [k for k, (w, m, o) in enumerate(zip(words, masks, org)) if (w ^ o) & m]
    print(f"{name}: {len(words) * 4:X} bytes; {len(bad)} words differ at {at:X}")
    for k in bad[:12]:
        print(f"  +{k * 4:04X} rom {org[k]:08X} obj {words[k]:08X}")


def members():
    """(rom, file) for each zlib object the yaml links."""
    with open(ultralib.YAML) as f:
        return [(int(a, 16), m) for a, m in
                re.findall(r"\[0x([0-9A-Fa-f]+),\s*lib,\s*libz,\s*(\w+)\s*\]", f.read())]


def link_inputs():
    """For canon-decompilation-tools/build_rom.py: compile the linked objects, archive them, and write the addresses of
    what they call that no object defines (read back from the ROM). Returns the ld arguments."""
    ms = members()
    if not ms:
        return []
    with open(ultralib.BASEROM, "rb") as f:
        rom = f.read()
    os.makedirs(os.path.join(ROOT, "build", "lib"), exist_ok=True)
    elves = []
    build.parallel([lambda m=m: compile_one(m) for _, m in ms])  # (907); failures below
    for at, m in ms:
        if not compile_one(m):
            sys.exit(f"FAILED: zlib {m}")
        elves.append((at, elf(m)))
    if os.path.exists(os.path.join(ROOT, ARCHIVE)):
        os.remove(os.path.join(ROOT, ARCHIVE))
    subprocess.run([ultralib.AR, "rcs", ARCHIVE, *[obj_path(m) for _, m in ms]], cwd=ROOT, check=True)
    defined = {n for _, e in elves for n, (_, _, bind) in e["syms"].items() if bind in (1, 2)}
    named = ultralib.symbol_addrs()  # calloc, free (romdecode.c) and memcpy (libultra) are named
    want = {}
    for at, e in elves:
        want.update(ultralib.targets(e, rom, at))
    with open(os.path.join(ROOT, SYMS), "w") as f:
        f.write("/* Written by canon-decompilation-tools/build_zlib.py: what the zlib objects call outside zlib that\n"
                "   symbol_addrs.txt does not name. */\n")
        for n, a in sorted(want.items(), key=lambda x: x[1]):
            if n not in defined and n not in named:
                f.write(f"{n} = 0x{a:08X};\n")
    return ["-T", SYMS, "--whole-archive", ARCHIVE, "--no-whole-archive"]


def main():
    args = sys.argv[1:]
    if args == ["build"]:
        return cmd_build()
    if args == ["map"]:
        return cmd_map()
    if len(args) == 3 and args[0] == "diff":
        return cmd_diff(int(args[1], 16), args[2])
    sys.exit(__doc__)


if __name__ == "__main__":
    main()
