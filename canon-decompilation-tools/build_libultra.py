"""libultra from ultralib's source, compiled as Nintendo compiled it.

    python canon-decompilation-tools/build_libultra.py build   compile lib/ultralib/src for version I into build/ultralib/
    python canon-decompilation-tools/build_libultra.py map     match each object's .text against main's code; print where
    python canon-decompilation-tools/build_libultra.py yaml    print main's library subsegments with every clean match a lib
    python canon-decompilation-tools/build_libultra.py diff ROM dir/file.c   where an object departs from the ROM
    python canon-decompilation-tools/build_libultra.py near ROM END          the closest objects for code nothing matches

canon-decompilation-tools/build_rom.py calls link_inputs() for the yaml's `[rom, lib, libultra_rom, <dir>_<file>]` entries:
it archives those objects into build/lib/libultra_rom.a and writes build/ultralib_syms.txt, the
addresses of what they reference that nothing else names.

The flags are ultralib's own (lib/ultralib/makefiles/ido.mk, Makefile) for libultra_rom 2.0I:
IDO 5.3, -mips2 -o32 -non_shared, -O1 (os, debug, host, log, rmon and by default) or -O3 (libc,
sched, gu, mgu, sp, audio, rg, gt); .s files through cc at -O1 (-O2 in libc and mgu). Each file is
compiled in its own directory, as ultralib does, then made fit for GNU ld the way ultralib's
MODERN_LD path does (o32 ABI bit, no .mdebug).

`map` needs the local decompressed ROM (baserom.us.decompressed.z64). Relocated fields (jal
targets, %hi/%lo halves) are masked in the comparison.

Changelog: first public release (2026-10-20). Earlier history is kept privately.
"""
import os
import re
import struct
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import build_rom as build  # noqa: E402  (ROOT, the msys runtime environment, binutils)

ROOT = build.ROOT
fwd = lambda p: os.path.abspath(p).replace("\\", "/")  # the Windows recomp misreads backslashes
ULTRA = fwd(os.path.join(ROOT, "lib", "ultralib"))
# The game's libultra is 2.0I with some files from a later release (2.0J-L: their SI and
# controller files match only there). An object is named `dir/file.c` for 2.0I and `J:dir/file.c`
# for another version; the versions tried are VERSIONS, the first preferred on a tie.
BASE = "I"
VERSIONS = os.environ.get("ULTRALIB_VERSIONS", "IJKLP")
OUT_ROOT = fwd(os.path.join(ROOT, "build", "ultralib"))

