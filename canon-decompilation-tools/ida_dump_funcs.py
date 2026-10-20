# ida_dump_funcs.py — headless (idalib) disassembly dumper. ONE canonical dumper, not a fork per
# game: the database and the target list are both inputs.
#
#   IDB=<path to .i64>  py canon-decompilation-tools/ida_dump_funcs.py 80019860 80018C60 ...
#
# First written for GameCube (PowerPC) work, then given a MIPS mode for this project.
# ⭐ MIPS (N64) IS SUPPORTED ALONGSIDE PPC. The architecture comes from the database (gate below) and
# picks the mnemonic sets for stores:/loads:/hilo:/jpt:. Everything else is ISA-neutral already.
#   Build an N64 database from one code segment (raw, big-endian MIPS, loaded at its VRAM):
#     dd if=baserom.us.z64 of=seg.bin bs=1 skip=$((ROMSTART)) count=$((ROMEND-ROMSTART))
#     "D:/IDA Pro 9.1.250226/IDA/idat.exe" -B -pmipsb -b<VRAM>>4 seg.bin      (-b is VRAM / 16, hex)
#   MIPS specifics: stores:/loads: skip $sp-relative accesses (the MIPS mirror of r1); hilo: resolves
#   lui + addiu/addi/daddiu/ori, lui + load/store displacement, and IDA's combined la/li macros; jpt:
#   counts `jr` on any register but $ra (a return) and `jalr`.
#
# MODES (a bare hex address decompiles/dumps that function):
#   owner:A[,B,..]      ⭐ WHICH FUNCTION CONTAINS THIS ADDRESS -- entry, name, size, +offset.
#                        From a profiler sample or a crash PC to its function's entry. Says NOT IN
#                        ANY FUNCTION out loud.
#   range:A:B            raw disassembly, incl. code IDA left outside any function
#   blocks:ADDR          basic blocks
#   callers:ADDR         who calls it
#   xrefsto:ADDR         every reader of an address — a table's extent and column count
#   data:ADDR:LEN        big-endian u32 view (LEN is a LENGTH, not an end address)
#   jpt:A:B              indirect jumps and their switch geometry, then named jpt_*/def_* tables —
#                        an opcode dispatch without guessing
#   imm:A:B:PAT          instructions whose disassembly contains PAT
#   callees:ADDR[:DEPTH] ⭐ the FORWARD call chain — `callers:` answers "who reaches this"; this
#                        answers "what does this reach", in one call.
#   bytes:PAT[:A:B]      byte-pattern search, hex or 'ASCII' — finds a magic/header/pointer that has
#                        no symbol and no xref.
#   strs:A:B             ⭐ STRING LITERALS referenced from a code range, with the referencing
#                        function — how you identify an unnamed function: BY WHAT IT PRINTS.
#   decomp:ADDR[:PAT]    pseudocode; with PAT, only the lines AROUND each match. ⛔ Hex-Rays does not
#                        run on every database: on some it throws, on others the plugin initialises
#                        and `decompile()` then returns None on real function starts. The tool says
#                        which. ⭐ `range:`/`forcerange:`/`blocks:` always work.
#   forcerange:A:B       range, FORCING undefined bytes to code first: `range:` steps by next_head
#                        and skips a region IDA never decoded.
#   decode:A:B           ⛔ WRITES: forces the range to code, makes functions there, SAVES the IDB
#   mkfuncs:A[,B..]      ⛔ WRITES: a function starting at each address (ending any IDA function
#                        that runs over it), SAVES the IDB. For the decomp's function starts
#   export:OUT.json      the database's facts as JSON for the other canon tools
#   hilo:A:B:TLO:THI     ⭐ lis+lo PAIRS whose EFFECTIVE address lands in [TLO,THI] — "WHAT TOUCHES
#                        THIS ADDRESS?" PPC builds an address from two halves and NEITHER carries an
#                        xref, so xref-based search cannot see it.
#                        ⛔⛔ IT ONLY SEES CODE IDA ALREADY DECODED. It walks by next_head, exactly
#                        like `range:` does — the reason `forcerange:` exists — so every undecoded
#                        region is INVISIBLE to it and comes back as a clean "0 references". On one
#                        GameCube database that was 17% of .text. ⛔ A ZERO FROM THIS MODE IS NOT A
#                        NEGATIVE until you have measured coverage (`coverage:`), and treat 0 as
#                        "my search did not look there".
#                        ⚠ Its sibling trap: a raw-byte search that caps the lis→addi gap (at 5
#                        instructions) also returns 0. Metrowerks schedules the halves far apart —
#                        40 was still not enough on one binary.
#   stores:A:B:DLO:DHI   ⭐ EVERY STORE whose displacement lands in [DLO,DHI] — "WHAT WRITES THIS
#                        STRUCT FIELD?" Indexed stores (stwx/sthx/stbx) are printed too, flagged,
#                        because a write through a register offset is exactly how a field write
#                        hides from a displacement search. See the fieldstores() docstring for why
#                        xrefs cannot answer this question.
#
# For every instruction we print: address, disasm, the resolved call target, and — the part that
# matters for table extraction — the DATA address of any operand that refers to memory, because
# a scene-id -> index map is a table and `lis/addi` hi-lo pairs are how PPC names one.
#
# Changelog: first public release (2026-10-20). Earlier history is kept privately.
import os, sys, re

# ⛔ THE TOOL USED TO CRASH ON ITS OWN ERROR MESSAGE. Windows consoles default to cp1252, which
# cannot encode the ⛔/⭐ this file prints — so `owner:` on an address IDA has not decoded died with
# a UnicodeEncodeError *instead of* printing "NOT IN ANY FUNCTION". The failure looked like a broken
# tool rather than a real answer about the database, which is the same class as reading an empty
# search result as a negative. Force UTF-8 on both streams; harmless where it already is.
for _s in (sys.stdout, sys.stderr):
    try:    _s.reconfigure(encoding="utf-8", errors="replace")
    except Exception: pass
os.environ.setdefault("IDADIR", r"D:\IDA Pro 9.1.250226\IDA")

# ⛔ `import idapro` DIES WITH ModuleNotFoundError THE MOMENT A PROJECT VENV IS ACTIVE. idalib is
# installed against the SYSTEM python (IDA's own py-activate-idalib.py writes a .pth there), so any
# venv in front on PATH shadows it — and the failure reads as "IDA is not installed" when the truth
# is "the wrong interpreter is in front". This tool worked minutes before it did not, from the same
# directory, which is exactly how much time that costs.
# ⭐ IDADIR already says where IDA lives, so derive the path rather than ask the environment for it.
_idalib_py = os.path.join(os.environ["IDADIR"], "idalib", "python")
if os.path.isdir(_idalib_py) and _idalib_py not in sys.path:
    sys.path.insert(0, _idalib_py)

IDB = os.environ.get("IDB")
if not IDB:
    print("set IDB=<path to .i64>"); sys.exit(2)

import idapro
rc = idapro.open_database(IDB, False)      # False = do not run auto-analysis again
if rc != 0:
    print("open_database rc", rc); sys.exit(1)

SAVE_DB = [False]   # set True only by `decode:` and `mkfuncs:`; every other mode leaves the IDB alone
import idautils, ida_funcs, ida_lines, ida_bytes, ida_xref, ida_name, ida_segment, ida_ua, ida_ida, ida_auto

# ⛔⛔ ENDIANNESS GATE — REFUSE, DO NOT WARN. GameCube is big-endian PPC and `-pPPC` DOES NOT SET IT:
# a database created without `inf_set_be(True)` before auto-analysis disassembles every instruction
# byte-swapped. ⭐ THE FAILURE MODE IS THE DANGEROUS ONE — it does not error, it produces PLAUSIBLE
# NONSENSE: real-looking mnemonics at real-looking addresses that mean nothing. Every address this
# tool reported would be worthless and nothing would say so.
_proc = ida_ida.inf_get_procname()
# big-endian MIPS (N64, `-pmipsb`) is accepted too. The same two halves are asserted —
# big-endian AND a known processor — and ARCH picks every ISA-specific mnemonic set below.
ARCH = "ppc" if _proc == "PPC" else ("mips" if _proc.lower().startswith("mips") else None)
if not ida_ida.inf_is_be() or ARCH is None:
    # ⛔ BOTH halves are asserted. The processor half matters as much as the byte order: a database
    # opened with the WRONG PROCESSOR disassembles PPC words as some other ISA and, like the
    # endianness case, does not error — it emits confident nonsense.
    print("⛔ REFUSING: %s is %s/%s. GameCube needs big-endian PPC; N64 needs big-endian MIPS (mipsb).\n"
          "   A database created without inf_set_be(True) before auto_wait, or with the wrong\n"
          "   processor, disassembles into byte-swapped or wrong-ISA garbage that still LOOKS like\n"
          "   valid code — every address reported from it would be worthless. Recreate the IDB."
          % (IDB, "BE" if ida_ida.inf_is_be() else "LE", _proc))
    idapro.close_database(False)
    sys.exit(3)

import ida_idp