# The game's release of ten files (Controller Pak, 64DD, PI manager) lies between 2.0I and 2.0J,
# which ultralib does not model. Their code is 2.0J's except for the edits
# below: lines as they were before 2.0J, and in pimgr.c one function 2.0J moved to initialize.c.
# That release is "P" (a patch release): `P:io/contpfs.c`, member P_io_contpfs. It is
# compiled as 2.0J from an edited copy under build/ultralib/P/src; lib/ultralib stays unedited.
# Each edit is (old text, new text, which occurrence of the old text); a missing one is an error.
PATCH = "P"
PATCHES = {
    # __osRepairPackId's failed re-read returns 2.0I's PFS_ERR_ID_FATAL (2.0J: PFS_ERR_DEVICE).
    "io/contpfs.c": [("#if BUILD_VERSION >= VERSION_J\n            return PFS_ERR_DEVICE;\n#else\n"
                      "            return PFS_ERR_ID_FATAL;\n#endif\n",
                      "            return PFS_ERR_ID_FATAL;\n", 0)],
    # No __osPfsGetStatus checks (2.0J added them): these end in `return 0;`, and the file
    # search reads each directory entry without one.
    "io/pfsfilestate.c": [("    ret = __osPfsGetStatus(pfs->queue, pfs->channel);\n    return ret;\n#else",
                           "    return 0;\n#else", 0)],
    "io/pfsreadwritefile.c": [("#if BUILD_VERSION >= VERSION_J\n"
                               "    ret = __osPfsGetStatus(pfs->queue, pfs->channel);\n"
                               "    return ret;\n#else\n    return 0;\n#endif", "    return 0;", 0)],
    "io/pfsnumfiles.c": [("#if BUILD_VERSION >= VERSION_J\n"
                          "    ret = __osPfsGetStatus(pfs->queue, pfs->channel);\n"
                          "    return ret;\n#else\n    return 0;\n#endif", "    return 0;", 0)],
    "io/pfssearchfile.c": [("#if BUILD_VERSION >= VERSION_J\n"
                            "        ERRCK(__osPfsGetStatus(pfs->queue, pfs->channel));\n#endif\n", "", 0)],
    # The track-mode C2 check (`bptr->errStatus`, the third LEO_ERROR_24) is LEO_ERROR_23.
    "io/leointerrupt.c": [("LEO_ERROR_24", "LEO_ERROR_23", 2)],
    # osPfsInitPak without 2.0J's RAM area test (__osPfsCheckRamArea, its call and definition).
    "io/pfsinitpak.c": [("#if BUILD_VERSION >= VERSION_J\n    ERRCK(__osPfsCheckRamArea(pfs));\n#endif\n", "", 0),
                        ("#if BUILD_VERSION >= VERSION_J\nstatic s32 __osPfsCheckRamArea(OSPfs* pfs);",
                         "#if 0\nstatic s32 __osPfsCheckRamArea(OSPfs* pfs);", 0),
                        ("#if BUILD_VERSION >= VERSION_J\nstatic s32 __osPfsCheckRamArea(OSPfs* pfs) {",
                         "#if 0\nstatic s32 __osPfsCheckRamArea(OSPfs* pfs) {", 0)],
    # osCreatePiManager fills 2.0J's two speed handles itself, from the PI's domain registers,
    # through a static function after it (2.0J does this in initialize.c's SPEED_PARAM_FUNC,
    # setting .type; this one sets .domain). Its name is ours; the game's is unknown.
    "io/pimgr.c": [("void osCreatePiManager(OSPri pri, OSMesgQueue* cmdQ, OSMesg* cmdBuf, s32 cmdMsgCnt) {",
                    "static void __osPiSetSpeedParam(void);\n\n"
                    "void osCreatePiManager(OSPri pri, OSMesgQueue* cmdQ, OSMesg* cmdBuf, s32 cmdMsgCnt) {", 0),
                   ("    if (__osPiDevMgr.active) {\n        return;\n    }\n",
                    "    if (__osPiDevMgr.active) {\n        return;\n    }\n    __osPiSetSpeedParam();\n", 0),
                   ("\n#ifndef _FINALROM\nstatic void ramromMain(void* arg) {",
                    "\nstatic void __osPiSetSpeedParam(void) {\n"
                    "    __Dom1SpeedParam.latency = IO_READ(PI_BSD_DOM1_LAT_REG);\n"
                    "    __Dom1SpeedParam.pulse = IO_READ(PI_BSD_DOM1_PWD_REG);\n"
                    "    __Dom1SpeedParam.pageSize = IO_READ(PI_BSD_DOM1_PGS_REG);\n"
                    "    __Dom1SpeedParam.relDuration = IO_READ(PI_BSD_DOM1_RLS_REG);\n"
                    "    __Dom1SpeedParam.domain = PI_DOMAIN1;\n\n"
                    "    __Dom2SpeedParam.latency = IO_READ(PI_BSD_DOM2_LAT_REG);\n"
                    "    __Dom2SpeedParam.pulse = IO_READ(PI_BSD_DOM2_PWD_REG);\n"
                    "    __Dom2SpeedParam.pageSize = IO_READ(PI_BSD_DOM2_PGS_REG);\n"
                    "    __Dom2SpeedParam.relDuration = IO_READ(PI_BSD_DOM2_RLS_REG);\n"
                    "    __Dom2SpeedParam.domain = PI_DOMAIN2;\n}\n"
                    "\n#ifndef _FINALROM\nstatic void ramromMain(void* arg) {", 0)],
    # corrupted_init's cache index takes the inode page as 2.0I did, without `& 0x7F`.
    "io/pfschecker.c": [("(tpage.inode_t.page & 0x7F)", "(tpage.inode_t.page)", 0)],
    # osLeoDiskInit sets up the handle without writing the PI's domain 2 timing registers.
    "io/leodiskinit.c": [("    IO_WRITE(PI_BSD_DOM2_LAT_REG, LeoDiskHandle.latency);\n"
                          "    IO_WRITE(PI_BSD_DOM2_PWD_REG, LeoDiskHandle.pulse);\n"
                          "    IO_WRITE(PI_BSD_DOM2_PGS_REG, LeoDiskHandle.pageSize);\n"
                          "    IO_WRITE(PI_BSD_DOM2_RLS_REG, LeoDiskHandle.relDuration);\n", "", 0)],
}


def split_q(q):
    """`J:io/x.c` -> ('J', 'io/x.c'); `io/x.c` -> (BASE, 'io/x.c')."""
    return tuple(q.split(":", 1)) if ":" in q else (BASE, q)
IDO53 = fwd(os.environ.get("IDO53", os.path.join(build.TOOLS, "ido-5.3", "cc" + build.EXE)))
OBJCOPY = build.tool_path(os.path.join(build.BINUTILS, "mips64-elf-objcopy" + build.EXE))
BASEROM = os.path.join(ROOT, "baserom.us.decompressed.z64")

MAIN_ROM, MAIN_VRAM, MAIN_DATA = 0x1060, 0x80000460, 0x250C0  # main's code, as in the yaml
O3_DIRS = ("libc", "sched", "gu", "mgu", "sp", "audio", "rg", "gt")
GBI = {"gu/parse_gbi": "-DF3D_GBI", "gu/us2dex_emu": None, "gu/us2dex2_emu": None,
       "sp/sprite": "-DF3D_GBI", "sp/spriteex": None, "sp/spriteex2": None}


def sources(version=BASE):
    """Every ultralib source file of a version, as `dir/name.ext` under src/, qualified by the
    version if not BASE (voice/ is the Japanese build's). PATCH has only its edited files."""
    if version == PATCH:
        return sorted(f"{PATCH}:{r}" for r in PATCHES)
    out = []
    for d, _, names in os.walk(os.path.join(ULTRA, "src")):
        rel = os.path.relpath(d, os.path.join(ULTRA, "src")).replace("\\", "/")
        if rel.split("/")[0] in ("voice", "."):
            continue
        out += [f"{rel}/{n}" for n in names if n.endswith((".c", ".s"))]
    return sorted(out if version == BASE else [f"{version}:{r}" for r in out])


def src_path(q):
    """A file's source: ultralib's, or for PATCH its edited copy (written by stage_patch)."""
    version, rel = split_q(q)
    return f"{OUT_ROOT}/{PATCH}/src/{rel}" if version == PATCH else f"{ULTRA}/src/{rel}"


def stage_patch(rel):
    """Write PATCHES' edited copy of a 2.0J file, if it changed."""
    with open(f"{ULTRA}/src/{rel}", "rb") as f:
        text = f.read().decode().replace("\r\n", "\n")
    for old, new, n in PATCHES[rel]:
        at = -1
        for _ in range(n + 1):
            at = text.find(old, at + 1)
            if at < 0:
                sys.exit(f"FAILED: {rel}: edit {old[:40]!r} #{n} not found")
        text = text[:at] + new + text[at + len(old):]
    out = src_path(f"{PATCH}:{rel}")
    os.makedirs(os.path.dirname(out), exist_ok=True)
    if os.path.exists(out):
        with open(out, "rb") as f:
            if f.read() == text.encode():
                return
    with open(out, "wb") as f:
        f.write(text.encode())


def command(q):
    """ido.mk's rule for one file of libultra_rom 2.0<version> (PATCH: 2.0J's)."""
    version, rel = split_q(q)
    if version == PATCH:
        version = "J"
    stem, ext = os.path.splitext(rel)
    top = stem.split("/")[0]
    old = version in "DEFGHI"  # ido.mk: these build at -O1, with -O3 for O3_DIRS; later ones -O2
    gbi = GBI.get(stem, "-DF3DEX_GBI")
    mips = ["-mips2", "-o32"]
    pic = ["-non_shared"]
    if stem in ("libc/ll", "libc/llbit", "libc/llcvt", "os/exceptasm"):
        mips = ["-mips3", "-32"]
    if stem == "log/delay":
        mips, pic = ["-mips1", "-o32"], ["-KPIC"]
    cpp = ["-D_MIPS_SZLONG=32", *([gbi] if gbi else []), f"-DBUILD_VERSION=VERSION_{version}",
           *pic, "-DNDEBUG", "-D_FINALROM"]
    inc = [f"-I{ULTRA}/include", f"-I{ULTRA}/include/compiler/ido", f"-I{ULTRA}/include/PR"]
    if split_q(q)[0] == PATCH:  # the copy's quoted includes, from the original's directory
        inc.append(f"-I{ULTRA}/src/{os.path.dirname(rel)}")
    if ext == ".s":
        opt = ["-O2"] if top in ("libc", "mgu") else ["-O1"]
        flags = ["-c", "-Wab,-r4300_mul", "-G", "0", "-nostdinc", "-woff", "516,649,838,712"]
    else:
        opt = ["-O1"] if old else ["-O2"]
        if top in ("debug", "host", "os", "rmon", "log"):
            opt = ["-O1"]
        if old and top in O3_DIRS:
            opt = ["-O3"]
        if stem == "os/initialize_isv":
            opt = ["-O2"]
        if stem in ("libc/ll", "libc/llbit", "libc/llcvt"):
            opt = ["-O1"]
        flags = ["-c", "-Wab,-r4300_mul", "-G", "0", "-nostdinc", "-Xcpluscomm", "-fullwarn",
                 "-woff", "516,649,838,712"]
    return [IDO53, *flags, *mips, *cpp, *opt, src_path(q), *inc]