def _reg(names, fallback):
    """A register NUMBER by name, asked of the processor module rather than assumed."""
    for n in names:
        r = ida_idp.str2reg(n)
        if r is not None and r >= 0:
            return r
    return fallback

# The stack register whose displacement accesses are frame locals, never struct fields.
SP_REG = 1 if ARCH == "ppc" else _reg(("sp", "$sp"), 29)
SP_TEXT = "(r1)" if ARCH == "ppc" else "($sp)"
RA_REG = _reg(("ra", "$ra"), 31) if ARCH == "mips" else None

STORE_OPS = {"ppc": ("stw", "sth", "stb", "stwu", "sthu", "stbu", "stfs", "stfd", "stfsu", "stfdu"),
             "mips": ("sb", "sh", "sw", "sd", "swl", "swr", "sdl", "sdr", "swc1", "sdc1", "sc", "scd")}[ARCH]
STORE_IDX = {"ppc": ("stwx", "sthx", "stbx", "stfsx", "stfdx"), "mips": ()}[ARCH]
LOAD_OPS = {"ppc": ("lwz", "lhz", "lha", "lbz", "lwzu", "lhzu", "lhau", "lbzu", "lfs", "lfd", "lfsu", "lfdu"),
            "mips": ("lb", "lbu", "lh", "lhu", "lw", "lwu", "ld", "lwl", "lwr", "ldl", "ldr", "lwc1", "ldc1",
                     "ll", "lld")}[ARCH]
LOAD_IDX = {"ppc": ("lwzx", "lhzx", "lhax", "lbzx", "lfsx", "lfdx"), "mips": ()}[ARCH]
is_float_store = (lambda m: m[:3] == "stf") if ARCH == "ppc" else (lambda m: m in ("swc1", "sdc1"))
is_float_load = (lambda m: m[:2] == "lf") if ARCH == "ppc" else (lambda m: m in ("lwc1", "ldc1"))

def disasm(ea):
    return ida_lines.generate_disasm_line(ea, ida_lines.GENDSM_REMOVE_TAGS) or ""

def mnem(ea):
    """The instruction's canonical mnemonic, DECODED — never split out of the printed line.

    ⛔⛔ THE BUG THIS EXISTS TO KILL. Both `stores:` and `jpt:` took `disasm(ea).split()[0].lower()`.
    IDA appends its auto-comment WITH NO SPACE, so a switch dispatch prints as
    `bctr# switch jump` and split() yields `bctr#` — the jump-table scanner walked a 1 MB range and
    reported ONE indirect jump while ten named jump tables sat inside it.
    ⭐ Same lesson as reading `o_displ` instead of regexing `0xNN(`: ask IDA what the instruction IS,
    do not parse what it PRINTS."""
    ins = ida_ua.insn_t()
    if ida_ua.decode_insn(ins, ea):
        m = ida_ua.ua_mnem(ea)
        if m:
            return m.strip().lower()
    d = disasm(ea) or ""
    return re.split(r"[^A-Za-z.]", d.strip(), maxsplit=1)[0].lower() if d else ""


def dump(ea):
    f = ida_funcs.get_func(ea)
    if not f:
        print("\n===== NO FUNCTION AT %08X =====" % ea); return
    nm = ida_funcs.get_func_name(f.start_ea)
    print("\n===== %s  %08X..%08X  (%d bytes) =====" % (nm, f.start_ea, f.end_ea, f.end_ea - f.start_ea))
    for h in idautils.FuncItems(f.start_ea):
        ann = []
        for x in idautils.XrefsFrom(h):
            if x.type in (ida_xref.fl_CN, ida_xref.fl_CF):
                ann.append("-> " + (ida_name.get_name(x.to) or "%08X" % x.to))
            elif x.type in (ida_xref.dr_R, ida_xref.dr_W, ida_xref.dr_O):
                # the data address an operand names — this is what finds tables
                dn = ida_name.get_name(x.to)
                ann.append("[%08X%s]" % (x.to, (" " + dn) if dn else ""))
        print("  %08X  %-52s %s" % (h, disasm(h), "  ".join(ann)))

def callers(ea):
    print("\n----- callers of %08X -----" % ea)
    for x in idautils.XrefsTo(ea):
        if x.type in (ida_xref.fl_CN, ida_xref.fl_CF):
            fn = ida_funcs.get_func(x.frm)
            print("  %08X  in %s   %s" % (
                x.frm, ida_funcs.get_func_name(fn.start_ea) if fn else "?", disasm(x.frm)))

def xrefsto(ea):
    """Every reader of an address — how you learn a table's extent and its column count."""
    print("\n----- xrefs to %08X -----" % ea)
    for x in idautils.XrefsTo(ea):
        fn = ida_funcs.get_func(x.frm)
        print("  %08X  in %-16s type=%d  %s" % (
            x.frm, ida_funcs.get_func_name(fn.start_ea) if fn else "?", x.type, disasm(x.frm)))

def data(ea, n):
    """u32 view. ⛔ BE database: get_bytes gives file order, so unpack big-endian explicitly."""
    import struct
    print("\n----- data %08X .. %08X -----" % (ea, ea + n))
    b = ida_bytes.get_bytes(ea, n)
    if not b:
        print("  get_bytes FAILED (unmapped?)"); return
    for i in range(0, len(b) - 3, 4):
        v = struct.unpack(">I", b[i:i + 4])[0]
        nm = ida_name.get_name(ea + i)
        print("  +%04X  %08X  %10d  %s" % (i, v, v, nm or ""))

def decomp(ea, pat=None, ctx=25):
    """Decompile a function; with PAT, print only matching lines and CTX lines around each.

    **The answer is often a few lines inside a big function** (a call site, a field), and printing
    the whole thing buries it; PAT prints a line window around each match instead.

    ⛔ HEX-RAYS IS NOT AVAILABLE ON EVERY DATABASE. It refuses some PPC databases outright ("only
    64-bit functions can be decompiled"), so GC/PPC work is often disassembly-only. That is REPORTED
    as the reason, not swallowed — a silent empty result here would read as "the pattern is absent"
    when it means "the decompiler never ran".

    ⛔⛔ AND THE REFUSAL HAS TWO SHAPES. Some databases THROW (caught below, reason printed); others
    return None instead, on verified function starts, with the plugin initialising normally first.
    Same outcome, different route. So DO NOT read "the plugin initialised" as "the decompiler works
    here": the only test that means anything is decompiling a function you already know is a
    function.
    """
    try:
        import ida_hexrays
        if not ida_hexrays.init_hexrays_plugin():
            print("\n⛔ Hex-Rays not available on this database — use range:/forcerange: instead")
            return
        cf = ida_hexrays.decompile(ea)
    except Exception as e:
        print("\n⛔ decompile(%08X) FAILED: %s\n   (PPC databases often refuse; this is the reason, "
              "NOT an empty result)" % (ea, e))
        return
    if cf is None:
        print("\n⛔ decompile(%08X) returned None — THE DECOMPILER NEVER RAN ON THIS FUNCTION.\n"
              "   This is NOT an empty result and NOT a failure to find PAT: there is no pseudocode\n"
              "   to search. A None here with the plugin initialised is how some databases refuse\n"
              "   — the other shape throws.\n"
              "   ⭐ Use range:%08X:<end>, forcerange:, or blocks:%08X instead; those work here.\n"
              "   ⛔ If %08X is not a function START, that alone returns None — check with blocks:."
              % (ea, ea, ea, ea))
        return
    lines = str(cf).splitlines()
    nm = ida_funcs.get_func_name(ea) or "%08X" % ea
    if not pat:
        print("\n===== decompiled %s (%d lines) =====" % (nm, len(lines)))
        for i, ln in enumerate(lines):
            print("  %4d: %s" % (i, ln))
        return
    print("\n===== %s: '%s' in %d decompiled lines =====" % (nm, pat, len(lines)))
    hits = 0
    for i, ln in enumerate(lines):
        if pat in ln:
            hits += 1
            print("  --- match at line %d ---" % i)
            for j in range(max(0, i - ctx), min(len(lines), i + ctx // 2)):
                print("  %4d: %s" % (j, lines[j]))
    print("  == %d match(es)" % hits)


def callees(ea, depth=2, _seen=None, _lvl=0):
    """What this function CALLS, recursively — the forward call chain.

    `callers:` answers "who reaches this"; this answers "what does this reach" (`CodeRefsFrom`),
    the question you ask when mapping a pipeline, in one call instead of one `callers:` at a time.
    ⛔ Depth-limited and cycle-guarded: engines are often mutually recursive (a tick function calls a
    handler that re-enters the tick), so an unbounded walk never terminates."""
    if _seen is None:
        _seen = set()
        fn0 = ida_funcs.get_func(ea)
        print("\n----- call chain from %08X (%s), depth %d -----"
              % (ea, ida_funcs.get_func_name(fn0.start_ea) if fn0 else "?", depth))
    if _lvl > depth or ea in _seen:
        return
    _seen.add(ea)
    f = ida_funcs.get_func(ea)
    if not f:
        return
    out = []
    for h in idautils.FuncItems(f.start_ea):
        for x in idautils.CodeRefsFrom(h, 0):        # 0 = calls/jumps AWAY, not flow-through
            t = ida_funcs.get_func(x)
            if t and t.start_ea != f.start_ea and t.start_ea not in out:
                out.append(t.start_ea)
    for t in out:
        print("  %s%08X  %s" % ("  " * _lvl, t, ida_funcs.get_func_name(t) or ""))
        callees(t, depth, _seen, _lvl + 1)


def findbytes(pat, a=None, b=None):
    """Every occurrence of a BYTE PATTERN — how you find a structure with no symbol and no xref.

    Neither a name search nor an xref search can locate a header/magic/signature that the code
    reaches by a computed address (`find_bytes` can): a file magic, a format header, a version word.
    ⛔ Pattern is HEX ("49424e4b") or a quoted ASCII string ("'IBNK'").

    ⭐⭐ `?` IS A WILDCARD NIBBLE, AND IT IS NOT A CONVENIENCE.
    Mapping one build's function onto another build, every exact pattern can return ZERO and
    read as "these binaries share nothing". ⛔ It is the QUERY: a PowerPC `stfs frS,d(rA)`
    encodes opcode+frS+rA in the first two bytes and the displacement in the last two, so `D0430024`
    pins frS=2 AND rA=3 — and REGISTER ALLOCATION is exactly what a different compiler build is most
    free to change. The opcode and the displacement are the stable halves.
    ⇒ `bytes:??????0248` now matches any `stw rS,0x248(rA)`. Before this, that query raised
    `ValueError` out of `bytes.fromhex` — a traceback, which teaches you the question is impossible
    rather than that the tool cannot ask it.
    ⛔ EXACT PATTERNS TAKE THE OLD PATH UNCHANGED, so nothing that worked before can behave
    differently: the wildcard branch is entered only when `?` is present."""
    if pat.startswith("'") and pat.endswith("'"):
        raw = pat[1:-1].encode("latin-1")
        wild = None
    else:
        pat = pat.replace(" ", "")
        if "?" in pat:
            # IDA's own pattern parser takes "90 ?? 02 48"; give it byte-spaced text. A '?' in
            # either nibble wildcards the whole byte, which is the right granularity here: the
            # fields we wildcard (frS/frD, rA, rS) straddle nibble boundaries anyway.
            if len(pat) % 2:
                raise SystemExit("⛔ bytes: hex pattern has an odd number of nibbles: %r" % pat)
            toks = [pat[i:i + 2] for i in range(0, len(pat), 2)]
            wild = " ".join("??" if "?" in t else t for t in toks)
            raw = None
        else:
            raw = bytes.fromhex(pat)
            wild = None
    lo = a if a is not None else ida_ida.inf_get_min_ea()
    hi = b if b is not None else ida_ida.inf_get_max_ea()
    print("\n----- %s in %08X..%08X -----" % (wild if wild else repr(raw), lo, hi))
    binpat = None
    if wild is not None:
        binpat = ida_bytes.compiled_binpat_vec_t()
        if ida_bytes.parse_binpat_str(binpat, lo, wild, 16) is None:
            raise SystemExit("⛔ bytes: IDA refused the wildcard pattern %r" % wild)
    n, ea = 0, lo
    while ea < hi:
        if binpat is not None:
            hit = ida_bytes.bin_search(ea, hi, binpat, ida_bytes.BIN_SEARCH_FORWARD)
            if isinstance(hit, tuple):      # newer IDA returns (ea, matched_index)
                hit = hit[0]
        else:
            hit = ida_bytes.find_bytes(raw, ea, hi)
        if hit == 0xFFFFFFFF or hit is None or hit >= hi:
            break
        nm = ida_name.get_name(hit)
        fn = ida_funcs.get_func(hit)
        print("  %08X  %-22s %s" % (hit, nm or "",
              ida_funcs.get_func_name(fn.start_ea) if fn else ""))
        n += 1
        ea = hit + 1
        if n >= 200:
            print("  ... stopping at 200 hits (narrow the range)"); break
    print("  == %d occurrence(s)" % n)


def strs(a, b):
    """Every STRING LITERAL referenced by code in a..b, with the referencing function.

    ⭐⭐ HOW YOU IDENTIFY AN UNNAMED FUNCTION: by what it PRINTS. Follows `DataRefsFrom` and tests
    `ida_bytes.is_strlit()` on each target. A binary's own debug output is the most honest
    documentation it has: a debug string can say outright what a function does.

    ⭐ Reports the REFERENCING function, not just the string's address — a string alone is a fact
    about the data segment; a string plus its caller is a fact about behaviour.
    """
    print("\n----- string literals referenced from %08X..%08X -----" % (a, b))
    seen, n = set(), 0
    ea = a
    while ea < b:
        for x in idautils.DataRefsFrom(ea):
            if x in seen:
                continue
            fl = ida_bytes.get_flags(x)
            if ida_bytes.is_strlit(fl):
                raw = ida_bytes.get_strlit_contents(x, -1, 0)
                if raw:
                    seen.add(x)
                    fn = ida_funcs.get_func(ea)
                    try:
                        txt = raw.decode("utf-8", "replace")
                    except Exception:
                        txt = repr(raw)
                    print("  %08X  ref@%08X  %-22s  %r" % (
                        x, ea, ida_funcs.get_func_name(fn.start_ea) if fn else "?", txt[:90]))
                    n += 1
        # ⛔ 64-BIT BADADDR — see rng(). `next_head` returns 0xFFFFFFFFFFFFFFFF, not 0xFFFFFFFF, so
        # this test never fired; and when next_head lands past `b` (undecoded bytes ahead) the scan
        # ENDED THERE while reporting a total, i.e. a silent partial scan that reads as complete.
        # Step over the gap instead of stopping in it.
        nxt = ida_bytes.next_head(ea, b)
        if not (ea < nxt < b):
            nxt = ea + max(4, ida_bytes.get_item_size(ea) or 4)
        if nxt <= ea:
            break
        ea = nxt
    print("  == %d distinct string(s) referenced" % n)


def _hilo_mips(ea, d, m2, ins, ok, pend, tlo, thi):
    """One MIPS instruction of a hi/lo address pair; returns 1 when it resolves into the window.

    MIPS builds a 32-bit address as `lui rX,HI` then `addiu/addi/daddiu rY,rX,LO` (LO
    SIGN-EXTENDS, so LO >= 0x8000 borrows from HI, exactly as PPC's addi), `ori rY,rX,LO` (zero-
    extends), or a load/store `LO(rX)`. IDA may also fold the pair into one `la`/`li` macro whose
    operand already holds the whole address. Operands come from decode_insn, never from the text.
    ⛔ A register a later instruction overwrites is dropped from `pend`, and `pend` is cleared at every
    function start: a stale HI is the trap hilo()'s PPC half records at length."""
    if not ok:
        return 0
    v, hi_ea = _hilo_resolve(ea, m2, ins, pend, carry=True)
    if v is not None and tlo <= v <= thi:
        fn = ida_funcs.get_func(ea)
        print("  %08X  -> %08X  (hi @%08X)  %-30s  %s" % (
            ea, v, hi_ea, d, ida_funcs.get_func_name(fn.start_ea) if fn else "?"))
        return 1
    return 0


def _hilo_resolve(ea, m2, ins, pend, carry=False):
    """The address one decoded MIPS instruction completes from a pending lui (or None), and the
    lui's address; updates pend. hilo: prints it, export: records it.
    carry (hilo: only): a whole address la/li/addiu leaves in a register stays in pend as a base,
    so a load/store off that register resolves to base + displacement; its keys are in
    pend["_carried"], which hilo() drops at each branch target."""
    ops = []
    for op in ins.ops:
        if op.type == ida_ua.o_void:
            break
        ops.append(op)
    v, hi_ea = None, ea
    if m2 == "lui":
        if len(ops) >= 2 and ops[0].type == ida_ua.o_reg and ops[1].type == ida_ua.o_imm:
            pend["r%d" % ops[0].reg] = (((ops[1].value & 0xFFFF) << 16) & 0xFFFFFFFF, ea)
        return None, ea
    if m2 in ("la", "li"):
        # IDA shows `lui rX` on its own line and the LO half as `li rX, <full>`: the HI is the lui.
        if ops and ops[0].type == ida_ua.o_reg and ("r%d" % ops[0].reg) in pend:
            hi_ea = pend["r%d" % ops[0].reg][1]
        for op in ops[1:]:
            if op.type == ida_ua.o_imm:
                v = op.value & 0xFFFFFFFF
                break
            if op.type in (ida_ua.o_mem, ida_ua.o_near):
                v = op.addr & 0xFFFFFFFF
                break
    elif m2 in ("addiu", "addi", "daddiu", "ori"):
        regs = [op for op in ops if op.type == ida_ua.o_reg]
        imm = next((op.value & 0xFFFF for op in ops if op.type == ida_ua.o_imm), None)
        src = regs[1] if len(regs) >= 2 else (regs[0] if regs else None)   # 2-operand form: rd == rs
        key = ("r%d" % src.reg) if src is not None else None
        if key in pend and imm is not None:
            o = imm - 0x10000 if (m2 != "ori" and imm >= 0x8000) else imm
            v, hi_ea = (pend[key][0] + o) & 0xFFFFFFFF, pend[key][1]
    elif m2 in LOAD_OPS or m2 in STORE_OPS:
        for op in ops:
            if op.type == ida_ua.o_displ and ("r%d" % op.reg) in pend:
                o = op.addr & 0xFFFF
                o = o - 0x10000 if o >= 0x8000 else o
                v, hi_ea = (pend["r%d" % op.reg][0] + o) & 0xFFFFFFFF, pend["r%d" % op.reg][1]
                break
    # `addu rX, rX, idx` (or the 2-operand `addu rX, idx`): a table's HI plus an index, the lo
    # still to come (lw lo(rX)): the HI stays pending (file 324's indexed tables)
    if m2 in ("addu", "daddu") and ops and ops[0].type == ida_ua.o_reg:
        regs = [op.reg for op in ops if op.type == ida_ua.o_reg]
        if ("r%d" % regs[0]) in pend and (len(regs) == 2 or regs[0] in regs[1:]):
            return v, hi_ea
    # The destination now holds something other than a bare HI (stores read ops[0], they do not write it).
    if ops and ops[0].type == ida_ua.o_reg and m2 not in STORE_OPS:
        key = "r%d" % ops[0].reg
        pend.pop(key, None)
        if carry:
            carried = pend.setdefault("_carried", set())
            carried.discard(key)
            if v is not None and m2 in ("la", "li", "addiu", "addi", "daddiu", "ori"):
                pend[key] = (v, hi_ea)  # a whole address: a base for the loads/stores after it
                carried.add(key)
    return v, hi_ea


def hilo(a, b, tlo, thi):
    """lis+lo pairs in a..b whose EFFECTIVE address lands in [tlo,thi] — "what touches this address?"

    ⭐⭐ PPC builds a 32-bit address from
    two halves — `lis rX,HI` then `ori/addi/lwz/stw ...,LO(rX)` — and **there is no xref on either
    half**. `dump()` annotates only what IDA already resolved, so a table or MMIO window reached this
    way is invisible to every xref-based search.

    ⛔ Counting functions that merely contain `lis rX,0xCC00` returns `__OSMaskInterrupts`-style code
    touching many DIFFERENT peripherals off one `lis`. **The pair must be resolved to an EXACT
    effective address** before it means anything.

    ⭐ Handles both forms. When IDA has already symbolised the pair it emits `sym@ha` / `sym@l`, and
    the address is read straight out of the symbol name. When it has not, the raw halves are combined
    with LO treated as SIGNED — ⛔ `addi` sign-extends, so a LO >= 0x8000 BORROWS from HI, and
    ignoring that puts the answer 64 KB high.
    """
    import re as _re
    print("\n----- lis/lo pairs reaching %08X..%08X within %08X..%08X -----" % (tlo, thi, a, b))
    sym = _re.compile(r'\b(?:dword|word|byte|off|unk|stru|asc|flt|dbl|qword)_([0-9A-Fa-f]{8})\b')
    lis = _re.compile(r'^lis\s+(r\d+),\s*(?:0x)?([0-9A-Fa-f]+)\s*$', _re.I)
    lo = _re.compile(r'(-?(?:0x)?[0-9A-Fa-f]+)\((r\d+)\)|,\s*(r\d+),\s*(-?(?:0x)?[0-9A-Fa-f]+)\s*$')
    pend = {}                      # reg -> (hi<<16, ea_of_lis)
    hits = 0
    ea = a
    while ea < b:
        d = (disasm(ea) or "").strip()
        if ARCH == "mips":                      # a HI never survives into the next function
            _f = ida_funcs.get_func(ea)
            if _f and _f.start_ea == ea:
                pend.clear()
            elif pend.get("_carried") and any(True for _ in idautils.CodeRefsTo(ea, False)):
                for _k in pend.pop("_carried"):  # a branch target: a base from one path only
                    pend.pop(_k, None)
        m = sym.search(d)
        if m and ARCH == "mips":                # keep the carried bases right past IDA's symbol
            _ins = ida_ua.insn_t()
            if ida_ua.decode_insn(_ins, ea):
                _hilo_resolve(ea, mnem(ea), _ins, pend, carry=True)
        if m:                                   # IDA already resolved it — trust the symbol
            v = int(m.group(1), 16)
            # a MIPS `lui` naming the symbol is the HI half only (PPC's `@ha` twin)
            if tlo <= v <= thi and "@ha" not in d and not (ARCH == "mips" and mnem(ea) == "lui"):
                fn = ida_funcs.get_func(ea)
                print("  %08X  -> %08X  %-44s  %s" % (
                    ea, v, d, ida_funcs.get_func_name(fn.start_ea) if fn else "?"))
                hits += 1
        else:
            # ⛔⛔ DECODE, DO NOT REGEX THE LINE. The first version matched
            # `^lis\s+(r\d+),\s*(?:0x)?([0-9A-Fa-f]+)\s*$` — anchored on END OF STRING — so ANY `lis`
            # carrying IDA's auto-comment (`lis r4, 0x8031# load hi`) was MISSED, exactly as
            # `bctr# switch jump` defeated the mnemonic split. Verified: the old pattern matches the
            # bare form and misses both commented forms. Operands come from decode_insn now.
            m2 = mnem(ea)
            ins = ida_ua.insn_t()
            ok = ida_ua.decode_insn(ins, ea)
            if ARCH == "mips":
                hits += _hilo_mips(ea, d, m2, ins, ok, pend, tlo, thi)
            elif ok and m2 in ("lis", "addis"):
                reg = "r%d" % ins.ops[0].reg
                imm = None
                for op in ins.ops:
                    if op.type == ida_ua.o_imm:
                        imm = op.value & 0xFFFF
                        break
                if imm is not None:
                    pend[reg] = ((imm << 16) & 0xFFFFFFFF, ea)
            elif ok and m2 in ("ori", "addi", "addic", "subi"):
                # ⛔ THE OTHER HALF OF THE PAIR (lis+displacement alone misses it): `lis rX,HI` +
                # `ori rY,rX,LO` is the classic way PPC materialises a 32-bit CONSTANT (a table base, an MMIO
                # window) that is then used with a register offset — so the address never appears as
                # a displacement at all and the displacement-only scan is blind to exactly the case
                # `hilo:` exists to find.
                # ⛔⛔ SKIP OPERAND 0 — IT IS THE DESTINATION, NOT A SOURCE. Searching from
                # ops[0] made `addi r3, r5, 0x568` resolve against a STALE `lis r3` whenever any
                # earlier lis anywhere in the scan had written pend["r3"]: the computed effective
                # address then failed the window test and the REAL reference was dropped silently.
                # ⭐ Proved by NON-MONOTONIC counts rather than by argument — the same target
                # window resolved 4 refs over 0x800A0000.. but only 3 over the LARGER
                # 0x80090000.., with the two true hits (0x800A49EC, 0x800A4A20) missing from both
                # and present only in a narrow band. A bigger range cannot yield fewer hits.
                # ⛔ For `addi rD,rA,imm` / `ori rA,rS,imm` the SOURCE is operand 1 — the
                # instruction form decides it. Never "prefer the last match": that guesses too.
                # ⚠ And note what this cost: validating this tool on a small known-answer band
                # passes even while the wide query is wrong, because `pend` is only poisoned at
                # scale. A state-carrying search must be validated at the SCALE it is used.
                src = None
                for _i in range(1, 8):
                    op = ins.ops[_i]
                    if op.type == ida_ua.o_void:
                        break
                    if op.type == ida_ua.o_reg and ("r%d" % op.reg) in pend:
                        src = "r%d" % op.reg
                        break
                imm = None
                for op in ins.ops:
                    if op.type == ida_ua.o_imm:
                        imm = op.value & 0xFFFF
                        break
                if src is not None and imm is not None:
                    o = imm - 0x10000 if (m2 != "ori" and imm >= 0x8000) else imm
                    v = (pend[src][0] + o) & 0xFFFFFFFF
                    if tlo <= v <= thi:
                        fn = ida_funcs.get_func(ea)
                        print("  %08X  -> %08X  (lis @%08X)  %-30s  %s" % (
                            ea, v, pend[src][1], d,
                            ida_funcs.get_func_name(fn.start_ea) if fn else "?"))
                        hits += 1
            elif ok:
                for op in ins.ops:
                    if op.type == ida_ua.o_void:
                        break
                    if op.type == ida_ua.o_displ:
                        reg = "r%d" % op.reg
                        if reg in pend:
                            o = op.addr
                            if o >= 0x8000:     # ⛔ SIGNED: addi/lwz sign-extend the low half
                                o -= 0x10000
                            v = (pend[reg][0] + o) & 0xFFFFFFFF
                            if tlo <= v <= thi:
                                fn = ida_funcs.get_func(ea)
                                print("  %08X  -> %08X  (lis @%08X)  %-30s  %s" % (
                                    ea, v, pend[reg][1], d,
                                    ida_funcs.get_func_name(fn.start_ea) if fn else "?"))
                                hits += 1
                        break
        # ⛔ 64-BIT BADADDR — see rng(). `next_head` returns 0xFFFFFFFFFFFFFFFF, not 0xFFFFFFFF, so
        # this test never fired; and when next_head lands past `b` (undecoded bytes ahead) the scan
        # ENDED THERE while reporting a total, i.e. a silent partial scan that reads as complete.
        # Step over the gap instead of stopping in it.
        nxt = ida_bytes.next_head(ea, b)
        if not (ea < nxt < b):
            nxt = ea + max(4, ida_bytes.get_item_size(ea) or 4)
        if nxt <= ea:
            break
        ea = nxt
    print("  == %d reference(s) resolved into the window" % hits)


def rng(a, b, force=False):
    """Raw range — for call sites IDA left outside any function (arg setup still reads fine).

    ⭐ `force=True` exists because bytes IDA left UNDEFINED disassemble as data here: this walk
    steps by `next_head`, so an undecoded region is skipped or printed as `.long`. Forcing
    `create_insn` first is the difference between reading a callback body and reading nothing.
    """
    print("\n----- range %08X .. %08X%s -----" % (a, b, "  (FORCE CODE)" if force else ""))
    if force:
        # ⛔⛔ UNDEFINE THE WINDOW FIRST. `create_insn` is a NO-OP on bytes that already belong to a
        # DEFINED DATA ITEM and it fails SILENTLY, so `force=True` did nothing at all inside an array
        # IDA had created. Worse, `disasm()` at ANY address inside an array returns the ARRAY's line,
        # so this printed the same twenty `.long` words for five different addresses and they read as
        # five identical functions.
        ida_bytes.del_items(a, ida_bytes.DELIT_SIMPLE, b - a)
    ea = a
    while ea < b:
        if force:
            # ⛔ create_insn lives in ida_ua, NOT ida_bytes (which offers create_align and friends);
            # the legacy form is `idc.create_insn`.
            ida_ua.create_insn(ea)
        ann = ""
        for x in idautils.XrefsFrom(ea):
            if x.type in (ida_xref.fl_CN, ida_xref.fl_CF):
                ann = "-> " + (ida_name.get_name(x.to) or "%08X" % x.to)
        print("  %08X  %-52s %s" % (ea, disasm(ea), ann))
        # ⛔ `next_head` returns the next DEFINED item, so over freshly-undefined bytes (or a region
        # IDA never analysed) it jumps straight past `b` and returns BADADDR — which this loop then
        # treated as "range finished" and printed ONE line for a 60-byte window. Fall back to the
        # item's own size, or a PPC instruction word, so the walk always advances by something real.
        # ⛔⛔ BADADDR IS 64-BIT HERE. Every sentinel test in this file compares against 0xFFFFFFFF,
        # but idalib is a 64-bit IDA and `next_head` returns 0xFFFFFFFFFFFFFFFF — so the test NEVER
        # matched, the huge value sailed past `while ea < b`, and the walk silently ended after ONE
        # line. Combined with the array bug above, `forcerange:` over an undecoded region printed a
        # single misleading line and looked like it had read the whole window.
        nxt = ida_bytes.next_head(ea, b)
        if not (ea < nxt < b):                    # BADADDR, a step backwards, or past the end
            nxt = ea + max(4, ida_bytes.get_item_size(ea) or 4)
        if nxt <= ea:
            break
        ea = nxt

def jumptables(a, b):
    """Every INDIRECT JUMP in a range, with its switch geometry — the opcode dispatch, not a guess.

    ⛔⛔ MATCHING ON NAMES (`jpt_*`, `def_*`) ALONE IS NOT ENOUGH. That is the same class of
    fragility as regexing disassembly text: **a table IDA did not NAME is invisible**, and IDA names
    a jump table only when its analysis recognised the switch. The case you most want — a dispatch it
    half-recognised, in code it never turned into a function — is exactly the case it will not name.
    ⭐ So: find every `bctr`/`bcctr` (PPC) or non-return `jr`/`jalr` (MIPS) — an indirect jump IS
    the jump-table candidate — then ask
    `ida_nalt.get_switch_info()` for the real geometry. Named-table search is kept as a second pass,
    because a table can be named while its `bctr` sits outside the scanned range.
    """
    import ida_nalt
    print("\n----- indirect jumps + switch geometry in %08X..%08X -----" % (a, b))
    n = 0
    ea = a
    while ea < b:
        d = (disasm(ea) or "").strip()
        mn = mnem(ea)
        if ARCH == "mips" and mn in ("jr", "jalr"):
            # `jr $ra` is a RETURN, not a dispatch; any other `jr`, and every `jalr`, is.
            _ins = ida_ua.insn_t()
            _is_ret = (mn == "jr" and ida_ua.decode_insn(_ins, ea)
                       and _ins.ops[0].type == ida_ua.o_reg and _ins.ops[0].reg == RA_REG)
        if (ARCH == "ppc" and mn in ("bctr", "bcctr", "bctrl", "bcctrl")) or \
           (ARCH == "mips" and mn in ("jr", "jalr") and not _is_ret):
            fn = ida_funcs.get_func(ea)
            fname = ida_funcs.get_func_name(fn.start_ea) if fn else "?"
            desc = ""
            try:
                si = ida_nalt.get_switch_info(ea)
                if si:
                    desc = ("  SWITCH ncases=%d jumps=%08X lowcase=%d elbase=%08X"
                            % (si.get_jtable_size(), si.jumps, si.lowcase, si.elbase))
                else:
                    desc = "  (no switch_info — IDA did not resolve this dispatch)"
            except Exception as e:
                desc = "  (switch_info failed: %s)" % e
            print("  %08X  %-20s in %-18s%s" % (ea, mn, fname, desc))
            n += 1
        # ⛔ 64-BIT BADADDR — see rng(). `next_head` returns 0xFFFFFFFFFFFFFFFF, not 0xFFFFFFFF, so
        # this test never fired; and when next_head lands past `b` (undecoded bytes ahead) the scan
        # ENDED THERE while reporting a total, i.e. a silent partial scan that reads as complete.
        # Step over the gap instead of stopping in it.
        nxt = ida_bytes.next_head(ea, b)
        if not (ea < nxt < b):
            nxt = ea + max(4, ida_bytes.get_item_size(ea) or 4)
        if nxt <= ea:
            break
        ea = nxt
    print("  == %d indirect jump(s)" % n)
    named = 0
    for ea, nm in idautils.Names():
        if a <= ea < b and nm.startswith(("jpt_", "def_")):
            fn = None
            for x in idautils.XrefsTo(ea):
                fn = ida_funcs.get_func(x.frm)
                if fn:
                    break
            print("  %08X  %-20s  (named table) in %s" % (ea, nm,
                  ida_funcs.get_func_name(fn.start_ea) if fn else "?"))
            named += 1
    print("  == %d named table(s)" % named)

def fieldstores(a, b, dlo, dhi):
    """Every STORE in a..b whose displacement lands in [dlo,dhi] — "what writes this struct field?"

    ⭐ A struct field is identified by WHO WRITES IT, and xrefs cannot answer this: the address is
    computed at run time and there is no xref to follow. This scans the code range instead, every
    store whose displacement lands in the field window.

    ⛔ An indexed store (stwx/sthx) has no literal displacement and would be silently skipped by an
    operand-only filter. Those are printed too, flagged INDEXED, because a store through a register
    offset is precisely how a field write hides from a displacement search.
    """
    import re as _re
    print("\n----- stores into +0x%X..+0x%X within %08X..%08X -----" % (dlo, dhi, a, b))
    # ⛔⛔ READ THE DECODED OPERAND, DO NOT REGEX THE DISASSEMBLY TEXT. The first version of this
    # function parsed `0xNNN(` out of the printed line. That is fragile in three ways IDA actually
    # exhibits: a SYMBOLIC displacement (`lbz r5, someStruct.field(r5)`) contains no hex at all and
    # was silently MISSED; a NEGATIVE displacement prints as `-0x10(r1)` and the pattern dropped the
    # sign; and the radix is a display setting, so a decimal-rendering database would misparse.
    # ⭐ `o_displ` + `get_operand_value()` is what IDA itself resolved. The text path is kept ONLY as a fallback for instructions IDA declines to decode, and any
    # disagreement between the two is REPORTED rather than silently preferred.
    disp = _re.compile(r"(-?)(?:0x)?([0-9A-Fa-f]+)\(")
    hits = idx = textonly = 0
    # ⛔⛔ FLOAT STORES ARE STORES (stfs/stfd, swc1/sdc1). With the integer forms only, a field written
    # as a float — a rate, a gain, a tick budget — reads as "nothing writes this field", and a
    # `stfs f0,0x344(r31)` inside the range gives `0 store(s)`.
    floats = 0
    skipped_r1 = [0]          # list so the inner loop can bump it
    ea = a
    while ea < b:
        d = disasm(ea)
        mn = mnem(ea)
        if mn in STORE_OPS:                        # per-ARCH set (PPC list unchanged)
            off = None
            sp_hit = False                         # see the fallback below
            insn = ida_ua.insn_t()
            if ida_ua.decode_insn(insn, ea):
                for op in insn.ops:
                    if op.type == ida_ua.o_void:
                        break
                    if op.type == ida_ua.o_displ:
                        # ⛔⛔ SKIP r1-RELATIVE STORES. This function answers "what writes this STRUCT
                        # FIELD?", and a store through r1 is a stack local by definition — never a
                        # struct field. A search for displacement 0x24 can return only frame stores in
                        # prologues, because 0x24 is a common frame offset, with the real writer
                        # invisible behind the noise. ⭐ The skipped count is PRINTED, so this
                        # exclusion can never be mistaken for an empty result.
                        if op.reg == SP_REG:       # r1 on PPC, $sp on MIPS
                            skipped_r1[0] += 1
                            sp_hit = True
                            break
                        # ⛔⛔ IDA HANDS BACK `addr` ALREADY SIGN-EXTENDED TO 32 BITS. The original
                        # `if off >= 0x8000: off -= 0x10000` turned 0xFFFF9AC0 into 0xFFFE9AC0
                        # instead of -0x6540, so NO NEGATIVE DISPLACEMENT COULD EVER MATCH — which on
                        # a GameCube DOL means every r13/r2 small-data store was invisible. Caught by
                        # a positive control: a store already read in the disassembly returned zero
                        # hits, and the scan claimed 0 literal stores across 4 MB of PowerPC.
                        # ⭐ Mask to the 16-bit field FIRST, then sign-extend.
                        off = op.addr & 0xFFFF
                        if off >= 0x8000:          # ⛔ SIGNED: PPC displacements sign-extend
                            off -= 0x10000
                        break
            # ⛔ THE TEXT FALLBACK MUST HONOUR THE SAME EXCLUSION. It parses the printed line and so
            # never saw `op.reg`; on the first run of this filter the operand path correctly skipped
            # 31,574 stack stores while the fallback quietly let `stw r0, 0x24(r1)` straight through.
            # A filter that one of two paths ignores is not a filter.
            # ⛔ And a store BOTH paths see must be counted ONCE. The operand
            # path skips it and leaves `off` None, so this fallback used to count the same store a
            # second time: `stores:` over sub_80000DE0 reported 6 $sp-relative skips for 3 stack
            # stores. The same double count is in the original PPC tool (as r1).
            # ⛔ And a store the operand path skipped must not reach the TEXT fallback either: with the
            # double count removed it fell through and `0x1C+var_s0($sp)` parsed as +0x0 — three
            # stack stores reported as field hits, caught by the same control.
            if sp_hit:
                off = None
            elif off is None and SP_TEXT in d:
                skipped_r1[0] += 1
                off = None
            elif off is None:                      # fallback: IDA would not decode it
                m = disp.search(d)
                if m:
                    try:
                        off = int(m.group(2), 16) * (-1 if m.group(1) else 1)
                        textonly += 1
                    except ValueError:
                        off = None
            if off is not None and dlo <= off <= dhi:
                fn = ida_funcs.get_func(ea)
                print("  %08X  +0x%03X  %-44s  %s" % (
                    ea, off, d, ida_funcs.get_func_name(fn.start_ea) if fn else "?"))
                hits += 1
                if is_float_store(mn):
                    floats += 1
        elif mn in STORE_IDX:
            fn = ida_funcs.get_func(ea)
            print("  %08X  INDEXED  %-44s  %s" % (
                ea, d, ida_funcs.get_func_name(fn.start_ea) if fn else "?"))
            idx += 1
        # ⛔ 64-BIT BADADDR — see rng(). `next_head` returns 0xFFFFFFFFFFFFFFFF, not 0xFFFFFFFF, so
        # this test never fired; and when next_head lands past `b` (undecoded bytes ahead) the scan
        # ENDED THERE while reporting a total, i.e. a silent partial scan that reads as complete.
        # Step over the gap instead of stopping in it.
        nxt = ida_bytes.next_head(ea, b)
        if not (ea < nxt < b):
            nxt = ea + max(4, ida_bytes.get_item_size(ea) or 4)
        if nxt <= ea:
            break
        ea = nxt
    # ⭐ The float count is printed SEPARATELY and always, even when zero, so a window that matched
    # only floats can never be confused with one that matched nothing — the failure this fix exists
    # to kill was a silent "0 store(s)" over two stfs, and a silent "2 store(s)" would hide it again.
    print("  == %d store(s) in the window (%d float), %d indexed store(s) (displacement not literal)%s%s"
          % (hits, floats, idx,
             ", %d resolved by TEXT FALLBACK (IDA would not decode them)" % textonly if textonly else "",
             ", %d %s-relative SKIPPED (stack frames, never struct fields)" % (skipped_r1[0], SP_TEXT[1:-1])
             if skipped_r1[0] else ""))


def fieldloads(a, b, dlo, dhi):
    """Every LOAD in a..b whose displacement lands in [dlo,dhi] — "what reads this struct field?"

    ⭐⭐ THE OTHER HALF OF `fieldstores`. `fieldstores`'s own docstring says a field is identified by
    who writes it BECAUSE xrefs cannot answer it — the address is computed at run time. The same is
    true of a READ: "what consumes `obj+0x2C`" has no xref to follow either.

    Mirrors `fieldstores`: decoded-operand displacement (not disassembly-text regex, for the
    same three reasons — symbolic displacements, negative sign, and display-radix settings), r1
    exclusion (a load through r1 is a stack local, never a struct field), and indexed loads
    (`lwzx`/`lbzx`/`lhzx`/`lfsx`/`lfdx`) reported separately because they have no literal
    displacement to match.

    ⛔⛔ FLOAT LOADS ARE LOADS (`lfs`/`lfd`/`lfsu`/`lfdu`, `lwc1`/`ldc1`). With the integer forms only,
    EVERY FLOAT READ OF A STRUCT FIELD IS INVISIBLE and comes back as `0 load(s)`, not as a filtered
    result: where volume, pitch and pan are floats, the answer would be "nothing reads this field".
    """
    import re as _re
    print("\n----- loads from +0x%X..+0x%X within %08X..%08X -----" % (dlo, dhi, a, b))
    disp = _re.compile(r"(-?)(?:0x)?([0-9A-Fa-f]+)\(")
    hits = idx = textonly = 0
    floats = 0
    skipped_r1 = [0]
    ea = a
    while ea < b:
        d = disasm(ea)
        mn = mnem(ea)
        # `lha`/`lhau`/`lhax` (signed halfword) count too: a field read only by `lha` must not
        # answer "no reader".
        if mn in LOAD_OPS:                         # per-ARCH set (PPC list unchanged)
            off = None
            sp_hit = False
            insn = ida_ua.insn_t()
            if ida_ua.decode_insn(insn, ea):
                for op in insn.ops:
                    if op.type == ida_ua.o_void:
                        break
                    if op.type == ida_ua.o_displ:
                        if op.reg == SP_REG:       # r1 on PPC, $sp on MIPS
                            skipped_r1[0] += 1
                            sp_hit = True
                            break
                        off = op.addr & 0xFFFF
                        if off >= 0x8000:
                            off -= 0x10000
                        break
            if sp_hit:                                        # counted once, never re-parsed
                off = None                                    #    as text (see stores:)
            elif off is None and SP_TEXT in d:
                skipped_r1[0] += 1
                off = None
            elif off is None:
                m = disp.search(d)
                if m:
                    try:
                        off = int(m.group(2), 16) * (-1 if m.group(1) else 1)
                        textonly += 1
                    except ValueError:
                        off = None
            if off is not None and dlo <= off <= dhi:
                fn = ida_funcs.get_func(ea)
                print("  %08X  +0x%03X  %-44s  %s" % (
                    ea, off, d, ida_funcs.get_func_name(fn.start_ea) if fn else "?"))
                hits += 1
                if is_float_load(mn):
                    floats += 1
        elif mn in LOAD_IDX:
            fn = ida_funcs.get_func(ea)
            print("  %08X  INDEXED  %-44s  %s" % (
                ea, d, ida_funcs.get_func_name(fn.start_ea) if fn else "?"))
            idx += 1
        nxt = ida_bytes.next_head(ea, b)
        if not (ea < nxt < b):
            nxt = ea + max(4, ida_bytes.get_item_size(ea) or 4)
        if nxt <= ea:
            break
        ea = nxt
    # ⭐ THE FLOAT COUNT IS PRINTED ALWAYS, EVEN AT ZERO — `fieldstores`'s reason applies here
    # identically: a window that matched only float loads must never read the same as a window
    # that matched nothing.
    print("  == %d load(s) in the window (%d float), %d indexed load(s) (displacement not literal)%s%s"
          % (hits, floats, idx,
             ", %d resolved by TEXT FALLBACK (IDA would not decode them)" % textonly if textonly else "",
             ", %d %s-relative SKIPPED (stack frames, never struct fields)" % (skipped_r1[0], SP_TEXT[1:-1])
             if skipped_r1[0] else ""))


def immscan(a, b, pat):
    """Instructions in a range whose disassembly contains PAT. Finds an opcode dispatch that the
    compiler emitted as a compare chain rather than a switch — a jump-table search cannot see one."""
    print("\n----- '%s' in %08X..%08X -----" % (pat, a, b))
    ea, hits = a, 0
    while ea < b and ea != 0xFFFFFFFF:
        d = disasm(ea)
        if pat in d:
            fn = ida_funcs.get_func(ea)
            print("  %08X  %-16s  %s" % (ea, ida_funcs.get_func_name(fn.start_ea) if fn else "?", d))
            hits += 1
        ea = ida_bytes.next_head(ea, b)
    print("  %d hits" % hits)

def blocks(ea):
    """Basic-block heads of a function, also as GATE_PC= lines for a block-hit counter.

    Every basic-block head is a place the CPU can be, so counting hits at all of them shows the
    path actually taken: run one input that works and one that fails, diff the columns, and the
    block where they stop agreeing IS the deciding branch. No threshold, and no judgement about
    which returns are 'the interesting ones'.
    ⛔ A GATE_PC= line holds at most 32 addresses (GATE_PC_MAX), so long functions print in
    numbered batches — run one batch per run. A silently dropped address reads as 'never
    executed', which is precisely the wrong conclusion."""
    import ida_gdl
    GATE_PC_MAX = 32
    f = ida_funcs.get_func(ea)
    if not f:
        print("\n===== NO FUNCTION AT %08X =====" % ea); return
    heads = sorted(b.start_ea for b in ida_gdl.FlowChart(f))
    print("\n----- %s  %08X..%08X  %d basic blocks -----" % (
        ida_funcs.get_func_name(f.start_ea), f.start_ea, f.end_ea, len(heads)))
    for h in heads:
        print("  %08X  %s" % (h, disasm(h)))
    for i in range(0, len(heads), GATE_PC_MAX):
        chunk = heads[i:i + GATE_PC_MAX]
        tag = "" if len(heads) <= GATE_PC_MAX else "   # batch %d of %d" % (
            i // GATE_PC_MAX + 1, (len(heads) + GATE_PC_MAX - 1) // GATE_PC_MAX)
        print("GATE_PC=%s%s" % (",".join("0x%08X" % h for h in chunk), tag))


def funcs(a, b):
    """Functions in a range, largest first. A bytecode interpreter is almost always the biggest
    function in its neighbourhood, which makes size a cheap first filter when the dispatch is
    neither a switch nor a pointer table."""
    print("\n----- functions in %08X..%08X, largest first -----" % (a, b))
    out = []
    for ea in idautils.Functions(a, b):
        f = ida_funcs.get_func(ea)
        if f:
            out.append((f.end_ea - f.start_ea, ea))
    for sz, ea in sorted(out, reverse=True)[:25]:
        print("  %08X  %6d bytes  %s" % (ea, sz, ida_funcs.get_func_name(ea)))
    print("  (%d functions in range)" % len(out))

def coverage(a, b):
    """HOW MUCH OF a..b IDA DECODED — the sanity check for a new database ("measure is_code
    coverage of .text"). hilo:/range:/strs: walk by next_head and are blind to undecoded bytes, so
    a zero from them is not a negative until this is measured.
    Counts 4-byte words: code (an instruction head, or the tail of a multi-word macro such as MIPS
    `la`), and inside a function."""
    words = code = infunc = 0
    for ea in range(a, b, 4):
        words += 1
        fl = ida_bytes.get_flags(ea)
        head = ida_bytes.get_item_head(ea)
        if ida_bytes.is_code(fl) or (ida_bytes.is_tail(fl) and ida_bytes.is_code(ida_bytes.get_flags(head))):
            code += 1
        if ida_funcs.get_func(ea):
            infunc += 1
    nf = sum(1 for _ in idautils.Functions(a, b))
    print("\n----- coverage %08X..%08X -----" % (a, b))
    print("  %d words: %d code (%.1f%%), %d inside a function (%.1f%%); %d function(s) start here"
          % (words, code, 100.0 * code / max(words, 1), infunc, 100.0 * infunc / max(words, 1), nf))


def owner(addrs):
    """⭐ WHICH FUNCTION CONTAINS THIS ADDRESS — from a profiler sample or a crash PC to an entry.

    `funcs:` lists a range top-25 BY SIZE; `blocks:` already needs an entry. A profiler's sample
    lands wherever the loop happens to be; this gives the entry, size and offset it belongs to.

    ⛔ AN ADDRESS INSIDE NO FUNCTION IS REPORTED AS SUCH, LOUDLY. IDA leaves regions undecoded (on
    one GameCube database, 17% of .text). Printing nothing (or the previous function) for those
    would read as "not code" — the same false negative this file's `hilo:` warning was written for.
    Use forcerange: on anything that comes back NOT IN ANY FUNCTION before concluding anything."""
    print("\n----- owning function per address -----")
    print("  %-10s %-10s %-8s %-7s %s" % ("addr", "entry", "+off", "size", "name"))
    for ea in addrs:
        f = ida_funcs.get_func(ea)
        if not f:
            print("  %08X  %s" % (ea, "⛔ NOT IN ANY FUNCTION (undecoded? try forcerange:)"))
            continue
        print("  %08X  %08X   +%-6X %-7d %s" % (
            ea, f.start_ea, ea - f.start_ea, f.end_ea - f.start_ea,
            ida_funcs.get_func_name(f.start_ea)))


def export(out_path):
    """The whole database as JSON, for the canon tools that must not decode
    instructions themselves. {"functions": [{"start", "end", "name", "ins": [[ea, opcode number,
    mnemonic, operands, data target or null], ...]}], "code_words": [[start, end], ...]}. The data
    target is what IDA resolved for the instruction (a lui/addiu or lui/load pair's address, from
    its data xref); the opcode number is the word's top six bits, as the tools compared them."""
    import json
    funcs_out = []
    def one(ea, pend):
        """One decoded instruction: [address, opcode, mnemonic, operands, data target or None]."""
        word = ida_bytes.get_dword(ea)
        target = None
        for dref in idautils.DataRefsFrom(ea):
            target = dref
            break
        # MIPS: a %hi/%lo pair's target belongs to the instruction that COMPLETES it (the
        # lwc1/lw/addiu, whose opcode says what the data is), resolved from IDA's decoded
        # operands as hilo: does; IDA hangs its own xref on the lui, and leaves some pairs
        # without one (a lui far from its lwc1). IDA's xref is used only where no pair does.
        if ARCH == "mips":
            m2 = mnem(ea)
            insn = ida_ua.insn_t()
            v = None
            if ida_ua.decode_insn(insn, ea):
                v, _hi = _hilo_resolve(ea, m2, insn, pend)
            target = v if v is not None else (None if m2 == "lui" else target)
            if word == 0x03E00008:
                pend.clear()
        ops = ida_lines.tag_remove(ida_ua.print_operand(ea, 0) or "")
        for i in range(1, 4):
            o = ida_lines.tag_remove(ida_ua.print_operand(ea, i) or "")
            if o:
                ops += ", " + o
        # A multi-word macro (lui + lw/addiu/lwc1 folded into one `lw x, sym` / `li x, sym`):
        # its target belongs to its LAST word, the instruction that completes the pair, with
        # that word's opcode (a jump table's lw, a float's lwc1); the lui opens it
        size = ida_bytes.get_item_size(ea)
        at, op = (ea + size - 4, ida_bytes.get_dword(ea + size - 4) >> 26) if size > 4 else (ea, word >> 26)
        return [at, op, ida_ua.ua_mnem(ea) or "", ops, target]

    for fea in idautils.Functions():
        f = ida_funcs.get_func(fea)
        ins, pend = [], {}
        for ea in idautils.FuncItems(fea):
            if ida_bytes.is_code(ida_bytes.get_flags(ea)):
                ins.append(one(ea, pend))
        funcs_out.append({"start": f.start_ea, "end": f.end_ea, "name": ida_funcs.get_func_name(fea), "ins": ins})
    # code IDA decoded but left outside every function (a prologue's lwc1 before IDA's start)
    orphans, pend = [], {}
    for s in idautils.Segments():
        seg = ida_segment.getseg(s)
        for ea in idautils.Heads(seg.start_ea, seg.end_ea):
            if ida_bytes.is_code(ida_bytes.get_flags(ea)) and not ida_funcs.get_func(ea):
                orphans.append(one(ea, pend))
    if orphans:
        funcs_out.append({"start": None, "end": None, "name": "(code outside any function)", "ins": orphans})
    # IDA's code, as word ranges (an instruction head, or the tail of a multi-word macro)
    spans, cur = [], None
    for s in idautils.Segments():
        seg = ida_segment.getseg(s)
        for ea in range(seg.start_ea, seg.end_ea, 4):
            fl = ida_bytes.get_flags(ea)
            code = ida_bytes.is_code(fl) or (ida_bytes.is_tail(fl) and ida_bytes.is_code(
                ida_bytes.get_flags(ida_bytes.get_item_head(ea))))
            if code and cur and cur[1] == ea:
                cur[1] = ea + 4
            elif code:
                cur = [ea, ea + 4]
                spans.append(cur)
    with open(out_path, "w", encoding="utf-8", newline="\n") as fo:
        json.dump({"idb": os.path.basename(IDB), "functions": funcs_out, "code_words": spans}, fo)
    print("export: %d functions, %d code spans -> %s" % (len(funcs_out), len(spans), out_path))


for a in sys.argv[1:]:
    if a.startswith("export:"):
        export(a.split(":", 1)[1])
    elif a.startswith("coverage:"):
        # coverage:A:B — how much of the range IDA decoded
        p = a.split(":")
        coverage(int(p[1], 16), int(p[2], 16))
    elif a.startswith("owner:"):
        owner([int(x, 16) for x in a.split(":", 1)[1].split(",") if x.strip()])
    elif a.startswith("funcs:"):
        p = a.split(":")
        funcs(int(p[1], 16), int(p[2], 16))
    elif a.startswith("imm:"):
        p = a.split(":", 3)
        immscan(int(p[1], 16), int(p[2], 16), p[3])
    elif a.startswith("jpt:"):
        p = a.split(":")
        jumptables(int(p[1], 16), int(p[2], 16))
    elif a.startswith("range:"):
        p = a.split(":")
        rng(int(p[1], 16), int(p[2], 16))
    elif a.startswith("blocks:"):
        blocks(int(a.split(":", 1)[1], 16))
    elif a.startswith("callers:"):
        callers(int(a.split(":", 1)[1], 16))
    elif a.startswith("xrefsto:"):
        xrefsto(int(a.split(":", 1)[1], 16))
    elif a.startswith("data:"):
        p = a.split(":")
        data(int(p[1], 16), int(p[2], 16))
    elif a.startswith("stores:"):
        # stores:LO:HI:DLO:DHI — "what writes this struct field?"
        p = a.split(":")
        fieldstores(int(p[1], 16), int(p[2], 16), int(p[3], 16), int(p[4], 16))
    elif a.startswith("loads:"):
        # loads:LO:HI:DLO:DHI — "what reads this struct field?"
        p = a.split(":")
        fieldloads(int(p[1], 16), int(p[2], 16), int(p[3], 16), int(p[4], 16))
    elif a.startswith("decomp:"):
        # decomp:ADDR[:PATTERN] — pseudocode, or just the lines around PATTERN
        p = a.split(":", 2)
        decomp(int(p[1], 16), p[2] if len(p) > 2 and p[2] else None)
    elif a.startswith("callees:"):
        # callees:ADDR[:DEPTH] — the forward call chain
        p = a.split(":")
        callees(int(p[1], 16), int(p[2]) if len(p) > 2 else 2)
    elif a.startswith("bytes:"):
        # bytes:PATTERN[:A:B] — hex or 'ASCII' byte search
        p = a.split(":")
        findbytes(p[1], int(p[2], 16) if len(p) > 3 else None,
                  int(p[3], 16) if len(p) > 3 else None)
    elif a.startswith("strs:"):
        # strs:A:B — string literals referenced from a code range
        p = a.split(":")
        strs(int(p[1], 16), int(p[2], 16))
    elif a.startswith("hilo:"):
        # hilo:LO:HI:TLO:THI — "what touches this address?"
        p = a.split(":")
        hilo(int(p[1], 16), int(p[2], 16), int(p[3], 16), int(p[4], 16))
    elif a.startswith("forcerange:"):
        # forcerange:A:B — range, forcing undefined bytes to code
        p = a.split(":")
        rng(int(p[1], 16), int(p[2], 16), force=True)
    elif a.startswith("decode:"):
        # ⭐⭐⭐ decode:A:B — force code across the range AND SAVE THE DATABASE.
        # ⛔⛔ WHY THIS EXISTS. `callers:` is an XREF query and IDA only creates an xref from an
        # instruction it has DECODED. Over an undecoded region `callers:` returns EMPTY — which is
        # indistinguishable from "this function is only reached through a vtable", a false
        # negative that conclusions get built on. `forcerange:` could not fix it: this file ends in close_database(False),
        # so every byte it forced was discarded at exit. It decoded for DISPLAY only.
        # ⛔ A SEPARATE MODE, deliberately: this is the only mode that MUTATES the database, and a
        # read-only tool that quietly starts writing is a worse trap than the one it fixes.
        p = a.split(":")
        lo, hi = int(p[1], 16), int(p[2], 16)
        # ⛔⛔ UNDEFINE FIRST — `create_insn` DOES NOTHING on bytes that are already a DEFINED DATA
        # ITEM, and it fails SILENTLY. That is a third state this mode did not handle: not
        # "undefined", not "code", but "IDA decided this is a dword array". The symptom is
        # vicious — `range:` prints the ENCLOSING
        # ARRAY's contents for EVERY address inside it, so five different addresses returned the
        # IDENTICAL twenty words and read as five identical functions. A tool that answers the same
        # thing to every question is worse than one that errors.
        # ⭐ del_items(DELIT_SIMPLE) over the window puts the bytes back to state (1), after which
        # create_insn does its job. Guarded to the requested range only.
        ida_bytes.del_items(lo, ida_bytes.DELIT_SIMPLE, hi - lo)
        made = 0
        ea = lo
        while ea < hi:
            if not ida_bytes.is_code(ida_bytes.get_flags(ea)):
                if ida_ua.create_insn(ea):
                    made += 1
            # ⛔⛔ SAME 64-BIT BADADDR BUG AS rng(). `next_head` returns 0xFFFFFFFFFFFFFFFF here, which
            # is != 0xFFFFFFFF AND > ea, so this assigned BADADDR to `ea` and the `while ea < hi`
            # loop exited after ONE address — then printed a confident "N instruction(s) created".
            # Advance by a real step whenever next_head does not land inside the window.
            nxt = ida_bytes.next_head(ea, hi)
            ea = nxt if (ea < nxt < hi) else ea + 4
        ida_auto.auto_wait()
        # ⭐⭐⭐ AND CREATE FUNCTIONS. There are THREE states, not two, and conflating the last two
        # is a trap: (1) undefined bytes, (2) DECODED CODE WITH NO
        # FUNCTION, (3) code inside a function. `callers:` and `dump()` both need (3) — with (2),
        # `dump()` prints "NO FUNCTION AT" and `callers:` has no `sub_` to attribute a call to, which
        # reads exactly like "nothing calls this". A whole subsystem can sit in state (2).
        fn = 0
        ea = lo
        while ea < hi:
            if ida_bytes.is_code(ida_bytes.get_flags(ea)) and not ida_funcs.get_func(ea):
                if ida_funcs.add_func(ea):
                    fn += 1
            # ⛔⛔ SAME 64-BIT BADADDR BUG AS rng(). `next_head` returns 0xFFFFFFFFFFFFFFFF here, which
            # is != 0xFFFFFFFF AND > ea, so this assigned BADADDR to `ea` and the `while ea < hi`
            # loop exited after ONE address — then printed a confident "N instruction(s) created".
            # Advance by a real step whenever next_head does not land inside the window.
            nxt = ida_bytes.next_head(ea, hi)
            ea = nxt if (ea < nxt < hi) else ea + 4
        ida_auto.auto_wait()
        print("decode %08X..%08X: %d instruction(s), %d function(s) created; saving database"
              % (lo, hi, made, fn))
        SAVE_DB[0] = True
    elif a.startswith("mkfuncs:"):
        # mkfuncs:A[,B..] — a function starting at each address (the decomp's split says one
        # starts there), the database saved. Mutates, like decode:. In address order, so a
        # function made at A that runs over B is ended at B when B's turn comes.
        starts = sorted({int(x, 16) for x in a[len("mkfuncs:"):].split(",") if x})
        made = ins = 0
        for ea in starts:
            f = ida_funcs.get_func(ea)
            if f and f.start_ea == ea:
                continue
            if f:
                ida_funcs.set_func_end(f.start_ea, ea)
            if not ida_bytes.is_code(ida_bytes.get_flags(ea)):
                ida_bytes.del_items(ea, ida_bytes.DELIT_SIMPLE, 4)
                ins += 1 if ida_ua.create_insn(ea) else 0
            made += 1 if ida_funcs.add_func(ea) else 0
        ida_auto.auto_wait()
        wrong = ["%08X" % ea for ea in starts if not ida_funcs.get_func(ea) or ida_funcs.get_func(ea).start_ea != ea]
        print("decode %08X..%08X: %d instruction(s), %d function(s) created; %d still not a function "
              "start%s; saving database" % (starts[0] if starts else 0, starts[-1] if starts else 0, ins, made,
                                            len(wrong), (": " + " ".join(wrong[:10])) if wrong else ""))
        SAVE_DB[0] = True
    else:
        dump(int(a, 16))

# ⛔ Saves ONLY when `decode:` or `mkfuncs:` ran. Every other mode leaves the database byte-identical, which is the
# property that makes this tool safe to point at a shared IDB.
idapro.close_database(SAVE_DB[0])