def obj_path(q):
    version, rel = split_q(q)
    return f"{OUT_ROOT}/{version}/{os.path.splitext(rel)[0]}.o"


def compile_one(q):
    out = obj_path(q)
    if split_q(q)[0] == PATCH:
        stage_patch(split_q(q)[1])
    src = src_path(q)
    if os.path.exists(out) and os.path.getmtime(out) > os.path.getmtime(src):
        return True
    os.makedirs(os.path.dirname(out), exist_ok=True)
    r = subprocess.run([*command(q), "-o", out], cwd=os.path.dirname(src), capture_output=True,
                       text=True, env=build.ENV)
    if r.returncode or not os.path.exists(out):
        return False
    subprocess.run([sys.executable, os.path.join(ULTRA, "tools", "set_o32abi_bit.py"), out],
                   capture_output=True)
    subprocess.run([OBJCOPY, "--remove-section", ".mdebug", out], capture_output=True)
    return True


def cmd_build():
    for v in VERSIONS:
        failed = [q for q in sources(v) if not compile_one(q)]
        print(f"2.0{v}: {len(sources(v)) - len(failed)} compiled, {len(failed)} failed")
        for q in failed:
            print("  FAILED", q)


# --- ELF (32-bit big-endian relocatable, as IDO writes it) ---------------------------------------

def read_elf(path):
    """{'text': bytes, 'relocs': [(offset, type, name, shndx)], 'syms': {name: (value, shndx,
    bind)}, 'sizes': {section: size}, 'funcs': {addresses of the functions in .text}}"""
    with open(path, "rb") as f:
        d = f.read()
    shoff, = struct.unpack_from(">I", d, 0x20)
    shnum, shstrndx = struct.unpack_from(">HH", d, 0x30)
    secs = [struct.unpack_from(">IIIIIIIIII", d, shoff + i * 40) for i in range(shnum)]
    names_off = secs[shstrndx][4]
    name = lambda off, base: d[base + off:d.index(b"\0", base + off)].decode()
    names = [name(s[0], names_off) for s in secs]
    out = {"text": b"", "relocs": [], "syms": {}, "sizes": {}, "bytes": {}, "srelocs": {}}
    symtab = next((i for i, s in enumerate(secs) if s[1] == 2), None)
    syms = []
    if symtab is not None:
        s = secs[symtab]
        strbase = secs[s[6]][4]
        for i in range(s[5] // 16):
            st_name, value, size, info, other, shndx = struct.unpack_from(">IIIBBH", d, s[4] + i * 16)
            syms.append((name(st_name, strbase), value, shndx, info >> 4, info & 0xF))
    for i, s in enumerate(secs):
        out["sizes"][names[i]] = s[5]
        if names[i] == ".text":
            out["text"] = d[s[4]:s[4] + s[5]]
        if names[i] in (".data", ".rodata") and s[1] == 1:
            out["bytes"][names[i]] = d[s[4]:s[4] + s[5]]
    out["value"] = {}  # (name, shndx) -> the symbol's value, for relocations against it
    for s in secs:
        if s[1] != 9:  # SHT_REL
            continue
        target = names[s[7]]
        for j in range(s[5] // 8):
            off, info = struct.unpack_from(">II", d, s[4] + j * 8)
            nm, value, shndx, _, typ = syms[info >> 8]
            if typ == 3:  # a section symbol: the reference is into this object's own section
                nm = names[shndx]
            if target == ".text":
                out["relocs"].append((off, info & 0xFF, nm, shndx))
                out["value"][(nm, shndx)] = value
            else:
                out["srelocs"].setdefault(target, []).append((off, info & 0xFF))
    out["secname"] = names
    out["funcs"] = set()
    for nm, value, shndx, bind, typ in syms:
        if nm and typ in (0, 1, 2) and shndx != 0:
            out["syms"][nm] = (value, shndx, bind)
            if typ in (0, 2) and names[shndx] == ".text" and not nm.startswith((".", "$")):
                out["funcs"].add(value)
    return out


MASK = {4: 0xFC000000, 5: 0xFFFF0000, 6: 0xFFFF0000, 2: 0x00000000, 7: 0xFFFF0000}


def masked_words(text, relocs):
    words = list(struct.unpack(f">{len(text) // 4}I", text))
    masks = [0xFFFFFFFF] * len(words)
    for off, typ, _, _ in relocs:
        masks[off // 4] &= MASK.get(typ, 0)
    return words, masks


MAIN_END = 0x3F910                       # main's data ends here; its bss follows
MAIN_BSS, MAIN_BSS_END = 0x8003ED10, 0x8003ED10 + 0x88600
DATA_SECS = (".data", ".rodata", ".bss")


def sext16(v):
    return (v & 0xFFFF) - ((v & 0x8000) << 1)


def bases(e, rom, at):
    """{section: vram} for the object's .data/.rodata/.bss in the game. From the original
    %hi/%lo pairs that reference them (their address less the object's addend and the symbol's
    value); a section its code never references is found by its bytes in main's data (pointer
    words masked), if they are there exactly once."""
    obj = struct.unpack(f">{len(e['text']) // 4}I", e["text"])
    org = struct.unpack_from(f">{len(obj)}I", rom, at)
    names, rel, found = e["secname"], e["relocs"], {}
    for i, (off, typ, nm, sh) in enumerate(rel):
        if typ != 5 or sh == 0 or names[sh] not in DATA_SECS:
            continue
        lo = next((o // 4 for o, t, n, s in rel[i + 1:] if t == 6 and n == nm and s == sh), None)
        if lo is None:
            continue
        k = off // 4
        addr = ((org[k] & 0xFFFF) << 16) + sext16(org[lo])
        addend = ((obj[k] & 0xFFFF) << 16) + sext16(obj[lo])
        value = 0 if nm == names[sh] else e["value"][(nm, sh)]
        found.setdefault(names[sh], set()).add((addr - addend - value) & 0xFFFFFFFF)
    out = {s: v.pop() for s, v in found.items() if len(v) == 1}
    for s in (".data", ".rodata"):
        data = e["bytes"].get(s)
        if s in out or not data:
            continue
        words = list(struct.unpack(f">{len(data) // 4}I", data))
        masks = [0xFFFFFFFF] * len(words)
        for off, typ in e["srelocs"].get(s, []):
            masks[off // 4] = 0
        n = (MAIN_END - MAIN_DATA) // 4
        region = struct.unpack_from(f">{n}I", rom, MAIN_DATA)
        hits = [i for i in range(0, n - len(words) + 1, 4)
                if all((region[i + k] ^ w) & m == 0 for k, (w, m) in enumerate(zip(words, masks)))]
        if len(hits) == 1:
            out[s] = MAIN_VRAM + MAIN_DATA + hits[0] * 4 - MAIN_ROM
    return out


def placed(e, b):
    """Every section the object has is found, and where it belongs (data in main's data, bss in
    main's bss)."""
    for s in DATA_SECS:
        if not e["sizes"].get(s):
            continue
        lo, hi = ((MAIN_BSS, MAIN_BSS_END) if s == ".bss" else
                  (MAIN_VRAM + MAIN_DATA - MAIN_ROM, MAIN_BSS))
        if s not in b or not lo <= b[s] < hi:
            return False
    return True


def matches():
    """[(rom, end, rel, elf, placed, ambiguous, bases)] for every object whose .text is in main's
    code. placed: every section it has is found (bases()), so it can be linked whole.
    ambiguous: the same code matches at several places, or another object matches here."""
    with open(BASEROM, "rb") as f:
        rom = f.read()
    n = (MAIN_DATA - MAIN_ROM) // 4
    code = struct.unpack_from(f">{n}I", rom, MAIN_ROM)
    out, seen = [], set()
    for v in VERSIONS:
        for q in sources(v):
            path = obj_path(q)
            if not os.path.exists(path):
                continue
            e = read_elf(path)
            if not e["text"]:
                continue
            words, masks = masked_words(e["text"], e["relocs"])
            hits = [MAIN_ROM + i * 4 for i in range(0, n - len(words) + 1, 4)  # 16-aligned objects
                    if all((code[i + k] ^ w) & m == 0 for k, (w, m) in enumerate(zip(words, masks)))]
            for h in hits:
                if (h, split_q(q)[1]) in seen:  # the same file matched in an earlier version
                    continue
                seen.add((h, split_q(q)[1]))
                b = bases(e, rom, h)
                out.append((h, h + len(e["text"]), q, e, placed(e, b), len(hits) > 1, b))
    starts = {}
    for m in out:
        starts.setdefault(m[0], []).append(m)
    return sorted(((*m[:5], m[5] or len(starts[m[0]]) > 1, m[6]) for m in out), key=lambda m: m[0])


def member_name(q):
    """The archive member for an object: `dir_file`, or `J_dir_file` for another version."""
    version, rel = split_q(q)
    name = "_".join(os.path.splitext(rel)[0].split("/"))
    return name if version == BASE else f"{version}_{name}"


def cmd_map():
    for at, end, rel, e, ok, amb, b in matches():
        secs = " ".join(f"{s[1:]}={e['sizes'][s]:X}@{b.get(s, 0):08X}" for s in DATA_SECS
                        if e["sizes"].get(s))
        print(f"{at:X} {end:X} {os.path.splitext(rel)[0]} {secs or '-'}"
              + ("" if ok else " UNPLACED") + (" AMBIGUOUS" if amb else ""))


def known_addresses(chosen, rom):
    """name -> vram: splat's names, and what the chosen objects define (code at its ROM place,
    data at its placed section) and reference (read back from the ROM)."""
    known = dict(symbol_addrs())
    for at, end, rel, e, ok, amb, b in chosen:
        for n, (value, shndx, bind) in e["syms"].items():
            sec = e["secname"][shndx] if shndx < len(e["secname"]) else ""  # not ABS/COMMON
            if bind in (1, 2) and sec == ".text":
                known.setdefault(n, MAIN_VRAM + at - MAIN_ROM + value)
            elif bind in (1, 2) and sec in b:
                known.setdefault(n, b[sec] + value)
        for n, a in targets(e, rom, at).items():
            known.setdefault(n, a)
    return known


def score(m, known, rom):
    """How well a candidate agrees with the known addresses: +1 for each of its functions or
    references at the address already known for that name, -10 for each at another."""
    at, end, rel, e, ok, amb, b = m
    s = 0
    for n, (value, shndx, bind) in e["syms"].items():
        if bind in (1, 2) and shndx < len(e["secname"]) and e["secname"][shndx] == ".text" \
                and n in known:
            s += 1 if known[n] == MAIN_VRAM + at - MAIN_ROM + value else -10
    for n, a in targets(e, rom, at).items():
        if n in known:
            s += 1 if known[n] == a else -10
    return s


def choose(all_matches):
    """The objects to link: every placed, unambiguous match, then, among code that matches
    several objects, the one that best agrees with what is known (a C file over an .s file on a
    tie). Code no candidate clearly wins stays assembly."""
    with open(BASEROM, "rb") as f:
        rom = f.read()
    cand = [m for m in all_matches if m[4] and m[0] >= 0xFB20]
    chosen = [m for m in cand if not m[5]]
    while True:
        taken = [(m[0], m[1]) for m in chosen]
        free = [m for m in cand if m not in chosen and
                not any(m[0] < t1 and t0 < m[1] for t0, t1 in taken)]
        if not free:
            return chosen
        known = known_addresses(chosen, rom)
        best = {}
        for m in free:
            key = (score(m, known, rom), m[2].endswith(".c"),
                   "mgu/" not in m[2],  # gu's C over mgu's, when both match (guRotate)
                   m[1] - m[0])
            if m[0] not in best or key > best[m[0]][0]:
                best[m[0]] = (key, [m])
            elif key == best[m[0]][0]:
                best[m[0]][1].append(m)
        picks = [ms[0] for key, ms in best.values() if len(ms) == 1 and key[0] >= 0]
        if not picks:
            return chosen
        chosen += picks[:1]  # one at a time: each pick adds to what is known


def cmd_yaml():
    """Main's subsegments from 0xFB20 on, with every chosen object linked: its .text in the
    code, its .data/.rodata cut out of the data, and its .bss out of the bss (the three blocks
    to put in the yaml)."""
    with open(YAML) as f:
        text = f.read()
    old = [int(a, 16) for a in re.findall(r"^\s*- \[0x([0-9A-F]+), (?:asm|lib)\b", text, re.M)]
    old = [a for a in old if 0xFB20 <= a < MAIN_DATA]
    ok = choose(matches())
    libs = {at: (end, member_name(rel)) for at, end, rel, e, _, _, _ in ok}
    cuts = set(old)
    for at, (end, _) in libs.items():
        cuts = {c for c in cuts if not at < c < end} | {at, end}
    for c in sorted(cuts):
        if c < MAIN_DATA:
            print(f"      - [0x{c:X}, lib, libultra_rom, {libs[c][1]}]" if c in libs else
                  f"      - [0x{c:X}, asm]")
    print()
    # splat's linker script puts a segment's .data entries before its .rodata ones, so a .rodata
    # piece is ordered with the .data ones (linker_section_order) to stay where the yaml puts it.
    # (Typing the filler rodata instead makes splat read jump tables in it, pointing at local
    # labels inside functions that other objects cannot see.)
    pieces = sorted((b[s] - MAIN_VRAM + MAIN_ROM, e["sizes"][s], member_name(rel), s)
                    for _, _, rel, e, _, _, b in ok for s in (".data", ".rodata") if e["sizes"].get(s))
    pos = MAIN_DATA
    for rom_at, size, m, s in pieces:
        if rom_at > pos:
            print(f"      - [0x{pos:X}, data]")
        print(f"      - [0x{rom_at:X}, lib, libultra_rom, {m}, {s}]" if s == ".data" else
              f"      - {{ start: 0x{rom_at:X}, type: lib, name: libultra_rom, object: {m}, "
              f"section: .rodata, linker_section_order: .data }}")
        pos = rom_at + size
    print(f"      - [0x{pos:X}, data]")
    print()
    pos = MAIN_BSS  # bss pieces are named by vram: splat names them by ROM offset, the same for all
    for v, size, m in sorted((b[".bss"], e["sizes"][".bss"], member_name(rel))
                             for _, _, rel, e, _, _, b in ok if e["sizes"].get(".bss")):
        if v > pos:
            print(f"      - {{ type: bss, vram: 0x{pos:X}, name: {pos:X} }}")
        print(f"      - {{ type: lib, vram: 0x{v:X}, name: libultra_rom, object: {m}, "
              f"section: .bss }}")
        pos = v + size
    print(f"      - {{ type: bss, vram: 0x{pos:X}, name: {pos:X} }}")


# --- linking: the yaml's `[rom, lib, libultra_rom, <dir>_<file>]` entries --------------------------

YAML = os.path.join(ROOT, f"{build.NAME}.yaml")
ARCHIVE = "build/lib/libultra_rom.a"  # the yaml's lib_path is build/lib
SYMS = "build/ultralib_syms.txt"
AR = build.tool_path(os.path.join(build.BINUTILS, "mips64-elf-ar" + build.EXE))


def members():
    """(rom, member) for each library object the yaml links, member as `<dir>_<file>`: its .text
    entry, and for an object with no code (the video modes vimodes/, gu/libm_vals.s) its first
    data entry, in list form `[0x.., lib, libultra_rom, X, .data]` or dict form
    `{ start: 0x.., type: lib, name: libultra_rom, object: X, ... }`."""
    with open(YAML) as f:
        text = f.read()
    ms = [(int(a, 16), m) for a, m in
          re.findall(r"\[0x([0-9A-Fa-f]+),\s*lib,\s*libultra_rom,\s*(\w+)\s*\]", text)]
    have = {m for _, m in ms}
    for g in re.findall(r"\[0x([0-9A-Fa-f]+),\s*lib,\s*libultra_rom,\s*(\w+),\s*\.\w+\s*\]"
                        r"|start:\s*0x([0-9A-Fa-f]+),\s*type:\s*lib,\s*name:\s*libultra_rom,"
                        r"\s*object:\s*(\w+)", text):
        a, m = (g[0], g[1]) if g[0] else (g[2], g[3])
        if m not in have:
            ms.append((int(a, 16), m))
            have.add(m)
    return ms


def member_rel(member):
    """member_name's inverse: the qualified source of an archive member."""
    version = BASE
    if re.match(r"^[A-Z]_", member):
        version, member = member[0], member[2:]
    d, n = member.split("_", 1)
    for ext in (".c", ".s"):
        if os.path.exists(f"{ULTRA}/src/{d}/{n}{ext}"):
            return f"{d}/{n}{ext}" if version == BASE else f"{version}:{d}/{n}{ext}"
    sys.exit(f"FAILED: no ultralib source for {member}")


def symbol_addrs():
    with open(os.path.join(ROOT, "symbol_addrs.txt")) as f:
        return {m.group(1): int(m.group(2), 16)
                for m in re.finditer(r"^\s*(\w+)\s*=\s*0x([0-9A-Fa-f]+)", f.read(), re.M)}


def targets(e, rom, at):
    """name -> address for each undefined symbol the object's .text relocates, read back from the
    original instructions at ROM offset `at` (minus the object's own addend)."""
    obj = struct.unpack(f">{len(e['text']) // 4}I", e["text"])
    org = struct.unpack_from(f">{len(obj)}I", rom, at)
    vram = MAIN_VRAM + at - MAIN_ROM
    sext = lambda v: (v & 0xFFFF) - ((v & 0x8000) << 1)
    out, relocs = {}, [r for r in e["relocs"] if r[3] == 0]
    for i, (off, typ, name, _) in enumerate(relocs):
        k = off // 4
        if typ == 4:
            hi = (vram + off + 4) & 0xF0000000
            out[name] = (hi | (org[k] & 0x3FFFFFF) << 2) - ((obj[k] & 0x3FFFFFF) << 2)
        elif typ == 5:
            lo = next((o // 4 for o, t, n, _ in relocs[i + 1:] if t == 6 and n == name), None)
            if lo is not None:
                out[name] = ((org[k] & 0xFFFF) << 16) + sext(org[lo]) - \
                            (((obj[k] & 0xFFFF) << 16) + sext(obj[lo]))
    return {n: a & 0xFFFFFFFF for n, a in out.items()}


def link_inputs():
    """For canon-decompilation-tools/build_rom.py: compile the linked members, archive them, and write the symbols they
    need (those no member defines and splat does not name). Returns the extra ld arguments."""
    ms = members()
    if not ms:
        return []
    with open(BASEROM, "rb") as f:
        rom = f.read()
    tmp = f"{OUT_ROOT}/members"
    os.makedirs(tmp, exist_ok=True)
    os.makedirs(os.path.join(ROOT, "build", "lib"), exist_ok=True)
    paths, elves = [], []
    build.parallel([lambda rel=member_rel(m): compile_one(rel) for _, m in ms])  # (907); failures below
    for at, m in ms:
        rel = member_rel(m)
        if not compile_one(rel):
            sys.exit(f"FAILED: ultralib {rel}")
        with open(obj_path(rel), "rb") as src, open(f"{tmp}/{m}.o", "wb") as dst:
            dst.write(src.read())
        paths.append(f"{tmp}/{m}.o")
        elves.append((at, read_elf(obj_path(rel))))
    if os.path.exists(os.path.join(ROOT, ARCHIVE)):
        os.remove(os.path.join(ROOT, ARCHIVE))
    subprocess.run([AR, "rcs", ARCHIVE, *paths], cwd=ROOT, check=True)
    defined = {n for _, e in elves for n, (_, _, bind) in e["syms"].items() if bind in (1, 2)}
    named = symbol_addrs()
    ranges = [(MAIN_VRAM + at - MAIN_ROM, MAIN_VRAM + at - MAIN_ROM + len(e["text"])) for at, e in elves]
    inside = lambda a: any(s <= a < t for s, t in ranges)
    want = {}
    for at, e in elves:
        want.update(targets(e, rom, at))
    for n, a in named.items():  # splat's names for code now inside a member, if the member lacks them
        if inside(a):
            want[n] = a
    with open(os.path.join(ROOT, SYMS), "w") as f:
        f.write("/* Written by canon-decompilation-tools/build_libultra.py: symbols the libultra members need. */\n")
        for n, a in sorted(want.items(), key=lambda x: x[1]):
            if n not in defined and (n not in named or inside(named[n])):
                f.write(f"{n} = 0x{a:08X};\n")
    return ["-T", SYMS, "--whole-archive", ARCHIVE, "--no-whole-archive"]


def cmd_diff(at, rel):
    """Why an object does not match at a ROM offset: the sizes, and the words that differ
    (relocated fields masked)."""
    if not compile_one(rel):
        sys.exit(f"FAILED: ultralib {rel}")
    with open(BASEROM, "rb") as f:
        rom = f.read()
    e = read_elf(obj_path(rel))
    words, masks = masked_words(e["text"], e["relocs"])
    org = struct.unpack_from(f">{len(words)}I", rom, at)
    bad = [k for k, (w, m, o) in enumerate(zip(words, masks, org)) if (w ^ o) & m]
    print(f"{rel}: {len(words) * 4:X} bytes; {len(bad)} words differ at {at:X}")
    for k in bad[:12]:
        print(f"  +{k * 4:04X} rom {org[k]:08X} obj {words[k]:08X}")


def cmd_near(at, end):
    """The objects, in every version, whose .text is as long as ROM [at, end), ranked by how
    many words differ: for code no object matches exactly."""
    with open(BASEROM, "rb") as f:
        rom = f.read()
    org = struct.unpack_from(f">{(end - at) // 4}I", rom, at)
    ranked = []
    for v in VERSIONS:
        for q in sources(v):
            if not os.path.exists(obj_path(q)):
                continue
            e = read_elf(obj_path(q))
            if len(e["text"]) != end - at:
                continue
            words, masks = masked_words(e["text"], e["relocs"])
            ranked.append((sum(1 for w, m, o in zip(words, masks, org) if (w ^ o) & m), q))
    for n, q in sorted(ranked)[:4]:
        print(f"{at:X}-{end:X} {q}: {n} words differ")
    if not ranked:
        print(f"{at:X}-{end:X}: no object that long")


def main():
    args = sys.argv[1:]
    if args == ["build"]:
        return cmd_build()
    if len(args) == 3 and args[0] == "near":
        return cmd_near(int(args[1], 16), int(args[2], 16))
    if len(args) == 3 and args[0] == "diff":
        return cmd_diff(int(args[1], 16), args[2])
    if args == ["map"]:
        return cmd_map()
    if args == ["yaml"]:
        return cmd_yaml()
    sys.exit(__doc__)


if __name__ == "__main__":
    main()
