"""decomp-permuter glue: set up a near-miss function for the permuter, and compile its candidates.

    python canon-decompilation-tools/match_function.py import src/dma.c func_80001880_main [DIR]
        writes permuter/func_80001880_main/ (or DIR): base.c, target.o, compile.sh, settings.toml.
        Each body line holding more than one statement (a macro, "if (0) osSyncPrintf") is kept
        on one line in every candidate (sameline pragmas), as the real compile has it
    python canon-decompilation-tools/match_function.py basecheck func_80001880_main [DIR]
        whether that directory's base.c compiles to the same code as the real file (full.c), own
        section addresses aside; a base that does not is a different function to permute
    python <decomp-permuter>/permuter.py permuter/func_80001880_main -j1
        runs the permuter (https://github.com/simonlindholm/decomp-permuter)
    python canon-decompilation-tools/match_function.py compile IN.c -o OUT.o
        what compile.sh runs: one C file through asm-processor and IDO, as canon-decompilation-tools/build_rom.py does
    python canon-decompilation-tools/match_function.py checkall permuter/checkall.txt
        check every parked function; a table of differing lines, closest first. Its work
        directories are under permuter/checkall/, so it can run beside other checks and permuter runs
        (`check FILE FUNC DIR` takes the directory too)
    python canon-decompilation-tools/match_function.py twins
        every parked function whose instructions (symbols and label addresses abstracted) equal
        another function's; a matched twin's C is the place to start
    python canon-decompilation-tools/match_function.py twins files [--all]
        each C file still holding assembly, with the C file most like it (IDA's mnemonics per
        function: same, or near at a 0.9 ratio): a whole-file run with that twin named in its
        brief. Files with no twin at all are listed only with --all
    python canon-decompilation-tools/match_function.py twins near [MIN] [FOLDER]
        each function still in assembly with its nearest matched function (IDA's mnemonics,
        score MIN, default 0.8: order and order-free), the twin's C file and the field-offset shift:
        the twin's C is the draft. permuter/twins/near.txt, nearest first
    python canon-decompilation-tools/match_function.py data-c src/tlb/tlb_0034.c
        the file's asm/data .s as C definitions in address order, compiled and byte-compared
        with the .s (permuter/datac/): the draft for turning its yaml data subsegment to .data
    python canon-decompilation-tools/match_function.py data-c src/tlb/tlb_0248.c --retype laputa511Places=EneDef
        an array already in C written as the type that reads it (a struct of narrower fields, or
        words where it was bytes), proven by data-c --verify; data-c --retype-list LIST takes
        lines "src/FILE.c NAME TYPE"
    python canon-decompilation-tools/match_function.py protos
        every func_ prototype in src/*.c whose return or parameter classes (int/float per
        position, and the count) differ from its C definition: a wrong prototype moves the
        argument registers of every call
    python canon-decompilation-tools/match_function.py headers [TREE] [apply]
        each function declared once, in a header made from its definition, as sm64 does: every
        main module's non-static functions in include/<module>.h, an archive file's shared ones
        in include/tlb/tlb_NNNN.h, each .c including what it uses, the local prototypes those
        cover removed. The plan goes to permuter/headers/headers.txt; apply also writes it and
        compiles every file before and after: a file that is not identical keeps, marked FAKE,
        the prototypes of the header that changes it. TREE: another copy of this repository
    python canon-decompilation-tools/match_function.py calls
        every parked function whose game assembly has more jalr (calls through a register) than
        its C has calls through a pointer: write those calls as (*func)(...)
    python canon-decompilation-tools/match_function.py pads [src/heap.c func_800054A4_main]
        a parked function's declarations varied (a pad of 1-4 words at each position, each pad
        removed, each adjacent pair swapped), each compiled as `check` does; the best listed.
        Frame slots follow declaration order, so this finds a frame-only near miss by brute force.
        No arguments: every parked function, permuter/pads.txt
    python canon-decompilation-tools/match_function.py splits src/zanzou.c func_800F9120_ovl_0006
        a parked function with one FAKE `do {} while (0);` after each statement, case label and
        block opening in turn, and a stack of 8 there (SPLIT_STACKS, "1,8"), each compiled as
        `check` does; the best listed. One block boundary
        at a path's head can decide which equally used pointers get the saved registers
        (IDO_NOTES 228). No arguments: every parked function, one line each where a split
        helps, also written to permuter/splits.txt (hours: run it in the background).
        "splits src/f.c func greedy": keep the best and sweep again until none helps
    python canon-decompilation-tools/match_function.py greedy [src/pakfiler.c func_8017570C_ovl_0008]
        the modes that keep what the C computes (unfakes, pads, joins, copies, inlines, empties,
        splits, rechains, commutes, compounds, volatiles, moves, scopes) stacked unattended:
        each round keeps the first find that is better (fewer lines with the mix distance not
        rising, or as many with a lower mix). When a round keeps nothing it escapes: of the
        variants closer to the game than where it is stuck, ranked by (mix distance, stack
        distance, t-register shift, lines), it steps onto up to GREEDY_ESCAPES (default 3) even
        if their lines rise, descends from each, and keeps a path only if it ends better. It
        helps right after a structural change or a new mode, not on a stuck function. Read the diff,
        build to both hashes, commit (FAKE lines carry their FAKE comment). No arguments:
        every parked function, closest first (permuter/greedy_done.txt); "greedy START TAG" is a
        pass of its own (permuter/greedy_doneTAG.txt; names written there first are skipped)
    python canon-decompilation-tools/match_function.py empties [src/vanime.c func_80173E8C_ovl_0010]
        a FAKE "if (v) {}" of each parameter and local at each top-level statement after it is
        set (IDO_NOTES 209: it keeps v live there while IDO colours), compiled as `check` does.
        No arguments: every parked function, permuter/empties.txt
    python canon-decompilation-tools/match_function.py reloads [src/plmain.c func_8018A5F8_ovl_0009]
        after each one-line store to a field, element or global, a FAKE "if (<lvalue>) {}" in
        turn: the word reload right after a store that as1 deletes still spends one ugen
        temporary (IDO_NOTES 268), for a C whose t-registers run one behind the game's. No
        arguments: every parked function, permuter/reloads.txt
    python canon-decompilation-tools/match_function.py unfakes [src/plmain.c func_8018A270_ovl_0009]
        each FAKE in the function taken out in turn (a one-line FAKE statement removed, an
        "if (1) { /* FAKE */" wrapper unwrapped), compiled as `check` does: a FAKE found for an
        earlier form of the C can block the next step (IDO_NOTES 260). No arguments: every
        parked function, permuter/unfakes.txt
    python canon-decompilation-tools/match_function.py unfakes [src/x.c func_X] apply
        matched functions too: each FAKE that can be taken out mechanically is taken out when
        the function's code stays identical, row for row, and the C file is written back, until
        none can go. No file: every function in src/*.c with a FAKE line, permuter/unfakes_apply.txt
    python canon-decompilation-tools/match_function.py enum-args include/enekind.h eneMakeEx:1 [TREE] [apply]
        bare numbers at call sites to an enum's names: each FUNC:POS call whose argument POS is
        an integer literal gets the header's name for it. Dry run by default (the sites listed
        to permuter/enumargs/enumargs.txt); apply writes TREE's src/ and proves every object
        identical to TREE's build/src/<file>.o
    python canon-decompilation-tools/match_function.py rename <map.txt> [dry]
        address names to real ones: one "func_XXXXXXXX_<seg> name" per line; refuses duplicates
        and names already defined; whole words in src/ and include/; pins each in symbol_addrs.txt
        (with its segment). Then delete asm/ and run build_rom.py (a fresh split names the asm).
    python canon-decompilation-tools/match_function.py drops src/plchain.c func_80175968_ovl_0009
        a diagnostic: each one-line statement removed in turn, compiled as `check` does, listing
        where the frame, the address-taken locals or the stack slots move against the C as
        written. Locals sit on the compiler temps, so a statement whose removal moves them down
        makes a temp (or was a local's only use): where this C has temp words the game lacks
    python canon-decompilation-tools/match_function.py commutes [src/particle.c func_800F53B0_ovl_0006]
        each assignment's right side, return value and one-line if test whose top level is one
        commutative operator (+ * & | ^, one == or !=) with its operands swapped at the last
        occurrence, compiled as `check` does (IDO_NOTES 259). No arguments: every parked
        function, permuter/commutes.txt
    python canon-decompilation-tools/match_function.py reprotos [src/plmain.c func_8018A270_ovl_0009]
        each callee whose prototype in the file disagrees with its C definition (return type,
        parameter count or classes) given the definition's own prototype in turn, compiled as
        `check` does (IDO_NOTES 217, 260); the full build decides for the file's other
        functions. No arguments: every parked function, permuter/reprotos.txt
    python canon-decompilation-tools/match_function.py rechains [src/camera.c func_800CAEF8_ovl_0006]
        each chained assignment with its targets in every other order (up to four; longer ones
        reversed) and split into separate statements, and each two adjacent stores of the same
        constant joined into a chain both ways round, compiled as `check` does (a chain spends
        a temporary and stores its innermost target first, IDO_NOTES 255, 261). No arguments:
        every parked function, permuter/rechains.txt
    python canon-decompilation-tools/match_function.py widens [src/plmain.c func_8018A270_ovl_0009]
        each narrow local (u8, s8, u16, s16) declared s32, then u32, in turn, compiled as `check`
        does (a byte read once into a u8 kept a copy the game's s32 does not, IDO_NOTES 260).
        No arguments: every parked function, permuter/widens.txt
    python canon-decompilation-tools/match_function.py inlines [src/floorhit.c func_80196024_ovl_0009]
        each straight-line definition of a local substituted into its uses (and the local's
        declaration dropped when nothing else uses it), compiled as `check` does; compiler temps
        sit below every declared scalar, so fewer names move them. No arguments: every parked
        function, permuter/inlines.txt
    python canon-decompilation-tools/match_function.py joins [src/plmain.c func_8018A5F8_ovl_0009]
        each pair of adjacent one-line statements joined onto one line, and each one-statement
        block closed onto its opening line, compiled as `check` does; the best listed. IDO
        schedules by source line. No arguments: every parked function, permuter/joins.txt
    python canon-decompilation-tools/match_function.py copies [src/polygon.c func_800D66B8_ovl_0006]
        each parameter read through a copy ("p_cp = p;") from each top-level statement on, every
        later use renamed (the C computes the same), compiled as `check` does; the best listed.
        The game keeps a parameter it uses over a stretch in a saved register through a copy of
        its own (IDO_NOTES 241). No arguments: every parked function, permuter/copies.txt
    python canon-decompilation-tools/match_function.py casts [src/zanzou.c func_800F7C6C_ovl_0006]
        each one-line assignment's right side cast to u8, s8, u16, s16 in turn (a conversion
        to the destination's narrow type spends the game's temporary, IDO_NOTES 235); a find
        is kept only if the destination has that type. No arguments: every parked function
    python canon-decompilation-tools/match_function.py swaps [src/sndloc.c func_800D9CD4_ovl_0006]
        each pair of adjacent one-line statements swapped in turn, the best listed (a swap can
        change what the C computes: read every find). No arguments: every parked function
    python canon-decompilation-tools/match_function.py moves [src/gauge.c func_800DD314_ovl_0006]
        each one-line statement moved up or down (up to MOVES_MAX, 6, lines) past one-line
        statements at its indent that it provably commutes with (locals written against locals
        read or written; memory, calls and macros; address-taken, volatile and array locals);
        the C computes the same; a dead PRINTF moves past anything. Run alone it lists every
        move with its lines. No arguments: every parked function
    python canon-decompilation-tools/match_function.py scopes [src/vram.c func_800E3630_ovl_0006]
        each scalar or pointer local declared in a block instead of at the top: the innermost
        existing block holding all its uses, or a new block from its first use's statement to
        the end (block-scoped locals take the frame words just above the compiler temps, which
        moves their homes). No arguments: every parked function
    python canon-decompilation-tools/match_function.py compounds [src/camera.c func_X]
        each "x = x op y;" written "x op= y;" and each "x op= y;" written "x = x op (y);" (the
        two colour registers differently: camera.c func_800CAEF8, 130 to 11). No arguments:
        every parked function
    python canon-decompilation-tools/match_function.py volatiles [src/sys.c func_80004880_main]
        each scalar or pointer local declared volatile in turn (FAKE: a variable the game
        stores and reloads at once). No arguments: every parked function
    python canon-decompilation-tools/match_function.py best
        every decomp-permuter run's best find so far: base and best score, the last iteration,
        and the best output's changed lines (read-only; harvest the natural ones by hand)
    python canon-decompilation-tools/match_function.py screen
        after checkall: every parked function whose game and C differ in what register
        allocation cannot change (length, operation counts, 2^32 fix-ups, branch sequence, call
        order); one that agrees on all of these differs only in registers and scheduling.
        Also the frame size and the callee-saved registers (game/C): a register only the game
        saves is a variable only the game has
    python canon-decompilation-tools/match_function.py bases
        every parked function whose last checkall diff has %hi addresses on one side only: a
        base apart is a symbol the game named at its own address (undefined_syms.txt); several
        of the game's close together ("unrolled?") is a loop IDO unrolled fully
    python canon-decompilation-tools/match_function.py dprintf [src/enemy.c func_X]
        check a parked function with its PRINTF( written as the argument-only DPRINTF( (the
        game's own debug print), on a copy under permuter/; no arguments: every parked function
        that prints
    python canon-decompilation-tools/match_function.py unroll [src/gfxmgr.c func_800D5390_ovl_0006]
        a loop IDO unrolls differently from the game: the function's instruction count at uopt
        -unrolllimit 16..1280, printed at each step. The default limit is 320-323; a step at L
        means a loop body of about L/4 (unrolled by 4) or L/2 (by 2), so it tells how much
        heavier or lighter the C's loop must be (casts and u32 sums weigh; IDO_NOTES rule 44).
        No arguments: every parked function whose size at limit 160 or 640 is the game's while
        the default's is not
    python canon-decompilation-tools/match_function.py check src/contpak.c func_80001F24_main
        import, then compile full.c (the unpruned cc -E output: pruning re-prints the C one
        statement per line, and IDO schedules by source line) and diff the function alone against
        target.o, relocations resolved to addresses: prints the differing lines and MATCH or
        "N differing lines".
        Unlike asm-differ on the linked ROM, this is not thrown off when an earlier function in
        the same file changes size. An unparked function is compiled from the real file as the
        build compiles it (other parked C can change its scheduling), so after unparking a
        function, check it again. "MATCH except own section addresses": the instructions are
        the same, but the C's references to its own .rodata/.data/.bss are section-relative
        (unlinked) where the game's have addresses; the full build decides those.

base.c   the C file with NON_MATCHING defined and every GLOBAL_ASM line dropped, preprocessed by
         IDO's own `cc -E` with the build's flags (so the permuter sees exactly what IDO sees),
         then pruned to what the function uses by the permuter's own ast_util (as its import.py
         does); unpruned, libultra's headers make every iteration ~10x slower.
PERMUTER  the decomp-permuter checkout (default ../vendor/decomp-permuter).
target.o a C file holding only the function's GLOBAL_ASM, compiled by the build's own pipeline.
The permuter runs compile.sh with bash (on Windows, Git for Windows' bash). Output is the
permuter's own; a result counts only when pasted into src/ and both ROM hashes match.

Changelog: first public release (2026-10-20). Earlier history is kept privately.
"""

import glob
import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import build_rom as build  # noqa: E402

GLOBAL_ASM = re.compile(r'^\s*#pragma\s+GLOBAL_ASM\(.*\)\s*$', re.M)
# a parked function: "#ifdef NON_MATCHING" C "#else" its GLOBAL_ASM (and maybe declarations)
# "#endif"; a block whose #else holds no GLOBAL_ASM is not one (the lambda keeps it whole)
PARKED = re.compile(r'^#ifdef NON_MATCHING\n((?:(?!^#(?:if|else|endif)).)*?)^#else\n'
                    r'((?:(?!^#(?:if|else|endif)).)*?)^#endif\n?', re.M | re.S)
PERMUTER = os.environ.get("PERMUTER", os.path.join(build.ROOT, "..", "vendor", "decomp-permuter"))


def prune(source, func):
    """Keep only the declarations func uses (decomp-permuter's import.py:prune_source)."""
    sys.path.insert(0, PERMUTER)
    from src import ast_util
    ast = ast_util.parse_c(mark_samelines(source, func), from_import=True)
    fn, _ = ast_util.extract_fn(ast, func)
    ast_util.prune_ast(fn, ast)
    return ast_util.to_c_raw(ast)


SAMELINE_KEYWORD = re.compile(r"\b(?:if|for|while|do|switch)\b")


def mark_samelines(source, func):
    """Wrap each line of func's body that holds more than one statement (a macro's expansion,
    "if (0) osSyncPrintf(...)", statements written on one line) in decomp-permuter's sameline
    pragmas. The reprint puts every statement on its own line, but IDO schedules by source line
    (IDO_NOTES 12): without the pragmas the permuter compiled a different function than `check`
    (basecheck), and its finds did not carry over. With them, each candidate joins those
    statements back onto one line before it is compiled (ast_util.process_pragmas)."""
    lines = source.split("\n")
    # the definition: at column 0 (a call in an earlier function is indented: plmain.c's
    # handler calls func_8018A5F8 before its definition, and its body was marked instead)
    start = next((i for i, l in enumerate(lines)
                  if re.search(r"\b%s\s*\(" % re.escape(func), l) and not l.rstrip().endswith(";")
                  and l[:1].strip() and not l.startswith("#")), None)
    if start is None:
        return source
    while "{" not in lines[start]:
        start += 1
    out, depth, i = lines[:start + 1], lines[start].count("{") - lines[start].count("}"), start + 1
    while i < len(lines) and depth > 0:
        line = lines[i]
        parens = braces = semis = 0
        balanced, quote, prev = True, None, ""
        for ch in line:
            if quote:
                if ch == quote and prev != "\\":
                    quote = None
            elif ch in "\"'":
                quote = ch
            elif ch == "(":
                parens += 1
            elif ch == ")":
                parens -= 1
            elif ch == "{":
                braces += 1
            elif ch == "}":
                braces -= 1
                balanced = balanced and braces >= 0
            elif ch == ";" and parens == 0:
                semis += 1
            prev = ch
        stripped = line.strip()
        many = semis >= 2 or (semis >= 1 and SAMELINE_KEYWORD.search(line) is not None)
        if (stripped and not stripped.startswith("#") and parens == 0 and braces == 0 and balanced
                and stripped[-1] in ";}" and many):
            out += ["#pragma _permuter sameline start", line, "#pragma _permuter sameline end"]
        else:
            out.append(line)
        depth += braces
        i += 1
    return "\n".join(out + lines[i:])


def fwd(path):
    """Absolute, forward slashes: asm-processor hands paths to cmd.exe (see build.tool_path)."""
    return os.path.abspath(path).replace("\\", "/")


def compile_one(src, out, extra=()):
    return subprocess.run(build.compile_c(fwd(src), fwd(out), extra), cwd=build.ROOT,
                          env=build.ENV).returncode


def import_function(c_file, func, out=None):
    """out: permuter/<func> unless given (the unroll sweep uses its own, so it never rewrites a
    directory a live permuter run or another check is using)."""
    # a matched function's asm is in asm/matchings/ (splat's disassemble_all), a parked one's in
    # asm/nonmatchings/
    # the C file's own folder (src/tlb/tlb_0182.c: asm/nonmatchings/tlb/tlb_0182/), else any
    stem = os.path.splitext(os.path.relpath(os.path.join(build.ROOT, c_file), os.path.join(build.ROOT, "src")))[0]
    asm = [p for kind in ("nonmatchings", "matchings")
           for p in glob.glob(os.path.join(build.ROOT, "asm", kind, stem, func + ".s"))]
    # else anywhere below asm/[non]matchings/ (a working copy under permuter/ of a src/tlb/ file)
    asm = asm or glob.glob(os.path.join(build.ROOT, "asm", "nonmatchings", "**", func + ".s"), recursive=True) + \
        glob.glob(os.path.join(build.ROOT, "asm", "matchings", "**", func + ".s"), recursive=True)
    if len(asm) != 1:
        sys.exit(f"{func}: expected one asm/[non]matchings/{stem}/{func}.s, found {len(asm)}")
    out = out or os.path.join(build.ROOT, "permuter", func)
    os.makedirs(out, exist_ok=True)

    # UTF-8 throughout: the game's strings (EUC-JP) are written as UTF-8 and converted back
    # when compiled (canon-decompilation-tools/build_rom.py's compile_c).
    with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
        source = f.read()
    # the other parked functions as the build sees them: their GLOBAL_ASM, not their C (a
    # parked function's C changes how IDO compiles the next function; tlb_0018_6, (697))
    defines = re.compile(r"\b%s\s*\([^;{]*\)\s*\{" % re.escape(func))
    source = PARKED.sub(lambda m: m.group(0) if defines.search(m.group(1))
                        or not GLOBAL_ASM.search(m.group(2)) else m.group(2), source)
    source = GLOBAL_ASM.sub("", source)
    if not re.search(r"\b%s\s*\(" % re.escape(func), source):
        sys.exit(f"{func} is not written in {c_file}")
    # every write here retries while Windows holds the file (greedy pass 4 died on full.c)
    if not _write_retry(os.path.join(out, "source.c"), source):
        sys.exit(f"{out}/source.c: locked")
    cflags = [a for a in build.CFLAGS if a != "-c"]
    r = subprocess.run([build.IDO, "-E", *cflags, "-DNON_MATCHING", fwd(os.path.join(out, "source.c"))],
                       cwd=build.ROOT, env=build.ENV, capture_output=True, text=True, encoding="utf-8")
    if r.returncode:
        sys.exit(f"cc -E failed:\n{r.stderr}")
    # unpruned: `check` compiles this, since IDO schedules by source line
    if not _write_retry(os.path.join(out, "full.c"), r.stdout):
        sys.exit(f"{out}/full.c: locked")
    # IDO's cpp writes the source's absolute path ("C:/...") in its line markers; the
    # permuter's parse-error report splits "file:line" on ':', so the parser gets the markers
    # without the drive letter; full.c, which check compiles, is untouched
    base = prune(re.sub(r'^(#\s*\d+\s+")[A-Za-z]:/', r"\1/", r.stdout, flags=re.M), func)
    if not _write_retry(os.path.join(out, "base.c"), base):
        sys.exit(f"{out}/base.c: locked")

    rel = os.path.relpath(asm[0], build.ROOT).replace("\\", "/")
    with open(os.path.join(out, "target.c"), "w", newline="\n") as f:
        f.write(f'#pragma GLOBAL_ASM("{rel}")\n')
    if compile_one(os.path.join(out, "target.c"), os.path.join(out, "target.o")):
        sys.exit("target.o failed to build")

    with open(os.path.join(out, "compile.sh"), "w", newline="\n") as f:
        f.write(f'#!/bin/bash\n"{fwd(sys.executable)}" "{fwd(__file__)}" compile "$@"\n')
    objdump = build.tool_path(os.path.join(build.BINUTILS, "mips64-elf-objdump" + build.EXE))
    with open(os.path.join(out, "settings.toml"), "w", newline="\n") as f:
        f.write(f'func_name = "{func}"\ncompiler_type = "ido"\n'
                f'objdump_command = "{objdump} -drz -m mips:4300"\n')
    print(f"{os.path.relpath(out, build.ROOT)}: base.c, target.o, compile.sh, settings.toml")


MAP_ADDRESSES = {}


def _call_address(name):
    """A call target's address as a string: from a func_XXXXXXXX name, else from the linker map
    (build/goemonsgreatadv.map, read once), else the name itself. Takes a row's last token: the
    game's "0x80011A40" or this C's relocation "guAlignF]"."""
    name = name.rstrip("]")
    m = re.match(r"(?:0x|func_)([0-9A-Fa-f]{8})\b", name)
    if m:
        return m.group(1).lower()
    if not MAP_ADDRESSES:
        path = os.path.join(build.ROOT, "build", "goemonsgreatadv.map")
        if os.path.exists(path):
            with open(path, encoding="utf-8", errors="replace") as f:
                for mm in re.finditer(r"^\s+0x(?:00000000)?([0-9a-f]{8})\s+([A-Za-z_]\w*)\s*$", f.read(), re.M):
                    MAP_ADDRESSES.setdefault(mm.group(2), mm.group(1))
        MAP_ADDRESSES.setdefault("", "")
    return MAP_ADDRESSES.get(name, name)


def symbol_addresses():
    """name -> address: symbol_addrs.txt and undefined_syms.txt (splat's own D_/func_ names carry
    their address in the name and need no table)."""
    table = {}
    for name in ("symbol_addrs.txt", "undefined_syms.txt"):
        path = os.path.join(build.ROOT, name)
        if os.path.exists(path):
            with open(path) as f:
                for m in re.finditer(r"^\s*(\w+)\s*=\s*(0x[0-9A-Fa-f]+)", f.read(), re.M):
                    table[m.group(1)] = int(m.group(2), 16)
    return table


def symbol_address(name, table):
    m = re.match(r"^(?:D|func|jtbl)_([0-9A-F]{6,8})(?:_|$)", name)
    return int(m.group(1), 16) if m else table.get(name)


def disassemble(obj, func):
    """func's instructions, one per line, with each relocated operand resolved to the address it
    links to (symbol + the addend in the instruction), so `D_80080430` and `D_80080188 + 0x2A8`
    compare equal, as they do in the ROM."""
    objdump = build.tool_path(os.path.join(build.BINUTILS, "mips64-elf-objdump" + build.EXE))
    text = subprocess.run([objdump, "-drz", "-m", "mips:4300", obj], capture_output=True, text=True).stdout
    rows, inside = [], False  # [instruction, reloc type, symbol]
    for line in text.splitlines():
        if re.match(r"^[0-9a-f]+ <", line):
            inside = line.endswith(f"<{func}>:")
            continue
        if not inside or not line.strip():
            continue
        if re.match(r"^\s*[0-9a-f]+:\t[0-9a-f]", line):
            ins = " ".join(p.strip() for p in line.split("\t")[2:])
            rows.append([re.sub(r"\s+", " ", re.sub(r"[0-9a-f]+ <%s\+(0x[0-9a-f]+)>" % re.escape(func),
                                                   r"<+\1>", ins)), None, None])
        elif rows and "R_MIPS" in line:
            kind, sym = line.strip().split(":", 1)[-1].split()
            rows[-1][1:] = [kind, sym]
    table = symbol_addresses()
    imm = lambda ins: int(re.search(r"(-?(?:0x[0-9a-f]+|\d+))(?:\(\w+\))?$", ins).group(1), 0)
    out = []
    for i, (ins, kind, sym) in enumerate(rows):
        addr = symbol_address(sym, table) if sym else None
        if kind == "R_MIPS_HI16" and addr is not None:
            # Its %lo is the next LO16 on the same symbol that uses the lui's register (scheduled
            # pairs on one symbol interleave: lui a0; lui a1; addiu a1,a1; addiu a0,a0).
            reg = re.match(r"lui (\w+),", ins).group(1)
            uses = re.compile(r"(\(%s\)$|^\w+ \w+,%s,)" % (reg, reg))
            lo = next((imm(r[0]) for r in rows[i + 1:]
                       if r[1] == "R_MIPS_LO16" and r[2] == sym and uses.search(r[0])), 0)
            full = addr + (imm(ins) << 16) + ((lo & 0xFFFF) ^ 0x8000) - 0x8000
            ins = re.sub(r",[^,]+$", ",%%hi(0x%08X)" % (full & 0xFFFFFFFF), ins)
        elif kind == "R_MIPS_LO16" and addr is not None:
            # The addend's high half is in its %hi: the nearest earlier HI16 on the same symbol
            # whose lui register this instruction uses as its base (or addiu source).
            base = re.search(r"\((\w+)\)$|^\w+ \w+,(\w+),", ins)
            reg = base and (base.group(1) or base.group(2))
            hi = next((imm(r[0]) for r in reversed(rows[:i]) if r[1] == "R_MIPS_HI16" and r[2] == sym
                       and re.match(r"lui %s," % reg, r[0])), 0) if reg else 0
            full = addr + (hi << 16) + ((imm(ins) & 0xFFFF) ^ 0x8000) - 0x8000
            ins = re.sub(r"-?(?:0x[0-9a-f]+|\d+)(\(\w+\))?$", r"%%lo(0x%08X)\1" % (full & 0xFFFFFFFF), ins)
        elif kind == "R_MIPS_26" and addr is not None:
            ins = re.sub(r"\S+$", "0x%08X" % addr, ins)
        elif kind:
            ins += f" [{kind} {sym}]"
        out.append(ins)
    while out and out[-1] == "nop":  # the file's 16-byte alignment padding, not the function's
        out.pop()
    return out


def _float_loads(obj, func):
    """The values func loads with lwc1/ldc1 from obj's own sections, in order, as hex: each
    load's %lo with its %hi (the nearest earlier lui on the same symbol into its base register)
    and the symbol's value, read from the section's bytes. Loads of other files' symbols are
    left out (the link decides them)."""
    objdump = build.tool_path(os.path.join(build.BINUTILS, "mips64-elf-objdump" + build.EXE))
    text = subprocess.run([objdump, "-drz", "-m", "mips:4300", obj], capture_output=True, text=True).stdout
    rows, inside = [], False
    for line in text.splitlines():
        if re.match(r"^[0-9a-f]+ <", line):
            inside = line.endswith(f"<{func}>:")
            continue
        if not inside or not line.strip():
            continue
        if re.match(r"^\s*[0-9a-f]+:\t[0-9a-f]", line):
            rows.append([re.sub(r"\s+", " ", " ".join(p.strip() for p in line.split("\t")[2:])), None, None])
        elif rows and "R_MIPS" in line:
            kind, sym = line.strip().split(":", 1)[-1].split()
            rows[-1][1:] = [kind, sym]
    syms = {}
    for line in subprocess.run([objdump, "-t", obj], capture_output=True, text=True).stdout.splitlines():
        m = re.match(r"^([0-9a-f]+)\s.{7}\s(\S+)\s+[0-9a-f]+\s+(\S+)$", line)
        if m and m.group(2) not in ("*UND*", "*ABS*", ".text"):
            syms[m.group(3)] = (m.group(2), int(m.group(1), 16))
    sections = {}

    def section_bytes(sec):
        if sec not in sections:
            out = subprocess.run([objdump, "-s", "-j", sec, obj], capture_output=True, text=True).stdout
            data = bytearray()
            for l in out.splitlines():
                m = re.match(r"^ ([0-9a-f]{4,}) ((?:[0-9a-f]{2,8} ){1,4})", l)
                if m:
                    off = int(m.group(1), 16)
                    chunk = bytes.fromhex(m.group(2).replace(" ", ""))
                    data[off:off + len(chunk)] = chunk
            sections[sec] = bytes(data)
        return sections[sec]

    imm = lambda ins: int(re.search(r"(-?(?:0x[0-9a-f]+|\d+))(?:\(\w+\))?$", ins).group(1), 0)
    values = {}  # instruction index -> the value it loads
    for i, (ins, kind, sym) in enumerate(rows):
        if kind != "R_MIPS_LO16" or not re.match(r"(lwc1|ldc1) ", ins):
            continue
        if sym in syms:
            sec, base = syms[sym]
        elif sym.startswith("."):
            sec, base = sym, 0
        else:
            continue
        reg = re.search(r"\((\w+)\)$", ins).group(1)
        hi = next((imm(r[0]) for r in reversed(rows[:i]) if r[1] == "R_MIPS_HI16" and r[2] == sym
                   and re.match(r"lui %s," % reg, r[0])), 0)
        off = base + (hi << 16) + ((imm(ins) & 0xFFFF) ^ 0x8000) - 0x8000
        width = 8 if ins.startswith("ldc1") else 4
        values[i] = section_bytes(sec)[off:off + width].hex()
    return values


SECTION_REL = re.compile(r" \[R_MIPS_(?:HI16|LO16) \.\w+\]$")


def normalize(ins):
    """An instruction with its relocated operand replaced by REL: the game's `%hi(0x8010C76C)` /
    `%lo(...)`, or this object's section-relative one (`lui at,0x0 [R_MIPS_HI16 .rodata]`, whose
    address only the link decides)."""
    if SECTION_REL.search(ins):
        ins = re.sub(r"-?(?:0x[0-9a-f]+|\d+)(\(\w+\))?$", r"REL\1", SECTION_REL.sub("", ins))
    return re.sub(r"%(?:hi|lo)\(0x[0-9A-F]+\)", "REL", ins)


def _compare(c_file, func, d):
    """check's compile and diff in d: (differing lines, the game's, this C's, parked)."""
    import difflib
    import_function(c_file, func, d)
    with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
        parked = re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), f.read()) is not None
    if parked:
        obj = os.path.join(d, "full.o")
        if compile_one(os.path.join(d, "full.c"), obj):
            sys.exit("full.c failed to compile")
    else:
        obj = os.path.join(d, "build.o")
        if compile_one(os.path.join(build.ROOT, c_file), obj):
            sys.exit(f"{c_file} failed to compile")
    want, got = disassemble(os.path.join(d, "target.o"), func), disassemble(obj, func)
    diff = [l for l in difflib.unified_diff(want, got, "target", "base", n=0, lineterm="")
            if l[:1] in "+-" and not l.startswith(("+++", "---"))]
    return diff, want, got, parked


def _stack_map(rows):
    """The frame size and every sp offset a function touches (homes, spills, locals, &local)."""
    frame, slots = None, set()
    for r in rows:
        m = re.match(r"addiu sp,sp,(-\d+)$", r)
        if m:
            frame = -int(m.group(1))
            continue
        m = re.search(r",(-?\d+)\(sp\)$", r) or re.match(r"addiu \w+,sp,(\d+)$", r)
        if m:
            slots.add(int(m.group(1)))
    return frame, sorted(slots)


def _mix_distance(want, got):
    """The instruction-mix distance: the sum, over opcodes, of the difference in counts."""
    from collections import Counter
    cw, cg = Counter(r.split()[0] for r in want), Counter(r.split()[0] for r in got)
    return sum(abs(cw[op] - cg[op]) for op in set(cw) | set(cg))


def _write_context_diff(want, got, path):
    """want against got with own-section addresses normalised, 3 lines of context, each line
    prefixed by its instruction index (game side for - and context, this C's for +), to path;
    prints the first index where they differ."""
    import difflib
    nw, ng = [normalize(r) for r in want], [normalize(r) for r in got]
    out, first = [], None
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, nw, ng, autojunk=False).get_opcodes():
        if tag == "equal":
            continue
        first = i1 if first is None else first
        out.append("@@ game %d-%d, this C %d-%d" % (i1, i2, j1, j2))
        out += ["  %4d  %s" % (k, want[k]) for k in range(max(0, i1 - 3), i1)]
        out += ["- %4d  %s" % (k, want[k]) for k in range(i1, i2)]
        out += ["+ %4d  %s" % (k, got[k]) for k in range(j1, j2)]
        out += ["  %4d  %s" % (k, want[k]) for k in range(i2, min(len(want), i2 + 3))]
    with open(path, "w", encoding="utf-8") as f:
        f.write("\n".join(out) + "\n")
    if first is not None:
        print(f"first real difference: instruction {first} ({path})")


def _jump_tables(obj, func):
    """func's jump tables in obj: for each table (in its section's order), the instruction index
    of each entry's target, relative to func's start. A table is a run of consecutive R_MIPS_32
    words in a data section relocated against .text (the addend is the word itself)."""
    objdump = build.tool_path(os.path.join(build.BINUTILS, "mips64-elf-objdump" + build.EXE))
    run = lambda *a: subprocess.run([objdump, *a, obj], capture_output=True, text=True).stdout
    start = size = None
    for line in run("-t").splitlines():
        p = line.split()
        if p and p[-1] == func and " F " in line:
            start, size = int(p[0], 16), int(p[-2], 16)
    if start is None:
        return []
    relocs, sec = {}, None  # section -> [offsets of R_MIPS_32 against .text]
    for line in run("-r").splitlines():
        m = re.match(r"RELOCATION RECORDS FOR \[(\S+)\]", line)
        if m:
            sec = m.group(1)
            continue
        p = line.split()
        if sec and len(p) == 3 and p[1] == "R_MIPS_32" and p[2] == ".text":
            relocs.setdefault(sec, []).append(int(p[0], 16))
    tables = []
    for sec, offs in relocs.items():
        words = {}
        for line in run("-s", "-j", sec).splitlines():
            m = re.match(r"^ ([0-9a-f]+) ((?:[0-9a-f]{1,8} ?){1,4})", line)
            if m:
                base = int(m.group(1), 16)
                for k, w in enumerate(m.group(2).split()):
                    if len(w) == 8:
                        words[base + 4 * k] = int(w, 16)
        run_, last = [], None
        for o in sorted(offs) + [None]:
            if o is None or (last is not None and o != last + 4):
                targets = [(words.get(x, -1) - start) // 4 for x in run_]
                if targets and all(0 <= t < size // 4 for t in targets):
                    tables.append(targets)
                run_ = []
            if o is not None:
                run_.append(o)
                last = o
    return tables


def _case_lengths(want_obj, got_obj, func, n_want, n_got):
    """Prints, per jump table, the cases whose code lengths differ between the game and this C.
    A case's length runs from its target to the next higher target of the same table (the last
    one to the function's end), so it includes the shared code a case falls into."""
    tw, tg = _jump_tables(want_obj, func), _jump_tables(got_obj, func)
    if len(tw) != len(tg):
        print(f"jump tables: game {len(tw)}, this C {len(tg)}")
        return
    for k, (a, b) in enumerate(zip(tw, tg)):
        if len(a) != len(b):
            print(f"jump table {k}: game {len(a)} entries, this C {len(b)}")
            continue
        length = lambda t, n: {x: (min([y for y in t if y > x], default=n) - x) for x in t}
        la, lb = length(a, n_want), length(b, n_got)
        bad = [f"case {i} (game @{x}) {la[x]} vs {lb[y]}" for i, (x, y) in enumerate(zip(a, b))
               if la[x] != lb[y]]
        if bad:
            print(f"jump table {k}, cases whose lengths differ: " + "; ".join(dict.fromkeys(bad)))


def check(c_file, func, d=None):
    """A parked function is compiled from full.c (NON_MATCHING defined). An unparked one is
    compiled from the real file as canon-decompilation-tools/build_rom.py compiles it (no NON_MATCHING, the other parked
    functions as GLOBAL_ASM): the file's other parked C can change how IDO schedules it
    (eneproc.c func_8019D748: two li's hoisted over a branch only with NON_MATCHING defined).
    d: the work directory, permuter/<func> unless given (checkall uses its own)."""
    d = d or os.path.join(build.ROOT, "permuter", func)
    diff, want, got, parked = _compare(c_file, func, d)
    if not parked:
        print(f"(unparked: compiled as the build does)")
    print("\n".join(diff))
    _write_context_diff(want, got, os.path.join(d, "diff.txt"))
    absolute = re.compile(r"%(?:hi|lo)\((0x[0-9A-F]+)\)")
    if diff and len(want) == len(got) and all(normalize(a) == normalize(b) for a, b in zip(want, got)) \
            and not any(absolute.search(a) and absolute.search(b) and absolute.findall(a) != absolute.findall(b)
                        for a, b in zip(want, got)):
        # Only its own .rodata/.data/.bss addresses differ, which the link fills in: the full
        # build (both hashes) decides. But the constants it loads must be the game's values.
        vw = _float_loads(os.path.join(d, "target.o"), func)
        vg = _float_loads(os.path.join(d, "full.o" if parked else "build.o"), func)
        # only where both sides are known: a game load of another file's symbol is left out
        bad = [(k, vw[k], vg[k]) for k in sorted(vw) if k in vg and vw[k] != vg[k]]
        if bad:
            print(f"{func}: the code matches, but {len(bad)} of the constants it loads differ: "
                  + ", ".join(f"instruction {k}: game {a}, this C {b}" for k, a, b in bad[:6]))
            return 1
        print(f"{func}: MATCH except own section addresses ({len(want)} lines)")
        return 0
    from collections import Counter
    # the instruction-mix distance: 0 when this C has the game's instructions (only registers,
    # slots and schedule differ); the permuter weighs an inserted or deleted instruction 20x a
    # register difference, `check`'s line count weighs them the same
    cw, cg = Counter(r.split()[0] for r in want), Counter(r.split()[0] for r in got)
    mix = sum(abs(cw[op] - cg[op]) for op in set(cw) | set(cg))
    sw, sg = _stack_map(want), _stack_map(got)
    if diff and sw != sg:
        # the stack map, in hex: which slots the game uses that this C does not, and the reverse
        show = lambda m: f"frame 0x{m[0] or 0:X} [{' '.join('%X' % s for s in m[1])}]"
        print(f"stack: game {show(sw)}; this C {show(sg)}")
    if diff and len(want) != len(got):
        _case_lengths(os.path.join(d, "target.o"), os.path.join(d, "full.o" if parked else "build.o"), func,
                      len(want), len(got))
    real = _real_lines(want, got) if diff else 0
    print(f"{func}: " + ("MATCH" if not diff else f"{len(diff)} differing lines") + f" ({len(want)} lines)"
          + (f"; this C {len(got)}, mix distance {mix}; real {real}, length {len(got) - len(want):+d}"
             if diff else ""))
    return 1 if diff else 0


def _real_lines(want, got):
    """Differing lines once every branch target reads the same: what is left when the shift a
    length difference makes in every later target is not counted."""
    import difflib
    t = re.compile(r"<\+0x[0-9a-fA-F]+>|\.L[0-9A-F]{8}\w*")
    # own-section addresses read as the same, as check's "MATCH except own section addresses"
    a = [normalize(t.sub("<T>", str(x))) for x in want]
    b = [normalize(t.sub("<T>", str(x))) for x in got]
    sm = difflib.SequenceMatcher(None, a, b, autojunk=False)
    absolute = re.compile(r"%(?:hi|lo)\((0x[0-9A-Fa-f]+)\)")
    n = 0
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag != "equal":
            n += max(i2 - i1, j2 - j1)
            continue
        # aligned as equal: both sides' absolute addresses must still be the same
        for x, y in zip(want[i1:i2], got[j1:j2]):
            ax, ay = absolute.findall(str(x)), absolute.findall(str(y))
            if ax and ay and ax != ay:
                n += 1
    return n


def basecheck(func, d=None):
    """Whether a permuter directory's base.c compiles to the same code as its full.c. base.c is
    the pruned, reprinted source the permuter mutates (pycparser puts each statement of a macro
    on its own line); full.c is the preprocessed file `check` compiles, a macro call still on
    one line. IDO schedules by source line (IDO_NOTES 12), so where the two differ the permuter
    scores its variants against code the real C does not produce, and its finds do not carry
    over. Read-only: both objects go to a temporary directory."""
    import difflib
    import tempfile
    sys.path.insert(0, PERMUTER)
    from src import ast_util
    d = d or os.path.join(build.ROOT, "permuter", func)
    tmp = tempfile.mkdtemp(prefix="basecheck")
    objs = []
    for name in ("base", "full"):
        with open(os.path.join(d, name + ".c"), encoding="utf-8") as f:
            text = f.read()
        if name == "base":
            text = ast_util.process_pragmas(text)  # as the permuter compiles it
        src = os.path.join(tmp, name + ".c")
        with open(src, "w", newline="\n", encoding="utf-8") as f:
            f.write(text)
        obj = os.path.join(tmp, name + ".o")
        if compile_one(src, obj):
            sys.exit(f"{name}.c failed to compile")
        # own-section addresses normalised: the pruned base holds less .rodata, so its offsets differ
        objs.append([normalize(r) for r in disassemble(obj, func)])
    diff = [l for l in difflib.unified_diff(objs[0], objs[1], "base", "full", n=0, lineterm="")
            if l[:1] in "+-" and not l.startswith(("+++", "---"))]
    for l in difflib.unified_diff(objs[0], objs[1], "base", "full", n=2, lineterm=""):
        print(l)  # - base.c, + full.c, two lines of context to find the statement
    print(f"{func}: base.c " + ("compiles as full.c" if not diff else
                                f"differs from full.c by {len(diff)} lines") + f" ({len(objs[1])} lines)")
    return 1 if diff else 0


def parked_functions():
    """(file, func) for every function a src/*.c file still pulls in with GLOBAL_ASM inside an
    #ifdef NON_MATCHING ... #else block."""
    out = []
    for path in sorted(glob.glob(os.path.join(build.ROOT, "src", "**", "*.c"), recursive=True)):
        in_else = False
        with open(path, encoding="utf-8") as f:
            for line in f:
                if line.startswith("#ifdef NON_MATCHING"):
                    in_else = False
                elif line.startswith("#else"):
                    in_else = True
                elif line.startswith("#endif"):
                    in_else = False
                elif in_else:
                    m = re.match(r'\s*#pragma\s+GLOBAL_ASM\("[^"]*/(\w+)\.s"\)', line)
                    if m:
                        out.append((os.path.relpath(path, build.ROOT).replace("\\", "/"), m.group(1)))
    return out


def checkall(out_path):
    """check every parked function (one subprocess each) and write `differing-lines file func
    verdict` per line to out_path, closest first. Its work directories and logs are under
    permuter/checkall/, so it never rewrites a directory a live permuter run or another check
    is using."""
    rows = []
    base = os.path.join(build.ROOT, "permuter", "checkall")
    os.makedirs(base, exist_ok=True)
    for c_file, func in parked_functions():
        log = os.path.join(base, f"chk_{func}.log")
        with open(log, "w", encoding="utf-8") as f:
            subprocess.run([sys.executable, os.path.abspath(__file__), "check", c_file, func,
                            os.path.join(base, func)],
                           cwd=build.ROOT, stdout=f, stderr=subprocess.STDOUT)
        with open(log, encoding="utf-8") as f:
            lines = [l for l in f.read().splitlines() if l.startswith(func + ":")]
        verdict = lines[-1] if lines else f"{func}: FAILED"
        m = re.search(r"real (\d+)", verdict) or re.search(r": (\d+) differing", verdict)
        n = int(m.group(1)) if m else (0 if "MATCH" in verdict else 99999)
        rows.append((n, c_file, func, verdict))
        print(verdict, flush=True)
    rows.sort()
    with open(out_path, "w", encoding="utf-8") as f:
        for n, c_file, func, verdict in rows:
            f.write(f"{n}\t{c_file}\t{func}\t{verdict}\n")
    return 0


SYMBOL = re.compile(r'\b(?:D|func|jtbl|sym|B|L)_[0-9A-F]{6,8}(?:_\w+)?\b')
LABEL = re.compile(r'\.L([0-9A-F]{8})_\w+')
ASM_LINE = re.compile(r'^\s*/\* [0-9A-F]+ ([0-9A-F]{8}) [0-9A-F]{8} \*/\s+(.*)$')


def asm_shape(path):
    """A function's .s as a tuple of instructions with every symbol abstracted and every local
    label as its offset from the function's start, so two functions that differ only in what
    they reference compare equal. Only the .text part counts."""
    out = []
    start = None
    in_text = True
    with open(path, encoding="utf-8") as f:
        for line in f:
            if line.startswith(".section"):
                in_text = ".text" in line
                continue
            m = ASM_LINE.match(line)
            if not (in_text and m):
                continue
            addr = int(m.group(1), 16)
            if start is None:
                start = addr
            ins = LABEL.sub(lambda l: "L+%X" % (int(l.group(1), 16) - start), m.group(2))
            out.append(" ".join(SYMBOL.sub("S", ins).split()))
    return tuple(out)


def twins():
    """For every parked function, the functions (parked or not) with the same asm shape."""
    parked = parked_functions()
    parked_names = {func for _, func in parked}
    shapes = {}
    for path in glob.glob(os.path.join(build.ROOT, "asm", "nonmatchings", "**", "func_*.s"), recursive=True):
        shape = asm_shape(path)
        if len(shape) >= 4:
            name = os.path.splitext(os.path.basename(path))[0]
            shapes.setdefault(shape, []).append((os.path.basename(os.path.dirname(path)), name))
    by_func = {name: (shape, folder) for shape, fs in shapes.items() for folder, name in fs}
    for c_file, func in parked:
        if func not in by_func:
            continue
        shape, _ = by_func[func]
        others = [(folder, name) for folder, name in shapes[shape] if name != func]
        if not others:
            continue
        done = [f"{folder}/{name}" for folder, name in others if name not in parked_names]
        still = [f"{folder}/{name}" for folder, name in others if name in parked_names]
        print(f"{c_file} {func} ({len(shape)} ins): matched twins {done or '-'}; parked twins {still or '-'}")
    return 0


def twins_all(listing=False):
    """Every function still in assembly (a GLOBAL_ASM in any src/ C file, src/tlb/ included,
    parked or not) against every function's asm shape (twin-copy's loose shape): those with a
    twin already in C (its C is the template), and the same-shape clusters still all assembly
    (one match, then copies). listing: every cluster's members (folder/function), smallest
    first. The asm-only files (asm/<ROM>.s, no C file yet) are counted apart."""
    import collections
    pulled = set()
    for path in glob.glob(os.path.join(build.ROOT, "src", "**", "*.c"), recursive=True):
        with open(path, encoding="utf-8") as f:
            pulled.update(os.path.splitext(os.path.basename(p))[0]
                          for p in re.findall(r'GLOBAL_ASM\("([^"]+)"\)', f.read()))
    shapes, folder = collections.defaultdict(list), {}
    for kind in ("nonmatchings", "matchings"):
        for path in glob.glob(os.path.join(build.ROOT, "asm", kind, "**", "*.s"), recursive=True):
            name = os.path.splitext(os.path.basename(path))[0]
            if name.startswith(("D_", "jtbl_")) or name in folder or _folded_empties(path):
                continue
            fn = _ida_function(path)  # IDA's view; splat's .s without an export
            shape = ("ida",) + _ida_shape(fn) if fn else ("splat",) + _loose_shape(path)
            if len(shape) >= 5:
                shapes[shape].append(name)
                folder[name] = os.path.relpath(os.path.dirname(path), os.path.join(build.ROOT, "asm", kind))
    with_c, clusters, alone, total = 0, [], 0, 0
    for shape, names in shapes.items():
        asm = [n for n in names if n in pulled]
        if not asm:
            continue
        total += len(asm)
        if len(asm) < len(set(names)):
            with_c += len(asm)
        elif len(asm) > 1:
            clusters.append((len(asm), len(shape), asm[0], asm))
        else:
            alone += 1
    if listing:
        for n, size, first, members in sorted(clusters, key=lambda c: (c[1], -c[0])):
            print(f"cluster {n} x {size} instructions: "
                  + " ".join(f"{folder[m].replace(os.sep, '/')}/{m}" for m in members))
    clusters = [c[:3] for c in clusters]
    clusters.sort(reverse=True)
    in_clusters = sum(n for n, _, _ in clusters)
    print(f"functions still assembly (4+ instructions): {total}")
    print(f"  with a same-shape twin already in C (its C is the template): {with_c}")
    print(f"  in {len(clusters)} same-shape clusters, all still assembly: {in_clusters} "
          f"(one match per cluster covers {in_clusters - len(clusters)} more)")
    print(f"  alone (no twin anywhere): {alone}")
    whole = 0
    for path in glob.glob(os.path.join(build.ROOT, "asm", "*.s")):
        with open(path, encoding="utf-8") as f:
            whole += len(re.findall(r"^nonmatching\s+func_", f.read(), re.M))
    print(f"  plus {whole} functions in asm-only files (asm/<ROM>.s; not split per function, not compared)")
    for n, size, first in clusters[:15]:
        print(f"    cluster of {n} x {size} instructions, e.g. {first}")
    return 0


def ida_starts(write=False, only=None):
    """Each split function whose start has no IDA function in its database's export, counted
    per database; write: ida_dump_funcs.py mkfuncs: makes them, and export: again. Also the
    functions whose IDA size (end - start) differs from the split's. only: one database."""
    import collections
    missing = collections.defaultdict(set)
    total, sized = 0, []
    for kind in ("nonmatchings", "matchings"):
        for path in glob.glob(os.path.join(build.ROOT, "asm", kind, "**", "*.s"), recursive=True):
            if os.path.basename(path).startswith(("D_", "jtbl_")):
                continue
            if only and _ida_key(path) != only:
                continue
            start, exp = _asm_start(path), _ida_export_for(path)
            if start is None or not exp:
                continue
            total += 1
            if start not in exp["by_start"]:
                missing[_ida_key(path)].add(start)
                continue
            with open(path, encoding="utf-8") as f:
                m = re.search(r"^nonmatching\s+(\w+),\s*(0x[0-9A-Fa-f]+)", f.read(), re.M)
            fn = exp["by_start"][start]
            if m and fn["end"] - fn["start"] != int(m.group(2), 16):
                sized.append((_ida_key(path), m.group(1), start, int(m.group(2), 16), fn["end"] - fn["start"]))
    print(f"ida-starts: {len(sized)} functions whose IDA size differs from the split's (DB name start "
          f"split/IDA): " + " ".join(f"{k} {n}@{s:08X} {a:#x}/{b:#x}" for k, n, s, a, b in sized[:12]))
    n = sum(len(v) for v in missing.values())
    print(f"ida-starts: {n} of {total} split functions have no IDA function at their start, in "
          f"{len(missing)} database(s): " + ", ".join(f"{k} {len(v)}" for k, v in sorted(missing.items()))
          + (": " + " ".join(f"{k}@{s:08X}" for k, v in sorted(missing.items()) for s in sorted(v)) if n <= 12 else ""))
    if not write or not missing:
        return 0
    import report_progress
    jobs = []
    for key, starts in sorted(missing.items()):
        idb = _IDA_CACHE[key]["idb"]
        idb = os.path.join(os.path.dirname(IDA_EXPORT), idb if idb == "main.bin.i64" else os.path.join("files", idb))
        jobs.append((key, idb, ["mkfuncs:" + ",".join(f"{s:08X}" for s in sorted(starts)),
                                "export:" + os.path.join(IDA_EXPORT, f"{key}.json")]))
    report_progress.decode_gaps(jobs)
    return 0


def twins_files(show_all=False):
    """Each C file still holding assembly against the C file most like it: per function, IDA's
    mnemonics (operands and constants aside) the same as a matched function's there, or near
    (difflib ratio 0.9 or more). Candidates are the three files with the most "same" functions
    of 5+ instructions. A whole-file run names the twin in its brief."""
    import collections
    import difflib
    pulled = set()
    for path in glob.glob(os.path.join(build.ROOT, "src", "**", "*.c"), recursive=True):
        with open(path, encoding="utf-8") as f:
            pulled.update(os.path.splitext(os.path.basename(p))[0]
                          for p in re.findall(r'GLOBAL_ASM\("([^"]+)"\)', f.read()))
    asm_by, done_by, index, seen = (collections.defaultdict(list), collections.defaultdict(list),
                                    collections.defaultdict(collections.Counter), set())
    no_ida, no_ida_where, no_ida_eg = 0, collections.Counter(), []
    for kind in ("nonmatchings", "matchings"):
        for path in glob.glob(os.path.join(build.ROOT, "asm", kind, "**", "*.s"), recursive=True):
            name = os.path.splitext(os.path.basename(path))[0]
            if name.startswith(("D_", "jtbl_")) or name in seen:
                continue
            seen.add(name)
            fn = _ida_function(path)
            if fn:
                mn = ("ida",) + tuple(i[2] for i in fn["ins"])
            else:  # no IDA function starts here: splat's .s, compared only with its own kind
                no_ida += 1
                if len(no_ida_eg) < 6 and "/tlb/" in path.replace(os.sep, "/"):
                    no_ida_eg.append(f"{name}@{_asm_start(path) or 0:08X}")
                no_ida_where["tlb" if "/tlb/" in path.replace(os.sep, "/") else
                             os.path.relpath(os.path.dirname(path), os.path.join(build.ROOT, "asm", kind)).split(os.sep)[0]] += 1
                mn = ("splat",) + tuple(s.split()[0] for s in _loose_shape(path) if s.split())
            folder = os.path.relpath(os.path.dirname(path), os.path.join(build.ROOT, "asm", kind)).replace(os.sep, "/")
            if name in pulled:
                asm_by[folder].append((name, mn))
            else:
                done_by[folder].append(mn)
                if len(mn) >= 6:
                    index[mn][folder] += 1
    rows, alone = [], []
    for folder, funcs in asm_by.items():
        votes = collections.Counter()
        for _, mn in funcs:
            if len(mn) >= 6:
                for other in index.get(mn, ()):
                    if other != folder:
                        votes[other] += 1
        best = None
        for twin, _ in votes.most_common(3):
            done = done_by[twin]
            exact = set(done)
            same = near = other_ins = 0
            for _, mn in funcs:
                if mn in exact:
                    same += 1
                    continue
                for d in done:
                    if d[0] != mn[0]:
                        continue  # IDA's mnemonics against splat's: not comparable
                    sm = difflib.SequenceMatcher(None, mn, d, autojunk=False)
                    if sm.real_quick_ratio() >= 0.9 and sm.quick_ratio() >= 0.9 and sm.ratio() >= 0.9:
                        near += 1
                        break
                else:
                    other_ins += len(mn) - 1  # no twin: its instructions are what matching it costs
            if best is None or (same + near, same) > (best[1] + best[2], best[1]):
                best = (twin, same, near, other_ins)
        if best:
            rows.append((folder, len(funcs), *best))
        else:
            alone.append((folder, len(funcs)))
    rows.sort(key=lambda r: (r[5], -r[1]))
    print(f"C files still holding assembly: {len(asm_by)} ({sum(len(f) for f in asm_by.values())} functions); "
          f"with a twin file: {len(rows)}; none: {len(alone)}. Functions with no IDA function at "
          f"their .s start (read from splat's .s): {no_ida} "
          f"({', '.join(f'{k} {v}' for k, v in no_ida_where.most_common(12))}; e.g. {' '.join(no_ida_eg)})")
    for folder, n, twin, same, near, other_ins in rows:
        print(f"  {folder:<22} {n:>3} asm  twin {twin:<22} {same:>3} same {near:>3} near "
              f"{n - same - near:>3} other ({other_ins} ins)")
    if show_all:
        for folder, n in sorted(alone, key=lambda r: r[1]):
            print(f"  {folder:<22} {n:>3} asm  no twin")
    return 0


DATA_ITEM = re.compile(r"^\s*/\*\s*[0-9A-F]+\s+([0-9A-F]{8})(?:\s+([0-9A-F]+))?\s*\*/\s*\.(\w+)\s+(.*?)\s*$")
DATA_BYTES = re.compile(r"^\s*/\*\s*([0-9A-F]+)\s*\*/\s*$")


def _data_items(path):
    """[(label, [(addr, kind, text, bytes)])] of a data .s, in order. Unlabelled items join the
    label before them."""
    import struct
    labels, cur = [], None
    with open(path, encoding="utf-8") as f:
        lines = f.read().splitlines()
    for i, line in enumerate(lines):
        m = re.match(r"^dlabel\s+(\w+)", line)
        if m:
            cur = (m.group(1), [])
            labels.append(cur)
            continue
        m = DATA_ITEM.match(line)
        if not (m and cur):
            continue
        addr, hexw, kind, text = int(m.group(1), 16), m.group(2), m.group(3), m.group(4)
        if kind == "ascii":
            # `.ascii "B"` then `.asciz "\x8C"` on the next line, the bytes in a comment after
            j = i + 1
            while j < len(lines) and re.match(r"^\s*\.(?:ascii|asciz)\s", lines[j]):
                j += 1
            nb = DATA_BYTES.match(lines[j]) if j < len(lines) else None
            kind, raw = "asciz", bytes.fromhex(nb.group(1)) if nb else b""
        elif kind == "asciz":
            nb = DATA_BYTES.match(lines[i + 1]) if i + 1 < len(lines) else None
            raw = bytes.fromhex(nb.group(1)) if nb else b""
        elif kind == "word":
            raw = bytes.fromhex(hexw) if hexw else b"\0\0\0\0"
        elif kind == "float":
            raw = bytes.fromhex(hexw) if hexw else struct.pack(">f", float(text))
        elif kind == "double":
            raw = bytes.fromhex(hexw) if hexw else struct.pack(">d", float(text))
        elif kind in ("half", "short"):
            raw = bytes.fromhex(hexw)[-2:] if hexw else int(text, 0).to_bytes(2, "big", signed=int(text, 0) < 0)
        elif kind == "byte":
            raw = bytes.fromhex(hexw)[-1:] if hexw else int(text, 0).to_bytes(1, "big", signed=int(text, 0) < 0)
        else:
            continue
        cur[1].append((addr, kind, text, raw))
    # in a label that also holds words, a "string" that is whole aligned words is words (splat's
    # guess: the stage files' 0x428C0000 as "B\x8C", a zero word as "")
    for n, (name, items) in enumerate(labels):
        if any(k != "asciz" for _, k, _, _ in items):
            fixed = []
            for a, k, t, r in items:
                if k == "asciz" and a % 4 == 0 and r and len(r) % 4 == 0:
                    fixed += [(a + o, "word", f"0x{struct.unpack('>I', r[o:o + 4])[0]:08X}", r[o:o + 4])
                              for o in range(0, len(r), 4)]
                else:
                    fixed.append((a, k, t, r))
            labels[n] = (name, fixed)
    return labels


def _f32_text(raw):
    """A float's shortest decimal that is the same f32 again (0.1f, not 0.10000000149011612f)."""
    import struct
    x = struct.unpack(">f", raw)[0]
    for p in range(1, 10):
        s = f"{x:.{p}g}"
        if struct.unpack(">f", struct.pack(">f", float(s)))[0] == x:
            break
    if "e" in s and x == int(x) and abs(x) < 1e7:
        s = str(int(x))  # -330.0f, not -3.3e+02f
    if "e" not in s and "." not in s and "inf" not in s and "nan" not in s:
        s += ".0"
    return s + "f"


def _is_animframes(items, symbol_words):
    """Words that are (address, duration << 16) pairs, the list ended by 0, 0: AnimFrame[]."""
    import struct
    if len(items) < 4 or len(items) % 2 or any(k != "word" for _, k, _, _ in items):
        return False
    words = [struct.unpack(">I", r)[0] for *_, r in items]
    for i in range(0, len(words), 2):
        a, d = words[i], words[i + 1]
        if d & 0xFFFF:
            return False
        if not (items[i][0] in symbol_words or a == 0 or (a >> 24) in (1, 2, 3, 4, 5, 6, 7)):
            return False
    return words[-2:] == [0, 0]


def data_c(c_file, apply=False):
    """data-c: the C file's asm/data .s as C definitions in address order, compiled with IDO and
    byte-compared with the .s. apply: also written into the C file (each extern replaced by its
    declarator with an initializer). IDO_NOTES 312."""
    import struct
    rel = os.path.splitext(os.path.relpath(c_file, os.path.join(build.ROOT, "src")))[0].replace(os.sep, "/")
    path = os.path.join(build.ROOT, "asm", "data", rel + ".data.s")
    if not os.path.exists(path):
        print(f"data-c: no {os.path.relpath(path, build.ROOT)}")
        return 1
    with open(c_file, encoding="utf-8") as f:
        csrc = f.read()
    labels = _data_items(path)
    # IDO starts every variable on a 4-byte boundary: labels that are not whole words, back to
    # back, are elements of one array in the game's source (tlb_0034's eight s16 at 0x08007550)
    sizes = {"u8": 1, "s8": 1, "u16": 2, "s16": 2, "u32": 4, "s32": 4, "f32": 4}

    def wanted(name):
        """The byte size the C file's extern declares for name ("extern u16 x[4];"), or 0."""
        d = re.search(r"extern\s+(\w+)\s+" + re.escape(name) + r"\s*\[\s*(\d+)\s*\]\s*;", csrc)
        if not d:
            # "extern f32 x[][4][4];": at least one whole outer element (tlb_0103's seat table,
            # cut after 3 floats where the code addresses its angle column)
            u = re.search(r"extern\s+(\w+)\s+" + re.escape(name) + r"\s*\[\s*\]\s*((?:\[\s*\d+\s*\]\s*)+);", csrc)
            if not u:
                return 0
            n = 1
            for x in re.findall(r"\d+", u.group(2)):
                n *= int(x)
            return sizes.get(u.group(1), 0) * n
        return sizes.get(d.group(1), 0) * int(d.group(2))

    asm_dir = os.path.join(build.ROOT, "asm", "nonmatchings", rel)
    parked_s = ""
    if os.path.isdir(asm_dir):
        for fn in os.listdir(asm_dir):
            if re.search(r'GLOBAL_ASM\("[^"]*/' + re.escape(fn) + r'"\)', csrc):
                with open(os.path.join(asm_dir, fn), encoding="utf-8") as f:
                    parked_s += f.read()

    def as_words(items):
        """Byte or short items regrouped as words (all 4-aligned, no symbols among them), or None."""
        raw = b"".join(r for *_, r in items)
        if not items or items[0][0] % 4 or len(raw) % 4:
            return None
        if all(k == "word" for _, k, _, _ in items):
            return items
        if any(k not in ("byte", "short", "half") for _, k, _, _ in items):
            return None
        a0 = items[0][0]
        return [(a0 + o, "word", f"0x{struct.unpack('>I', raw[o:o + 4])[0]:08X}", raw[o:o + 4])
                for o in range(0, len(raw), 4)]

    merged, aliases = [], []
    for name, items in labels:
        prev = merged[-1] if merged else None
        have = sum(len(r) for *_, r in prev[1]) if prev else 0
        sd = re.search(r"extern\s+(\w+)\s+" + re.escape(prev[0]) + r"\s*\[\s*\]\s*;", csrc) if prev else None
        if sd and _plain_struct(csrc, sd.group(1)) and re.fullmatch(r"D_[0-9A-F]+(_\w+)?", name) \
                and not re.search(r"\b" + re.escape(name) + r"\b", csrc) and items \
                and all(k in ("byte", "short", "half", "word", "float") for _, k, _, _ in items):
            # the struct array runs on into a label the C does not name (splat cut it at a field
            # read); one a parked function's .s names keeps its address as an alias. The struct
            # filler reads the raw bytes, so a 1-byte label (MapArea's s8 stage) joins as it is
            if re.search(r"\b" + re.escape(name) + r"\b", parked_s):
                aliases.append((name, items[0][0]))
            prev[1][:] = (as_words(prev[1]) or prev[1]) + (as_words(items) or items)
            prev[2].append(name)
            continue
        pk = {k for _, k, _, _ in prev[1]} if prev else set()
        if (prev and items and prev[1] and have < wanted(prev[0]) and pk <= {"short", "half"}
                and {k for _, k, _, _ in items} == {"word"} and re.fullmatch(r"D_[0-9A-F]+(_\w+)?", name)
                and not re.search(r"\b" + re.escape(name) + r"\b", csrc)):
            # the C's array runs on into a label splat cut as words, which nothing names
            # (tlb_0035's gGoverButton[4]: two .short, then D_08003A34 as one .word)
            items = [(a + o, "short", f"0x{struct.unpack('>H', r[o:o + 2])[0]:04X}", r[o:o + 2])
                     for a, k, t, r in items for o in range(0, len(r), 2)]
        if (prev and items and prev[1] and (have % 4 or have < wanted(prev[0]))
                and {k for _, k, _, _ in prev[1]} == {k for _, k, _, _ in items}):
            prev[1].extend(items)
            prev[2].append(name)
        else:
            merged.append((name, list(items), []))
    labels = [(n, it) for n, it, _ in merged]
    joined = {n: j for n, _, j in merged if j}
    out, syms, checks, symbol_words, want, defs = [], set(), 0, set(), bytearray(), []
    start = labels[0][1][0][0] if labels and labels[0][1] else 0
    own = {n for n, it in labels if it}  # labels this file's data defines

    def ref(t):
        """A symbol word: this file's own label by its address (it is defined here, maybe later,
        so it is declared ahead), anything else as splat names it."""
        m = re.fullmatch(r"(\w+)(\s*\+\s*0x[0-9A-Fa-f]+)?", t.strip())
        if m and m.group(1) in own:
            return f"(u32) &{m.group(1)}{m.group(2) or ''}"
        return f"(u32) {t}"
    for name, items in labels:
        if not items:
            continue
        decl = re.search(r"extern\s+([\w ]+?\s*\**)\s*\b" + re.escape(name) + r"\b\s*(\[[^\]]*\])?\s*;", csrc)
        ctype = decl.group(1).strip() if decl else None
        kinds = {k for _, k, _, _ in items}
        for a, k, t, raw in items:
            if k == "word" and not re.fullmatch(r"-?0x[0-9A-Fa-f]+|-?\d+", t):
                symbol_words.add(a)
            want += raw
        size = sum(len(r) for *_, r in items)
        note = f" /* one array: {', '.join(joined[name])} are its elements */" if name in joined else ""
        if kinds == {"asciz"} and len(items) == 1:
            out.append(f"char {name}[] = {items[0][2]};")
            defs.append((name, items, "char", [items[0][2]], ""))
            continue
        if kinds <= {"float"} or (ctype == "f32" and kinds <= {"word", "float"}):
            vals = [_f32_text(r) for *_, r in items]
            typ = "f32"
        elif kinds == {"double"}:
            vals = [repr(struct.unpack(">d", r)[0]) for *_, r in items]
            typ = "f64"
        elif kinds <= {"short", "half"}:
            typ = "u16" if ctype == "u16" else "s16"
            vals = [str(struct.unpack(">H" if typ == "u16" else ">h", r)[0]) for *_, r in items]
        elif _is_animframes(items, symbol_words) and (ctype in (None, "AnimFrame")):
            typ = "AnimFrame"
            vals = []
            for i in range(0, len(items), 2):
                a, k, t, r = items[i]
                if a in symbol_words:
                    syms.add(t)
                    addr = ref(t)
                else:
                    addr = f"0x{struct.unpack('>I', r)[0]:08X}" if struct.unpack(">I", r)[0] else "0"
                vals.append(f"{{ {addr}, {struct.unpack('>I', items[i + 1][3])[0] >> 16} }}")
        elif kinds <= {"word"} and ctype and ctype.endswith("*") and len(items) == 1 and not symbol_words & {items[0][0]}:
            typ = "void *"  # the C's own pointer type in the merge
            vals = [f"(void *) 0x{struct.unpack('>I', items[0][3])[0]:08X}"]
            note = f" /* the C declares it {ctype} */"
        elif kinds <= {"word", "float"}:
            typ = "s32" if ctype == "s32" else "u32"
            vals = []
            for a, k, t, r in items:
                if a in symbol_words:
                    syms.add(t)
                    vals.append(ref(t))
                elif typ == "s32":
                    vals.append(str(struct.unpack(">i", r)[0]))
                else:
                    vals.append(f"0x{struct.unpack('>I', r)[0]:08X}")
            if ctype and ctype not in ("u32", "s32", "f32"):
                note = f" /* CHECK: the C declares it {ctype} */"
                checks += 1
        else:
            vals = [f"0x{b:02X}" for *_, r in items for b in r]
            typ = "u8"
            note = f" /* bytes ({', '.join(sorted(kinds))}) */"
            checks += 1
        defs.append((name, items, typ, vals, note))
        if len(vals) == 1 and not (decl and decl.group(2)):
            out.append(f"{typ} {name} = {vals[0]};{note}")
        else:
            body = ",\n    ".join(vals) if typ == "AnimFrame" else ", ".join(vals)
            out.append(f"{typ} {name}[] = {{ {body} }};{note}" if typ != "AnimFrame"
                       else f"{typ} {name}[] = {{\n    {body},\n}};{note}")
    os.makedirs(os.path.join(build.ROOT, "permuter", "datac"), exist_ok=True)
    base = os.path.join(build.ROOT, "permuter", "datac", rel.replace("/", "_"))
    # standalone for the byte proof: no headers (their prototypes would clash with the symbols'
    # u8[] declarations); the merge uses the C file's own declarations
    header = ("typedef unsigned char u8;\ntypedef unsigned int u32;\ntypedef int s32;\n"
              "typedef short s16;\ntypedef unsigned short u16;\n"
              "typedef float f32;\ntypedef double f64;\n"
              "typedef struct AnimFrame { u32 addr; s16 duration; } AnimFrame;\n")
    def base_name(s):
        return re.split(r"\s*\+", s.strip())[0]

    arrays = {d[0]: not (len(d[3]) == 1 and d[2] != "char" and not re.search(r"\b" + re.escape(d[0]) + r"\[\]", o))
              for d, o in zip(defs, out)}
    typ_of = {d[0]: d[2] for d in defs}
    externs = "".join(f"extern u8 {base_name(s)}[];\n" for s in sorted({base_name(s) for s in syms})
                      if base_name(s) not in own)
    # this file's own labels that a word points at: declared ahead with their own types
    externs += "".join(f"extern {typ_of[n]} {n}{'[]' if arrays[n] else ''};\n"
                       for n in sorted({base_name(s) for s in syms} & own) if n in typ_of)
    text = header + externs + "\n" + "\n".join(out) + "\n"
    with open(base + ".c", "w", encoding="utf-8") as f:
        f.write(text)
    # the proof: compile, take .data, compare with the .s bytes (symbol words skipped)
    obj = base + ".o"
    if compile_one(base + ".c", obj) != 0:
        print(f"data-c: {os.path.relpath(base + '.c', build.ROOT)} does not compile (CHECK the types)")
        return 1
    objcopy = build.tool_path(os.path.join(build.BINUTILS, "mips64-elf-objcopy" + build.EXE))
    binf = base + ".data.bin"
    subprocess.run([objcopy, "-O", "binary", "-j", ".data", obj, binf], check=True)
    with open(binf, "rb") as f:
        got = f.read()
    bad = [start + i for i in range(0, min(len(got), len(want)), 4)
           if got[i:i + 4] != want[i:i + 4] and (start + i) not in symbol_words]
    extra = len(want) - len(got)
    print(f"data-c: {rel}: {len(labels)} labels, {len(want):#x} bytes from {start:08X}, "
          f"{len(symbol_words)} symbol words, {checks} CHECK; compiled .data {len(got):#x} bytes; "
          f"{'IDENTICAL' if not bad and extra in (0,) else f'{len(bad)} words differ (first {bad[0]:08X})' if bad else f'.s longer by {extra:#x} (trailing alignment?)'}"
          f" -> {os.path.relpath(base + '.c', build.ROOT)}")
    if bad:
        return 1
    # element labels merged into an array vanish from splat's data; parked asm that still names
    # one needs it in undefined_syms.txt (tlb_0034's controller arrays)
    addr_of = {lab: it[0][0] for lab, it in _data_items(path) if it}
    folder = os.path.join(build.ROOT, "asm", "nonmatchings", *rel.split("/"))
    asm_text = ""
    for p in glob.glob(os.path.join(folder, "*.s")):
        with open(p, encoding="utf-8") as f:
            asm_text += f.read()
    for name in [n for js in joined.values() for n in js]:
        if re.search(r"\b" + re.escape(name) + r"\b", asm_text):
            print(f"data-c: undefined_syms.txt needs {name} = 0x{addr_of[name]:08X}; (an element its parked asm names)")
    if apply:
        rc = _data_apply(c_file, csrc, defs, symbol_words, out, base + ".data.bin")
        if rc == 0 and aliases:
            # labels joined into an array that a parked function's .s still names: an absolute
            # alias, as undefined_syms.txt has for others (D_08008910_tlb_0031_clr)
            us = os.path.join(build.ROOT, "undefined_syms.txt")
            with open(us, encoding="utf-8") as f:
                have_us = f.read()
            add = [f"{n} = 0x{a:08X};" for n, a in aliases if not re.search(r"^" + re.escape(n) + r"\b", have_us, re.M)]
            if add:
                with open(us, "a", encoding="utf-8", newline="\n") as f:
                    f.write(("" if have_us.endswith("\n") else "\n") +
                            f"\n/* {rel}: labels joined into its C arrays, as parked functions' asm still names "
                            f"them (data-c --apply). */\n" + "\n".join(add) + "\n")
                print(f"data-c --apply: undefined_syms.txt: {', '.join(add)} (named by parked assembly)")
        return rc
    return 0


def _data_flip(c_file):
    """The C file's yaml data subsegment to .data (its data comes from the C now)."""
    rel = os.path.splitext(os.path.relpath(c_file, os.path.join(build.ROOT, "src")))[0].replace(os.sep, "/")
    yaml_path = os.path.join(build.ROOT, "goemonsgreatadv.yaml")
    with open(yaml_path, encoding="utf-8", newline="") as f:
        ytext = f.read()
    ytext2 = re.sub(r"(\[0x[0-9A-Fa-f]+,\s*)data(,\s*" + re.escape(rel) + r"\])", r"\1.data\2", ytext, count=1)
    if ytext2 != ytext:
        with open(yaml_path, "w", encoding="utf-8", newline="") as f:
            f.write(ytext2)
    return ytext2 != ytext


def data_verify(c_file):
    """data-c FILE --verify: the C file as the build compiles it, its .data against the data .s
    (symbol words skipped): the per-file proof of an --apply, seconds instead of a split and a
    full build. Prints IDENTICAL or the first differing address."""
    rel = os.path.splitext(os.path.relpath(c_file, os.path.join(build.ROOT, "src")))[0].replace(os.sep, "/")
    path = os.path.join(build.ROOT, "asm", "data", rel + ".data.s")
    if not os.path.exists(path):
        print(f"data-c --verify: no {os.path.relpath(path, build.ROOT)} to compare with")
        return 1
    want, skip, start = bytearray(), set(), None
    for name, items in _data_items(path):
        for a, k, t, raw in items:
            if start is None:
                start = a
            if k == "word" and not re.fullmatch(r"-?0x[0-9A-Fa-f]+|-?\d+", t):
                skip.add(a)
            want += raw
    base = os.path.join(build.ROOT, "permuter", "datac", rel.replace("/", "_") + "_verify")
    os.makedirs(os.path.dirname(base), exist_ok=True)
    if compile_one(c_file, base + ".o") != 0:
        print(f"data-c --verify: {rel} does not compile")
        return 1
    objcopy = build.tool_path(os.path.join(build.BINUTILS, "mips64-elf-objcopy" + build.EXE))
    subprocess.run([objcopy, "-O", "binary", "-j", ".data", base + ".o", base + ".bin"], check=True)
    with open(base + ".bin", "rb") as f:
        got = f.read()
    # the .s runs to the next section's 16-byte boundary; the object's .data may stop short
    trail = want[len(got):]
    bad = [start + i for i in range(0, min(len(got), len(want)), 4)
           if got[i:i + 4] != want[i:i + 4] and (start + i) not in skip]
    ok = not bad and len(got) <= len(want) and not any(trail)
    print(f"data-c --verify: {rel}: {'IDENTICAL' if ok else 'DIFFERS'} (.data {len(got):#x}, .s {len(want):#x}"
          f"{f', first at {bad[0]:08X}' if bad else ''})")
    return 0 if ok else 1


RETYPE_NOTES = re.compile(r"\s*/\* (bytes \(|one array: |CHECK: |declared \[)[^*]*\*/\s*$")


def _retype_one(csrc, name, newtype, c_file):
    """One --retype: (new C text, None) or (None, why it was refused)."""
    import struct
    scal = {"u8": (1, ">B"), "s8": (1, ">b"), "u16": (2, ">H"), "s16": (2, ">h"), "u32": (4, ">I"),
            "s32": (4, ">i"), "f32": (4, ">f")}
    d = re.search(r"^(?:static\s+)?(\w+)\s+" + re.escape(name) + r"\s*\[\s*(\d*)\s*\]\s*=\s*\{(.*?)\};([^\n]*)$",
                  csrc, re.M | re.S)
    if not d:
        return None, "no array definition"
    old, count, body, trail = d.group(1), d.group(2), d.group(3), d.group(4)
    if old not in scal:
        return None, f"defined {old}, not a plain array"
    if old == newtype:
        return None, f"already {newtype}"
    esz, fmt = scal[old]
    raw, sym_at = bytearray(), {}
    for tok in [t.strip() for t in re.sub(r"/\*.*?\*/", "", body, flags=re.S).split(",")]:
        if not tok:
            continue
        c = re.fullmatch(r"\((?:u32|s32)\)\s*(.+)", tok)
        if c and esz == 4 and not re.fullmatch(r"-?(0x[0-9A-Fa-f]+|\d+)", c.group(1).strip()):
            sym_at[len(raw)] = c.group(1).strip()
            raw += b"\0\0\0\0"
        elif re.fullmatch(r"-?(0x[0-9A-Fa-f]+|\d+)", tok):
            raw += struct.pack({1: ">B", 2: ">H", 4: ">I"}[esz], int(tok, 0) & ((1 << (esz * 8)) - 1))
        elif old == "f32" and re.fullmatch(r"-?[\d.]+(e[-+]?\d+)?f?", tok):
            raw += struct.pack(">f", float(tok.rstrip("f")))
        else:
            return None, f"initializer element {tok!r} is not a number or a (u32) symbol"
    if count and len(raw) < int(count) * esz:
        raw += b"\0" * (int(count) * esz - len(raw))
    if newtype in scal:
        fields, ssize = [(newtype, 0, scal[newtype][0], ())], scal[newtype][0]
    else:
        ps = _plain_struct(csrc, newtype)
        if not ps:
            return None, f"{newtype} is not a plain struct this tool can fill"
        fields, ssize = ps
    whole = len(raw) // ssize * ssize
    if whole == 0:
        return None, f"{len(raw)} bytes, less than one {newtype}"
    starts = {o for _, o, s, _ in fields if s == 4}
    for a in sym_at:
        if a >= whole or (a % ssize) not in starts:
            return None, f"symbol word {sym_at[a]} at +{a:#x} is not a whole 4-byte field of {newtype}"
    if whole < len(raw):
        later = re.search(r"^(?:static\s+)?[A-Za-z_]\w*(?:\s*\*+\s*|\s+)\w+\s*(\[[^\]]*\])*\s*=", csrc[d.end():], re.M)
        if any(raw[whole:]) or later:
            return None, (f"{len(raw) - whole} bytes past the last whole {newtype}"
                          + (" are not zero" if any(raw[whole:]) else ", and more data follows in the file"))
    if newtype in scal:
        vals = [s for s in _struct_elems(fields, ssize, bytes(raw), whole, 0, sym_at, by_value=False)]
        init = "{ " + ", ".join(v[2:-2] for v in vals) + " }"
    else:
        init = "{\n    " + ",\n    ".join(_struct_elems(fields, ssize, bytes(raw), whole, 0, sym_at,
                                                            by_value=False)) + ",\n}"
    trail = "" if RETYPE_NOTES.fullmatch(trail) else trail
    new = csrc[:d.start()] + f"{newtype} {name}[] = {init};{trail}" + csrc[d.end():]
    return new, None


def _retype_uses(name, texts):
    """Uses of name that index or offset it as its old elements, outside its definition and its
    extern lines: [(file, line text)]. texts: {path: file text}."""
    bad, word = [], re.compile(r"\b" + re.escape(name) + r"\b")
    for p, text in texts.items():
        if name not in text:
            continue
        for l in text.split("\n"):
            if not word.search(l) or re.match(r"^\s*extern\b", l) or \
                    re.match(r"^(?:static\s+)?\w+\s+" + re.escape(name) + r"\s*\[", l):
                continue
            for m in word.finditer(l):
                after = l[m.end():].lstrip()
                before = l[:m.start()].rstrip()
                if after.startswith("[") or (after[:1] in ("+", "-") and not re.search(r"\((u32|s32)\)\s*&?$", before)):
                    bad.append((p, l.strip()))
    return bad


def data_retype(pairs):
    """data-c --retype: pairs [(c_file, name, type)]. Each file's arrays are retyped together,
    every extern of them in src/ and include/ takes the type, and the file is proven by
    data_verify; a file that is not IDENTICAL is put back (and the externs with it)."""
    texts = {}
    for p in glob.glob(os.path.join(build.ROOT, "src", "**", "*.c"), recursive=True) + \
            glob.glob(os.path.join(build.ROOT, "include", "**", "*.h"), recursive=True):
        with open(p, encoding="utf-8", newline="") as f:
            texts[os.path.abspath(p)] = f.read()
    by_file = {}
    for c_file, name, typ in pairs:
        by_file.setdefault(os.path.abspath(c_file), []).append((name, typ))
    done = refused = failed = 0
    for c_file, items in by_file.items():
        rel = os.path.relpath(c_file, build.ROOT).replace(os.sep, "/")
        text, took = texts[c_file], []
        for name, typ in items:
            bad = _retype_uses(name, texts)
            if bad:
                print(f"retype: REFUSED {rel} {name}: used as its old elements ({bad[0][1][:100]})")
                refused += 1
                continue
            new, why = _retype_one(text, name, typ, c_file)
            if why:
                print(f"retype: REFUSED {rel} {name}: {why}")
                refused += 1
                continue
            text = new
            took.append((name, typ))
        if not took:
            continue
        changed = {c_file: text}
        for name, typ in took:
            pat = re.compile(r"^extern\s+[\w ]+?\**\s*\b" + re.escape(name) + r"\b\s*(\[[^\]]*\])*\s*;(.*)$", re.M)
            for p, t in texts.items():
                t = changed.get(p, t)
                if name in t:
                    t2 = pat.sub(lambda m: f"extern {typ} {name}[];{m.group(2)}", t)
                    if t2 != t:
                        changed[p] = t2
        for p, t in changed.items():
            with open(p, "w", encoding="utf-8", newline="") as f:
                f.write(t)
        ref = os.path.join(build.ROOT, "build", os.path.splitext(rel)[0] + ".o")
        out = os.path.join(build.ROOT, "permuter", "datac", "retype", rel.replace("/", "_") + ".o")
        os.makedirs(os.path.dirname(out), exist_ok=True)
        why = "no build object to compare with (run build_rom.py first)" if not os.path.exists(ref) else \
            "does not compile" if compile_one(c_file, out) != 0 else None
        if not why:
            k0, k1 = _hdr_objkey(ref), _hdr_objkey(out)
            d0, d1 = k0.get(".data", b""), k1.get(".data", b"")
            if d0 != d1 and isinstance(d0, bytes) and d0.startswith(d1) and not any(d0[len(d1):]) \
                    and len(d0) - len(d1) < 16:
                k1[".data"] = d0  # zero padding dropped from the end: the next section's alignment
            why = None if k0 == k1 else "the object differs from build's in " + \
                ", ".join(sorted(k for k in set(k0) | set(k1) if k0.get(k) != k1.get(k)))
        if not why:
            texts.update(changed)
            done += len(took)
        else:
            for p in changed:
                with open(p, "w", encoding="utf-8", newline="") as f:
                    f.write(texts[p])
            print(f"retype: PUT BACK {rel} ({', '.join(n for n, _ in took)}): {why}")
            failed += len(took)
    print(f"retype: {done} arrays retyped and proven, {refused} refused, {failed} put back")
    return 0 if not failed else 1


def _comment_above(text, at):
    """Where the /* ... */ block right above position at (a line start) begins, or at."""
    lines = text[:at].split("\n")[:-1]
    if not lines or not lines[-1].rstrip().endswith("*/"):
        return at
    k = len(lines) - 1
    while k > 0 and not lines[k].lstrip().startswith("/*"):
        k -= 1
    return sum(len(l) + 1 for l in lines[:k])


def data_move(src, dst, names):
    """data-c SRC --move DST NAME...: (914) each named top-level data definition taken out of SRC,
    with the comment block right above it, and written into DST in the order given, before DST's
    first function (and its comment), or at the end. Writes nothing if one is missing in SRC or
    already defined in DST."""
    with open(src, encoding="utf-8", newline="") as f:
        s = f.read()
    with open(dst, encoding="utf-8", newline="") as f:
        d = f.read()
    blocks = []
    for name in names:
        pat = re.compile(r"^(?:static\s+)?[A-Za-z_][\w ]*?\**\s*\b" + re.escape(name) + r"\b\s*(?:\[[^\]]*\]\s*)*=", re.M)
        m = pat.search(s)
        if not m:
            print(f"data-c --move: REFUSED, no definition of {name} in {src}")
            return 1
        if pat.search(d):
            print(f"data-c --move: REFUSED, {name} is already defined in {dst}")
            return 1
        i, depth = m.end(), 0
        while i < len(s) and not (s[i] == ";" and depth == 0):
            depth += {"{": 1, "}": -1}.get(s[i], 0)
            i += 1
        nl = s.find("\n", i)
        end = len(s) if nl < 0 else nl + 1
        start = _comment_above(s, m.start())
        blocks.append(s[start:end])
        s = s[:start] + s[end:]
    f = re.search(r"^[A-Za-z_][\w \t\*]*?\b\w+\s*\([^;{]*\)\s*\{", d, re.M)
    at = _comment_above(d, f.start()) if f else len(d)
    block = "".join(b if b.endswith("\n") else b + "\n" for b in blocks)
    d = d[:at] + block + ("\n" if f else "") + d[at:] if f else d.rstrip("\n") + "\n" + block
    with open(src, "w", encoding="utf-8", newline="") as fh:
        fh.write(s)
    with open(dst, "w", encoding="utf-8", newline="") as fh:
        fh.write(d)
    print(f"data-c --move: {len(names)} definitions from {src} to {dst}")
    return 0


def data_unapply(c_file):
    """data-c FILE --unapply (915): the file's shape before --apply, its round trip's start."""
    rel = os.path.splitext(os.path.relpath(c_file, os.path.join(build.ROOT, "src")))[0].replace(os.sep, "/")
    path = os.path.join(build.ROOT, "asm", "data", rel + ".data.s")
    if not os.path.exists(path):
        print(f"data-c --unapply: no {os.path.relpath(path, build.ROOT)}")
        return 1
    labels = [n for n, items in _data_items(path) if items]
    with open(c_file, encoding="utf-8", newline="") as f:
        s = f.read()
    for name in labels:
        # the label's extern lines (forward declarations) first: --apply declares what it needs
        s = re.sub(r"^extern\s+[^;(\n]*?\b%s\b\s*(?:\[[^\]\n]*\]\s*)*;[^\n]*\n" % re.escape(name), "", s,
                   flags=re.M)
        pat = re.compile(r"^(?:static\s+)?([A-Za-z_][\w ]*?\**)\s*\b" + re.escape(name) + r"\b\s*((?:\[[^\]]*\]\s*)*)=", re.M)
        m = pat.search(s)
        if not m:
            print(f"data-c --unapply: REFUSED, no definition of {name} (a label of {rel}'s .data) in the C")
            return 1
        i, depth = m.end(), 0
        while i < len(s) and not (s[i] == ";" and depth == 0):
            depth += {"{": 1, "}": -1}.get(s[i], 0)
            i += 1
        nl = s.find("\n", i)
        end = len(s) if nl < 0 else nl
        trail = s[i + 1:end]  # a comment after the definition's ";"
        s = s[:m.start()] + f"extern {m.group(1).strip()} {name}{m.group(2).strip()};{trail}" + s[end:]
    with open(c_file, "w", encoding="utf-8", newline="") as fh:
        fh.write(s)
    yaml_path = os.path.join(build.ROOT, "goemonsgreatadv.yaml")
    with open(yaml_path, encoding="utf-8", newline="") as f:
        y = f.read()
    y2 = re.sub(r"(\[0x[0-9A-Fa-f]+,\s*)\.data(,\s*" + re.escape(rel) + r"\])", r"\1data\2", y, count=1)
    with open(yaml_path, "w", encoding="utf-8", newline="") as f:
        f.write(y2)
    print(f"data-c --unapply: {rel}: {len(labels)} definitions back to externs; yaml row "
          f"{'data' if y2 != y else 'unchanged'}; now build_rom.py --split, then data-c --apply")
    return 0


def _shortest_f32(r):
    """The shortest float literal that packs back to the 4 bytes r."""
    return _f32_text(r)


def _plain_struct(csrc, base, _depth=0):
    """A struct typedef'd in csrc or include/*.h whose fields are plain (u8..u32, s8..s32, f32,
    pointers), typedef'd structs of such fields, or arrays of either, of any rank, one per line:
    ([(type, offset, size, path)], size) at natural alignment, else None. path is the field's
    place in the element's braces (() for a top-level scalar; (k, i) for element i of field k)."""
    pat = r"typedef struct \w*\s*\{([^{}]*)\}\s*%s\s*;" % re.escape(base)
    m = re.search(pat, csrc)
    for p in ([] if m else glob.glob(os.path.join(build.ROOT, "include", "*.h"))):
        with open(p, encoding="utf-8") as f:
            m = re.search(pat, f.read())
        if m:
            break
    if not m or _depth > 4:
        return None
    sizes = {"u8": 1, "s8": 1, "char": 1, "u16": 2, "s16": 2, "u32": 4, "s32": 4, "f32": 4}
    fields, off, align = [], 0, 1
    for k, line in enumerate(l.strip() for l in re.sub(r"/\*.*?\*/", "", m.group(1), flags=re.S).split(";")
                             if l.strip()):  # comments may hold ";"
        f = re.match(r"^(\w+(?:\s*\*+\s*|\s+))(\w+)((?:\s*\[\s*\d+\s*\])*)$", line)  # "Gfx *road" too
        if not f:
            return None
        ftyp = f.group(1).replace(" ", "")
        dims = [int(d) for d in re.findall(r"\d+", f.group(3))]
        if ftyp.endswith("*") or ftyp in sizes:
            sub, fsz, falign = [(ftyp, 0, 4 if ftyp.endswith("*") else sizes[ftyp], ())], \
                4 if ftyp.endswith("*") else sizes[ftyp], 4 if ftyp.endswith("*") else sizes[ftyp]
        else:
            inner = _plain_struct(csrc, ftyp, _depth + 1)
            if not inner:
                return None
            sub, fsz = inner
            falign = max([s for _, _, s, _ in sub] or [1])
        off = (off + falign - 1) // falign * falign
        n = 1
        for d in dims:
            n *= d
        for i in range(n):  # an array's elements in order; their brace path from the dims
            idx, rem = [], i
            for d in reversed(dims):
                idx.insert(0, rem % d)
                rem //= d
            if len(sub) > 1:  # a struct (or an array of them): a brace each, its fields inside
                group = (k,) + tuple(idx)
            elif dims:  # a plain array: one brace, a brace to a row when it has rows
                group = (k,) + tuple(idx[:-1])
            else:
                group = ()
            for t, o, s, p in sub:
                fields.append((t, off + o, s, group + p))
            off += fsz
        align = max(align, falign)
    return fields, (off + align - 1) // align * align


def _braced(vals, paths, depth=0):
    """Values with their brace paths (from _plain_struct) as one C initializer list, nested;
    an all-zero brace is { 0 }, and one at the end of its brace is left to C's zero fill."""
    out, i = [], 0
    while i < len(vals):
        if len(paths[i]) <= depth:
            out.append(vals[i])
            i += 1
            continue
        j = i
        while j < len(vals) and len(paths[j]) > depth and paths[j][depth] == paths[i][depth]:
            j += 1
        inner = _braced(vals[i:j], paths[i:j], depth + 1)
        allzero = all(re.fullmatch(r"0|0x0+|0\.0f|NULL", v) for v in vals[i:j])
        out.append("{ 0 }" if allzero else "{ " + inner + " }")
        i = j
    while len(out) > 1 and out[-1] == "{ 0 }":  # an all-zero brace at the end: C's zero fill
        out.pop()
    return ", ".join(out)


def _struct_elems(fields, ssize, raw, whole, addr0, sym_at, own_labels=(), seg_s=None,
                  sym_values=None, filler_refs=None, by_value=True):
    """data-c's struct filler: a struct array's elements as C initializers, one "{ ... }" each,
    from the big-endian bytes raw at address addr0 (whole: the bytes its whole elements take).
    fields from _plain_struct; sym_at: address -> the symbol text a 4-byte word holds. A pointer
    field whose value is one of own_labels (D_<addr>_tlb_NNNN, seg_s the file's tlb number) is
    written as that label; sym_values and filler_refs collect what the byte check must know."""
    import struct
    sym_values = {} if sym_values is None else sym_values
    filler_refs = set() if filler_refs is None else filler_refs
    elems, sym_by_val = [], {}
    for a, t in sym_at.items():
        if by_value and re.fullmatch(r"\w+", t.strip()):
            sym_by_val.setdefault(raw[a - addr0:a - addr0 + 4], t.strip())
    for e in range(0, whole, ssize):
        vs = []
        for ftyp, off, fsz, _ in fields:
            chunk = raw[e + off:e + off + fsz]
            if fsz == 4 and (addr0 + e + off) in sym_at:
                t = sym_at[addr0 + e + off]
                if "*" not in ftyp and re.split(r"\s*\+", t.strip())[0] in own_labels and not t.lstrip().startswith("&"):
                    t = "&" + t.lstrip()  # this file's own label: its address, as data-c's u32 arrays write it (915)
                vs.append(f"({ftyp}) {t}" if "*" in ftyp else f"(u32) {t}")
            elif (ftyp == "u32" or "*" in ftyp) and chunk in sym_by_val:
                vs.append(f"({ftyp}) {sym_by_val[chunk]}")
                sym_values[sym_by_val[chunk]] = chunk
            elif "*" in ftyp and seg_s and chunk[0] == 0x08 and \
                    f"D_{struct.unpack('>I', chunk)[0]:08X}_tlb_{seg_s.group(1)}" in own_labels:
                own = f"D_{struct.unpack('>I', chunk)[0]:08X}_tlb_{seg_s.group(1)}"
                vs.append(f"({ftyp}) {own}")
                sym_values[own] = chunk
                filler_refs.add(own)
            elif ftyp == "u8":
                vs.append(str(chunk[0]))
            elif ftyp == "f32":
                vs.append(_shortest_f32(chunk))
            elif "*" in ftyp:
                v = struct.unpack(">I", chunk)[0]
                vs.append("NULL" if v == 0 else f"({ftyp}) 0x{v:08X}")
            else:
                fmt = {"u8": ">B", "s8": ">b", "char": ">b", "u16": ">H", "s16": ">h", "u32": ">I",
                       "s32": ">i"}[ftyp]
                v = struct.unpack(fmt, chunk)[0]
                vs.append(str(v) if ftyp[0] == "s" else f"0x{v:0{fsz * 2}X}")
        elems.append("{ " + _braced(vs, [p for *_, p in fields]) + " }")
    return elems


def _is_scalar_var(csrc, name):
    """name is declared (in csrc or a header it includes) as a variable that is not an array."""
    if not re.fullmatch(r"\w+", name):
        return False
    texts = [csrc]
    todo, seen = re.findall(r'^#include\s+"([^"]+)"', csrc, re.M), set()
    while todo:
        h = todo.pop()
        p = os.path.join(build.ROOT, "include", h)
        if h in seen or not os.path.exists(p):
            continue
        seen.add(h)
        with open(p, encoding="utf-8") as f:
            texts.append(f.read())
        todo += re.findall(r'^#include\s+"([^"]+)"', texts[-1], re.M)
    for text in texts:
        m = re.search(r"^extern\b[^;(]*\b%s\b\s*(\[)?[^;(]*;" % re.escape(name), text, re.M)
        if m:
            return m.group(1) is None
    return False


def _header_names(csrc):
    """Every name an extern declaration declares in the include/*.h headers csrc includes,
    following the headers' own #includes (data-c's forward declarations leave these out)."""
    names, todo, seen = set(), re.findall(r'^#include\s+"([^"]+)"', csrc, re.M), set()
    while todo:
        h = todo.pop()
        p = os.path.join(build.ROOT, "include", h)
        if h in seen or not os.path.exists(p):
            continue
        seen.add(h)
        with open(p, encoding="utf-8") as f:
            text = f.read()
        names.update(re.findall(r"^extern\b[^;(]*?\b(\w+)\s*(?:\[[^\]]*\])*\s*;", text, re.M))
        todo += re.findall(r'^#include\s+"([^"]+)"', text, re.M)
    return names


def _data_apply(c_file, csrc, defs, symbol_words, standalone, binf=None):
    """data-c --apply: each extern of a data label becomes its own declarator with an
    initializer; labels the C does not declare go in before the next one in address order.
    Refuses (writes nothing) if the definitions would not come in address order."""
    import struct
    lines = csrc.split("\n")
    # lines the matching build does not compile: #ifdef NON_MATCHING up to its #else/#endif. An
    # extern there must not become the definition (tlb_0017_2 lost 0xC0 bytes of .data so)
    parked, depth = set(), 0
    for i, l in enumerate(lines):
        s = l.strip()
        if s.startswith("#ifdef NON_MATCHING"):
            depth = 1
        elif depth and s.startswith(("#else", "#endif")):
            depth = 0
        elif depth:
            parked.add(i)
    placed, inserted, deftext, extern_at = {}, [], {}, {}
    sym_values = {}  # a symbol written for a plain word: the word's value (the byte check's)
    filler_refs = set()  # this file's labels the struct filler named for pointer words
    # this file's own labels (its .data, .rodata and the .s files its pragmas include)
    rel_c = os.path.splitext(os.path.relpath(c_file, os.path.join(build.ROOT, "src")))[0].replace(os.sep, "/")
    own_labels = set()
    for sfx in (".data.s", ".rodata.s"):
        p = os.path.join(build.ROOT, "asm", "data", rel_c + sfx)
        if os.path.exists(p):
            with open(p, encoding="utf-8") as f:
                own_labels.update(re.findall(r"^dlabel\s+(\w+)", f.read(), re.M))
    nm = os.path.join(build.ROOT, "asm", "nonmatchings", rel_c)
    if os.path.isdir(nm):
        own_labels.update(os.path.splitext(n)[0] for n in os.listdir(nm) if n.startswith("D_"))
    ends = {}  # an inserted definition's last line (placed[] is its first)
    for (name, items, typ, vals, note), text in zip(defs, standalone):
        pat = re.compile(r"^extern\s+(.*\b" + re.escape(name) + r"\b[^;]*);(.*)$")
        idx = next((i for i, l in enumerate(lines) if pat.match(l) and i not in parked), None)
        if idx is None:
            continue
        m = pat.match(lines[idx])
        decl, rest = m.group(1), m.group(2)
        base = decl.split(name)[0].strip()
        if base in ("u8", "s8") and typ not in ("u8", "char") and "[" in decl:
            # "extern u8 x[];" was a stand-in for data nobody had typed: it is defined as what
            # its words are (tlb_0125's sekisyo5YakuninData holds addresses)
            decl = typ + " " + decl[len(decl.split(name)[0]):].lstrip()
            base = typ
            words = [r for *_, r in items]
            if len(words) % 2 == 0 and len(words) >= 2 and all(len(r) == 4 for r in words) and \
                    words[-2] == b"\xDF\0\0\0" and words[-1] == b"\0\0\0\0":
                # words that end in G_ENDDL are a display list (tlb_0137's kensukeMoneyDigitDl)
                decl = "Gfx " + decl[len(decl.split(name)[0]):].lstrip()
                base = "Gfx"
        if "*" in base and typ not in ("AnimFrame", "char"):
            elems = []
            for a, k, t, r in items:
                if a in symbol_words:
                    # a scalar variable's address is &name; an array's or a function's, name
                    elems.append(f"&{t}" if _is_scalar_var(csrc, t.strip()) else t)
                else:
                    v = struct.unpack(">I", r)[0]
                    elems.append("NULL" if v == 0 else f"({base}) 0x{v:08X}" if "(" not in base else f"0x{v:08X}")
        elif base == "f32" and typ in ("u32", "s32") and all(len(r) == 4 for *_, r in items) and \
                not any(a in symbol_words for a, *_ in items):
            # the C declares floats (tlb_0060's kkomaAtkOfs2[5][3]): each word as its float, never
            # the word's integer converted (0x42F40000 compiled to 0x4E85E800)
            elems = [_f32_text(r) for *_, r in items]
        elif base in ("u16", "s16", "u8", "s8") and typ in ("u32", "s32") and \
                not any(a in symbol_words for a, *_ in items):
            # the C's elements are narrower than the data's words: each word is its big-endian
            # halves or bytes, in order (tlb_0018_7's u16 [5][16] took one word per u16 so)
            size = 2 if base in ("u16", "s16") else 1
            fmt = {("u16", 2): ">H", ("s16", 2): ">h", ("u8", 1): ">B", ("s8", 1): ">b"}[(base, size)]
            raw = b"".join(r for *_, r in items)
            elems = [str(struct.unpack(fmt, raw[i:i + size])[0]) if base[0] == "s"
                     else f"0x{struct.unpack(fmt, raw[i:i + size])[0]:0{size * 2}X}"
                     for i in range(0, len(raw), size)]
        elif base in ("s32", "s16", "s8") and typ in ("u32", "s32", "u16", "s16"):
            fmt = {"s32": ">i", "s16": ">h", "s8": ">b"}[base]
            elems = [str(struct.unpack(fmt, r[-struct.calcsize(fmt):])[0]) for *_, r in items]
        elif base in ("Gfx", "AnimFrame") and typ != "AnimFrame" and all(k == "word" for _, k, _, _ in items) \
                and len(items) % 2 == 0:
            # struct arrays: one brace pair per element; a flat list does not fill a union (Gfx)
            # and truncates an AnimFrame's duration word (tlb_0017_2 lost 0xC0 bytes so)
            def word(a, t, r):
                return f"(u32) {t}" if a in symbol_words else f"0x{struct.unpack('>I', r)[0]:08X}"
            elems = []
            seg_a = re.search(r"tlb_(\d{4})", os.path.basename(c_file))
            if base == "AnimFrame" and seg_a and isinstance(items[0][0], int):
                # an animation table ends at its { 0, 0 }: data after it is another (unnamed) label
                for i in range(0, len(items) - 2, 2):
                    if items[i][3] == b"\0" * 4 and items[i + 1][3] == b"\0" * 4 \
                            and items[i][0] not in symbol_words:
                        after = items[i + 2:]
                        words = [f"(u32) {t}" if a in symbol_words else f"0x{struct.unpack('>I', r)[0]:08X}"
                                 for a, k, t, r in after]
                        rest += f"\nu32 D_{after[0][0]:08X}_tlb_{seg_a.group(1)}[] = {{ {', '.join(words)} }};"
                        items = items[:i + 2]
                        break
            for i in range(0, len(items), 2):
                (a0, _, t0, r0), (a1, _, t1, r1) = items[i], items[i + 1]
                if base == "AnimFrame":
                    w1 = struct.unpack(">I", r1)[0]
                    if a1 in symbol_words or w1 & 0xFFFF:
                        print(f"data-c --apply: REFUSED, {name} is AnimFrame in the C but its words are not")
                        return 1
                    elems.append(f"{{ {word(a0, t0, r0)}, {w1 >> 16} }}")
                else:
                    elems.append(f"{{ {word(a0, t0, r0)}, {word(a1, t1, r1)} }}")
        elif _plain_struct(csrc, base) and all(len(r) == 4 for a, k, t, r in items if a in symbol_words):
            fields, ssize = _plain_struct(csrc, base)
            raw = b"".join(r for *_, r in items)
            addr0 = items[0][0]
            sym_at = {a: t for a, k, t, r in items if a in symbol_words}
            seg_s = re.search(r"tlb_(\d{4})", os.path.basename(c_file))
            whole = len(raw) // ssize * ssize
            if not isinstance(addr0, int) or whole == 0 or (len(raw) - whole) % 4 or \
                    (whole < len(raw) and not seg_s):
                print(f"data-c --apply: REFUSED, {name} is {base} in the C, its bytes are not whole {base}s")
                return 1
            if whole < len(raw) and name == defs[-1][0] and len(raw) - whole < 16 and not any(raw[whole:]) \
                    and not any(addr0 + o in sym_at for o in range(whole, len(raw), 4)):
                pass  # zero padding at the end of the file's .data: IDO pads the object itself (915)
            elif whole < len(raw):
                # the data after the last whole element is another (unnamed) label
                words = [f"(u32) {sym_at[addr0 + o]}" if (addr0 + o) in sym_at
                         else f"0x{struct.unpack('>I', raw[o:o + 4])[0]:08X}" for o in range(whole, len(raw), 4)]
                rest += f"\nu32 D_{addr0 + whole:08X}_tlb_{seg_s.group(1)}[] = {{ {', '.join(words)} }};"
            elems = _struct_elems(fields, ssize, raw, whole, addr0, sym_at, own_labels, seg_s,
                                  sym_values, filler_refs)
        elif base == "Vtx" and not any(a in symbol_words for a, *_ in items) and \
                len(b"".join(r for *_, r in items)) >= 16 and \
                not any(b"".join(r for *_, r in items)[len(b"".join(r for *_, r in items)) // 16 * 16:]):
            raw = b"".join(r for *_, r in items)
            raw = raw[:len(raw) // 16 * 16]  # zero bytes past the last vertex: padding
            elems = []
            for o in range(0, len(raw), 16):
                x, y, z, flag, s, t = struct.unpack(">hhhHhh", raw[o:o + 12])
                c = raw[o + 12:o + 16]
                elems.append(f"{{{{{{ {x}, {y}, {z} }}, {flag}, {{ {s}, {t} }}, "
                             f"{{ 0x{c[0]:02X}, 0x{c[1]:02X}, 0x{c[2]:02X}, 0x{c[3]:02X} }}}}}}")
        elif base == "Vec3f" and typ == "f32" and vals and len(vals) % 3 == 0 and \
                not any(a in symbol_words for a, *_ in items):
            elems = [f"{{ {vals[i]}, {vals[i + 1]}, {vals[i + 2]} }}" for i in range(0, len(vals), 3)]
        elif base == "Vec3f" and typ in ("u32", "s32") and len(items) % 3 == 0 and \
                all(len(r) == 4 for *_, r in items) and not any(a in symbol_words for a, *_ in items):
            def shortest(r):
                v = struct.unpack(">f", r)[0]
                for n in range(1, 10):
                    s = f"{v:.{n}g}"
                    if struct.pack(">f", float(s)) == r:
                        break
                return (s if any(c in s for c in ".en") else s + ".0") + "f"
            fl = [shortest(r) for *_, r in items]
            elems = [f"{{ {fl[i]}, {fl[i + 1]}, {fl[i + 2]} }}" for i in range(0, len(fl), 3)]
        elif base not in ("u8", "s8", "u16", "s16", "u32", "s32", "f32", "f64", "char", "AnimFrame") \
                and not base.endswith("*"):
            print(f"data-c --apply: REFUSED, {name} is {base} in the C, a type this tool does not fill")
            return 1
        else:
            elems = vals
        scalar = "[" not in decl
        # a fully sized array whose data runs on in zero bytes: the excess is alignment padding
        # before the next label (tlb_0018_7's u16 [5][16], 4 zero bytes before an 8-aligned Gfx)
        all_dims = re.findall(r"\[\s*(\d+)\s*\]", decl.split(name, 1)[1]) if name in decl else []
        esz = {"u8": 1, "s8": 1, "u16": 2, "s16": 2, "u32": 4, "s32": 4, "f32": 4}.get(base) or \
            (4 if base.endswith("*") else None)
        if all_dims and esz and "[]" not in decl.replace(" ", ""):
            count = 1
            for d in all_dims:
                count *= int(d)
            raw = b"".join(r for *_, r in items)
            if len(elems) > count and len(raw) > count * esz and not any(raw[count * esz:]):
                elems = elems[:count]
        inner = re.match(r"\s*\[\s*\]\s*((?:\[\s*\d+\s*\]\s*)+)$", decl.split(name, 1)[1]) if name in decl else None
        row = 1
        for d in re.findall(r"\d+", inner.group(1) if inner else ""):
            row *= int(d)
        raw = b"".join(r for *_, r in items)
        if inner and esz and row > 1 and len(elems) == len(raw) // esz and len(elems) % row and \
                not any(raw[(len(elems) // row) * row * esz:]):
            # [][3] whose data runs on in zero bytes past its last whole row: padding (tlb_0149's
            # itemKobanThrowVel, 25 rows and a zero word; C made it 26 rows)
            elems = elems[:(len(elems) // row) * row]
        if scalar and len(elems) > 1 and esz == 4 and len(items) > 1 and all(len(r) == 4 for *_, r in items[1:]):
            # a scalar whose label holds more words (tlb_0028_0's D_08004BF0: the frame count,
            # then a zero word nothing names): the scalar takes the first, the rest become the
            # next word's own label, defined right after it
            m = re.match(r"D_([0-9A-Fa-f]{7,8})(_\w+)?$", name)
            seg_n = re.search(r"tlb_(\d{4})", os.path.basename(c_file))
            if not m and seg_n and isinstance(items[1][0], int):
                # a named scalar: the rest takes its next address's name
                m = re.match(r"D_([0-9A-Fa-f]{8})(_\w+)?$", f"D_{items[1][0] - 4:08X}_tlb_{seg_n.group(1)}")
            if m:
                nxt_name = f"D_{int(m.group(1), 16) + 4:08X}{m.group(2) or ''}"
                words = [f"(u32) {t}" if a in symbol_words else f"0x{struct.unpack('>I', r)[0]:08X}"
                         for a, k, t, r in items[1:]]
                rest += (f"\nu32 {nxt_name}[] = {{ {', '.join(words)} }};" if len(words) > 1
                         else f"\nu32 {nxt_name} = {words[0]};")
                elems = elems[:1]
        if scalar and esz in (1, 2) and len(elems) > 1 and \
                all(re.fullmatch(r"(0x)?0+", e.strip()) for e in elems[1:]):
            elems = elems[:1]  # the rest is padding the compiler adds itself
        raw_all = b"".join(r for *_, r in items)
        seg = re.search(r"tlb_(\d{4})", os.path.basename(c_file))
        if scalar and esz == 4 and len(elems) > 1 and len(raw_all) > 4 and seg and isinstance(items[0][0], int) \
                and not any(a in symbol_words for a, *_ in items):
            # a named scalar, or one whose label is written in bytes, holding more than its word:
            # the scalar takes the first word, the rest is a u8 array named by the next address
            first = struct.unpack(">i" if base == "s32" else ">I", raw_all[:4])[0]
            nxt_name = f"D_{items[0][0] + 4:08X}_tlb_{seg.group(1)}"
            rest += f"\nu8 {nxt_name}[] = {{ {', '.join(f'0x{b:02X}' for b in raw_all[4:])} }};"
            elems = [str(first) if base == "s32" else f"0x{first:08X}"]
        dims = re.search(re.escape(name) + r"\s*\[\s*(\d+)\s*\]", decl)
        if dims and len(elems) > int(dims.group(1)) and decl.count("[") == 1:
            # the data runs past the length the C declared: every word is kept, unsized
            decl = decl[:dims.start(1)] + decl[dims.end(1):]
            rest = f" /* declared [{dims.group(1)}] before its data was C */" + rest
        if typ == "AnimFrame":
            init = "{\n    " + ",\n    ".join(elems) + ",\n}"
        elif scalar and len(elems) == 1:
            init = elems[0]
        elif re.fullmatch(r"\s*\[\s*\d*\s*\]\s*(\[\s*\d+\s*\]\s*)+", decl.split(name, 1)[1] if name in decl else "") \
                and not any("{" in e for e in elems):
            # a 2-D (or deeper) array, a brace to a row ({ 0.0f, 4.5f, 0.0f }, ...), rows of rows
            inner_dims = [int(x) for x in re.findall(r"\[\s*(\d+)\s*\]", decl.split(name, 1)[1].lstrip()[1:].split("]", 1)[1])]
            groups = list(elems)
            for w in reversed(inner_dims):
                if len(groups) % w:
                    groups = None
                    break
                groups = ["{ " + ", ".join(groups[i:i + w]) + " }" for i in range(0, len(groups), w)]
            init = ("{\n    " + ",\n    ".join(groups) + ",\n}") if groups else "{ " + ", ".join(elems) + " }"
        elif elems and all(e.startswith("{") for e in elems):  # one element too (915)
            init = "{\n    " + ",\n    ".join(elems) + ",\n}"  # an element (struct, Gfx) a line
        else:
            init = "{ " + ", ".join(elems) + " }"
        comment, nl, extra = rest.partition("\n")
        if nl and comment.count("/*") > comment.count("*/"):
            # the comment runs on: the split's definitions go after the line that closes it
            k = next((j for j in range(idx + 1, len(lines)) if "*/" in lines[j]), idx)
            lines[idx] = f"{decl} = {init};{comment}"
            lines[k] += "\n" + extra
            placed[name] = idx
            deftext[name] = [f"{decl} = {init};"] + extra.split("\n")
            extern_at[name] = idx
            dt = decl.split(name)[0].strip()
            for j, l in enumerate(lines):
                em = re.match(r"^extern\s+[^;(]*?\b%s\b\s*(\[[^\]]*\])?\s*;(.*)$" % re.escape(name), l)
                if em and j != idx:
                    lines[j] = f"extern {dt} {name}{'[]' if '[' in decl else ''};{em.group(2)}"
            continue
        lines[idx] = f"{decl} = {init};{rest}"
        # the name's other extern lines (one inside a parked block) take the definition's type
        dt = decl.split(name)[0].strip()
        for j, l in enumerate(lines):
            em = re.match(r"^extern\s+[^;(]*?\b%s\b\s*(\[[^\]]*\])?\s*;(.*)$" % re.escape(name), l)
            if em and j != idx:
                lines[j] = f"extern {dt} {name}{'[]' if '[' in decl else ''};{em.group(2)}"
        placed[name] = idx
        deftext[name] = lines[idx].split("\n")
        extern_at[name] = idx
    order = [d[0] for d in defs]
    for i, ((name, *_), text) in enumerate(zip(defs, standalone)):
        if name in placed:
            continue
        nxt = next((placed[n] for n in order[i + 1:] if n in placed), None)
        if nxt is None:
            prv = [ends.get(n, placed[n]) for n in order[:i] if n in placed]
            if prv:
                nxt = max(prv) + 1
            else:
                # the file's head: its #include/#define lines before the first function or
                # GLOBAL_ASM, never a macro defined further down (tlb_0026_1's ROADTEST)
                first = next((j for j, l in enumerate(lines)
                              if l.startswith("#pragma GLOBAL_ASM") or re.match(r"^\w[^;=]*\([^;]*\)\s*\{\s*$", l)),
                             len(lines))
                tops = [j for j, l in enumerate(lines[:first]) if l.startswith(("#include", "#define"))]
                nxt = (tops[-1] + 1) if tops else 0
        while 0 < nxt <= len(lines) and lines[nxt - 1].rstrip().endswith("\\"):
            nxt += 1  # past a multi-line #define's continuation lines
        have = "\n".join(lines)
        # the draft's own "extern u8 X[];" lines for a symbol the file already declares go
        hdr = _header_names(csrc)
        # the file's own extern lines for this name take the definition's type
        dm = re.search(r"^([A-Za-z_][\w ]*?\**)\s*\b%s\b\s*(\[[^\]]*\])?\s*=" % re.escape(name), text, re.M)
        if dm:
            for j, l in enumerate(lines):
                em = re.match(r"^extern\s+[^;(]*?\b%s\b\s*(\[[^\]]*\])?\s*;(.*)$" % re.escape(name), l)
                if em:
                    lines[j] = f"extern {dm.group(1).strip()} {name}{'[]' if dm.group(2) else ''};{em.group(2)}"
        new = [l for l in text.split("\n")
               if not (re.match(r"^extern u8 (\w+)\[\];$", l)
                       and (re.match(r"^extern u8 (\w+)", l).group(1) in hdr
                            or re.search(r"^extern\b[^;]*\b%s\b" % re.escape(re.match(r"^extern u8 (\w+)", l).group(1)),
                                         have, re.M)))]
        # symbols the C file has never declared: as u8[] (addresses, as splat names them)
        used = sorted({re.split(r"\s*\+", t.strip())[0] for a, k, t, r in defs[i][1] if a in symbol_words})
        used = [s for s in used if re.search(r"\b" + re.escape(s) + r"\b", "\n".join(new))]  # written (915)
        ext = [f"extern u8 {s}[];" for s in used if not re.search(r"\b" + re.escape(s) + r"\b", have)
               and s not in hdr and not any(re.match(r"^extern\b[^;]*\b%s\b" % re.escape(s), l) for l in new)]
        new = ext + new
        lines[nxt:nxt] = new
        for n in placed:
            if placed[n] >= nxt:
                placed[n] += len(new)
        for n in ends:
            if ends[n] >= nxt:
                ends[n] += len(new)
        placed[name] = nxt + len(ext)
        ends[name] = nxt + len(new) - 1
        deftext[name] = new
        inserted.append(name)
    idxs = [placed[n] for n in order]
    # the definitions' text: a symbol splat named but the filler wrote as its number (a narrow
    # field) is not declared (915)
    written = "\n".join(l for v in deftext.values() for l in v)
    if idxs != sorted(idxs):
        # the externs are not in address order: they all stay, as forward declarations, and the
        # definitions go together in address order after the last of them, where everything
        # their initializers name is declared (a head block failed: tlb_0017_2's D_3000530)
        lines = csrc.split("\n")
        # the forward declarations take the definitions' types ("extern u8 x[];" stood in for a
        # display list defined Gfx: "redeclaration of kensukeMoneyDigitDl")
        for n in order:
            head = next((l for l in deftext.get(n, []) if re.search(r"\b" + re.escape(n) + r"\b", l) and "=" in l), "")
            hm = re.match(r"^\s*([\w ]+?\**)\s*\b" + re.escape(n) + r"\b\s*(\[[^=]*\])?\s*=", head)
            if not hm:
                continue
            for j, l in enumerate(lines):
                em = re.match(r"^extern\s+[^;(]*?\b%s\b\s*(\[[^\]]*\])*\s*;(.*)$" % re.escape(n), l)
                if em:
                    lines[j] = f"extern {hm.group(1).strip()} {n}{hm.group(2) or ''};{em.group(2)}"
        last = max((i for i, l in enumerate(lines) for n in extern_at
                    if re.match(r"^extern\s+.*\b" + re.escape(n) + r"\b", l)), default=None)
        if last is None:
            first = next((j for j, l in enumerate(lines)
                          if l.startswith("#pragma GLOBAL_ASM") or re.match(r"^\w[^;=]*\([^;]*\)\s*\{\s*$", l)),
                         len(lines))
            tops = [j for j, l in enumerate(lines[:first]) if l.startswith(("#include", "#define"))]
            last = tops[-1] if tops else -1
        block = ["", "/* The file's .data, in address order (its externs above stay as declarations). */"]
        above = re.sub(r"/\*.*?\*/", "", "\n".join(lines[:last + 1]), flags=re.S)  # code, not comments
        above = re.sub(r"^#pragma GLOBAL_ASM\(.*$", "", above, flags=re.M)  # nor a .s path's name
        refs = sorted({re.split(r"\s*\+", t.strip())[0] for d in defs for a, k, t, r in d[1] if a in symbol_words}
                      | filler_refs)
        refs = [s for s in refs if re.search(r"\b" + re.escape(s) + r"\b", written)]  # (915)
        for s in refs:
            if re.search(r"\b" + re.escape(s) + r"\b", above):
                continue
            if s in deftext:  # this file's own label, defined further on in the block
                head = next((l for l in deftext[s] if re.search(r"\b" + re.escape(s) + r"\b", l) and "=" in l), "")
                m = re.match(r"^\s*([\w ]+?\**)\s*\b" + re.escape(s) + r"\b\s*(\[[^\]]*\])?", head)
                if m:
                    block.append(f"extern {m.group(1).strip()} {s}{'[]' if m.group(2) else ''};")
            else:
                block.append(f"extern u8 {s}[];")
        for n in order:
            block += [l for l in deftext[n] if not re.match(r"^extern u8 \w+\[\];$", l)]
        lines[last + 1:last + 1] = block
        print(f"data-c --apply: {os.path.relpath(c_file, build.ROOT)}'s externs were not in address "
              f"order: the {len(order)} definitions go together after the last of them")
    else:
        # in order: a table that points at one of this file's labels defined further on needs
        # that label declared ahead of the first definition (tlb_0016_0's D_08008050)
        first_def = min(placed.values()) if placed else 0
        above = re.sub(r"/\*.*?\*/", "", "\n".join(lines[:first_def]), flags=re.S)  # code, not comments
        above = re.sub(r"^#pragma GLOBAL_ASM\(.*$", "", above, flags=re.M)  # nor a .s path's name
        refs = sorted({re.split(r"\s*\+", t.strip())[0] for d in defs for a, k, t, r in d[1] if a in symbol_words}
                      | filler_refs)
        refs = [s for s in refs if re.search(r"\b" + re.escape(s) + r"\b", written)]  # (915)
        fwd = []
        for s in refs:
            if re.search(r"\b" + re.escape(s) + r"\b", above):
                continue
            if s not in deftext and s in _header_names(csrc):
                continue  # a header the file includes declares it already
            later = re.search(r"^extern\b[^;(]*?\b%s\b[^;(]*;" % re.escape(s), csrc, re.M)
            if s not in deftext and later:
                fwd.append(later.group(0))  # its own declaration, copied ahead
                continue
            if s not in deftext:  # an outside symbol the file declares only further down
                fdef = re.search(r"^([A-Za-z_][\w \*]*?\**)\s*\b%s\s*\(([^;{]*)\)\s*\{" % re.escape(s),
                                 csrc, re.M)
                # a function the file defines: declared ahead by its own prototype
                fwd.append(f"{fdef.group(1).strip()} {s}({fdef.group(2).strip()});" if fdef
                           else f"extern u8 {s}[];")
                continue
            head = next((l for l in deftext[s] if re.search(r"\b" + re.escape(s) + r"\b", l) and "=" in l), "")
            m = re.match(r"^\s*([\w ]+?\**)\s*\b" + re.escape(s) + r"\b\s*(\[[^\]]*\])?", head)
            if m:
                fwd.append(f"extern {m.group(1).strip()} {s}{'[]' if m.group(2) else ''};")
        # a symbol now declared ahead is not declared again further down as u8[]
        fwd_names = {m.group(1) for m in (re.search(r"\b(\w+)\s*(?:\[\])?\s*(?:\([^)]*\))?;$", l) for l in fwd) if m}
        lines = lines[:first_def] + [l for l in lines[first_def:]
                                     if not (re.match(r"^extern u8 (\w+)\[\];$", l)
                                             and re.match(r"^extern u8 (\w+)", l).group(1) in fwd_names)]
        lines[first_def:first_def] = fwd
    with open(c_file, "w", encoding="utf-8", newline="\n") as f:  # LF, as the tree's files
        f.write("\n".join(lines))
    obj = os.path.join(build.ROOT, "permuter", "datac", "applied.o")
    if compile_one(c_file, obj) != 0:
        failed = os.path.join(build.ROOT, "permuter", "datac", os.path.basename(c_file)[:-2] + ".failed.c")
        with open(failed, "w", encoding="utf-8", newline="\n") as f:  # the text the errors are about
            f.write("\n".join(lines))
        with open(c_file, "w", encoding="utf-8", newline="\n") as f:  # as it was before --apply
            f.write(csrc)
        print(f"data-c --apply: {os.path.relpath(c_file, build.ROOT)} did not compile with its data; put back "
              f"(the text tried: {os.path.relpath(failed, build.ROOT)})")
        return 1
    # the applied file's .data, byte for byte against the game's (definitions in the file's order)
    objcopy = build.tool_path(os.path.join(build.BINUTILS, "mips64-elf-objcopy" + build.EXE))
    got = os.path.join(build.ROOT, "permuter", "datac", "applied.data.bin")
    subprocess.run([objcopy, "-O", "binary", "-j", ".data", os.path.abspath(obj).replace("\\", "/"),
                    os.path.abspath(got).replace("\\", "/")], cwd=build.ROOT, env=build.ENV)
    with open(got, "rb") as f:
        have = f.read()
    want = have
    if binf and os.path.exists(binf):
        with open(binf, "rb") as f:
            want = f.read()
    # the section's alignment against where the game's data starts
    objdump = build.tool_path(os.path.join(build.BINUTILS, "mips64-elf-objdump" + build.EXE))
    hdr = subprocess.run([objdump, "-h", os.path.abspath(obj).replace("\\", "/")], cwd=build.ROOT,
                         env=build.ENV, capture_output=True, text=True).stdout
    al = re.search(r"\s\.data\s+\S+\s+\S+\s+\S+\s+\S+\s+2\*\*(\d+)", hdr)
    starts = [items[0][0] for _, items, *_ in defs if items and isinstance(items[0][0], int)]
    if al and starts and min(starts) % (1 << int(al.group(1))):
        with open(c_file, "w", encoding="utf-8", newline="\n") as f:  # as it was before --apply
            f.write(csrc)
        print(f"data-c --apply: {os.path.relpath(c_file, build.ROOT)}: its .data is aligned to "
              f"{1 << int(al.group(1))}, the game's starts at 0x{min(starts):08X}; put back")
        return 1
    # the game's symbol words by .data offset: (name, its word in the game)
    game_syms = {}
    if starts:
        for _, items, *_ in defs:
            for a, k, t, r in items:
                if a in symbol_words and isinstance(a, int):
                    game_syms[a - min(starts)] = (re.split(r"\s*\+", t.strip())[0], r)
    if (sym_values or game_syms) and len(have) == len(want):
        # a word written as its symbol: a relocation in the object, the symbol's value in the game
        # (where the draft wrote the label as bytes, tlb_0016_0's MapArea); accepted only where
        # the game's word is the one splat named by that symbol, or the value a plain word's
        # symbol was chosen for
        rel = subprocess.run([objdump, "-r", "-j", ".data", os.path.abspath(obj).replace("\\", "/")],
                             cwd=build.ROOT, env=build.ENV, capture_output=True, text=True).stdout
        fixed = bytearray(have)
        for m in re.finditer(r"^([0-9a-f]{8})\s+R_MIPS_32\s+(\w+)\s*$", rel, re.M):
            off, s = int(m.group(1), 16), m.group(2)
            if have[off:off + 4] != b"\0\0\0\0":
                continue
            if s in sym_values and want[off:off + 4] == sym_values[s]:
                fixed[off:off + 4] = sym_values[s]
            elif off in game_syms and game_syms[off][0] == s and want[off:off + 4] == game_syms[off][1]:
                fixed[off:off + 4] = game_syms[off][1]
        have = bytes(fixed)
    if game_syms and len(have) == len(want):
        # a game symbol word the filler wrote as its number (a field narrower than a word): the
        # applied object holds the game's own bytes there, the draft only its relocation's
        # placeholder (915)
        w = bytearray(want)
        for off, (_, real) in game_syms.items():
            if have[off:off + 4] == real and want[off:off + 4] != real:
                w[off:off + 4] = real
        want = bytes(w)
    if have != want:
        with open(c_file, "w", encoding="utf-8", newline="\n") as f:  # as it was before --apply
            f.write(csrc)
        print(f"data-c --apply: {os.path.relpath(c_file, build.ROOT)}: its .data compiles to other bytes "
              f"({len(have)} against the game's {len(want)}: the definitions' order); put back")
        return 1
    _data_flip(c_file)
    print(f"data-c --apply: {os.path.relpath(c_file, build.ROOT)}: {len(placed) - len(inserted)} externs "
          f"defined in place, {len(inserted)} inserted ({', '.join(inserted[:6])}"
          f"{'...' if len(inserted) > 6 else ''}; a function static among them must move into its "
          f"function). Next: the yaml subsegment to .data, build_rom.py --split")
    return 0


NEAR_LIKELY = {b + "l": b for b in ("beq", "bne", "beqz", "bnez", "blez", "bgtz", "bltz", "bgez",
                                    "bc1t", "bc1f")}
NEAR_DISP = re.compile(r"(?<![\w+])(-?(?:0x[0-9A-Fa-f]+|\d+))\((\$\w+)\)")


def _near_view(path):
    """(mnemonics, displacements) of a function: IDA's view, or splat's .s without an export.
    A displacement is the offset of a load or store off any base but $sp, else None."""
    fn = _ida_function(path)
    if fn:
        rows = [(m, ops) for _, _, m, ops, _ in fn["ins"]]
        kind = "ida"
    else:
        rows = []
        for ins in asm_shape(path):
            parts = ins.split(None, 1)
            rows.append((parts[0], parts[1] if len(parts) > 1 else ""))
        kind = "splat"
    mn, disp = [kind], [None]
    for m, ops in rows:
        if m == "nop":
            continue  # delay-slot filling, not the code
        m = NEAR_LIKELY.get(m, m)
        mn.append(m)
        d = NEAR_DISP.search(ops) if m[:1] in "ls" else None
        disp.append(int(d.group(1), 0) if d and d.group(2) != "$sp" else None)
    return tuple(mn), disp


def _near_bag(a, b):
    """The two mnemonic lists' overlap as multisets (order ignored): what IDO's scheduling
    cannot move. 2 * shared / total."""
    import collections
    ca, cb = collections.Counter(a), collections.Counter(b)
    return 2 * sum((ca & cb).values()) / (len(a) + len(b))


NEAR_CONTROLS = [
    # (target, its known twin, the shift the aligned loads/stores must show, or None)
    ("func_0800A8D4_tlb_0325", "func_0800BEA8_tlb_0324", 0xC),  # found by eye, (496)
    ("func_0800A85C_tlb_0325", "func_0800BE30_tlb_0324", 0xC),  # the step that calls them
]


IDIOM_FIXED = {"$sp", "$zero", "$ra", "$at", "$gp", "$fp"}


def _idiom_norm(fn_ins, strict=False):
    """IDA instructions -> normalized tokens: registers renamed by first use (fixed ones kept),
    names and frame offsets as S, numbers as N (kept when strict)."""
    out, regs = [], {}
    for ea, op, m, ops, target in fn_ins:
        o = re.sub(r"0x[0-9A-Fa-f]+\+(?=var_|arg_)", "", ops)
        o = re.sub(r"\b(?:var|arg)_[0-9A-Fa-f]+(?:\+\d+)?", "F", o)
        o = re.sub(r"(?<![\w$])(?:loc|def|jpt|sub|off|dword|flt|dbl|unk|byte|word|a[A-Z_]\w*)_?\w*", "S", o)

        def reg(mm):
            r = mm.group(0)
            if r in IDIOM_FIXED:
                return r
            if r.startswith("$f"):
                kind = "F"
            elif not strict:
                kind = "R"
            elif r in ("$a4", "$a5", "$a6", "$a7"):
                kind = "T"
            else:
                kind = r[1].upper()
            if r not in regs:
                k = sum(1 for v in regs.values() if v[0] == kind)
                regs[r] = kind + "abcdefghijklmnopqrstuvwxyz"[k % 26] * (1 + k // 26)
            return regs[r]
        o = re.sub(r"\$\w+", reg, o)
        o = re.sub(r"-?(?:0x[0-9A-Fa-f]+|\d+(?:\.\d+)?)", "N", o)
        out.append(f"{m} {o}".strip())
    return out


def _loc_lines(t, func, mnems):
    """trace's full.s for func: the C lines (file, line, text) of the .loc groups holding the
    first in-order run of mnems (mnemonics) in func's body."""
    with open(os.path.join(t, "full.s"), errors="replace") as f:
        text = f.read()
    files = {m.group(1): m.group(2) for m in re.finditer(r'^\s*\.file\s+(\d+)\s+"([^"]+)"', text, re.M)}
    body = text.split(f"\t.ent\t{func} ", 1)[-1].split("\t.end\t", 1)[0].split("\n")
    seq, loc = [], None
    for line in body:
        m = re.match(r"\s*\.loc\s+(\d+)\s+(\d+)", line)
        if m:
            loc = (m.group(1), int(m.group(2)))
            continue
        m = re.match(r"\s+([a-z][a-z0-9.]*)\b", line)
        if m and not line.lstrip().startswith("."):
            seq.append((m.group(1), loc))
    want = [x.split()[0] for x in mnems]
    # IDA's real instructions against ugen's listing, which still has the assembler's pseudo-ops
    alias = {"li": {"li", "lui", "addiu", "ori", "la"}, "move": {"move", "or", "addu"},
             "dli": {"li", "addiu", "daddiu"}, "b": {"b", "j"},
             "addiu": {"addiu", "addu", "add", "sub", "subu", "la"}, "addu": {"addu", "add"},
             "subu": {"subu", "sub", "negu", "neg"}, "negu": {"negu", "neg", "subu"},
             "bgez": {"bgez", "bge"}, "bltz": {"bltz", "blt"}, "blez": {"blez", "ble"},
             "bgtz": {"bgtz", "bgt"}, "beqz": {"beqz", "beq"}, "bnez": {"bnez", "bne"},
             "beq": {"beq", "beqz"}, "bne": {"bne", "bnez"}, "slt": {"slt", "sgt", "blt", "bgt"},
             "sltu": {"sltu", "sgtu", "bltu", "bgtu"}, "slti": {"slti", "slt", "blt", "bge"},
             "sltiu": {"sltiu", "sltu", "bltu", "bgeu"}, "andi": {"andi", "and"},
             "ori": {"ori", "or"}, "xori": {"xori", "xor"}, "sll": {"sll", "mul"},
             "lwc1": {"lwc1", "l.s"}, "swc1": {"swc1", "s.s"}, "ldc1": {"ldc1", "l.d"},
             "sdc1": {"sdc1", "s.d"}, "mfc1": {"mfc1"}, "mtc1": {"mtc1", "li.s", "li.d"},
             "jalr": {"jalr", "jal"}, "jal": {"jal", "jalr"}}
    best = None
    for s in range(len(seq)):
        k, hit = 0, []
        for i in range(s, min(len(seq), s + 4 * len(want))):
            if k < len(want) and seq[i][0] in alias.get(want[k], {want[k]}):
                hit.append(seq[i][1])
                k += 1
        if k == len(want):
            best = hit
            break
    if not best:
        return []
    out, seen = [], set()
    for loc in best:
        if loc and loc not in seen:
            seen.add(loc)
            path = files.get(loc[0])
            try:
                with open(path, encoding="utf-8", errors="replace") as f:
                    src = f.read().split("\n")
                out.append((loc[1], src[loc[1] - 1].strip()))
            except (OSError, IndexError, TypeError):
                out.append((loc[1], "?"))
    return sorted(out)


IDIOM_STORES = {"sb", "sh", "sw", "swc1", "sdc1", "sd", "swl", "swr"}


def _idiom_vregs(ins):
    """(defs, uses, bases) of one IDA instruction's registers: a memory operand's base apart,
    and $zero/$sp/$ra/$at/$gp/$fp never link."""
    m, ops = ins[2], ins[3]
    parts = [p.strip() for p in ops.split(",")] if ops else []
    bases = {r for p in parts for r in re.findall(r"\((\$\w+)\)", p) if r not in IDIOM_FIXED}
    regs = [re.findall(r"\$\w+", re.sub(r"\(\$\w+\)", "", p)) for p in parts]
    regs = [[r for r in rs if r not in IDIOM_FIXED] for rs in regs]
    if not regs:
        return set(), set(), bases
    if m in IDIOM_STORES or m.startswith(("b", "j", "c.", "mt")):
        return set(), {r for rs in regs for r in rs}, bases
    return set(regs[0]), {r for rs in regs[1:] for r in rs}, bases


def _idiom_slice(win, anchor_ea):
    """The instructions of win linked to the one at anchor_ea: through value registers, and
    through a base register the slice itself defined."""
    live, defined = set(), set()
    for ins in win:
        if ins[0] == anchor_ea:
            d, u, b = _idiom_vregs(ins)
            live, defined = d | u, set(d)
    keep, changed = {anchor_ea}, True
    while changed:
        changed = False
        for ins in win:
            if ins[0] in keep:
                continue
            d, u, b = _idiom_vregs(ins)
            if (d | u) & live or b & defined:
                keep.add(ins[0])
                live |= d | u
                defined |= d
                changed = True
    return [ins for ins in win if ins[0] in keep]


def _idiom_find(ins, query_norm, mnems, strict, gap=4):
    """The first in-order run of mnems in ins (at most gap others between two) whose normalized
    form is query_norm, as its instructions, or None."""
    n = len(mnems)
    for i, x in enumerate(ins):
        if x[2] != mnems[0]:
            continue
        pick, j = [x], i
        for k in range(1, n):
            nxt = next((p for p in range(j + 1, min(len(ins), j + 2 + gap)) if ins[p][2] == mnems[k]), None)
            if nxt is None:
                break
            pick.append(ins[nxt])
            j = nxt
        if len(pick) == n and _idiom_norm(pick, strict) == query_norm:
            return pick
    return None


def idiom(c_file, func, index=None, strict=False, before=4, after=6, show=5):
    """idiom: which C the game's matched code uses for the instructions a parked function differs
    in: C is copied from what matched code shows, never made up."""
    folder = os.path.splitext(os.path.relpath(os.path.join(build.ROOT, c_file), os.path.join(build.ROOT, "src")))[0]
    asm_dir = os.path.join(build.ROOT, "asm", "nonmatchings", folder.replace("\\", "/"))
    spath = os.path.join(asm_dir, f"{func}.s")
    fn = _ida_function(spath)
    if fn is None:
        print(f"idiom: no IDA record for {func} ({os.path.relpath(spath, build.ROOT)})")
        return 1
    if index is None:
        with open(os.path.join(asm_dir, "diff.txt"), encoding="utf-8", errors="replace") as f:
            text = f.read()
        game = {int(m.group(1)): m.group(2) for m in re.finditer(r"^-\s+(\d+)\s+(.*)$", text, re.M)}
        mine = {int(m.group(1)): m.group(2) for m in re.finditer(r"^\+\s+(\d+)\s+(.*)$", text, re.M)}

        def content(s):  # a branch target or a stack offset is not content
            s = re.sub(r"<\+0x[0-9a-f]+>", "T", s)
            return re.sub(r"-?\d+\(sp\)", "S(sp)", s)
        idx = [i for i in sorted(game) if i not in mine or content(game[i]) != content(mine[i])]
        if not idx:
            print(f"idiom: no content difference in {os.path.relpath(asm_dir, build.ROOT)}/diff.txt (run check first)")
            return 1
        index = idx[0]
    start = _asm_start(spath)
    if isinstance(index, list):  # the named instructions alone
        eas = {start + 4 * i for i in index}
        win = [x for x in fn["ins"] if x[0] in eas]
        index, before, after = index[0], 0, index[-1] - index[0]
    else:
        lo, hi = start + 4 * (index - before), start + 4 * (index + after)
        win = [x for x in fn["ins"] if lo <= x[0] <= hi]
        anchor = max((x[0] for x in win if x[0] <= start + 4 * index), default=None)
        win = _idiom_slice(win, anchor)
    query = _idiom_norm(win, strict)
    mnems = [x[2] for x in win]
    print(f"idiom: {func} game instruction {index}, its value slice in {index - before}..{index + after} (IDA):")
    for x, q in zip(win, query):
        print(f"    {x[0]:08X}  {x[2]:<6} {x[3]:<40}  {q}")
    parked = {f for _, f in parked_functions()}
    defined = {}
    for path in glob.glob(os.path.join(build.ROOT, "src", "**", "*.c"), recursive=True):
        with open(path, encoding="utf-8") as f:
            for name in re.findall(r"^[A-Za-z][\w \*]*?\b(\w+)\([^;{]*\)\s*\{", f.read(), re.M):
                defined.setdefault(name, os.path.relpath(path, build.ROOT).replace("\\", "/"))
    hits, seen = [], set()
    n = len(query)
    for kind in ("nonmatchings", "matchings"):
        for path in glob.glob(os.path.join(build.ROOT, "asm", kind, "**", "*.s"), recursive=True):
            name = os.path.splitext(os.path.basename(path))[0]
            if name in seen or name == func or name in parked or name not in defined:
                continue
            seen.add(name)
            other = _ida_function(path)
            if other is None or len(other["ins"]) < n:
                continue
            pick = _idiom_find(other["ins"], query, mnems, strict)
            if pick:
                hits.append((name, defined[name], pick[0][0], mnems))
    print(f"idiom: {len(hits)} matched functions hold this {'strict' if strict else 'loose'} window")
    for name, cfile, ea, _ in hits:
        print(f"    {name}  ({cfile}, at {ea:08X})")
    import contextlib
    import io
    for name, cfile, ea, mn in hits[:show]:
        d = os.path.join(build.ROOT, "permuter", name)
        with contextlib.redirect_stdout(io.StringIO()):
            ok = trace(cfile, name) == 0
        if not ok:
            print(f"--- {name}: trace failed")
            continue
        print(f"--- {name} ({cfile}): the C that emitted the hit")
        for line, src in _loc_lines(os.path.join(d, "trace"), name, mn):
            print(f"    {line:5d}  {src}")
    return 0


def twins_near(min_ratio=0.8, only=None, control=False):
    """Each function still in assembly (in a C file's GLOBAL_ASM) with its nearest matched
    function by mnemonics: the twin's C is the draft. Lengths within 15% only, behind difflib's
    quick gates. The shift is what the aligned loads and stores say about the struct: the most
    common target-minus-twin offset differences. control: the known pairs
    in NEAR_CONTROLS as targets instead, each checked against its expected twin and shift."""
    import bisect
    import collections
    import difflib
    import time
    t0 = time.time()
    pulled = {}
    for path in glob.glob(os.path.join(build.ROOT, "src", "**", "*.c"), recursive=True):
        with open(path, encoding="utf-8") as f:
            for p in re.findall(r'GLOBAL_ASM\("([^"]+)"\)', f.read()):
                pulled[os.path.splitext(os.path.basename(p))[0]] = os.path.relpath(path, build.ROOT)
    defined = {}
    for path in glob.glob(os.path.join(build.ROOT, "src", "**", "*.c"), recursive=True):
        with open(path, encoding="utf-8") as f:
            for name in re.findall(r"^[A-Za-z][\w \*]*?\b(\w+)\([^;{]*\)\s*\{", f.read(), re.M):
                defined.setdefault(name, os.path.relpath(path, build.ROOT))
    targets, done, seen = [], [], set()
    for kind in ("nonmatchings", "matchings"):
        for path in glob.glob(os.path.join(build.ROOT, "asm", kind, "**", "*.s"), recursive=True):
            name = os.path.splitext(os.path.basename(path))[0]
            if name.startswith(("D_", "jtbl_")) or name in seen:
                continue
            seen.add(name)
            folder = os.path.relpath(os.path.dirname(path), os.path.join(build.ROOT, "asm", kind)).replace(os.sep, "/")
            if only and name in pulled and folder != only:
                continue
            mn, disp = _near_view(path)
            if len(mn) < 9:
                continue
            if control:
                if name in {c[0] for c in NEAR_CONTROLS}:
                    targets.append((name, folder, mn, disp))
                    pulled.setdefault(name, defined.get(name, "?"))
                elif name in defined:
                    done.append((len(mn), name, folder, mn, disp))
            elif name in pulled:
                targets.append((name, folder, mn, disp))
            elif name in defined:
                done.append((len(mn), name, folder, mn, disp))
    done.sort(key=lambda d: d[0])
    lengths = [d[0] for d in done]
    rows = []
    for name, folder, mn, disp in targets:
        n = len(mn)
        lo, hi = bisect.bisect_left(lengths, n * 0.85), bisect.bisect_right(lengths, n / 0.85)
        best = None
        sm = difflib.SequenceMatcher(None, autojunk=False)
        sm.set_seq2(mn)
        for _, tname, tfolder, tmn, tdisp in done[lo:hi]:
            if tmn[0] != mn[0]:
                continue
            sm.set_seq1(tmn)
            floor = max(min_ratio, best[0] if best else 0)
            if sm.real_quick_ratio() < floor:
                continue
            bag = sm.quick_ratio()  # order ignored: what IDO's scheduling cannot move
            if bag < floor:
                continue
            r = sm.ratio()
            r = (r + bag) / 2 if n > 20 else r
            if r >= floor and (best is None or r > best[0]):
                shift = collections.Counter()
                for a, b, size in sm.get_matching_blocks():
                    for k in range(size):
                        if tdisp[a + k] is not None and disp[b + k] is not None:
                            shift[disp[b + k] - tdisp[a + k]] += 1
                best = (r, tname, tfolder, shift, len(tmn))
        if best:
            rows.append((best[0], n - 1, name, folder, best))
    rows.sort(key=lambda r: (-r[0], -r[1]))
    if control:
        found = {name: best for _, _, name, _, best in rows}
        bad = 0
        for target, twin, shift in NEAR_CONTROLS:
            best = found.get(target)
            ok = best is not None and best[1] == twin and (shift is None or best[3].most_common(1)[0][0] == shift)
            bad += not ok
            got = (f"{best[1]} at {best[0]:.3f}, shift "
                   + ", ".join(f"{d:+#x} x{c}" for d, c in best[3].most_common(2))) if best else "nothing"
            print(f"twins near control: {'PASS' if ok else 'FAIL'} {target} -> want {twin}"
                  f"{f' {shift:+#x}' if shift else ''}; got {got}")
            if not ok:  # why: each side's view, length and the ratio between them
                t = next((x for x in targets if x[0] == target), None)
                w = next((x for x in done if x[1] == twin), None)
                if not (t and w):
                    print(f"  why: target {'seen' if t else 'NOT SEEN'}, twin {'seen' if w else 'NOT SEEN (not defined in src?)'}")
                else:
                    r = difflib.SequenceMatcher(None, w[3], t[2], autojunk=False).ratio()
                    print(f"  why: target {t[2][0]} {len(t[2]) - 1} ins, twin {w[3][0]} {len(w[3]) - 1} ins, "
                          f"ratio {r:.3f}, bag {_near_bag(w[3], t[2]):.3f}")
        print(f"twins near control: {len(NEAR_CONTROLS) - bad}/{len(NEAR_CONTROLS)} ({time.time() - t0:.0f} s)")
        return 1 if bad else 0
    os.makedirs(os.path.join(build.ROOT, "permuter", "twins"), exist_ok=True)
    out = os.path.join(build.ROOT, "permuter", "twins", "near.txt")
    parked = {func for _, func in parked_functions()}
    rows.sort(key=lambda r: (r[2] in parked, -r[0], -r[1]))  # bare first: no C yet, the twin's is the draft
    with open(out, "w", encoding="utf-8") as f:
        for r, n, name, folder, (_, tname, tfolder, shift, tn) in rows:
            moved = ", ".join(f"{d:+#x} x{c}" for d, c in shift.most_common(3) if d) or "-"
            f.write(f"{r:.3f} {'parked' if name in parked else 'BARE  '} {folder}/{name} ({n} ins, in {pulled[name]}) <- {tfolder}/{tname} "
                    f"({tn - 1} ins, {defined[tname]}); same offsets x{shift[0]}, moved {moved}\n")
    print(f"twins near: {len(targets)} functions still in assembly, {len(done)} matched; "
          f"{len(rows)} with a matched twin at {min_ratio} or more -> {os.path.relpath(out, build.ROOT)} "
          f"({time.time() - t0:.0f} s)")
    with open(out, encoding="utf-8") as f:
        for line in f.readlines()[:15]:
            print("  " + line.rstrip())
    return 0


LOOSE_SYMBOL = re.compile(r"%(?:hi|lo|gp_rel)\(([A-Za-z_]\w*)\)|\bjal\s+([A-Za-z_]\w*)")


def _asm_symbols(path):
    """Every symbol a function's .s names, in instruction order (.text only): address names,
    and any name in %hi/%lo or a jal target (twin-copy's loose shape abstracts them all)."""
    out, in_text = [], True
    with open(path, encoding="utf-8") as f:
        for line in f:
            if line.startswith(".section"):
                in_text = ".text" in line
                continue
            m = ASM_LINE.match(line)
            if in_text and m:
                ins = LABEL.sub("L", m.group(2))
                for s in re.finditer(r"%(?:hi|lo|gp_rel)\(([A-Za-z_]\w*)\)|\bjal\s+([A-Za-z_]\w*)|" + SYMBOL.pattern, ins):
                    out.append(s.group(1) or s.group(2) or s.group(0))
    return out


def _loose_shape(path):
    """asm_shape with every symbol abstracted (see LOOSE_SYMBOL), for twin-copy."""
    return tuple(SYMBOL.sub("S", LOOSE_SYMBOL.sub(lambda m: "%X(S)" if m.group(1) else "jal S", ins))
                 for ins in asm_shape(path))


# a file-scope declaration on one line: extern data, a prototype, or tlb.h's far-call macro
DECL_LINE = re.compile(r'^(?:extern\s[^;{]*?\b(\w+)\s*(?:\[[^\]]*\])*\s*;'
                       r'|(?!return\b|typedef\b)[A-Za-z_][\w \t\*]*?\b(\w+)\s*\([^;{]*\)\s*;'
                       r'|#define\s+(\w+)\s+\(\*\w+\))\s*(?:/\*.*\*/)?\s*$', re.M)


def _declared(text):
    """{name: its declaration line} for every one-line file-scope declaration in a C file."""
    out = {}
    for m in DECL_LINE.finditer(text):
        out.setdefault(m.group(1) or m.group(2) or m.group(3), m.group(0).strip())
    return out


def _typedefs(text):
    """[(name, full text)] of every file-scope typedef in a C file, in order (one line, or a
    struct/union/enum block)."""
    out, i = [], 0
    for m in re.finditer(r"^typedef\b", text, re.M):
        j = m.start()
        if j < i:
            continue
        semi, brace = text.find(";", j), text.find("{", j)
        if brace != -1 and brace < semi:
            depth, k = 0, brace
            while k < len(text):
                depth += {"{": 1, "}": -1}.get(text[k], 0)
                if depth == 0:
                    break
                k += 1
            semi = text.find(";", k)
        block = text[j:semi + 1]
        name = re.findall(r"(\w+)\s*(?:\[[^\]]*\])*\s*;$", block) or re.findall(r"\(\s*\*\s*(\w+)\s*\)", block)
        if name:
            out.append((name[-1], block))
        i = semi + 1
    return out


IDA_EXPORT = os.path.join(os.path.dirname(build.ROOT), "ida", "goemons_great_adventure", "export")
_IDA_CACHE = {}


def _map_names():
    """{address: name} of every symbol in the link map (build/goemonsgreatadv.map) at 0x80000000
    and up (main and the overlays; the TLB files all share 0x08000000), leaving out the linker's
    segment symbols and .NON_MATCHING aliases. An address with two names is left out."""
    if "map" not in _IDA_CACHE:
        names, seen = {}, set()
        path = os.path.join(build.ROOT, "build", "goemonsgreatadv.map")
        if os.path.exists(path):
            with open(path, encoding="utf-8", errors="replace") as f:
                for m in re.finditer(r"^\s+0x([0-9a-f]{8,16})\s+([A-Za-z_]\w*)\s*$", f.read(), re.M):
                    addr, name = int(m.group(1), 16) & 0xFFFFFFFF, m.group(2)
                    if addr < 0x80000000 or name.startswith("__") or re.search(
                            r"_(?:ROM|VRAM|TEXT|DATA|BSS|RODATA)_(?:START|END)|SegmentStart|SegmentEnd", name):
                        continue
                    if addr in names and names[addr] != name:
                        seen.add(addr)
                    names.setdefault(addr, name)
        _IDA_CACHE["map"] = {a: n for a, n in names.items() if a not in seen}
    return _IDA_CACHE["map"]


def _ida_key(asm_path):
    """The IDA database a function's .s belongs to, by its folder: tlb/tlb_NNNN[_K] -> NNNN, an
    overlay's file -> 0006/0008/0009/0010, else main."""
    rel = os.path.relpath(asm_path, os.path.join(build.ROOT, "asm")).replace("\\", "/")
    m = re.search(r"/tlb/tlb_(\d{4})(?:_\d+)?/", "/" + rel)
    if m:
        return m.group(1)
    seg = None
    try:
        import report_progress
        if "segs" not in _IDA_CACHE:
            _IDA_CACHE["segs"] = report_progress.file_segments()
        seg = _IDA_CACHE["segs"].get(rel.split("/", 1)[1].rsplit("/", 1)[0])
    except Exception:
        pass
    return seg[len("ovl_"):] if seg and seg.startswith("ovl_") else "main"


def _ida_export_for(asm_path):
    """The IDA export holding a function's code (_ida_key), or None."""
    key = _ida_key(asm_path)
    if key not in _IDA_CACHE:
        path = os.path.join(IDA_EXPORT, f"{key}.json")
        exp = None
        if os.path.exists(path):
            import json
            with open(path, encoding="utf-8") as f:
                exp = json.load(f)
            exp["by_start"] = {fn["start"]: fn for fn in exp["functions"] if fn["start"] is not None}
        _IDA_CACHE[key] = exp
    return _IDA_CACHE[key]


def _asm_start(path):
    """A function's first address, from its .s: the first instruction line in .text (a .s may
    begin with the function's .rodata/.late_rodata, whose lines carry data addresses)."""
    in_text = True
    with open(path, encoding="utf-8") as f:
        for line in f:
            if line.startswith(".section"):
                in_text = ".text" in line
                continue
            m = ASM_LINE.match(line) if in_text else None
            if m:
                return int(m.group(1), 16)
    return None


def _ida_function(path):
    """The IDA export's record of the function whose .s this is (same start), or None. NO_IDA=1
    in the environment gives splat's view instead, to compare the two."""
    if os.environ.get("NO_IDA"):
        return None
    exp = _ida_export_for(path)
    start = _asm_start(path)
    return exp["by_start"].get(start) if exp and start is not None else None


def _ida_shape(fn):
    """IDA's loose shape of a function: mnemonic and operands, every name and every address-sized
    immediate abstracted (frame var_/arg_ offsets and registers kept)."""
    out = []
    for ea, op, m, ops, target in fn["ins"]:
        o = re.sub(r"(?<![\w$])(?!var_|arg_)[A-Za-z_]\w*", "S", ops)
        o = re.sub(r"0x[0-9A-Fa-f]{5,}", "S", o)
        out.append(f"{m} {o}")
    return tuple(out)


def _ida_symbols(fn):
    """The addresses a function names, in instruction order: IDA's resolved data targets and the
    targets of its calls (sub_XXXXXXXX), for twin-copy's pairing."""
    out = []
    for ea, op, m, ops, target in fn["ins"]:
        if target is not None:
            out.append(target)
        for c in re.findall(r"\bsub_([0-9A-Fa-f]{7,8})\b", ops):
            out.append(int(c, 16))
    return out


def _label_names(asm_path):
    """{address: name} of every label in the .s files of a function's C file (its folder in
    asm/nonmatchings/ and asm/matchings/): the names the C uses for those addresses."""
    folder = os.path.basename(os.path.dirname(asm_path))
    parent = os.path.basename(os.path.dirname(os.path.dirname(asm_path)))
    out = {}
    for kind in ("nonmatchings", "matchings"):
        d = os.path.join(build.ROOT, "asm", kind, parent, folder) if parent == "tlb" else \
            os.path.join(build.ROOT, "asm", kind, folder)
        for p in glob.glob(os.path.join(d, "*.s")):
            with open(p, encoding="utf-8") as f:
                name = None
                for line in f:
                    mm = re.match(r"^(?:glabel|dlabel)\s+(\w+)", line)
                    if mm:
                        name = mm.group(1)
                        continue
                    am = ASM_LINE.match(line) or re.match(r"^\s*/\* [0-9A-F]+ ([0-9A-F]{8}) ", line)
                    if am and name:
                        out.setdefault(int(am.group(1), 16), name)
                        name = None
    return out


def _asm_strings(path):
    """[(data label, string)] for every string in a function's .s (its migrated .rodata), one
    per .asciz, in .rodata order. Splat can put several strings (unreferenced prints) under one
    label, so the label is the one each string sits under."""
    out, label = [], None
    with open(path, encoding="utf-8") as f:
        for line in f:
            m = re.match(r"^dlabel (\w+)", line)
            if m:
                label = m.group(1)
                continue
            m = re.search(r'\.asciz\s+"((?:[^"\\]|\\.)*)"', line)
            if m and label:
                out.append((label, m.group(1)))
            if line.startswith("enddlabel"):
                label = None
    return out


def _asm_floats(path):
    """[(".float" or ".double", exact bits as hex)] of a function's .s constants, in order."""
    out = []
    with open(path, encoding="utf-8") as f:
        for line in f:
            m = re.match(r"^\s*/\* [0-9A-F]+ [0-9A-F]{8} ([0-9A-F]{8,16}) \*/\s+\.(float|double)\b", line)
            if m:
                out.append((m.group(2), m.group(1)))
    return out


C_FLOAT = re.compile(r"(?<![\w.])((?:\d+\.\d*|\.\d+)(?:[eE][-+]?\d+)?)(f?)(?![\w.])")


def _float_text(kind, bits):
    """The shortest C literal for these exact bits (with f for a float)."""
    import struct
    if kind == "float":
        v = struct.unpack(">f", bytes.fromhex(bits))[0]
        for p in range(6, 12):
            s = format(v, f".{p}g")
            if struct.pack(">f", float(s)).hex().upper() == bits:
                break
    else:
        v = struct.unpack(">d", bytes.fromhex(bits))[0]
        s = format(v, ".17g")
    if not any(c in s for c in ".en"):
        s += ".0"
    return s + ("f" if kind == "float" else "")


def _map_floats(body, tfl, ffl):
    """body with each literal whose bits are one of the twin's constants rewritten as the
    target's constant at the same index (lists of equal length and kinds only). (body, n)."""
    import struct
    if len(tfl) != len(ffl) or any(a[0] != b[0] for a, b in zip(tfl, ffl)):
        return body, 0
    fmap = {}
    for (kind, tb), (_, fb) in zip(tfl, ffl):
        if tb != fb:
            fmap.setdefault((kind, tb), fb)
    if not fmap:
        return body, 0

    changed = [0]

    def repl(m):
        kind = "float" if m.group(2) else "double"
        try:
            bits = (struct.pack(">f", float(m.group(1))) if kind == "float"
                    else struct.pack(">d", float(m.group(1)))).hex().upper()
        except (ValueError, OverflowError):
            return m.group(0)
        if (kind, bits) in fmap:
            changed[0] += 1
            return _float_text(kind, fmap[(kind, bits)])
        return m.group(0)
    return C_FLOAT.sub(repl, body), changed[0]  # only the literals actually rewritten


def _folded_empties(path):
    """How many empty functions a .s holds when it is only `jr ra; nop` pairs (0 otherwise)."""
    words = []
    with open(path, encoding="utf-8") as f:
        for line in f:
            m = re.match(r"^\s*/\* [0-9A-F]+ [0-9A-F]{8} ([0-9A-F]{8}) \*/", line)
            if m:
                words.append(m.group(1))
    if len(words) < 4 or len(words) % 2:
        return 0
    return len(words) // 2 if all(w == ("03E00008" if i % 2 == 0 else "00000000")
                                  for i, w in enumerate(words)) else 0


M2C_PLAIN_NOT = re.compile(r"M2C_|\bunk|\barg\d|\btemp_|\bvar_|\bphi_|\?|\bsp[0-9A-Fa-f]{2,}\b|saved_reg")


def m2c_drafts(files=None, limit=None, apply=False, min_size=8, max_size=None):
    """Each function still GLOBAL_ASM (not parked), smallest first: m2c's draft (--valid-syntax;
    m2c_macros.h for the fields it cannot type), put in place of its pragma in a copy of its file
    under permuter/m2c/<func>/, and checked there. Prints each verdict and the hit rate. src/
    never changes: an M2C_FIELD draft is no readable C, so a match is a start."""
    vendor = os.path.join(os.path.dirname(build.ROOT), "vendor", "m2c")
    with open(os.path.join(vendor, "m2c_macros.h"), encoding="utf-8") as f:
        macros = f.read()
    todo = []
    for path in sorted(glob.glob(os.path.join(build.ROOT, "src", "**", "*.c"), recursive=True)):
        rel = os.path.relpath(path, build.ROOT).replace("\\", "/")
        if files and rel not in files:
            continue
        with open(path, encoding="utf-8") as f:
            text = f.read()
        parked = set(re.findall(r'#else\s*\n\s*#pragma GLOBAL_ASM\("([^"]+)"\)', text))
        for asm in re.findall(r'^#pragma GLOBAL_ASM\("([^"]+)"\)', text, re.M):
            func = os.path.splitext(os.path.basename(asm))[0]
            if asm in parked or func.startswith(("D_", "jtbl_")) or not os.path.exists(os.path.join(build.ROOT, asm)):
                continue
            with open(os.path.join(build.ROOT, asm), encoding="utf-8") as f:
                m = re.search(r"^nonmatching\s+\w+,\s*(0x[0-9A-Fa-f]+)", f.read(), re.M)
            todo.append((int(m.group(1), 16) if m else 1 << 30, rel, func, asm))
    todo.sort()
    # a 4-byte "function" is one nop at a file's start (padding, not a function)
    todo = [t for t in todo if t[0] >= min_size and (max_size is None or t[0] <= max_size)]
    if limit:
        todo = todo[:limit]
    import collections
    import concurrent.futures
    import hashlib
    results, skipped = collections.Counter(), 0

    def one(item):
        """(kind, verdict, cached) for one function; its verdict.txt keeps the key (the asm and
        the C file it was drafted against), so an unchanged function is not drafted again."""
        size, rel, func, asm = item
        d = os.path.join(build.ROOT, "permuter", "m2c", func)
        with open(os.path.join(build.ROOT, asm), encoding="utf-8") as f:
            asm_text = f.read()
        with open(os.path.join(build.ROOT, rel), encoding="utf-8") as f:
            text = f.read()
        key = hashlib.sha1((asm_text + "\0" + text).encode("utf-8")).hexdigest()[:16]
        vpath = os.path.join(d, "verdict.txt")
        if not apply and os.path.exists(vpath):
            with open(vpath, encoding="utf-8") as f:
                old = f.read().rstrip("\n").split("\t")
            if len(old) == 4 and old[0] == key:
                return old[2], old[3], True
        r = subprocess.run([sys.executable, os.path.join(vendor, "m2c.py"), "-t", "mips-ido-c", "--valid-syntax",
                            os.path.join(build.ROOT, asm)], cwd=build.ROOT, capture_output=True, text=True,
                           encoding="utf-8", errors="replace")
        draft = "\n".join(l for l in r.stdout.splitlines() if not l.rstrip().endswith("// extern"))
        os.makedirs(d, exist_ok=True)
        if r.returncode or not draft.strip() or "M2C_ERROR" in draft:
            kind, verdict = "m2c could not", f"m2c could not ({(r.stderr.strip().splitlines() or ['M2C_ERROR'])[-1][:80]})"
        else:
            pragma = f'#pragma GLOBAL_ASM("{asm}")'
            copy_text = text.replace(pragma, draft, 1)
            inc = copy_text.find("\n", copy_text.find('#include "')) + 1 if '#include "' in copy_text else 0
            copy_text = copy_text[:inc] + macros + "\n" + copy_text[inc:]
            copy = os.path.join(d, os.path.basename(rel))
            _write_retry(copy, copy_text)
            c = subprocess.run([sys.executable, os.path.abspath(__file__), "check",
                                os.path.relpath(copy, build.ROOT).replace("\\", "/"), func, os.path.join(d, "chk")],
                               cwd=build.ROOT, capture_output=True, text=True, encoding="utf-8", errors="replace")
            last = [l for l in c.stdout.splitlines() if l.startswith(func + ":")]
            verdict = last[-1][len(func) + 2:] if last else "does not compile"
            kind = "MATCH" if re.search(r"^MATCH\b", verdict) else "compiles, differs" if last else "does not compile"
            if kind == "MATCH" and not M2C_PLAIN_NOT.search(draft):
                kind = "MATCH, plain C"
                if apply:
                    src = os.path.join(build.ROOT, rel)
                    with open(src, encoding="utf-8") as f:
                        real = f.read()
                    if real.count(pragma) == 1:
                        _write_retry(src, real.replace(pragma, draft.strip("\n"), 1))
                        kind = "MATCH, plain C, applied"
        with open(vpath, "w", encoding="utf-8", newline="\n") as f:
            f.write(f"{key}\t{rel}\t{kind}\t{verdict}\n")
        return kind, verdict, False

    # two at a time (not the whole machine at once); one when --apply
    # writes src/ (two drafts into one file must not race)
    with concurrent.futures.ThreadPoolExecutor(1 if apply else 2) as pool:
        for (size, rel, func, asm), (kind, verdict, cached) in zip(todo, pool.map(one, todo)):
            results[kind] += 1
            skipped += cached
            print(f"{func} (0x{size:X}): {verdict[:100]}" + ("  [applied]" if kind.endswith("applied") else "")
                  + ("  [cached]" if cached else ""), flush=True)
    # every function's latest verdict, for picking runs and for the briefs
    rows = []
    for vpath in glob.glob(os.path.join(build.ROOT, "permuter", "m2c", "*", "verdict.txt")):
        with open(vpath, encoding="utf-8") as f:
            parts = f.read().rstrip("\n").split("\t")
        if len(parts) == 4:
            rows.append((os.path.basename(os.path.dirname(vpath)), parts[1], parts[2], parts[3]))
    with open(os.path.join(build.ROOT, "permuter", "m2c", "summary.tsv"), "w", encoding="utf-8", newline="\n") as f:
        f.write("func\tfile\tkind\tverdict\n" + "".join("\t".join(r) + "\n" for r in sorted(rows, key=lambda r: (r[1], r[0]))))
    print(f"m2c-drafts: {len(todo)} tried ({skipped} unchanged, from cache): "
          + ", ".join(f"{k} {v}" for k, v in results.most_common()) + "; permuter/m2c/summary.tsv")
    return 0


def fold_empties():
    """every function still in assembly that is only `jr ra; nop` pairs
    becomes that many empty C functions, the extra ones named for their addresses. The
    single-function check cannot see past the first, so the full build is the proof."""
    done = 0
    for path in glob.glob(os.path.join(build.ROOT, "src", "**", "*.c"), recursive=True):
        with open(path, encoding="utf-8") as f:
            text = f.read()
        new = text
        for asm in re.findall(r'#pragma GLOBAL_ASM\("([^"]+)"\)', text):
            name = os.path.splitext(os.path.basename(asm))[0]
            m = re.match(r"^func_([0-9A-F]{8})(_\w+)$", name)
            full = os.path.join(build.ROOT, asm)
            n = _folded_empties(full) if m and os.path.exists(full) else 0
            pragma = f'#pragma GLOBAL_ASM("{asm}")'
            if not n or re.search(r"#else\s*\n\s*" + re.escape(pragma), new):
                continue
            addr = int(m.group(1), 16)
            funcs = [f"void func_{addr + 8 * k:08X}{m.group(2)}(void) {{\n}}" for k in range(n)]
            funcs[0] = f"void {name}(void) {{\n}}"
            new = new.replace(pragma, "\n\n".join(funcs), 1)
            done += 1
            print(f"{os.path.relpath(path, build.ROOT)}: {name} as {n} empty functions")
        if new != text and not _write_retry(path, new):
            sys.exit(f"empties: {path} locked")
    print(f"folded-empties: {done} written; now the full build (check cannot see past the first)")
    return 0


def twin_copy(funcs=None):
    """each function still in assembly that has a same-shape twin in C gets
    that twin's C. Every address-named symbol is swapped for its counterpart, paired in
    instruction order from the two .s files (one pairing per symbol, else refused); the
    declarations the copy needs come from the twin's file, renamed the same way; a file on
    common.h takes tlb.h when the twin's did (its far calls). Kept only when check says MATCH;
    otherwise the file is put back exactly. Then: the full build, and `nisitenma_archive.py
    yaml-diffs`: "MATCH except own section addresses" is only provisional. A copy that brings
    its own .rodata can still place it differently in the file (sekisyo2VramProc's copy into
    files 121, 123, 124 and 125: check passed, the build did not); revert those files."""
    pulled, where = set(), {}
    for path in glob.glob(os.path.join(build.ROOT, "src", "**", "*.c"), recursive=True):
        with open(path, encoding="utf-8") as f:
            for p in re.findall(r'GLOBAL_ASM\("([^"]+)"\)', f.read()):
                name = os.path.splitext(os.path.basename(p))[0]
                pulled.add(name)
                where[name] = (os.path.relpath(path, build.ROOT).replace("\\", "/"), p)
    shapes, asm_of, ida_of = {}, {}, {}
    for kind in ("nonmatchings", "matchings"):
        for path in glob.glob(os.path.join(build.ROOT, "asm", kind, "**", "*.s"), recursive=True):
            name = os.path.splitext(os.path.basename(path))[0]
            if name.startswith(("D_", "jtbl_")) or (kind == "matchings" and name in asm_of):
                continue
            if _folded_empties(path):
                continue  # folded empty functions: the empties mode writes them
            # IDA's view of the function; splat's .s only without an export
            fn = _ida_function(path)
            shape = ("ida",) + _ida_shape(fn) if fn else ("splat",) + _loose_shape(path)
            if len(shape) >= 5:
                shapes.setdefault(shape, []).append(name)
                asm_of[name] = path
                ida_of[name] = fn
    srcs = {}
    for path in glob.glob(os.path.join(build.ROOT, "src", "**", "*.c"), recursive=True):
        with open(path, encoding="utf-8") as f:
            srcs[os.path.relpath(path, build.ROOT).replace("\\", "/")] = f.read()
    todo = []
    for shape, names in shapes.items():
        in_c = [n for n in names if n not in pulled]
        for n in names:
            if n in pulled and in_c and (not funcs or n in funcs) and n in where:
                todo.append((n, in_c))
    kept, tried, skipped = [], 0, 0
    # tries that failed before, with a hash of both files then: skipped while neither changed
    import hashlib
    failed_path = os.path.join(build.ROOT, "permuter", "twincopy", "failed.txt")
    failed = set()
    if os.path.exists(failed_path):
        with open(failed_path, encoding="utf-8") as f:
            failed = {line.strip() for line in f if line.strip()}
    # each kept copy's file and failure key: land puts the key in failed.txt when the full build
    # rejects the file
    kept_path = os.path.join(build.ROOT, "permuter", "twincopy", "kept.txt")
    os.makedirs(os.path.dirname(kept_path), exist_ok=True)
    open(kept_path, "w").close()
    for func, twins in todo:
        c_file, asm_path = where[func]
        # the pragma itself, not one inside #ifdef NON_MATCHING ... #else (a parked function)
        text = srcs[c_file]
        pragma = f'#pragma GLOBAL_ASM("{asm_path}")'
        if text.count(pragma) != 1:
            continue
        ppos = text.index(pragma)
        # a parked function: its whole #ifdef NON_MATCHING ... #endif block gives way
        # (karasuFadeOut's twin had matched while it stayed parked)
        parked_fn = bool(re.search(r"#else\s*\n\s*" + re.escape(pragma), text))
        if parked_fn:
            s0, e0 = text.rfind("#ifdef NON_MATCHING", 0, ppos), text.find("#endif", ppos)
            if s0 < 0 or e0 < 0 or "#endif" in text[s0:ppos]:
                continue
            site = (s0, e0 + len("#endif"))
        else:
            site = (ppos, ppos + len(pragma))
        for twin in twins:
            t_file = next((p for p, t in srcs.items() if _func_span(t, twin)), None)
            if not t_file:
                continue
            key = f"{func} {twin} " + hashlib.sha1((text + "\0" + srcs[t_file]).encode("utf-8")).hexdigest()[:16]
            if key in failed:
                skipped += 1
                continue
            pairs, ok = {twin: func}, True
            if ida_of.get(twin) and ida_of.get(func):
                # IDA's targets in order, each named as its own file's C names it; an address
                # both share (a global) needs no pair
                a, b = _ida_symbols(ida_of[twin]), _ida_symbols(ida_of[func])
                tnames, fnames = _label_names(asm_of[twin]), _label_names(asm_of[func])
                ok = len(a) == len(b)
                for x, y in zip(a, b):
                    if x == y and x not in tnames:
                        continue
                    tn, fname = tnames.get(x), fnames.get(y)
                    if tn is None and fname is None:
                        # two different addresses outside both files: the same code only when
                        # each is exactly a named symbol (then the names swap); a field of a
                        # global (gGameWork+0x12 against +0x0) is not
                        gx, gy = _map_names().get(x), _map_names().get(y)
                        if not gx or not gy:
                            ok = False
                        elif pairs.setdefault(gx, gy) != gy:
                            ok = False
                        continue
                    if tn is None or fname is None:
                        ok = False  # one side local, the other not: not the same code
                        continue
                    if pairs.setdefault(tn, fname) != fname:
                        ok = False
            else:
                a, b = _asm_symbols(asm_of[twin]), _asm_symbols(asm_of[func])
                ok = len(a) == len(b)
                for x, y in zip(a, b):
                    if pairs.setdefault(x, y) != y:
                        ok = False
            if not ok:
                continue
            sub = re.compile(r"\b(" + "|".join(map(re.escape, sorted(pairs, key=len, reverse=True))) + r")\b")
            span = _func_span(srcs[t_file], twin)
            body = srcs[t_file][span[0]:span[1]]
            # without the twin's comment: it describes the twin, which may be false here
            body = body[re.search(r"^[A-Za-z][^\n;]*\b%s\(" % re.escape(twin), body, re.M).start():]
            body = sub.sub(lambda m: pairs[m.group(1)], body)
            # string literals: the twin's text becomes the target's own string's text. Paired in
            # .rodata order (IDO emits literals in source order), which also reaches print
            # strings no instruction references; by symbol pair when the counts differ
            tstr, fstr = _asm_strings(asm_of[twin]), _asm_strings(asm_of[func])
            strings_note = f"strings {len(tstr)}/{len(fstr)}"
            if len(tstr) == len(fstr):
                # text for text by .rodata index (the body's literal order need not be the
                # .rodata order), in one pass so a replacement is never replaced again
                tmap = {t: f for (_, t), (_, f) in zip(tstr, fstr) if t != f}
                if tmap:
                    lit = re.compile("|".join('"%s"' % re.escape(t) for t in sorted(tmap, key=len, reverse=True)))
                    body, n = lit.subn(lambda m: '"%s"' % tmap[m.group(0)[1:-1]], body)
                    strings_note += f", {n} replaced"
            else:
                tfirst, ffirst = dict(reversed(tstr)), dict(reversed(fstr))  # a label's first string
                for x, y in pairs.items():
                    if x in tfirst and y in ffirst and tfirst[x] != ffirst[y]:
                        body = body.replace(f'"{tfirst[x]}"', f'"{ffirst[y]}"', 1)
            body, nf = _map_floats(body, _asm_floats(asm_of[twin]), _asm_floats(asm_of[func]))
            strings_note += f"; constants {nf} replaced"
            have = _declared(text[:site[0]])  # only what is declared before the copy
            decls = []
            for name, line in _declared(srcs[t_file]).items():
                new = sub.sub(lambda m: pairs[m.group(1)], line)
                nname = pairs.get(name, name)
                if nname != func and nname not in have and re.search(r"\b%s\b" % re.escape(nname), body):
                    decls.append(new)
            # the twin file's own types the copy uses (and the types those use), in its order
            mine = {n for n, _ in _typedefs(text)}
            theirs = [(n, sub.sub(lambda m: pairs[m.group(1)], t)) for n, t in _typedefs(srcs[t_file])
                      if n not in mine]
            need, used = set(), body + "\n".join(decls)
            while True:
                more = {n for n, t in theirs if n not in need and re.search(r"\b%s\b" % re.escape(n), used)}
                if not more:
                    break
                need |= more
                used += "\n".join(t for n, t in theirs if n in more)
            # a copied type's comments describe the twin's file: only its offsets come along
            types = [re.sub(r"[ \t]*/\*(?!\s*0x[0-9A-Fa-f]+\s*\*/).*?\*/", "", t, flags=re.S)
                     for n, t in theirs if n in need]
            head = "\n\n".join(types + (["\n".join(decls)] if decls else []))
            new_text = text[:site[0]] + (head + "\n\n" if head else "") + body.rstrip("\n") + text[site[1]:]
            if '#include "tlb.h"' in srcs[t_file] and '#include "tlb.h"' not in new_text:
                new_text = new_text.replace('#include "common.h"', '#include "tlb.h"', 1)
            full = os.path.join(build.ROOT, c_file)
            tried += 1
            if not _write_retry(full, new_text):
                continue
            d = os.path.join(build.ROOT, "permuter", "twincopy", func)
            r = subprocess.run([sys.executable, os.path.abspath(__file__), "check", c_file, func, d],
                               cwd=build.ROOT, capture_output=True, text=True)
            os.makedirs(d, exist_ok=True)
            with open(os.path.join(d, f"check_{twin}.log"), "w", encoding="utf-8", newline="\n") as f:
                f.write(r.stdout + r.stderr)  # the whole check, for a failure's reason
            last = [l for l in r.stdout.splitlines() if l.startswith(func + ":")]
            verdict = last[-1] if last else f"{func}: FAILED ({(r.stdout + r.stderr).strip().splitlines()[-1:]})"
            if re.search(r": MATCH\b", verdict):
                srcs[c_file] = new_text
                kept.append(func)
                with open(kept_path, "a", encoding="utf-8", newline="\n") as f:
                    f.write(f"{c_file}\t{key}\n")
                print(f"KEPT {func} from {twin} ({t_file}): {verdict}; {strings_note}"
                      + (f"; copied types to rename for this file: {', '.join(sorted(need))}" if need else "")
                      + ("; WAS PARKED: rewrite its NON_MATCHING note above it" if parked_fn else ""),
                      flush=True)
                break
            _write_retry(full, text)  # put back exactly
            print(f"no   {func} from {twin}: {verdict[:120]}", flush=True)
            os.makedirs(os.path.dirname(failed_path), exist_ok=True)
            with open(failed_path, "a", encoding="utf-8", newline="\n") as f:
                f.write(key + "\n")
    print(f"twin-copy: {len(kept)} kept of {len(todo)} with a twin in C ({tried} tries, {skipped} "
          f"known failures skipped); now the full build")
    return 0


FUNC_NAME = re.compile(r'\b(func_[0-9A-F]{8}_\w+)\s*\(')
COMMENT = re.compile(r'/\*.*?\*/|//[^\n]*', re.S)


def _arg_class(text):
    """'F' for a float/double value, 'I' for anything else (ints, pointers, arrays)."""
    if "*" in text or "[" in text:
        return "I"
    return "F" if re.search(r'\b(f32|f64|float|double)\b', text) else "I"


def _signatures(path):
    """Every func_ prototype (';') and definition ('{') in a C file: (name, kind, line, ret, args)
    with args a string of 'I'/'F' per parameter, None for an old-style ()."""
    with open(path, encoding="utf-8") as f:
        src = COMMENT.sub(lambda m: re.sub(r'[^\n]', ' ', m.group(0)), f.read())
    out = []
    for m in FUNC_NAME.finditer(src):
        line_start = src.rfind("\n", 0, m.start()) + 1
        head = src[line_start:m.start()]
        if not re.match(r'^[A-Za-z_][\w\s\*]*$', head) or not head.strip():
            continue  # a call, not a declaration
        if re.match(r'^\s*(return|else|case)\b', head):
            continue
        depth, i = 1, m.end()
        while i < len(src) and depth:
            depth += {"(": 1, ")": -1}.get(src[i], 0)
            i += 1
        params = src[m.end():i - 1]
        rest = src[i:i + 40].lstrip()
        if not rest or rest[0] not in ";{":
            continue
        ret = "V" if head.split() == ["void"] else _arg_class(head)
        p = " ".join(params.split())
        if p in ("", ):
            args = None
        elif p == "void":
            args = ""
        else:
            parts, depth, cur = [], 0, ""
            for ch in p:
                if ch == "," and depth == 0:
                    parts.append(cur)
                    cur = ""
                    continue
                depth += {"(": 1, ")": -1}.get(ch, 0)
                cur += ch
            parts.append(cur)
            args = "".join("..." if x.strip() == "..." else _arg_class(x) for x in parts)
        out.append((m.group(1), "def" if rest[0] == "{" else "proto",
                    src.count("\n", 0, m.start()) + 1, ret, args))
    return out


def protos():
    """Every func_ prototype in src/*.c whose return class or parameter classes (int/float per
    position, and the count) differ from the function's C definition in src/."""
    sigs = []
    for path in sorted(glob.glob(os.path.join(build.ROOT, "src", "*.c"))):
        for s in _signatures(path):
            sigs.append((os.path.basename(path),) + s)
    defs = {}
    for c_file, name, kind, line, ret, args in sigs:
        if kind == "def":
            defs.setdefault(name, (c_file, line, ret, args))
    n = 0
    for c_file, name, kind, line, ret, args in sigs:
        if kind != "proto" or name not in defs or args is None:
            continue
        d_file, d_line, d_ret, d_args = defs[name]
        if d_args is None or "..." in (args + d_args):
            continue
        if (ret, args) != (d_ret, d_args):
            n += 1
            print(f"src/{c_file}:{line} {name}: {ret}({args or 'void'}) "
                  f"but src/{d_file}:{d_line} defines {d_ret}({d_args or 'void'})")
    print(f"{n} prototype(s) disagree with their definitions")
    return 0


def _decl_span(src, name, line):
    """(start, end) of the declaration or definition header of name that begins on line (1-based):
    from the line's start to its closing ')' (inclusive)."""
    start = 0
    for _ in range(line - 1):
        start = src.index("\n", start) + 1
    i = src.index(name + "(", start) + len(name) + 1
    depth = 1
    while depth:
        depth += {"(": 1, ")": -1}.get(src[i], 0)
        i += 1
    return start, i


def reprotos(c_file, func, quiet=False):
    """Each callee of a parked function whose prototype in its file disagrees with the callee's C
    definition (return type, or the parameters' count or classes), given the definition's own
    prototype in turn, compiled as `check` does. A callee declared void that returns a value
    changes the caller's registers (gauge.c func_800DE7FC, IDO_NOTES 217), and a parameter the
    callee does not take moves a constant (plmain.c func_8018A270, 260). The declaration also
    changes the file's other functions: the full build decides. A definition whose types the file
    does not know fails to compile ("fails"). quiet (the sweep): (base, results) or None."""
    path = os.path.join(build.ROOT, c_file)
    with open(path, encoding="utf-8") as f:
        text = f.read()
    if not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), text):
        if quiet:
            return None
        sys.exit(f"{func} is not parked in {c_file}")
    m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), text, re.M)
    if not m:
        if quiet:
            return None
        sys.exit(f"{func}: no definition in {c_file}")
    body = text[m.end():]
    body = body[:body.index("\n}") + 2]
    defs = {}
    for p in sorted(glob.glob(os.path.join(build.ROOT, "src", "*.c"))):
        for name, kind, line, ret, args in _signatures(p):
            if kind == "def":
                defs.setdefault(name, (p, line, ret, args))
    variants = []
    for name, kind, line, ret, args in _signatures(path):
        if kind != "proto" or name not in defs or name == func or not re.search(r"\b%s\b" % name, body):
            continue
        d_path, d_line, d_ret, d_args = defs[name]
        if (ret, args) == (d_ret, d_args) or d_args is None or "..." in ((args or "") + d_args):
            continue
        with open(d_path, encoding="utf-8") as f:
            d_src = f.read()
        ds, de = _decl_span(d_src, name, d_line)
        proto = " ".join(d_src[ds:de].split())
        ps, pe = _decl_span(text, name, line)
        variants.append((f"{name} as defined in {os.path.basename(d_path)}: '{proto[:70]}'",
                         text[:ps] + proto + text[pe:]))
    d = os.path.join(build.ROOT, "permuter", "reprotos", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)
    base = _variant_lines(d, func, want, text)
    base_len = VARIANT_LEN.get(d)
    if not quiet:
        print(f"{base}  as written; {len(variants)} prototypes to try", flush=True)
    results = []
    for where, variant in variants:
        c = _variant_lines(d, func, want, variant)
        if not quiet:
            print(f"{c if c is not None else 'fails':>5}  {where}", flush=True)
        if c is not None and base is not None and c < base:
            where += _len_note(d, want, base_len)
            results.append((c, where))
            VARIANT_SOURCES[where] = variant
    results.sort()
    if quiet:
        return base, results
    print(f"{func}: {base} as written; {len(variants)} prototypes tried; best:")
    for c, where in results[:8]:
        print(f"  {c:5d}  {where}")
    return 0


def _dprintf_variant(c_file, func):
    """A copy of c_file under permuter/ with func's PRINTF( calls as DPRINTF( (and a
    `#define DPRINTF` if the file has none); None if func's C has no PRINTF."""
    with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
        text = f.read()
    m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{' % re.escape(func), text, re.M)
    if not m:
        return None
    end = re.compile(r'^\}', re.M).search(text, m.end())
    body = text[m.start():end.end()]
    if "PRINTF(" not in body.replace("DPRINTF(", ""):
        return None
    body = re.sub(r'(?<![A-Z])PRINTF\(', 'DPRINTF(', body)
    text = text[:m.start()] + body + text[end.end():]
    if not re.search(r'^#define DPRINTF\b', text, re.M):
        last = list(re.finditer(r'^#include[^\n]*\n', text, re.M))[-1]
        text = text[:last.end()] + "#define DPRINTF\n" + text[last.end():]
    out = os.path.join("permuter", "dprintf_" + os.path.basename(c_file))
    with open(os.path.join(build.ROOT, out), "w", encoding="utf-8", newline="\n") as f:
        f.write(text)
    return out


def dprintf(c_file=None, func=None):
    """check func with its PRINTFs as the argument-only DPRINTF; with no arguments, every parked
    function that prints."""
    todo = [(c_file, func)] if func else parked_functions()
    for c, fn in todo:
        variant = _dprintf_variant(c, fn)
        if variant is None:
            continue
        r = subprocess.run([sys.executable, os.path.abspath(__file__), "check", variant, fn],
                           cwd=build.ROOT, capture_output=True, text=True, encoding="utf-8")
        verdicts = [l for l in r.stdout.splitlines() if l.startswith(fn + ":")]
        print(f"{c} {fn} as DPRINTF: {verdicts[-1].split(': ', 1)[1] if verdicts else 'FAILED'}",
              flush=True)
    return 0


def calls():
    """Every parked function whose game assembly makes more calls through a register (jalr)
    than its C makes through a pointer or a function-pointer field: a call the game made as
    `(*func)(...)` is written direct, so IDO emits a jal (and never hoists the address into an
    s-register). Report only; shim.c's func_801A06E8/func_801A0A44 matched that way."""
    pointer_call = re.compile(r'\(\s*\*\s*[\w.\->\[\]]+\s*\)\s*\(|(?:->|\.)\w+\s*\(')
    for c_file, func in parked_functions():
        with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
            text = f.read()
        m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{' % re.escape(func), text, re.M)
        asm = glob.glob(os.path.join(build.ROOT, "asm", "nonmatchings", "**", func + ".s"), recursive=True)
        if not m or len(asm) != 1:
            continue
        end = re.compile(r'^\}', re.M).search(text, m.end())
        in_c = len(pointer_call.findall(text[m.end():end.start()]))
        with open(asm[0], encoding="utf-8") as f:
            in_asm = sum(1 for l in f if re.search(r'\bjalr\s', l))
        if in_asm > in_c:
            print(f"{c_file} {func}: the game {in_asm} jalr, the C {in_c} pointer calls",
                  flush=True)
    return 0


DECL = re.compile(r"^\s+(?!return\b|if\b|for\b|while\b|switch\b|do\b|goto\b|case\b|break\b)"
                  r"(?:(?:const|volatile|unsigned|signed|struct|union|enum)\s+)*[A-Za-z_]\w*[\s*]+"
                  r"\w+(?:\s*\[[^\]]*\])*(?:\s*,\s*\**\s*\w+(?:\s*\[[^\]]*\])*)*\s*(?:=[^;]*)?;"
                  r"\s*(?:/\*.*\*/)?\s*$")


def _write_retry(path, text):
    """Write text to path; Windows can hold the last variant's files open for a moment (the
    splits sweep died twice on it), so retry for five seconds. False if it stays locked."""
    import time
    for attempt in range(5):
        try:
            with open(path, "w", newline="\n", encoding="utf-8") as f:
                f.write(text)
            return True
        except PermissionError:
            time.sleep(1)
    return False


VARIANT_SOURCES = {}  # a mode's find (its "where") -> the whole C file it compiled (for greedy)
VARIANT_LEN = {}  # d -> the instruction count of the last variant compiled there
VARIANT_OPS = {}  # d -> the opcode counts of the last variant compiled there
VARIANT_ROWS = {}  # d -> the last variant's disassembly (drops reads its stack map)


def _ops_note(d, want, base_ops, ops=("andi", "sll", "sra")):
    """" [+andi]" (etc.) when the last variant compiled in d has more of a conversion op than
    both the game and the C as written: a cast that added a mask or a sign extension."""
    from collections import Counter
    got = VARIANT_OPS.get(d)
    if got is None or base_ops is None:
        return ""
    cw = Counter(r.split()[0] for r in want)
    extra = [op for op in ops if got[op] > max(cw[op], base_ops[op])]
    return f" [+{'/'.join(extra)}]" if extra else ""


def _len_note(d, want, base_len):
    """" [length +k]" when the last variant compiled in d is further from the game's length
    than the C as written (base_len), else "": its lower count may be the diff aligning."""
    got = VARIANT_LEN.get(d)
    if got is None or base_len is None or abs(got - len(want)) <= abs(base_len - len(want)):
        return ""
    return f" [length {got - len(want):+d}]"


def _variant_lines(d, func, want, source):
    """A whole C file's variant, compiled as `check` compiles a parked function (IDO's cc -E with
    NON_MATCHING, GLOBAL_ASM lines dropped) in d: its differing lines against want, or None when
    it does not compile (or its files stay locked). Its length goes into VARIANT_LEN[d]."""
    import difflib
    cflags = [a for a in build.CFLAGS if a != "-c"]
    src = os.path.join(d, "variant.c")
    # the other parked functions out, as check and the build have them (import_function's rule)
    defines = re.compile(r"\b%s\s*\([^;{]*\)\s*\{" % re.escape(func))
    source = PARKED.sub(lambda m: m.group(0) if defines.search(m.group(1))
                        or not GLOBAL_ASM.search(m.group(2)) else m.group(2), source)
    if not _write_retry(src, GLOBAL_ASM.sub("", source)):
        return None
    r = subprocess.run([build.IDO, "-E", *cflags, "-DNON_MATCHING", fwd(src)], cwd=build.ROOT,
                       env=build.ENV, capture_output=True, text=True, encoding="utf-8")
    if r.returncode:
        return None
    full, obj = os.path.join(d, "variant_full.c"), os.path.join(d, "variant.o")
    if not _write_retry(full, r.stdout):
        return None
    try:
        if compile_one(full, obj):
            return None
    except PermissionError:
        return None
    got = disassemble(obj, func)
    VARIANT_LEN[d] = len(got)
    VARIANT_ROWS[d] = got
    from collections import Counter
    VARIANT_OPS[d] = Counter(r.split()[0] for r in got)
    count = sum(1 for l in difflib.unified_diff(want, got, n=0, lineterm="")
                if l[:1] in "+-" and not l.startswith(("+++", "---")))
    if COLLECT is not None:
        COLLECT.offer(count, _mix_distance(want, got), _stack_distance(want, got),
                      _tshift_size(want, got), source)
    return count


def _tshift_size(want, got):
    """How many t-registers this C's temporaries run on from (or behind) the game's, as screen's
    _tshift finds it (0 when none is found)."""
    ts = _tshift(want, got)
    return abs(ts[0]) if ts else 0


def _stack_distance(want, got):
    """How far this C's stack map is from the game's: the frame's difference in words plus the
    slots one touches and the other does not (0: the same frame and slots)."""
    (fw, sw), (fg, sg) = _stack_map(want), _stack_map(got)
    return abs((fw or 0) - (fg or 0)) // 4 + len(set(sw) ^ set(sg))


class _Collected:
    """greedy's escape candidates: while a round runs, every variant its modes compile is
    offered here. Kept: those closer to the game than where greedy is stuck, by three signals in
    turn: the instruction mix, then the stack map (most of the last functions are at mix 0: their
    gap is registers and slots), then the t-register shift (a temporary spent or not). A hill
    that is no closer by any of them is a plateau neighbour, and is not kept (scenario.c
    func_800E7278's three were a round each, for nothing). The best few by (mix distance, stack
    distance, t-shift, lines)."""

    def __init__(self, lines, mix, stack, tshift, keep=40):
        self.lines, self.mix, self.stack, self.tshift = lines, mix, stack, tshift
        self.keep, self.mode = keep, ""
        self.items = {}  # source -> (mix, stack, tshift, lines, mode)

    def offer(self, lines, mix, stack, tshift, source):
        closer = mix < self.mix or (mix == self.mix and (
            stack < self.stack or (stack == self.stack and tshift < self.tshift)))
        if closer and source not in self.items:
            self.items[source] = (mix, stack, tshift, lines, self.mode)
            if len(self.items) > 2 * self.keep:
                best = sorted(self.items.items(), key=lambda kv: kv[1][:4])[:self.keep]
                self.items = dict(best)

    def best(self):
        return sorted(((v[0], v[1], v[2], v[3], v[4], s) for s, v in self.items.items()),
                      key=lambda t: t[:4])


COLLECT = None  # a _Collected while greedy's round runs


def pads(c_file, func, quiet=False):
    """A parked function's declaration block, varied: a pad (one to four words) at each position,
    each existing pad removed, each adjacent pair of declarations swapped. Each variant is compiled
    as `check` compiles a parked function (cc -E with NON_MATCHING, the whole file) under
    permuter/pads/<func>/ and its differing lines counted; the best are listed. IDO gives locals
    their frame slots in declaration order (IDO_NOTES 196), so this finds a frame-only near miss
    by brute force. Report only: the source is left as it is.
    quiet (the sweep): return (base, the variants that beat it, sorted) or None."""
    with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
        text = f.read()
    if not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), text):
        if quiet:
            return None
        sys.exit(f"{func} is not parked in {c_file}")
    m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), text, re.M)
    if not m:
        if quiet:
            return None
        sys.exit(f"{func}: no definition in {c_file}")
    lines = text[m.end():].split("\n")
    n = 0
    while n < len(lines) and DECL.match(lines[n]):
        n += 1
    decls, rest = lines[:n], lines[n:]
    d = os.path.join(build.ROOT, "permuter", "pads", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)

    def count(block):
        return _variant_lines(d, func, want, text[:m.end()] + "\n".join(block + rest))

    indent = re.match(r"\s*", decls[0]).group(0) if decls else "    "
    variants = [("as written", decls)]
    for p in range(len(decls) + 1):
        where = f"before '{decls[p].strip()}'" if p < len(decls) else "at the end"
        for k in (1, 2, 3, 4):
            pad = f"{indent}s32 pad_x{'[%d]' % k if k > 1 else ''};"
            variants.append((f"{k}-word pad {where}", decls[:p] + [pad] + decls[p:]))
    for i, line in enumerate(decls):
        if re.match(r"\s*s32 pad\w*(\[\d+\])?;", line):
            variants.append((f"without '{line.strip()}'", decls[:i] + decls[i + 1:]))
    for i in range(len(decls) - 1):
        variants.append((f"'{decls[i + 1].strip()}' before '{decls[i].strip()}'",
                         decls[:i] + [decls[i + 1], decls[i]] + decls[i + 2:]))
    # each declaration moved to every other place (the adjacent ones are the swaps above):
    # gameproc.c func_800C7B10 went 58 to 52 on one swap
    for i in range(len(decls)):
        if not decls[i].strip():
            continue
        others = decls[:i] + decls[i + 1:]
        for q in range(len(others) + 1):
            if q in (i - 1, i, i + 1):
                continue
            where = f"before '{others[q].strip()}'" if q < len(others) else "to the end"
            variants.append((f"'{decls[i].strip()}' moved {where}", others[:q] + [decls[i]] + others[q:]))
    results = []
    for name, block in variants:
        c = count(block)
        if c is not None:
            results.append((c, name))
            VARIANT_SOURCES[name] = text[:m.end()] + "\n".join(block + rest)
            if not quiet:
                print(f"{c:5d}  {name}", flush=True)
    results.sort()
    if quiet:
        base = next((c for c, name in results if name == "as written"), None)
        return base, [(c, name) for c, name in results if base is not None and c < base]
    print(f"{func}: best of {len(results)} variants ({len(decls)} declarations):")
    for c, name in results[:8]:
        print(f"  {c:5d}  {name}")
    return 0


def splits_sweep():
    """`splits` over every parked function, one line each where a split beats the C as written
    (the base, the best and where), in permuter/splits.txt as well. Closest first by the last
    checkall (permuter/checkall.txt), so the long functions come last. Each function finished
    is appended to permuter/splits_done.txt; a restart skips those (delete it to start over)."""
    done_path = os.path.join(build.ROOT, "permuter", "splits_done.txt")
    done = {}
    if os.path.exists(done_path):
        with open(done_path, encoding="utf-8") as f:
            for line in f:
                line = line.rstrip("\r\n")
                if line.strip():
                    done[line.split("\t")[0].strip()] = line if "\t" in line else line + "\t"
    rank = {}
    table = os.path.join(build.ROOT, "permuter", "checkall.txt")
    if os.path.exists(table):
        with open(table, encoding="utf-8") as f:
            for i, line in enumerate(f):
                parts = line.split("\t")
                if len(parts) > 2:
                    rank[parts[2]] = i
    def huge(c_file, func):  # over 1500 instructions: hours of variants, so they come last
        path = os.path.join(build.ROOT, "asm", "nonmatchings", os.path.basename(c_file)[:-2],
                            func + ".s")
        if not os.path.exists(path):
            return False
        with open(path, encoding="utf-8") as f:
            return sum(1 for l in f if l.lstrip().startswith("/*")) > 1500

    order = sorted(parked_functions(), key=lambda p: (huge(*p), rank.get(p[1], len(rank))))
    for c_file, func in order:
        if func in done:
            continue
        r = splits(c_file, func, quiet=True)
        line = ""
        if r is not None:
            base, results = r
            if results and base is not None and results[0][0] < base:
                line = f"{base - results[0][0]:5d}  {c_file} {func}: {base} -> {results[0][0]}, {results[0][1]}"
                print(line, flush=True)
        done[func] = f"{func}\t{line}"
        with open(done_path, "a", encoding="utf-8") as f:
            f.write(done[func] + "\n")
    out = [v.split("\t", 1)[1] for v in done.values() if v.split("\t", 1)[1:] and v.split("\t", 1)[1]]
    with open(os.path.join(build.ROOT, "permuter", "splits.txt"), "w", encoding="utf-8") as f:
        f.write("\n".join(sorted(out, reverse=True)) + "\n")
    return 0


# the boundaries splits puts at each position: one, and a stack (polygon.c func_800D6990 matched
# on seven at one place, six were not enough, and no single one moved anything; IDO_NOTES 270)
SPLIT_STACKS = tuple(int(k) for k in os.environ.get("SPLIT_STACKS", "1,8").split(","))


def splits(c_file, func, quiet=False, greedy=False, pairs=False):
    """A parked function with one FAKE block boundary, `do {} while (0);`, put after each of its
    statements, case labels and block openings in turn (the declarations skipped), each compiled
    as `check` compiles a parked function under permuter/splits/<func>/; the best are listed.
    One boundary at a path's head can decide which of several equally used pointers get the
    saved registers (IDO_NOTES 228: zanzou.c func_800F9120, 669 to 194). Report only: the source
    is left as it is; a find goes in by hand with its FAKE comment, or as a dead print.
    quiet (the sweep): print nothing, return (base, results sorted) or None.
    greedy: keep the best boundary and sweep again until none helps; print each one kept, in
    order (line numbers count the boundaries already kept).
    pairs: every pair of positions, for small functions where no single boundary helps."""
    with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
        text = f.read()
    if not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), text):
        if quiet:
            return None
        sys.exit(f"{func} is not parked in {c_file}")
    defn = re.compile(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), re.M)
    if not defn.search(text):
        if quiet:
            return None
        sys.exit(f"{func}: no definition in {c_file}")
    d = os.path.join(build.ROOT, "permuter", "splits", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)
    verbose = not quiet and not greedy

    def scan(text):
        m = defn.search(text)
        lines = text[m.end():].split("\n")
        end = next(i for i, l in enumerate(lines) if l.startswith("}"))
        n = 0
        while n < end and (DECL.match(lines[n]) or not lines[n].strip()):
            n += 1
        base = _variant_lines(d, func, want, text)
        base_len = VARIANT_LEN.get(d)
        if verbose:
            print(f"{base}  as written", flush=True)
        results = []
        for j in range(n - 1, end):
            code = re.sub(r"/\*.*?\*/|//.*$", "", lines[j]).rstrip()
            if j >= n and not code.endswith((";", "{", ":")):
                continue
            nxt = lines[j + 1]
            indent = re.match(r"\s*", nxt).group(0) if nxt.strip() not in ("", "}") else \
                re.match(r"\s*", lines[j]).group(0) + "    "
            for k in SPLIT_STACKS:
                if k == 1:
                    fake = f"{indent}do {{}} while (0); /* FAKE: a block boundary (splits) */"
                else:  # a stack: lowers the priority of what is live only here (IDO_NOTES 270)
                    fake = f"{indent}{'do {} while (0); ' * k}/* FAKE: {k} block boundaries (splits) */"
                variant = lines[:j + 1] + [fake] + lines[j + 1:]
                source = text[:m.end()] + "\n".join(variant)
                c = _variant_lines(d, func, want, source)
                if c is not None:
                    where = f"line {j + 1}, after '{lines[j].strip()[:60]}'" if j >= n else \
                        "before the first statement"
                    where += (f" (x{k})" if k > 1 else "") + _len_note(d, want, base_len)
                    results.append((c, where, source))
                    VARIANT_SOURCES[where] = source
                    if verbose:
                        print(f"{c:5d}  {where}", flush=True)
        results.sort(key=lambda r: r[0])
        return base, results

    if pairs:
        m = defn.search(text)
        lines = text[m.end():].split("\n")
        end = next(i for i, l in enumerate(lines) if l.startswith("}"))
        n = 0
        while n < end and (DECL.match(lines[n]) or not lines[n].strip()):
            n += 1
        pos = [j for j in range(n - 1, end) if j < n or
               re.sub(r"/\*.*?\*/|//.*$", "", lines[j]).rstrip().endswith((";", "{", ":"))]
        base = _variant_lines(d, func, want, text)
        print(f"{base}  as written; {len(pos)} positions, {len(pos) * (len(pos) - 1) // 2} pairs",
              flush=True)
        found = []
        for a in range(len(pos)):
            for b in range(a + 1, len(pos)):
                variant = list(lines)
                for j in (pos[b], pos[a]):  # the later one first, so the earlier index holds
                    indent = re.match(r"\s*", lines[j + 1]).group(0) \
                        if lines[j + 1].strip() not in ("", "}") else "    "
                    variant.insert(j + 1, f"{indent}do {{}} while (0);")
                c = _variant_lines(d, func, want, text[:m.end()] + "\n".join(variant))
                if c is not None and base is not None and c < base:
                    where = f"lines {pos[a] + 1} and {pos[b] + 1}"
                    found.append((c, where))
                    print(f"{c:5d}  {where}", flush=True)
        found.sort()
        print(f"{func}: {base} as written; best pairs:")
        for c, where in found[:8]:
            print(f"  {c:5d}  {where}")
        return 0
    base, results = scan(text)
    if quiet:
        return base, [(c, where) for c, where, _ in results]
    if greedy:
        print(f"{base}  as written", flush=True)
        while results and base is not None and results[0][0] < base:
            base, where, text = results[0]
            print(f"{base:5d}  + {where}", flush=True)
            _, results = scan(text)
        print(f"{func}: greedy {base} (each '+' is one more FAKE boundary, kept in that order)")
        return 0
    print(f"{func}: {base} as written; best of {len(results)} splits:")
    for c, where, _ in results[:8]:
        print(f"  {c:5d}  {where}")
    return 0


CAST_TYPES = ("u8", "s8", "u16", "s16")
ASSIGN = re.compile(r"^(\s*)([A-Za-z_][^=;!<>+\-*/%&|^]*?)\s=\s([^=;][^;]*);(\s*(?:/\*.*\*/)?)$")


def casts(c_file, func, quiet=False):
    """IDO_NOTES 235 as a search: each one-line plain assignment ("x = expr;") of a parked
    function, with its right side cast to u8, s8, u16 and s16 in turn, compiled as `check`
    does. A conversion to the destination's own narrow type spends the temporary the game
    spends (zanzou.c func_800F7C6C and romdecode.c func_80002F40 matched that way). A cast can
    change what the C computes: keep a find only where the destination has that type.
    quiet (the sweep): return (base, results sorted) or None."""
    with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
        text = f.read()
    if not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), text):
        if quiet:
            return None
        sys.exit(f"{func} is not parked in {c_file}")
    m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), text, re.M)
    if not m:
        if quiet:
            return None
        sys.exit(f"{func}: no definition in {c_file}")
    lines = text[m.end():].split("\n")
    end = next(i for i, l in enumerate(lines) if l.startswith("}"))
    n = 0
    while n < end and (DECL.match(lines[n]) or not lines[n].strip()):
        n += 1
    d = os.path.join(build.ROOT, "permuter", "casts", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)
    base = _variant_lines(d, func, want, text)
    base_len = VARIANT_LEN.get(d)
    base_ops = VARIANT_OPS.get(d)
    if not quiet:
        print(f"{base}  as written", flush=True)
    results = []
    tried = 0
    for j in range(n, end):
        a = ASSIGN.match(lines[j])
        if not a or a.group(2).strip().startswith(("for ", "if ", "while ", "return")):
            continue
        tried += 1
        for t in CAST_TYPES:
            variant = list(lines)
            variant[j] = f"{a.group(1)}{a.group(2)} = ({t}) ({a.group(3)});{a.group(4)}"
            c = _variant_lines(d, func, want, text[:m.end()] + "\n".join(variant))
            if c is not None and base is not None and c < base:
                where = f"line {j + 1}: ({t}) in '{lines[j].strip()[:60]}'"
                where += _len_note(d, want, base_len) + _ops_note(d, want, base_ops)
                results.append((c, where))
                if not quiet:
                    print(f"{c:5d}  {where}", flush=True)
    results.sort()
    if quiet:
        return base, results
    print(f"{func}: {base} as written; {tried} assignments tried; best casts:")
    for c, where in results[:8]:
        print(f"  {c:5d}  {where}")
    return 0


SIMPLE = re.compile(r"^(\s*)(?!(?:if|for|while|switch|return|case|default|do|goto|break|continue)\b)"
                    r"[^{};]+;\s*(?:/\*.*\*/)?$")


def swaps(c_file, func, quiet=False):
    """Each pair of adjacent one-line statements at the same indent, swapped in turn, compiled as
    `check` does; the best listed, marked when the length or the conversion ops move away from
    the game's. Statement order decided zanzou.c func_800F7C6C, impact.c func_80185EDC, sndloc.c
    func_800D9CD4 and vram.c func_800E42B0. A swap can change what the C computes: read every
    find before keeping it. quiet (the sweep): return (base, results sorted) or None."""
    with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
        text = f.read()
    if not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), text):
        if quiet:
            return None
        sys.exit(f"{func} is not parked in {c_file}")
    m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), text, re.M)
    if not m:
        if quiet:
            return None
        sys.exit(f"{func}: no definition in {c_file}")
    lines = text[m.end():].split("\n")
    end = next(i for i, l in enumerate(lines) if l.startswith("}"))
    n = 0
    while n < end and (DECL.match(lines[n]) or not lines[n].strip()):
        n += 1
    d = os.path.join(build.ROOT, "permuter", "swaps", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)
    base = _variant_lines(d, func, want, text)
    base_len, base_ops = VARIANT_LEN.get(d), VARIANT_OPS.get(d)
    if not quiet:
        print(f"{base}  as written", flush=True)
    results = []
    tried = 0
    for j in range(n, end - 1):
        a, b = SIMPLE.match(lines[j]), SIMPLE.match(lines[j + 1])
        if not a or not b or a.group(1) != b.group(1) or DECL.match(lines[j]) or DECL.match(lines[j + 1]):
            continue
        if lines[j].strip() == lines[j + 1].strip():
            continue
        tried += 1
        variant = lines[:j] + [lines[j + 1], lines[j]] + lines[j + 2:]
        c = _variant_lines(d, func, want, text[:m.end()] + "\n".join(variant))
        if c is not None and base is not None and c < base:
            where = f"lines {j + 1}-{j + 2}: '{lines[j + 1].strip()[:40]}' before '{lines[j].strip()[:40]}'"
            where += _len_note(d, want, base_len) + _ops_note(d, want, base_ops)
            results.append((c, where))
            if not quiet:
                print(f"{c:5d}  {where}", flush=True)
    results.sort()
    if quiet:
        return base, results
    print(f"{func}: {base} as written; {tried} pairs tried; best swaps:")
    for c, where in results[:8]:
        print(f"  {c:5d}  {where}")
    return 0


def alias_uses(c_file, func, syms, undo=False):
    """alias-uses: each use of syms inside func renamed <sym>_uN (or put back with undo), the
    aliases declared after the symbol's own extern and pinned in undefined_syms.txt."""
    path = os.path.join(build.ROOT, c_file)
    with open(path, encoding="utf-8") as f:
        text = f.read()
    m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), text, re.M)
    if not m:
        sys.exit(f"{func}: no definition in {c_file}")
    end = text.index("\n}", m.end()) + 2
    body = text[m.end():end]
    us_path = os.path.join(build.ROOT, "undefined_syms.txt")
    with open(us_path, encoding="utf-8") as f:
        us = f.read()
    for sym in syms:
        sym, _, local = sym.partition(":")
        if local and not undo:  # the function static's own declaration goes; its uses get sym's name
            body = re.sub(r"^[ \t]*static\s+[^;\n]*\b%s\b[^;\n]*;[^\n]*\n" % re.escape(local), "", body, flags=re.M)
            body = re.sub(r"(?<![.>\w])%s\b" % re.escape(local), sym, body)
        if undo:
            body = re.sub(r"\b%s_u\d+\b" % re.escape(sym), sym, body)
            text = re.sub(r"^extern [^;\n]*\b%s_u\d+\b[^;\n]*;\n" % re.escape(sym), "", text, flags=re.M)
            us = re.sub(r"^%s_u\d+ = 0x[0-9A-F]+;\n" % re.escape(sym), "", us, flags=re.M)
            continue
        n = [0]

        def sub(mm):
            n[0] += 1
            return f"{sym}_u{n[0]}"
        body = re.sub(r"\b%s\b" % re.escape(sym), sub, body)
        dm = re.search(r"^extern\s+([\w ]+?)\s*\**\s*\b%s\b\s*(\[[^\]]*\])?\s*;[^\n]*\n" % re.escape(sym), text, re.M)
        if not dm:  # defined in this file: the definition's type, the aliases after it
            dm = re.search(r"^(?!extern|static)([A-Za-z_][\w ]*?)\s*\**\s*\b%s\b\s*(\[[^\]]*\])?\s*(?:=[^;]*)?;[^\n]*\n"
                           % re.escape(sym), text, re.M)
        addr = re.search(r"^%s\s*=\s*(0x[0-9A-Fa-f]+)" % re.escape(sym), us + open(
            os.path.join(build.ROOT, "undefined_syms_auto.txt"), encoding="utf-8").read(), re.M)
        if not addr:  # a D_XXXXXXXX name carries its own address
            addr = re.match(r"D_(?:0x)?([0-9A-Fa-f]{7,8})(?:_|$)", sym)
            addr = addr and re.match(r"(.*)", "0x" + addr.group(1))
        if not dm or not addr:
            sys.exit(f"alias-uses: {sym}: no extern in the file or no address in undefined_syms")
        decl = "".join(f"extern {dm.group(1)} {sym}_u{k}{dm.group(2) or ''};\n" for k in range(1, n[0] + 1))
        text = text[:dm.end()] + decl + text[dm.end():]
        m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), text, re.M)
        end = m.end() + len(text[m.end():].split("\n}", 1)[0]) + 2
        us += f"\n/* {func}'s uses of {sym}, one name each (match_function.py alias-uses) */\n" + "".join(
            f"{sym}_u{k} = 0x{int(addr.group(1), 16):08X};\n" for k in range(1, n[0] + 1))
        print(f"alias-uses: {sym}: {n[0]} uses")
    m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), text, re.M)
    end = m.end() + len(text[m.end():].split("\n}", 1)[0]) + 2
    text = text[:m.end()] + body + text[end:]
    _write_retry(path, text)
    _write_retry(us_path, re.sub(r"\n/\* \w+'s uses of \w+, one name each[^\n]*\*/\n(?=\n|$)", "\n", us))
    return 0


def _ret_type(name, texts):
    """The return type a prototype in texts gives name ("Obj *", "u32"), or None."""
    for t in texts:
        m = re.search(r"^\s*(?:extern\s+)?([A-Za-z_]\w*)\s*(\**)\s*\(?\s*\*?\s*%s\s*\)?\s*\(" % re.escape(name), t, re.M)
        if m and m.group(1) not in ("return", "if", "else", "typedef", "define"):
            return (m.group(1) + (" " + m.group(2) if m.group(2) else "")).strip()
    return None


def hoist(c_file, func, quiet=False, apply=True):
    """IDO_NOTES 358 as a search: each call taking one other call's result as an argument, with
    that result read into a new local first; kept (written to the file) when `real` drops."""
    path = os.path.join(build.ROOT, c_file)
    with open(path, encoding="utf-8") as f:
        text = f.read()
    if not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), text):
        return None
    d = os.path.join(build.ROOT, "permuter", "hoist", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)
    if _variant_lines(d, func, want, text) is None:
        return None
    base = best = _real_lines(want, VARIANT_ROWS[d])
    texts = [text] + [open(p, encoding="utf-8").read() for p in glob.glob(os.path.join(build.ROOT, "include", "*.h"))]
    call = re.compile(r"\b([A-Za-z_]\w*)\s*\(")
    kept, k = [], 0
    while True:
        m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), text, re.M)
        lines = text[m.end():].split("\n")
        end = next(i for i, l in enumerate(lines) if l.startswith("}"))
        n = 0
        while n < end and (DECL.match(lines[n]) or not lines[n].strip()):
            n += 1
        found = None
        j = n
        while j < end:
            # a statement may run over several lines (an if's condition): join until balanced
            span = [lines[j]]
            while span[-1].count("(") + sum(l.count("(") for l in span[:-1]) > \
                    sum(l.count(")") for l in span) and j + len(span) < end:
                span.append(lines[j + len(span)])
            stmt = " ".join(s.strip() for s in span)
            head = stmt.split("(", 1)[0].strip()
            if head in ("for", "while", "switch", "return", "case") or stmt.startswith(("#", "/")) or \
                    not (stmt.endswith(";") or (head == "if" and stmt.endswith("{"))):
                j += len(span)
                continue
            names = [c for c in call.finditer(stmt) if c.group(1) not in C_WORDS and c.group(1) != "sizeof"
                     and not re.match(r"[A-Z0-9_]+$", c.group(1))]
            if len(names) != 2:
                j += len(span)
                continue
            inner = names[1]
            depth, e = 0, inner.end() - 1
            for e in range(inner.end() - 1, len(stmt)):
                depth += {"(": 1, ")": -1}.get(stmt[e], 0)
                if depth == 0:
                    break
            expr = stmt[inner.start():e + 1]
            cast = re.search(r"\(\s*([A-Za-z_]\w*\s*\**)\s*\)\s*$", stmt[:inner.start()])
            typ = cast.group(1).strip() if cast else _ret_type(inner.group(1), texts)
            if cast:
                expr = stmt[cast.start():e + 1]
            if not typ or typ == "void" or expr in kept:
                j += len(span)
                continue
            found = (j, len(span), stmt, expr, typ)
            break
        if not found:
            break
        j, nspan, stmt, expr, typ = found
        k += 1
        name = f"hoist{k}"
        indent = re.match(r"\s*", lines[j]).group(0)
        body_new = [f"{indent}{name} = {expr};", indent + stmt.replace(expr, name, 1)]
        kept.append(expr)
        # two places for the new local: after the declarations, or in an `s32 pad;`'s word
        pads = [i for i in range(n) if re.match(r"^\s*s32 pad\w*;\s*$", lines[i])]
        variants = [lines[:n] + [f"    {typ} {name};"] + lines[n:j] + body_new + lines[j + nspan:]]
        for p in pads[:1]:
            variants.append(lines[:p] + [f"    {typ} {name};"] + lines[p + 1:j] + body_new + lines[j + nspan:])
        for vi, v in enumerate(variants):
            cand = text[:m.end()] + "\n".join(v)
            if _variant_lines(d, func, want, cand) is None:
                if not quiet:
                    print(f"{func}: {expr}: variant {vi} does not compile", flush=True)
                continue
            r = _real_lines(want, VARIANT_ROWS[d])
            if not quiet:
                print(f"{func}: {expr}: variant {vi} ({'pad word' if vi else 'after the declarations'}) real {r}", flush=True)
            if r < best:
                text, best = cand, r
                if not quiet:
                    print(f"{func}: {expr} into a local: real {r}", flush=True)
    if best < base and apply:
        _write_retry(path, text)
    if not quiet:
        print(f"{func}: real {base} -> {best}")
    return base, ([(best, f"hoisted ({best} real){' MATCH' if best == 0 else ''}")] if best < base else [])


def move_lines(c_file, first, last, before):
    """move-lines: lines first..last (1-based, inclusive) moved to stand before line `before`
    (numbered as before the move; `before` must lie outside the range)."""
    path = os.path.join(build.ROOT, c_file)
    with open(path, encoding="utf-8") as f:
        lines = f.read().split("\n")
    if not (1 <= first <= last <= len(lines)) or first <= before <= last + 1 or not 1 <= before <= len(lines) + 1:
        sys.exit(f"move-lines: bad range {first}-{last} before {before} ({len(lines)} lines)")
    block = lines[first - 1:last]
    if before < first:
        out = lines[:before - 1] + block + lines[before - 1:first - 1] + lines[last:]
    else:
        out = lines[:first - 1] + lines[last:before - 1] + block + lines[before - 1:]
    _write_retry(path, "\n".join(out))
    print(f"move-lines: {c_file}: {last - first + 1} lines from {first} to before {before}")
    return 0


def _macro_defs(text):
    """The function-like macros #defined in text: name -> (params, body lines). Macros using
    # or ## are left out."""
    defs = {}
    lines = text.split("\n")
    i = 0
    while i < len(lines):
        m = re.match(r"#define\s+([A-Za-z_]\w*)\(([^)]*)\)(.*)$", lines[i])
        if not m:
            i += 1
            continue
        body = [m.group(3)]
        while body[-1].rstrip().endswith("\\") and i + 1 < len(lines):
            body[-1] = body[-1].rstrip()[:-1]
            i += 1
            body.append(lines[i])
        i += 1
        body = [b.rstrip() for b in body if b.strip()]
        if body and "#" not in "".join(body):
            defs[m.group(1)] = ([p.strip() for p in m.group(2).split(",") if p.strip()], body)
    return defs


def _split_args(s):
    """A call's argument text split at its top-level commas."""
    out, depth, cur = [], 0, ""
    for ch in s:
        if ch in "([{":
            depth += 1
        elif ch in ")]}":
            depth -= 1
        if ch == "," and depth == 0:
            out.append(cur.strip())
            cur = ""
        else:
            cur += ch
    if cur.strip():
        out.append(cur.strip())
    return out


def macros(c_file, func, quiet=False, apply=True):
    """IDO_NOTES 12's macro addendum as a search: a macro use is one source line, so each use of
    a function-like macro standing as its own statement line in func is written out on the
    macro's own lines, one use at a time; kept (written to the file) when `real` drops."""
    path = os.path.join(build.ROOT, c_file)
    with open(path, encoding="utf-8") as f:
        text = f.read()
    if not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), text):
        return None
    defs = _macro_defs(text)
    if not defs:
        return None
    d = os.path.join(build.ROOT, "permuter", "macros", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)
    if _variant_lines(d, func, want, text) is None:
        return None
    base = best = _real_lines(want, VARIANT_ROWS[d])

    def body_of(t):
        m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), t, re.M)
        return m.end(), m.end() + len(t[m.end():].split("\n}", 1)[0])

    use = re.compile(r"^(\s*)(%s)\((.*)\);?\s*$" % "|".join(map(re.escape, defs)))
    s, e = body_of(text)
    tries, seen = [], {}
    for line in text[s:e].split("\n"):
        m = use.match(line)
        if not m or line.count("(") != line.count(")"):
            continue
        params, body = defs[m.group(2)]
        args = _split_args(m.group(3))
        if len(args) != len(params):
            continue
        occ = seen.get(line, 0)
        seen[line] = occ + 1
        lead = min(len(b) - len(b.lstrip()) for b in body)
        out = []
        for b in body:
            for p, a in zip(params, args):
                b = re.sub(r"\b%s\b" % re.escape(p), lambda _m, a=a: a, b)
            out.append(m.group(1) + b[lead:])
        if not out[-1].rstrip().endswith((";", "}")):
            out[-1] = out[-1].rstrip() + ";"  # the use's own semicolon
        tries.append((line, occ, "\n".join(out)))
    if not quiet:
        print(f"{func}: real {base}; {len(tries)} macro uses to write out", flush=True)
    for line, occ, repl in tries:
        s, e = body_of(text)
        body = text[s:e].split("\n")
        idx = [i for i, l in enumerate(body) if l == line]
        if occ >= len(idx):
            continue
        body[idx[occ]] = repl
        cand = text[:s] + "\n".join(body) + text[e:]
        if _variant_lines(d, func, want, cand) is None:
            if not quiet:
                print(f"{func}: {line.strip()} written out: does not compile", flush=True)
            continue
        r = _real_lines(want, VARIANT_ROWS[d])
        if not quiet:
            print(f"{func}: {line.strip()} written out: real {r}", flush=True)
        if r < best:
            best, text = r, cand
    if best < base and apply:
        _write_retry(path, text)
    if not quiet:
        print(f"{func}: real {base} -> {best}")
    return base, ([(best, f"macros ({best} real){' MATCH' if best == 0 else ''}")] if best < base else [])


def unpark(c_file, func):
    """unpark: once check says MATCH, the function's #else/GLOBAL_ASM/#endif and the
    #ifdef NON_MATCHING opening its block are dropped. The build proves the rest."""
    import contextlib
    import io
    with contextlib.redirect_stdout(io.StringIO()):
        r = check(c_file, func)
    if r != 0:
        sys.exit(f"unpark: {func} does not match (run check)")
    path = os.path.join(build.ROOT, c_file)
    with open(path, encoding="utf-8") as f:
        lines = f.read().split("\n")
    pragma = next((i for i, l in enumerate(lines)
                   if re.match(r'\s*#pragma GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), l)), None)
    els = pragma - 1 if pragma is not None else None
    # a static's file-scope stand-in may sit between the #else and the pragma
    while els is not None and els > 0 and re.match(r"[A-Za-z_][\w\s\*\[\]]*\s=?.*;\s*(/\*.*\*/)?\s*$", lines[els]):
        els -= 1
    if pragma is None or not lines[els].startswith("#else") or not lines[pragma + 1].startswith("#endif"):
        sys.exit(f"unpark: {func}: no '#else / GLOBAL_ASM / #endif' block in {c_file}")
    opening = None
    for i in range(els - 1, -1, -1):
        if lines[i].startswith("#endif"):
            break
        if lines[i].startswith("#ifdef NON_MATCHING"):
            opening = i
            break
    if opening is None:
        sys.exit(f"unpark: {func}: no '#ifdef NON_MATCHING' opens its block in {c_file}")
    out = lines[:opening] + lines[opening + 1:els] + lines[pragma + 2:]
    # prints whose strings came with this function's asm, guarded until it matched
    guards, i = 0, 0
    while i < len(out):
        if re.match(r"#ifdef NON_MATCHING\b.*\b%s\b" % re.escape(func), out[i]):
            j = i + 1
            while j < len(out) and re.match(r"\s*D?PRINTF\(", out[j]):
                j += 1
            if j > i + 1 and j < len(out) and out[j].startswith("#endif"):
                out = out[:i] + out[i + 1:j] + out[j + 1:]
                guards += 1
                continue
        i += 1
    _write_retry(path, "\n".join(out))
    print(f"unpark: {c_file}: {func} (lines {opening + 1} and {pragma}-{pragma + 2} dropped"
          + (f"; {guards} print guards" if guards else "") + ")")
    return 0


def spellings(c_file, func, quiet=False, apply=True):
    """IDO_NOTES 365 as a search: IDO tells real constants apart by their spelling, so equal
    literals spelled two ways are two constants. Each group of one type and value spelled more
    than one way in func's body is written all one way, each spelling in turn; kept (written
    to the file) when `real` drops."""
    import struct
    path = os.path.join(build.ROOT, c_file)
    with open(path, encoding="utf-8") as f:
        text = f.read()
    if not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), text):
        return None
    d = os.path.join(build.ROOT, "permuter", "spellings", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)
    if _variant_lines(d, func, want, text) is None:
        return None
    base = best = _real_lines(want, VARIANT_ROWS[d])
    num = r"(?:\d+\.\d*|\.\d+)(?:[eE][-+]?\d+)?[fF]?"
    macros = dict(re.findall(r"^#define\s+([A-Za-z_]\w*)\s+\(?\s*(%s)\s*\)?\s*$" % num, text, re.M))

    def key(tok):  # a spelling's (type, value)
        s = macros.get(tok, tok)
        v = float(s.rstrip("fF"))
        return ("f", struct.unpack(">f", struct.pack(">f", v))[0]) if s[-1] in "fF" else ("d", v)

    def body_of(t):
        m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), t, re.M)
        return m.end(), m.end() + len(t[m.end():].split("\n}", 1)[0])

    tok_re = re.compile(r"(?<![\w.])(%s|%s)(?![\w.])" % (num, "|".join(map(re.escape, macros)) or "(?!)"))
    s, e = body_of(text)
    groups = {}
    for tok in set(tok_re.findall(text[s:e])):
        groups.setdefault(key(tok), set()).add(tok)
    groups = [sorted(g) for g in groups.values() if len(g) > 1]
    if not quiet:
        print(f"{func}: real {base}; {len(groups)} constants spelled more than one way: {groups}", flush=True)
    for g in groups:
        for keep in g:
            s, e = body_of(text)
            others = re.compile(r"(?<![\w.])(%s)(?![\w.])" % "|".join(re.escape(x) for x in g if x != keep))
            cand = text[:s] + others.sub(keep, text[s:e]) + text[e:]
            if _variant_lines(d, func, want, cand) is None:
                continue
            r = _real_lines(want, VARIANT_ROWS[d])
            if not quiet:
                print(f"{func}: {g} all as {keep}: real {r}", flush=True)
            if r < best:
                best, text = r, cand
    if best < base and apply:
        _write_retry(path, text)
    if not quiet:
        print(f"{func}: real {base} -> {best}")
    return base, ([(best, f"spellings ({best} real){' MATCH' if best == 0 else ''}")] if best < base else [])


def results(c_file, func, quiet=False, apply=True):
    """IDO_NOTES 369 as a search: whether a call statement leaves its result in v0 decides
    whether v0 is free for the values live around it. Each bare call statement in func's body,
    one at a time: a value-returning callee's result tested by an empty if, a void callee called
    through a cast to an s32-returning type (its prototype's parameters kept). Kept (written to
    the file) when `real` drops."""
    path = os.path.join(build.ROOT, c_file)
    with open(path, encoding="utf-8") as f:
        text = f.read()
    if not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), text):
        return None
    d = os.path.join(build.ROOT, "permuter", "results", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)
    if _variant_lines(d, func, want, text) is None:
        return None
    base = best = _real_lines(want, VARIANT_ROWS[d])
    texts = [text] + [open(p, encoding="utf-8").read() for p in glob.glob(os.path.join(build.ROOT, "include", "*.h"))]

    def params(name):  # a void prototype's parameter list, or None
        for t in texts:
            m = re.search(r"^\s*(?:extern\s+)?void\s+%s\s*\(([^;{]*)\)\s*;" % re.escape(name), t, re.M)
            if m:
                return " ".join(m.group(1).split())
        return None

    def body_of(t):
        m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), t, re.M)
        return m.end(), m.end() + len(t[m.end():].split("\n}", 1)[0])

    stmt = re.compile(r"^(\s*)(\(\s*\*\s*)?([A-Za-z_]\w*)(\s*\))?\s*\((.*)\);\s*$", re.S)
    s, e = body_of(text)
    tries = []  # (statement's text, may span lines; occurrence in the body; replacement)
    seen = {}
    blines = text[s:e].split("\n")
    i = 0
    while i < len(blines):
        j = i  # a call statement may run over several lines: join until its parentheses close
        while j < len(blines) and "\n".join(blines[i:j + 1]).count("(") > "\n".join(blines[i:j + 1]).count(")"):
            j += 1
        span = "\n".join(blines[i:min(j, len(blines) - 1) + 1])
        nxt = j + 1
        m = stmt.match(span)
        if not m or m.group(3) in C_WORDS or m.group(3) == "sizeof" or span.count("(") != span.count(")"):
            i = nxt if j > i else i + 1
            continue
        i = nxt
        occ = seen.get(span, 0)
        seen[span] = occ + 1
        indent, name, args = m.group(1), m.group(3), " ".join(m.group(5).split())
        rt = _ret_type(name, texts)
        if rt and rt != "void":
            tries.append((span, occ, f"{indent}if ({span.strip()[:-1]}) {{\n{indent}}}", name))
        elif rt == "void" and params(name) is not None:
            tries.append((span, occ, f"{indent}/* FAKE: {name} as returning s32 (IDO_NOTES 369; match_function.py "
                                     f"results) */\n{indent}((s32 (*)({params(name)})) {name})({args});", name))
    # each void callee as int-returning for the whole function (#undef/#define around it)
    callees = []
    for span, occ, repl, name in tries:
        if "((s32 (*)(" in repl and name not in callees:
            callees.append(name)
    tries = [t[:3] for t in tries]
    for name in callees:
        tries.append((None, name, None))
    if not quiet:
        print(f"{func}: real {base}; {len(tries)} tries ({len(callees)} whole-function callees)", flush=True)
    for line, occ, repl in tries:
        if line is None:  # a whole-function override of callee `occ`
            name = occ
            far = any(re.search(r"^#define\s+%s\s+\(\*%s\)" % (re.escape(name), re.escape(name)), t, re.M)
                      for t in texts)
            m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), text, re.M)
            s, e = body_of(text)
            before = (f"/* FAKE: {name} as int-returning for this function (IDO_NOTES 369; match_function.py "
                      f"results), restored after it */\n#undef {name}\n"
                      f"#define {name} (*(s32 (*)({params(name)})) & {name})\n")
            after = f"\n#undef {name}" + (f"\n#define {name} (*{name})" if far else "")
            end = e + len("\n}")
            cand = text[:m.start()] + before + text[m.start():end] + after + text[end:]
            repl = f"{name} as int-returning for the function"
        else:
            s, e = body_of(text)
            body = "\n" + text[s:e] + "\n"
            hits = [h.start() for h in re.finditer(r"(?<=\n)" + re.escape(line) + r"(?=\n)", body)]
            if occ >= len(hits):
                continue
            body = body[:hits[occ]] + repl + body[hits[occ] + len(line):]
            cand = text[:s] + body[1:-1] + text[e:]
        if _variant_lines(d, func, want, cand) is None:
            if not quiet:
                print(f"{func}: {repl.strip()}: does not compile", flush=True)
            continue
        r = _real_lines(want, VARIANT_ROWS[d])
        if not quiet:
            print(f"{func}: {' '.join(repl.split())}: real {r}", flush=True)
        if r < best:
            best, text = r, cand
    if best < base and apply:
        _write_retry(path, text)
    if not quiet:
        print(f"{func}: real {base} -> {best}")
    return base, ([(best, f"results ({best} real){' MATCH' if best == 0 else ''}")] if best < base else [])


def probe(c_file, funcs, opt=None):
    """probe: a test file compiled as the build compiles, each function's instructions printed.
    opt: an -O level in place of the build's -O2."""
    obj = os.path.join(build.ROOT, "permuter", "probe", os.path.splitext(os.path.basename(c_file))[0] + ".o")
    os.makedirs(os.path.dirname(obj), exist_ok=True)
    if os.path.exists(obj):
        os.remove(obj)  # never read a stale object (an -O3 probe once printed the last run's)
    if opt:
        # straight to IDO: asm-processor accepts only -O0/-O1/-O2/-g, and a probe has no asm
        flags = [opt if f == "-O2" else f for f in build.CFLAGS]
        rc = subprocess.run([build.IDO, *flags, "-o", fwd(obj), fwd(os.path.join(build.ROOT, c_file))],
                            cwd=build.ROOT, env=build.ENV).returncode
    else:
        rc = compile_one(os.path.join(build.ROOT, c_file), obj)
    if rc != 0 or not os.path.exists(obj):
        return 1
    for func in funcs:
        rows = disassemble(obj, func)
        print(f"== {func}: {len(rows)} instructions")
        for i, r in enumerate(rows):
            print(f"{i:4d}  {r}")
    return 0


def _bss_layout(c_file, n):
    """bsswrap: c_file compiled all C behind n extra symbol numbers; its .bss layout as text
    lines ("global <name> <offset>", "<func> <offsets>"), or None when it does not compile."""
    wrap = os.path.join(build.ROOT, "permuter", "probe", "bsswrap.c")
    obj = os.path.splitext(wrap)[0] + ".o"
    os.makedirs(os.path.dirname(wrap), exist_ok=True)
    strs = "".join(f'    "bsswrap{i}",\n' for i in range(n))
    with open(wrap, "w", encoding="utf-8", newline="\n") as f:
        f.write(f"/* written by match_function.py bsswrap: {n} extra symbol numbers (IDO_NOTES 337) */\n"
                f"char *bsswrap_strs[] = {{\n{strs}    0\n}};\n\n"
                f'#define NON_MATCHING\n#include "{c_file}"\n')
    if os.path.exists(obj):
        os.remove(obj)
    if compile_one(wrap, obj) != 0 or not os.path.exists(obj):
        return None
    objdump = build.tool_path(os.path.join(build.BINUTILS, "mips64-elf-objdump" + build.EXE))
    out = []
    for line in subprocess.run([objdump, "-t", obj], capture_output=True, text=True).stdout.splitlines():
        p = line.split()
        if len(p) >= 6 and p[3] in (".bss", ".data") and p[2] == "O":
            kind = "global" if p[3] == ".bss" else "data"
            out.append((p[3], int(p[0], 16), f"{kind} {p[-1]} {int(p[0], 16)}"))
    out = [s for _, _, s in sorted(out)]
    funcs, fn, prev = {}, None, ""
    for line in subprocess.run([objdump, "-dr", obj], capture_output=True, text=True).stdout.splitlines():
        m = re.match(r"^[0-9a-f]+ <(\w+)>:", line)
        if m:
            fn = m.group(1)
        else:
            m = re.search(r"R_MIPS_LO16\s+\.(bss|data)$", line)
            if m and fn:
                off = re.split(r"[(]", prev.split("\t")[-1].split(",")[-1])[0].strip()
                funcs.setdefault((fn, m.group(1)), set()).add(int(off, 0))
        prev = line
    out += [f"{k}{'' if s == 'bss' else ' data'} {' '.join(map(str, sorted(v)))}"
            for (k, s), v in funcs.items()]
    return out


UOPT_ECVT = os.path.join(build.ROOT, "..", "vendor", "uopt-ecvt", "uopt.exe")


def trace(c_file, func):
    """trace: uopt's own live ranges for func (IDO_NOTES 283/285), from the build's compile."""
    d = os.path.join(build.ROOT, "permuter", func)
    import_function(c_file, func, d)
    t = os.path.join(d, "trace")
    os.makedirs(t, exist_ok=True)
    # only what trace and ugen-edit write: proof blocks kept beside them must survive
    mine = re.compile(r"^(full\..*|x\..*|col\.txt|pri\.txt|ranges\.txt|ugen_diff\.txt|tmpu|uoptlist)$")
    for name in os.listdir(t):
        if mine.match(name) and os.path.isfile(os.path.join(t, name)):  # cc -K also leaves a dir " "
            os.remove(os.path.join(t, name))
    cflags = [a for a in build.CFLAGS if a != "-c"]
    r = subprocess.run([build.IDO, "-K", "-S", *cflags, fwd(os.path.join(d, "full.c"))],
                       cwd=t, env=build.ENV, capture_output=True, text=True)
    if not os.path.exists(os.path.join(t, "full.B")):
        print(f"trace: cc -K -S left no full.B\n{r.stderr[-2000:]}")
        return 1
    for level, out in (("6", "col.txt"), ("5", "pri.txt")):
        if os.path.exists(os.path.join(t, "uoptlist")):
            os.remove(os.path.join(t, "uoptlist"))
        subprocess.run([fwd(UOPT_ECVT), "-G", "0", "-Olimit", "3000", f"-zdbug:{level}", "-mips2", "-EB",
                        "-g0", "-O2", "full.B", "full.x.O", "-t", "full.T", "tmpu"],
                       cwd=t, env=build.ENV, capture_output=True)
        if not os.path.exists(os.path.join(t, "uoptlist")):
            print(f"trace: uopt -zdbug:{level} wrote no uoptlist")
            return 1
        os.replace(os.path.join(t, "uoptlist"), os.path.join(t, out))
    # ranges.awk's table: each range's expression, from the redundant-store removal listing
    desc, inside = {}, False
    with open(os.path.join(t, "pri.txt"), errors="replace") as f:
        for line in f:
            if f"REMOVAL OF REDUNDANT STORES OF {func}" in line:
                inside = True
            elif f"CODE MOTION OF {func}" in line:
                inside = False
            elif inside:
                m = re.match(r"^\{ *\d+\|\d+\} +(\d+) (.*)$", line.rstrip("\n"))
                if m:
                    desc[m.group(1)] = re.sub(r"\s+", " ", m.group(2).replace("vreg", "")).strip()
    fregs = {24: "f0", 25: "f2", 26: "f12", 27: "f14", 28: "f16", 29: "f18", 30: "f20", 31: "f22"}

    def regname(r):
        try:
            n = int(r)
        except ValueError:
            return r
        return fregs.get(n, "--" if n <= 0 else f"r{n}")
    # pri.sh's reading: one row per live range of func
    rows, fn, cur = [], None, None
    with open(os.path.join(t, "pri.txt"), errors="replace") as f:
        for line in f:
            p = line.split()
            if "SECONDS IN LOCAL OPTIMIZATION OF" in line and p:
                fn = p[-1]
            elif ">>>active<<<" in line and len(p) >= 2:
                if cur and cur["fn"] == func:
                    rows.append(cur)
                cur = {"fn": fn, "lr": p[-2], "reg": p[-1], "save": "", "bbs": [], "nint": 0}
            elif cur and line.startswith("adjsave") and len(p) >= 3:
                cur["save"] = p[2]
            elif cur and "interfere with" in line:
                cur["nint"] = len(p) - 3
            elif cur and line.startswith("- live bb - ") and len(p) >= 8:
                cur["bbs"].append(f"{p[4]}:{p[5]}{p[6]}{p[7]}")
    if cur and cur["fn"] == func:
        rows.append(cur)

    def key(r):
        try:
            return -float(r["save"])
        except ValueError:
            return 0.0
    print(f"trace: {func}: {len(rows)} live ranges (range reg adjsave blocks interferes; "
          f"{os.path.relpath(t, build.ROOT)}/pri.txt, col.txt)")
    lines = []
    for r in sorted(rows, key=key):
        lines.append(f"  {r['lr']:>6} {regname(r['reg']):>4}  save {r['save']:>14}  bbs {len(r['bbs']):3}  "
                     f"int {r['nint']:3}  {desc.get(r['lr'], ''):40.40}  {' '.join(r['bbs'][:6])}")
    with open(os.path.join(t, "ranges.txt"), "w", newline="\n") as f:
        f.write("\n".join(lines) + "\n")
    print("\n".join(lines[:60]))
    return 0


UOPT_SPILL = os.path.join(build.ROOT, "..", "vendor", "uopt-spill", "uopt.exe")


def _uopt_spill(t, cfg_lines):
    """uopt-spill on trace's full.B -> full.f.O (spill.txt beside it); cfg_lines go to force.cfg."""
    for name in ("force.cfg", "spill.txt", "full.f.O", "full.f.G", "full.f.s"):
        if os.path.exists(os.path.join(t, name)):
            os.remove(os.path.join(t, name))
    if cfg_lines:
        with open(os.path.join(t, "force.cfg"), "w", newline="\n") as f:
            f.write("\n".join(cfg_lines) + "\n")
    subprocess.run([fwd(UOPT_SPILL), "-G", "0", "-Olimit", "3000", "-mips2", "-EB", "-g0", "-O2",
                    "full.B", "full.f.O", "-t", "full.T", "tmpu"], cwd=t, env=build.ENV, capture_output=True)
    return os.path.exists(os.path.join(t, "full.f.O"))


def force(func, pairs):
    """force: trace's full.B through uopt-spill with force.cfg pinning live ranges to registers,
    then the build's ugen and as1; check's real against the game."""
    t = os.path.join(build.ROOT, "permuter", func, "trace")
    if not all(os.path.exists(os.path.join(t, n)) for n in ("full.B", "full.T", "ranges.txt")):
        print(f"force: no full.B/full.T/ranges.txt in {os.path.relpath(t, build.ROOT)}: run trace first")
        return 1
    if pairs != ["none"] and len(pairs) % 2:
        print("force: give <range> <reg> pairs, or none")
        return 1
    if not _uopt_spill(t, None):
        print("force: uopt-spill wrote no full.f.O")
        return 1
    same = open(os.path.join(t, "full.f.O"), "rb").read() == open(os.path.join(t, "full.O"), "rb").read()
    if pairs != ["none"]:
        # the procedure: the PROC whose COLOR lines share the most (range, adjsave) with ranges.txt
        want = {}
        with open(os.path.join(t, "ranges.txt")) as f:
            for line in f:
                p = line.split()
                if len(p) >= 4 and p[2] == "save":
                    try:
                        want[p[0]] = float(p[3])
                    except ValueError:
                        pass
        score, proc = {}, 0
        with open(os.path.join(t, "spill.txt"), errors="replace") as f:
            for line in f:
                p = line.split()
                if line.startswith("PROC "):
                    proc = int(p[1]) + 1
                elif line.startswith("COLOR range ") and p[2] in want and abs(float(p[4]) - want[p[2]]) < 1e-6:
                    score[proc] = score.get(proc, 0) + 1
        if not score:
            print("force: no procedure's colouring matches ranges.txt")
            return 1
        proc = max(score, key=score.get)
        print(f"force: procedure {proc} ({score[proc]} of {len(want)} ranges agree)")
        if not _uopt_spill(t, [f"{proc} {pairs[i]} {pairs[i + 1]}" for i in range(0, len(pairs), 2)]):
            print("force: uopt-spill wrote no full.f.O with force.cfg (it died: see spill.txt's tail)")
            return 1
    ido = os.path.dirname(build.IDO)
    subprocess.run([fwd(os.path.join(ido, "ugen" + build.EXE)), "-G", "0", "-mips2", "-EB", "-g0", "-O2",
                    "full.f.O", "-o", "full.f.G", "-l", "full.f.s", "-t", "full.T", "-temp", "tmpf"],
                   cwd=t, env=build.ENV, capture_output=True)
    if not os.path.exists(os.path.join(t, "full.f.s")):
        print("force: ugen wrote no full.f.s")
        return 1
    if os.path.exists(os.path.join(t, "spill.txt")):
        with open(os.path.join(t, "spill.txt"), errors="replace") as f:
            for line in f:
                if line.startswith("FORCE"):
                    print("force: " + line.rstrip())
    print(f"force: plain uopt-spill output {'identical to' if same else 'DIFFERS from'} the compile's full.O")
    # ugen's binary .G straight to as1, as cc does (as0 re-reading the listing can refuse a file
    # whose global and function static share a name: tlb_0324_1's btaiDizzyHits)
    if os.path.exists(os.path.join(t, "full.f.o")):
        os.remove(os.path.join(t, "full.f.o"))
    r = subprocess.run([fwd(os.path.join(ido, "as1" + build.EXE)), "-t5_ll_sc_bug", "-elf", "-G", "0", "-p0",
                        "-r4300_mul", "-mips2", "-EB", "-g0", "-O2", "-Olimit", "3000", "full.f.G", "-o",
                        "full.f.o", "-t", "full.T"], cwd=t, env=build.ENV, capture_output=True, text=True)
    if not os.path.exists(os.path.join(t, "full.f.o")):
        print(f"force: as1 gave no object\n{r.stderr[-1500:]}")
        return 1
    d = os.path.dirname(t)
    want = disassemble(os.path.join(d, "target.o"), func)
    got = disassemble(os.path.join(t, "full.f.o"), func)
    _write_context_diff(want, got, os.path.join(t, "force_diff.txt"))
    print(f"force: {func}: real {_real_lines(want, got)}, length {len(got) - len(want):+d} "
          f"({os.path.relpath(t, build.ROOT)}/force_diff.txt)")
    return 0


def ugen_edit(func, a, z, block):
    """ugen-edit: trace's full.s with lines a-z replaced by block (a file; "none" for no change),
    assembled by as0 + as1 as the C path does; check's real against the game."""
    d = os.path.join(build.ROOT, "permuter", func)
    t = os.path.join(d, "trace")
    base = os.path.join(t, "full.s")
    if not os.path.exists(base):
        print(f"ugen-edit: no {os.path.relpath(base, build.ROOT)}: run trace first")
        return 1
    with open(base, encoding="latin-1") as f:
        lines = f.read().split("\n")
    if block != "none":
        with open(block, encoding="latin-1") as f:
            new = f.read().rstrip("\n").split("\n")
        lines = lines[:a - 1] + new + lines[z:]
    src = os.path.join(t, "x.s")
    with open(src, "w", encoding="latin-1", newline="\n") as f:
        f.write("\n".join(lines))
    out = os.path.join(os.path.dirname(build.IDO))
    for name in ("x.bin", "x.sym", "x.o"):
        if os.path.exists(os.path.join(t, name)):
            os.remove(os.path.join(t, name))
    subprocess.run([fwd(os.path.join(out, "as0" + build.EXE)), "-G", "0", "-r4300_mul", "-mips2", "-EB",
                    "-g0", "-O2", "x.s", "-o", "x.bin", "-t", "x.sym"],
                   cwd=t, env=build.ENV, capture_output=True)
    r = subprocess.run([fwd(os.path.join(out, "as1" + build.EXE)), "-t5_ll_sc_bug", "-elf", "-G", "0", "-p0",
                        "-r4300_mul", "-mips2", "-EB", "-g0", "-O2", "-Olimit", "3000", "x.bin", "-o", "x.o",
                        "-t", "x.sym"], cwd=t, env=build.ENV, capture_output=True, text=True)
    if not os.path.exists(os.path.join(t, "x.o")):
        print(f"ugen-edit: as0/as1 gave no object\n{r.stderr[-1500:]}")
        return 1
    want = disassemble(os.path.join(d, "target.o"), func)
    got = disassemble(os.path.join(t, "x.o"), func)
    _write_context_diff(want, got, os.path.join(t, "ugen_diff.txt"))
    real = _real_lines(want, got)
    print(f"ugen-edit: {func} ({block}): real {real}, length {len(got) - len(want):+d} "
          f"({os.path.relpath(t, build.ROOT)}/ugen_diff.txt)")
    return 0


def bsswrap(c_file, n):
    """bsswrap: the .bss layout of c_file behind n extra symbol numbers; with "all", the src/tlb
    files whose layout differs between 0 and n."""
    if c_file != "all":
        lay = _bss_layout(c_file, n)
        print("\n".join(lay) if lay is not None else f"{c_file}: does not compile")
        return 0 if lay is not None else 1
    for path in sorted(glob.glob(os.path.join(build.ROOT, "src", "tlb", "*.c"))):
        rel = os.path.relpath(path, build.ROOT).replace("\\", "/")
        a, b = _bss_layout(rel, 0), _bss_layout(rel, n)
        if a is None or b is None:
            print(f"{rel}: does not compile", flush=True)
        elif a != b:
            print(f"{rel}: CHANGES", flush=True)
            for x, y in zip(a, b):
                if x != y:
                    print(f"    {x}  ->  {y}", flush=True)
    return 0


def alias_merge(c_file, func, syms):
    """alias-merge: greedy merging of alias-uses' names, kept where the diff gets no worse."""
    path = os.path.join(build.ROOT, c_file)
    d = os.path.join(build.ROOT, "permuter", "aliasmerge", func)
    with open(path, encoding="utf-8") as f:
        text = f.read()
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)

    def measure(t):  # real lines, or None when the variant does not compile
        return None if _variant_lines(d, func, want, t) is None else _real_lines(want, VARIANT_ROWS[d])
    best = measure(text)
    print(f"alias-merge: {func}: real {best} as it is", flush=True)
    for sym in syms:
        m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), text, re.M)
        body = text[m.end():].split("\n}", 1)[0]
        order = list(dict.fromkeys(re.findall(r"\b%s_u\d+\b" % re.escape(sym), body)))
        for k in range(1, len(order)):
            cur, prev_name = order[k], None
            m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), text, re.M)
            body = text[m.end():].split("\n}", 1)[0]
            uses = re.findall(r"\b%s_u\d+\b" % re.escape(sym), body)
            i = uses.index(cur) if cur in uses else -1
            if i <= 0:
                continue
            prev_name = uses[i - 1]
            if prev_name == cur:
                continue
            new_body = re.sub(r"\b%s\b" % re.escape(cur), prev_name, body)
            cand = text[:m.end()] + new_body + text[m.end() + len(body):]
            c = measure(cand)
            if c is not None and best is not None and c <= best:
                text, best = cand, c
                _write_retry(path, text)
                print(f"alias-merge: {cur} -> {prev_name}: {c}", flush=True)
            else:
                print(f"alias-merge: {cur} kept ({c})", flush=True)
    print(f"alias-merge: {func}: real {best} at the end")
    return 0


def swaps_apply(sweep_path, dry=False, skip=()):
    """swaps-apply: each function's best swap from a swaps sweep's output, applied where its two
    lines still read as recorded and the diff, measured again, is lower. skip: current names whose
    swap was read and refused (it changes what the C computes)."""
    pat = re.compile(r"^\s+(\d+)\s+(src/\S+) (\w+): (\d+) -> (\d+), lines (\d+)-(\d+): '(.*?)' before '(.*?)'")
    best = {}
    with open(sweep_path, encoding="utf-8", errors="replace") as f:
        for line in f:
            m = pat.match(line)
            if m and (m.group(3) not in best or int(m.group(5)) < best[m.group(3)][1]):
                best[m.group(3)] = (m.group(2), int(m.group(5)), int(m.group(6)) - 1, m.group(8), m.group(9))
    renamed = {}
    for p in glob.glob(os.path.join(build.ROOT, "permuter", "names_*.txt")) + \
            glob.glob(os.path.join(build.ROOT, "permuter", "rename_*.txt")):
        with open(p, encoding="utf-8") as f:
            for l in f:
                parts = l.split()
                if len(parts) == 2:
                    renamed[parts[0]] = parts[1]
    applied = 0
    for func, (c_file, score, j, first, second) in sorted(best.items(), key=lambda kv: kv[1][1]):
        name = renamed.get(func, func)
        if name in skip or func in skip:
            print(f"swaps-apply: {name}: refused on review; skipped")
            continue
        with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
            text = f.read()
        m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(name), text, re.M)
        if not m or not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(name), text):
            print(f"swaps-apply: {name}: not parked in {c_file} (landed or moved); skipped")
            continue
        lines = text[m.end():].split("\n")
        if j + 1 >= len(lines) or lines[j].strip()[:40] != second or lines[j + 1].strip()[:40] != first:
            print(f"swaps-apply: {name}: lines {j + 1}-{j + 2} no longer read as recorded; skipped")
            continue

        def has_call(s):
            return re.search(r"\b(?!if\b|while\b|for\b|switch\b|return\b|sizeof\b)\w+\s*\(", re.sub(r"\(\s*[\w ]+\**\s*\)", "", s))

        def local_set(s):  # "x = ...;" or "x++;" to a plain local of the function
            mm = re.match(r"\s*(\w+)\s*(?:[-+*/|&^]?=|\+\+|--)", s)
            return mm and re.search(r"^\s+[\w ]+\**\s*\b%s\b\s*(?:=[^;]*)?;" % re.escape(mm.group(1)),
                                    "\n".join(lines[:j]), re.M)
        if (has_call(lines[j]) and not local_set(lines[j + 1])) or (has_call(lines[j + 1]) and not local_set(lines[j])):
            print(f"swaps-apply: {name}: moves a call past a store or call (the jal order decides); skipped")
            continue
        d = os.path.join(build.ROOT, "permuter", "swaps", name)
        import_function(c_file, name, d)
        want = disassemble(os.path.join(d, "target.o"), name)
        base = _variant_lines(d, name, want, text)
        new_text = text[:m.end()] + "\n".join(lines[:j] + [lines[j + 1], lines[j]] + lines[j + 2:])
        c = _variant_lines(d, name, want, new_text)
        if c is None or base is None or c >= base:
            print(f"swaps-apply: {name}: {base} -> {c}, not lower; skipped")
            continue
        if not dry:
            _write_retry(os.path.join(build.ROOT, c_file), new_text)
        applied += 1
        print(f"swaps-apply: {name} ({c_file}): {base} -> {c}{' (dry)' if dry else ''}\n"
              f"    now: {lines[j + 1].strip()}\n    then: {lines[j].strip()}")
    print(f"swaps-apply: {applied} applied")
    return 0


C_WORDS = {"if", "else", "for", "while", "do", "switch", "case", "default", "return", "break",
           "continue", "goto", "sizeof", "struct", "union", "enum", "const", "volatile",
           "unsigned", "signed", "register", "static", "extern", "void", "int", "char", "short",
           "long", "float", "double", "NULL", "TRUE", "FALSE"}
# a cast: a scalar type or a capitalised typedef (Vec3f, Gfx), never a parenthesised local
CAST = re.compile(r"\(\s*(?:(?:const|volatile|unsigned|signed|struct|union|enum)\s+)*"
                  r"(?:[us](?:8|16|32|64)|f32|f64|int|char|short|long|float|double|void|[A-Z]\w*)"
                  r"(?:\s*\*)*\s*\)")


def _effects(stmt, local_names, mem_names):
    """For moves: a one-line statement's (writes, reads, memory written, memory read). writes and
    reads are local names; memory is anything else: a field, an index, a dereference, a global,
    a local whose address is taken or that is volatile (mem_names), and a call or a macro (which
    may store anywhere). Read off the text, conservatively: a doubt counts as memory."""
    code = re.sub(r'/\*.*?\*/|//.*$|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', " ", stmt).strip()
    if re.match(r"PRINTF\s*\(", code):
        # "if (0) osSyncPrintf": dead, its arguments never evaluated, so it moves past anything;
        # where a dead print sits decides blocks and registers (scenario.c func_800E7B34
        # matched on its print moved after a call, IDO_NOTES 271)
        return set(), set(), False, False
    code = re.sub(r"\bsizeof\s*\([^()]*\)", " 0 ", CAST.sub(" ", code))
    call = re.search(r"[\w)\]]\s*\(", code) is not None
    # the assigned targets: "a = b = c" chains, compound assignments, ++/--
    targets = [t.strip() for t in re.findall(r"([^=!<>]+?)(?:[-+*/%&|^]|<<|>>)?=(?!=)", code)]
    incdec = re.findall(r"(\w+)\s*(?:\+\+|--)|(?:\+\+|--)\s*(\w+)", code)
    writes, mem_w = set(), call
    for t in targets + [a or b for a, b in incdec]:
        t = t.split("=")[-1].strip()
        if re.fullmatch(r"[A-Za-z_]\w*", t) and t in local_names and t not in mem_names:
            writes.add(t)
        else:
            mem_w = True
    names = set(re.findall(r"(?<![\w.])(?<!->)[A-Za-z_]\w*(?!\s*\()", code)) - C_WORDS
    reads, mem_r = set(), call or bool(re.search(r"->|\[|(?:^|[=(,+\-*/&|^!~?:<>]\s*)\*", code))
    for name in names:
        if name in mem_names:
            mem_r = True
        elif name in local_names:
            reads.add(name)
        elif not re.fullmatch(r"[A-Z][A-Z0-9_]*", name):  # an ALL_CAPS name is a constant
            mem_r = True
    if "." in re.sub(r"\d\.\d|\d\.(?=f)|\.\d", "", code):
        mem_r = True  # a local struct's field: its memory
    return writes, reads, mem_w, mem_r


def _independent(a, b):
    """Whether two statements' effects (see _effects) allow swapping them."""
    wa, ra, mwa, mra = a
    wb, rb, mwb, mrb = b
    if wa & (rb | wb) or wb & ra:
        return False
    return not (mwa and (mrb or mwb)) and not (mwb and mra)


MOVES_MAX = 6  # the farthest a statement is moved, in lines


def moves(c_file, func, quiet=False):
    """Statement placement as a search, keeping what the C computes: each one-line statement
    moved up or down, one to MOVES_MAX lines, past one-line statements at its indent that it
    commutes with (_effects: the locals each writes against those the other reads or writes, and
    memory, calls and macros written by one against memory the other touches). Where a value is
    computed decides its registers and whether its multiply is in place (gauge.c func_800DD314,
    302 to 124: x computed after the inner switch and between the sync and the load; plmain.c
    func_8018A5F8: the flags2 store before flags). quiet (the sweep): (base, results) or None."""
    with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
        text = f.read()
    if not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), text):
        if quiet:
            return None
        sys.exit(f"{func} is not parked in {c_file}")
    m = re.search(r'^[A-Za-z][^\n;]*\b%s\(([^;{]*)\)\s*\{\n' % re.escape(func), text, re.M)
    if not m:
        if quiet:
            return None
        sys.exit(f"{func}: no definition in {c_file}")
    lines = text[m.end():].split("\n")
    end = next(i for i, l in enumerate(lines) if l.startswith("}"))
    n = 0
    while n < end and (DECL.match(lines[n]) or not lines[n].strip()):
        n += 1
    # the function's own names: its parameters and every declared local (arrays and structs
    # too, whose element and field accesses _effects already counts as memory)
    local_names = set()
    for p in m.group(1).split(","):
        pm = re.search(r"([A-Za-z_]\w*)\s*(?:\[[^\]]*\])*\s*$", p.strip())
        if pm and p.strip() not in ("void", "..."):
            local_names.add(pm.group(1))
    mem_names = set()
    for j in range(end):  # block-scoped declarations too (sys.c func_80004880's fade block)
        if j >= n and not DECL.match(lines[j]):
            continue
        decl = re.sub(r"/\*.*?\*/", "", lines[j]).split("=")[0]
        for dm in re.finditer(r"([A-Za-z_]\w*)\s*(?:\[[^\]]*\])*\s*(?:,|;|$)", decl):
            local_names.add(dm.group(1))
            if "volatile" in decl or "[" in decl:
                mem_names.add(dm.group(1))
    body = "\n".join(lines[n:end])
    mem_names |= {a for a in re.findall(r"&\s*([A-Za-z_]\w*)", body) if a in local_names}
    d = os.path.join(build.ROOT, "permuter", "moves", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)
    base = _variant_lines(d, func, want, text)
    base_len = VARIANT_LEN.get(d)
    if not quiet:
        print(f"{base}  as written", flush=True)

    def movable(j):
        s = SIMPLE.match(lines[j])
        if s and not DECL.match(lines[j]) and not re.match(r"\s*\w+\s*:(?!:)", lines[j]) and \
                lines[j].count(";") == 1:
            return s
        return None

    variants = []
    for j in range(n, end):
        s = movable(j)
        if not s:
            continue
        mine = _effects(lines[j], local_names, mem_names)
        for step in (-1, 1):
            k = j
            for dist in range(1, MOVES_MAX + 1):
                k += step
                if not (n <= k < end):
                    break
                o = movable(k)
                if not o or o.group(1) != s.group(1) or lines[k].strip() == lines[j].strip() or \
                        not _independent(mine, _effects(lines[k], local_names, mem_names)):
                    break
                rest = lines[:j] + lines[j + 1:]  # line k is at k (above j) or k - 1 (below)
                variant = rest[:k] + [lines[j]] + rest[k:]
                variants.append((f"line {j + 1} moved {'up' if step < 0 else 'down'} {dist}: "
                                 f"'{lines[j].strip()[:40]}' {'before' if step < 0 else 'after'} "
                                 f"'{lines[k].strip()[:40]}'", variant))
    results = []
    for where, variant in variants:
        c = _variant_lines(d, func, want, text[:m.end()] + "\n".join(variant))
        if not quiet:  # every move, so the commute test can be read
            print(f"{'-' if c is None else c:>5}  {where}", flush=True)
        if c is not None and base is not None and c < base:
            where += _len_note(d, want, base_len)
            results.append((c, where))
            VARIANT_SOURCES[where] = text[:m.end()] + "\n".join(variant)
    results.sort()
    if quiet:
        return base, results
    print(f"{func}: {base} as written; {len(variants)} moves tried; best:")
    for c, where in results[:8]:
        print(f"  {c:5d}  {where}")
    return 0


def joins(c_file, func, quiet=False):
    """Same-line statements as a search: each pair of adjacent one-line statements at the same
    indent joined onto one line, and each block of one statement closed onto its opening line
    ("for (j = 0; j < 3; j++) { p->queue[j] = -1; }"), compiled as `check` does. IDO schedules by
    source line, so two statements on one line can interleave as the game's do (plmain.c
    func_8018A5F8: 190 to 176; itex.c's colours, effect.c's alpha clamp). The C computes the
    same. quiet (the sweep): return (base, results sorted) or None."""
    with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
        text = f.read()
    if not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), text):
        if quiet:
            return None
        sys.exit(f"{func} is not parked in {c_file}")
    m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), text, re.M)
    if not m:
        if quiet:
            return None
        sys.exit(f"{func}: no definition in {c_file}")
    lines = text[m.end():].split("\n")
    end = next(i for i, l in enumerate(lines) if l.startswith("}"))
    n = 0
    while n < end and (DECL.match(lines[n]) or not lines[n].strip()):
        n += 1
    d = os.path.join(build.ROOT, "permuter", "joins", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)
    base = _variant_lines(d, func, want, text)
    base_len = VARIANT_LEN.get(d)
    if not quiet:
        print(f"{base}  as written", flush=True)
    variants = []
    for j in range(n, end - 1):
        a, b = SIMPLE.match(lines[j]), SIMPLE.match(lines[j + 1])
        if a and b and a.group(1) == b.group(1) and not DECL.match(lines[j + 1]):
            variants.append((f"lines {j + 1}-{j + 2} on one line: '{lines[j].strip()[:40]}' '{lines[j + 1].strip()[:40]}'",
                             lines[:j] + [lines[j].rstrip() + " " + lines[j + 1].strip()] + lines[j + 2:]))
    for j in range(n, end - 2):
        if lines[j].rstrip().endswith("{") and SIMPLE.match(lines[j + 1]) and lines[j + 2].strip() == "}" and \
                re.match(r"\s*", lines[j + 2]).group(0) == re.match(r"\s*", lines[j]).group(0):
            variants.append((f"line {j + 1}'s block on one line: '{lines[j].strip()[:40]}'",
                             lines[:j] + [f"{lines[j].rstrip()} {lines[j + 1].strip()} }}"] + lines[j + 3:]))
    results = []
    for where, variant in variants:
        c = _variant_lines(d, func, want, text[:m.end()] + "\n".join(variant))
        if c is not None and base is not None and c < base:
            where += _len_note(d, want, base_len)
            results.append((c, where))
            VARIANT_SOURCES[where] = text[:m.end()] + "\n".join(variant)
            if not quiet:
                print(f"{c:5d}  {where}", flush=True)
    results.sort()
    if quiet:
        return base, results
    print(f"{func}: {base} as written; {len(variants)} joins tried; best:")
    for c, where in results[:8]:
        print(f"  {c:5d}  {where}")
    return 0


FLOW = re.compile(r"\b(?:if|for|while|switch|do|goto|case|default|break|continue|return)\b|[{}]|^\s*\w+\s*:(?!:)")


def inlines(c_file, func, quiet=False):
    """A named temporary as a search, the other way: each straight-line definition of a local
    ("v = expr;" on its own line, expr with no call and no assignment) substituted, in
    parentheses, into its uses up to v's next definition, the definition dropped (and the
    declaration, when nothing else uses v). Only where nothing between the definition and its
    last use branches, jumps, calls (if expr reads memory) or assigns one of expr's operands, so
    the C computes the same. IDO puts its compiler temps below every declared scalar: floorhit.c
    func_80196024 got the game's frame with only two named (53 to 19). quiet (the sweep): return
    (base, results sorted) or None."""
    with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
        text = f.read()
    if not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), text):
        if quiet:
            return None
        sys.exit(f"{func} is not parked in {c_file}")
    m = re.search(r'^[A-Za-z][^\n;]*\b%s\(([^;{]*)\)\s*\{\n' % re.escape(func), text, re.M)
    if not m:
        if quiet:
            return None
        sys.exit(f"{func}: no definition in {c_file}")
    lines = text[m.end():].split("\n")
    end = next(i for i, l in enumerate(lines) if l.startswith("}"))
    n = 0
    while n < end and (DECL.match(lines[n]) or not lines[n].strip()):
        n += 1
    local_decl = {}
    for j in range(n):
        dm = re.match(r"^\s*(?:register\s+)?(?:unsigned\s+|signed\s+)?[A-Za-z_]\w*\s*\**\s*([A-Za-z_]\w*)\s*;", lines[j])
        if dm and "," not in lines[j] and "[" not in lines[j]:
            local_decl[dm.group(1)] = j
    d = os.path.join(build.ROOT, "permuter", "inlines", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)
    base = _variant_lines(d, func, want, text)
    base_len = VARIANT_LEN.get(d)
    if not quiet:
        print(f"{base}  as written; locals: {', '.join(local_decl)}", flush=True)

    def ref(name):
        return re.compile(r"(?<!->)(?<!\.)(?<![\w])%s(?!\w)" % re.escape(name))

    def setter(name):
        return re.compile(r"(?<!->)(?<!\.)(?<![\w])%s\s*(?:[-+*/&|^%%]|<<|>>)?=(?!=)|(?:\+\+|--)\s*%s(?!\w)|"
                          r"(?<![\w])%s\s*(?:\+\+|--)|&\s*%s(?!\w)" % ((re.escape(name),) * 4))

    results = []
    tried = 0
    for v, decl_line in local_decl.items():
        use, sets = ref(v), setter(v)
        for k in range(n, end):
            dm = re.match(r"^(\s*)%s\s*=\s*([^=;][^;]*);\s*(?:/\*.*\*/)?$" % re.escape(v), lines[k])
            if not dm:
                continue
            expr = dm.group(2).strip()
            if re.search(r"\w\s*\(", expr) or re.search(r"(?<![=!<>])=(?!=)|\+\+|--", expr) or use.search(expr):
                continue
            nxt = next((j for j in range(k + 1, end) if sets.search(lines[j])), end)
            if nxt < end and not re.match(r"^\s*%s\s*=(?!=)" % re.escape(v), lines[nxt]):
                continue  # the next is "v += ...", "v++" or "&v": it reads this value too
            uses = [j for j in range(k + 1, min(nxt + 1, end)) if use.search(lines[j])]
            if nxt < end and nxt in uses:
                # the next definition reads v on its right: it is a use of this one
                rhs = lines[nxt].split("=", 1)[1] if "=" in lines[nxt] else ""
                if not use.search(rhs):
                    uses.remove(nxt)
            if not uses:
                continue
            last = uses[-1]
            between = range(k + 1, last)
            if any(FLOW.search(re.sub(r'"(?:\\.|[^"\\])*"', "", lines[j])) for j in between):
                continue
            if FLOW.search(lines[last]) and not re.match(r"\s*(?:if|return)\b", lines[last]):
                continue
            operands = set(re.findall(r"(?<!->)(?<!\.)(?<![\w])([A-Za-z_]\w*)", expr)) - {"sizeof"}
            if any(setter(o).search(lines[j]) for o in operands for j in range(k + 1, last + 1)):
                continue
            if re.search(r"->|\[|\*\s*[A-Za-z_(]", expr) and any(
                    re.search(r"\w\s*\(", lines[j]) or re.search(r"(?:->|\]|\*\w+)\s*(?:[-+*/&|^]|<<|>>)?=(?!=)", lines[j])
                    for j in between):
                continue
            variant = list(lines)
            for j in uses:
                if j == nxt:
                    lhs, rhs = variant[j].split("=", 1)
                    variant[j] = lhs + "=" + use.sub(f"({expr})", rhs)
                else:
                    variant[j] = use.sub(f"({expr})", variant[j])
            variant[k] = None
            if not any(use.search(l) for i, l in enumerate(variant[n:end], n) if l is not None and i != k):
                variant[decl_line] = None
            tried += 1
            c = _variant_lines(d, func, want,
                               text[:m.end()] + "\n".join(l for l in variant if l is not None))
            if c is not None and base is not None and c < base:
                where = f"{v} = {expr[:40]} (line {k + 1}) into lines {', '.join(str(u + 1) for u in uses)}"
                where += _len_note(d, want, base_len)
                results.append((c, where))
                VARIANT_SOURCES[where] = text[:m.end()] + "\n".join(l for l in variant if l is not None)
                if not quiet:
                    print(f"{c:5d}  {where}", flush=True)
    results.sort()
    if quiet:
        return base, results
    print(f"{func}: {base} as written; {tried} inlines tried; best:")
    for c, where in results[:8]:
        print(f"  {c:5d}  {where}")
    return 0


def _body_scan(m, text):
    """For copies and empties: a parked function's body (m: its definition's match, the
    parameter list in group 1) as (lines, end, n, variables, first_set, starts, straddled):
    the body's lines, the closing brace's index, the first statement's, every parameter and
    pointer/scalar local as (type, name), the line each is first set on (-1 for a parameter),
    the top-level statement starts, and the starts a goto and its label lie on both sides of."""
    params = []
    for p in m.group(1).split(","):
        if "(" in p or "[" in p or p.strip() in ("void", "..."):
            continue
        pm = PARAM.match(p.strip())
        if pm:
            params.append((re.sub(r"\s*\*\s*", " *", pm.group(1).strip()), pm.group(2)))
    lines = text[m.end():].split("\n")
    end = next(i for i, l in enumerate(lines) if l.startswith("}"))
    n = 0
    while n < end and (DECL.match(lines[n]) or not lines[n].strip()):
        n += 1
    # the locals too: pointers and plain scalars, one declarator to a line, each from the line
    # that first sets it (a parameter is set from the start)
    first_set = {name: -1 for _, name in params}
    for j in range(n):
        dm = re.match(r"^\s*(?:register\s+)?((?:unsigned\s+|signed\s+)?[A-Za-z_]\w*\s*\**)\s*"
                      r"([A-Za-z_]\w*)\s*(=[^;]*)?;", lines[j])
        if not dm or "," in lines[j].split("=")[0] or "[" in lines[j].split("=")[0]:
            continue
        ltype = re.sub(r"\s*\*\s*", " *", dm.group(1).strip())
        if "*" not in ltype and ltype.split()[-1] not in SCALARS:
            continue
        params.append((ltype, dm.group(2)))
        if dm.group(3):
            first_set[dm.group(2)] = j
        else:
            setter = re.compile(r"(?<!->)(?<!\.)(?<![\w])%s\s*(?:[-+*/&|^]?=(?!=)|\+\+|--)|(?:\+\+|--)%s(?!\w)"
                                % (re.escape(dm.group(2)), re.escape(dm.group(2))))
            first_set[dm.group(2)] = next((k for k in range(n, end) if setter.search(lines[k])), end)
    # top-level statement starts: at the body's own brace depth (counted, not read off the
    # indent: geography.c func_800E0C44's body sits at 12 spaces after its switch), the previous
    # code line having ended a statement or a block
    starts = []
    prev = ";"
    depth = 0
    for j in range(n, end):
        line = lines[j]
        code = re.sub(r'/\*.*?\*/|//.*$|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', "", line).rstrip()
        if not code.strip() or code.strip().startswith(("/*", "*")):
            continue
        if depth == 0 and not code.strip().startswith(("}", "case ", "default", "else", "#")) and \
                not re.match(r"\s*\w+\s*:(?!:)", code) and prev.endswith((";", "{", "}")):
            starts.append(j)
        depth += code.count("{") - code.count("}")
        prev = code
    # a goto and its label on either side of an inserted line would reach code past it without
    # it (sndloc.c func_800DA52C: three "goto done" before a copy, "done:" after; the return
    # read an unset copy), so those positions are skipped
    labels = {lm.group(1): k for k in range(n, end)
              for lm in [re.match(r"\s*(\w+)\s*:(?!:)", lines[k])] if lm and lm.group(1) != "default"}
    gotos = [(k, gm.group(1)) for k in range(n, end) for gm in re.finditer(r"\bgoto\s+(\w+)", lines[k])]
    straddled = {j for j in starts for k, lab in gotos
                 if lab in labels and min(k, labels[lab]) < j <= max(k, labels[lab])}
    return lines, end, n, params, first_set, starts, straddled


def empties(c_file, func, quiet=False):
    """IDO_NOTES 209 as a search: a FAKE empty test "if (v) {}" of each parameter and
    pointer/scalar local, at each top-level statement after v is first set, compiled as `check`
    does. It keeps v live to that point while IDO colours (vanime.c func_80173E8C: "if (addr ==
    0) {}" after the call, 18 to 6). A find goes in with its FAKE comment. quiet (the sweep):
    return (base, results sorted) or None."""
    with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
        text = f.read()
    if not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), text):
        if quiet:
            return None
        sys.exit(f"{func} is not parked in {c_file}")
    m = re.search(r'^[A-Za-z][^\n;]*\b%s\(([^;{]*)\)\s*\{\n' % re.escape(func), text, re.M)
    if not m:
        if quiet:
            return None
        sys.exit(f"{func}: no definition in {c_file}")
    lines, end, n, params, first_set, starts, straddled = _body_scan(m, text)
    d = os.path.join(build.ROOT, "permuter", "empties", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)
    base = _variant_lines(d, func, want, text)
    base_len = VARIANT_LEN.get(d)
    if not quiet:
        print(f"{base}  as written; variables: {', '.join(name for _, name in params)}", flush=True)
    results = []
    tried = 0
    for _, name in params:
        for j in starts + [end]:
            if j <= first_set[name] or j in straddled:
                continue
            tried += 1
            variant = lines[:j] + [f"    if ({name}) {{}} /* FAKE: keeps {name} live here (empties) */"] + lines[j:]
            c = _variant_lines(d, func, want, text[:m.end()] + "\n".join(variant))
            if c is not None and base is not None and c < base:
                where = f"if ({name}) {{}} before line {j + 1}, '{lines[j].strip()[:50]}'"
                where += _len_note(d, want, base_len)
                results.append((c, where))
                VARIANT_SOURCES[where] = text[:m.end()] + "\n".join(variant)
                if not quiet:
                    print(f"{c:5d}  {where}", flush=True)
    results.sort()
    if quiet:
        return base, results
    print(f"{func}: {base} as written; {tried} empty tests tried; best:")
    for c, where in results[:8]:
        print(f"  {c:5d}  {where}")
    return 0


STORE = re.compile(r"^(\s*)([A-Za-z_]\w*(?:(?:->|\.)\w+|\[[^\]=;]*\])*)\s*=(?!=)[^;]*;\s*(?:/\*.*\*/)?\s*$")


def reloads(c_file, func, quiet=False):
    """IDO_NOTES 268 as a search: after each one-line store to a field, an array element or a
    global, a FAKE "if (<that lvalue>) {}". ugen reloads the word right after its store, as1
    deletes the reload (it forwards the stored register), and the ugen temporary stays spent
    with no instruction, so every t-register after it moves one on: for a C whose t-registers
    run one behind the game's (plmain.c func_8018A5F8). Plain locals and parameters are left
    out (uopt forwards those itself). Measured: a dead test of a FIELD through a pointer is
    deleted by uopt before ugen (no spend; 18A5F8, 78 tries, no change); a global's reaches
    ugen (scenario.c func_800E7278). quiet (the sweep): return (base, results sorted) or None."""
    with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
        text = f.read()
    if not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), text):
        if quiet:
            return None
        sys.exit(f"{func} is not parked in {c_file}")
    m = re.search(r'^[A-Za-z][^\n;]*\b%s\(([^;{]*)\)\s*\{\n' % re.escape(func), text, re.M)
    if not m:
        if quiet:
            return None
        sys.exit(f"{func}: no definition in {c_file}")
    lines, end, n, params, first_set, starts, straddled = _body_scan(m, text)
    plain = {name for _, name in params}
    d = os.path.join(build.ROOT, "permuter", "reloads", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)
    base = _variant_lines(d, func, want, text)
    base_len = VARIANT_LEN.get(d)
    if not quiet:
        print(f"{base}  as written", flush=True)
    results = []
    tried = 0
    for j in range(n, end):
        sm = STORE.match(re.sub(r"/\*.*?\*/", "", lines[j]))
        if not sm or "FAKE" in lines[j] or (j + 1) in straddled:
            continue
        indent, lv = sm.group(1), sm.group(2)
        if lv in plain:
            continue
        tried += 1
        variant = lines[:j + 1] + [f"{indent}if ({lv}) {{}} /* FAKE: the reload after the store (reloads) */"] \
            + lines[j + 1:]
        c = _variant_lines(d, func, want, text[:m.end()] + "\n".join(variant))
        if c is not None and base is not None and c < base:
            where = f"if ({lv}) {{}} after line {j + 1}, '{lines[j].strip()[:50]}'"
            where += _len_note(d, want, base_len)
            results.append((c, where))
            VARIANT_SOURCES[where] = text[:m.end()] + "\n".join(variant)
            if not quiet:
                print(f"{c:5d}  {where}", flush=True)
    results.sort()
    if quiet:
        return base, results
    print(f"{func}: {base} as written; {tried} reloads tried; best:")
    for c, where in results[:8]:
        print(f"  {c:5d}  {where}")
    return 0


FAKE_STMT = re.compile(r"^\s*(?:if \(.*\) \{\}|do \{\} while \(0\);|if \(.*\) \{ do \{\} while \(0\); \}"
                       r"|[A-Za-z_]\w*\s+\**\w+(?:\[\d+\])?;)\s*/\*.*FAKE.*\*/\s*$")
FAKE_WRAP = re.compile(r"^(\s*)if \([01]\) \{\s*/\*.*FAKE.*\*/\s*$")


def unfakes(c_file, func, quiet=False):
    """Each FAKE in a parked function taken out in turn, compiled as `check` does: a one-line
    FAKE statement (an empty test, a "do {} while (0);", a FAKE pad declaration) removed, an
    "if (1) { /* FAKE */ ... }" wrapper unwrapped. A FAKE found for an earlier form of the C can
    be what blocks the next step (plmain.c func_8018A270: dropping the permuter's old block split
    took it from 101 to 45, IDO_NOTES 260). Other FAKEs (casts, expressions) are listed, not
    tried. quiet (the sweep): return (base, results sorted) or None."""
    with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
        text = f.read()
    if not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), text):
        if quiet:
            return None
        sys.exit(f"{func} is not parked in {c_file}")
    m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), text, re.M)
    if not m:
        if quiet:
            return None
        sys.exit(f"{func}: no definition in {c_file}")
    lines = text[m.end():].split("\n")
    end = next(i for i, l in enumerate(lines) if l.startswith("}"))
    variants, untried = [], []
    for j in range(end):
        if "FAKE" not in lines[j]:
            continue
        w = FAKE_WRAP.match(lines[j])
        if FAKE_STMT.match(lines[j]):
            variants.append((f"line {j + 1} removed: '{lines[j].strip()[:60]}'", lines[:j] + lines[j + 1:]))
        elif w:
            close = next((k for k in range(j + 1, end) if lines[k] == w.group(1) + "}"), None)
            if close is not None:
                variants.append((f"line {j + 1}'s wrapper unwrapped: '{lines[j].strip()[:60]}'",
                                 lines[:j] + lines[j + 1:close] + lines[close + 1:]))
        elif lines[j].strip().startswith(("/*", "*")):
            continue
        else:
            untried.append(f"line {j + 1}: '{lines[j].strip()[:70]}'")
    d = os.path.join(build.ROOT, "permuter", "unfakes", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)
    base = _variant_lines(d, func, want, text)
    base_len = VARIANT_LEN.get(d)
    if not quiet:
        print(f"{base}  as written; {len(variants)} FAKEs to take out", flush=True)
    results = []
    for where, variant in variants:
        c = _variant_lines(d, func, want, text[:m.end()] + "\n".join(variant))
        if not quiet:
            print(f"{c if c is not None else 'fails':>5}  {where}", flush=True)
        if c is not None and base is not None and c < base:
            where += _len_note(d, want, base_len)
            results.append((c, where))
            VARIANT_SOURCES[where] = text[:m.end()] + "\n".join(variant)
    results.sort()
    if quiet:
        return base, results
    for u in untried:
        print(f"  not tried (not a one-line statement): {u}")
    print(f"{func}: {base} as written; {len(variants)} FAKEs taken out; best:")
    for c, where in results[:8]:
        print(f"  {c:5d}  {where}")
    return 0


FAKE_ONE_LINE = re.compile(r"^\s*(?!return\b|break\b|continue\b|goto\b|case\b|default\b|else\b|for\b"
                           r"|while\b|switch\b|if\b.*\{\s*(/\*.*)?$)[^{}]*;\s*/\*.*FAKE.*\*/\s*$")
FAKE_PREFIX = re.compile(r"^(\s*)((?:do \{\} while \(0\); )+)(\S.*)$")


def _fake_candidates(lines, end):
    """(where, lines without it) for each FAKE that can be taken out mechanically: a one-line
    FAKE statement (an empty test, a block boundary, a pad, a dead store; its FAKE comment block
    right above it goes with it), a FAKE comment block over a one-line statement, leading
    "do {} while (0); " boundaries on a line, an "if (1) { /* FAKE */" wrapper. Lines inside a
    preprocessor conditional are left alone (a variant compiles with NON_MATCHING defined)."""
    out, depth = [], 0
    for j in range(end):
        s = lines[j].strip()
        if s.startswith("#if"):
            depth += 1
        elif s.startswith("#endif"):
            depth -= 1
        if depth or s.startswith(("/*", "*", "#")) or not s:
            continue
        if '"' in s:
            continue  # a print keeps its string in .rodata: no code, but the bytes (window.c)
        top = j
        if j and lines[j - 1].strip().endswith("*/"):
            k = j - 1
            while k > 0 and not lines[k].strip().startswith("/*"):
                k -= 1
            if all(lines[i].strip().startswith(("/*", "*")) for i in range(k, j)) and \
                    "FAKE" in " ".join(lines[k:j]):
                top = k
        fake_here = "FAKE" in lines[j] or top < j
        w = FAKE_WRAP.match(lines[j])
        p = FAKE_PREFIX.match(lines[j])
        if fake_here and (FAKE_STMT.match(lines[j]) or FAKE_ONE_LINE.match(lines[j])
                          or (top < j and ONE_STMT.match(lines[j]))):
            out.append((f"line {j + 1} removed: '{s[:60]}'", lines[:top] + lines[j + 1:]))
        elif fake_here and p:
            out.append((f"line {j + 1}'s boundaries removed: '{s[:60]}'",
                        lines[:top] + [p.group(1) + p.group(3)] + lines[j + 1:]))
        elif w:
            close = next((k for k in range(j + 1, end) if lines[k] == w.group(1) + "}"), None)
            if close is not None:
                out.append((f"line {j + 1}'s wrapper unwrapped: '{s[:60]}'",
                            lines[:top] + lines[j + 1:close] + lines[close + 1:]))
    return out


def unfakes_apply(c_file, func, quiet=False):
    """Matched functions too: each FAKE that can be taken out mechanically
    (_fake_candidates) is taken out when the function's code stays the same, every row of its
    disassembly (relocations resolved) identical to the C as written, so a match stays a match
    and nothing the code does changes; repeated until none can go; the C file written back.
    Returns (taken out, FAKE lines left in the body)."""
    path = os.path.join(build.ROOT, c_file)
    d = os.path.join(build.ROOT, "permuter", "unfakes_apply", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)
    removed = []
    while True:
        with open(path, encoding="utf-8") as f:
            text = f.read()
        m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), text, re.M)
        if not m:
            break
        lines = text[m.end():].split("\n")
        end = next(i for i, l in enumerate(lines) if l.startswith("}"))
        base = _variant_lines(d, func, want, text)
        base_rows = VARIANT_ROWS.get(d)
        if base is None:
            break
        for where, variant in _fake_candidates(lines, end):
            source = text[:m.end()] + "\n".join(variant)
            if _variant_lines(d, func, want, source) is not None and VARIANT_ROWS.get(d) == base_rows:
                if not _write_retry(path, source):
                    sys.exit(f"{c_file}: locked")
                removed.append(where)
                if not quiet:
                    print(f"  taken out: {where}", flush=True)
                break
        else:
            break
    left = sum(1 for l in lines[:end] if "FAKE" in l)
    if not quiet:
        print(f"{c_file} {func}: {len(removed)} FAKE(s) taken out, {left} FAKE line(s) left")
    return removed, left


def unfakes_apply_sweep():
    """unfakes_apply over every function in src/*.c whose body has a FAKE line; one line each
    into permuter/unfakes_apply.txt (a restart skips the functions listed there)."""
    log = os.path.join(build.ROOT, "permuter", "unfakes_apply.txt")
    done = set()
    if os.path.exists(log):
        with open(log, encoding="utf-8") as f:
            done = {l.split("\t")[0] for l in f if "\t" in l}
    for c_path in sorted(glob.glob(os.path.join(build.ROOT, "src", "*.c")) +
                         glob.glob(os.path.join(build.ROOT, "src", "tlb", "*.c"))):
        c_file = os.path.relpath(c_path, build.ROOT).replace("\\", "/")
        with open(c_path, encoding="utf-8") as f:
            text = f.read()
        for m in re.finditer(r'^[A-Za-z][^\n;=]*?\b([A-Za-z_]\w*)\([^;{]*\)\s*\{\n', text, re.M):
            func = m.group(1)
            body = text[m.end():].split("\n}", 1)[0]
            if "FAKE" not in body or func in done:
                continue
            removed, left = unfakes_apply(c_file, func, quiet=True)
            line = f"{func}\t{c_file}: {len(removed)} taken out, {left} left"
            print(line, flush=True)
            with open(log, "a", encoding="utf-8") as f:
                f.write(line + "\n")
    return 0


def _func_span(text, func):
    """(start, end) of func's definition in text, with the comment block right above it: from
    that block's "/*" line to the closing "}" line (inclusive), or None."""
    m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), text, re.M)
    if not m:
        return None
    end = text.find("\n}\n", m.end())
    if end < 0:
        return None
    end += 3
    start = m.start()
    above = text[:start].split("\n")[:-1]  # the lines before the definition
    if above and above[-1].rstrip().endswith("*/"):
        k = len(above) - 1
        while k > 0 and not above[k].lstrip().startswith("/*"):
            k -= 1
        start -= sum(len(l) + 1 for l in above[k:])
    return start, end


def copyout(src, copy_path):
    """a working copy of SRC, byte-exact, made by the tool. Only
    under permuter/ (copies are never written elsewhere); an existing copy is kept, not
    overwritten, so a second call cannot throw work in progress away."""
    dest = os.path.normpath(os.path.join(build.ROOT, copy_path))
    if not dest.startswith(os.path.join(build.ROOT, "permuter") + os.sep):
        sys.exit(f"copyout: {copy_path} is not under permuter/")
    if os.path.exists(dest):
        print(f"copyout: {copy_path} exists; kept as it is (your work)")
        return 0
    with open(os.path.join(build.ROOT, src), "rb") as f:
        data = f.read()
    os.makedirs(os.path.dirname(dest), exist_ok=True)
    with open(dest, "wb") as f:
        f.write(data)
    # a run's manifest (permuter/runs/<run>/manifest.txt): what land puts back where
    m = re.match(r"^permuter/runs/([A-Za-z0-9_-]+)/", copy_path.replace("\\", "/"))
    if m:
        with open(os.path.join(build.ROOT, "permuter", "runs", m.group(1), "manifest.txt"), "a",
                  encoding="utf-8", newline="\n") as f:
            f.write(f"{copy_path.replace(chr(92), '/')}\t{src.replace(chr(92), '/')}\n")
    print(f"copyout: {src} -> {copy_path} ({len(data)} bytes, byte-exact)")
    return 0


def _build_and_put_back(touched, label):
    """Full build; while it differs, put back the touched TLB files that differ (yaml-diffs)
    and build again. Returns (both hashes match, files put back)."""
    back = []
    for attempt in range(4):
        r = subprocess.run([sys.executable, os.path.join(os.path.dirname(os.path.abspath(__file__)), "build_rom.py")],
                           cwd=build.ROOT, capture_output=True, text=True)
        if r.stdout.count("MATCH") >= 2 and "DIFFERS" not in r.stdout:
            return True, back
        d = subprocess.run([sys.executable, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                                         "nisitenma_archive.py"), "yaml-diffs"],
                           cwd=build.ROOT, capture_output=True, text=True)
        line = next((l for l in d.stdout.splitlines() if "differ:" in l), "")
        ids = [int(x) for x in re.findall(r"\d+", line.split("differ:")[-1])] if line else []
        bad = [p for p in touched if any(re.search(r"/tlb_%04d(_\d+)?\.c$" % i, p) for i in ids)]
        if not bad:  # it failed to build, or differs outside the TLB files: put back everything
            bad = list(touched)
        for p in bad:
            subprocess.run(["git", "checkout", "--", p], cwd=build.ROOT)
            back.append(p)
            touched.remove(p)
        print(f"land: {label}: build differs or failed; put back {', '.join(bad)}", flush=True)
        if not touched:
            break
    r = subprocess.run([sys.executable, os.path.join(os.path.dirname(os.path.abspath(__file__)), "build_rom.py")],
                       cwd=build.ROOT, capture_output=True, text=True)
    return r.stdout.count("MATCH") >= 2 and "DIFFERS" not in r.stdout, back


def land(run, base):
    """a run of working copies landed in one command. Adopt every copy in
    permuter/runs/RUN/manifest.txt against BASE; build and put back the TLB files that differ;
    twin-copy (known failures skipped); build and put back again; summary. The commit stays the
    lead's (CHANGELOG, the identity gate)."""
    manifest = os.path.join(build.ROOT, "permuter", "runs", run, "manifest.txt")
    if not os.path.exists(manifest):
        sys.exit(f"land: no {manifest} (copies made by copyout write it)")
    pairs = []
    with open(manifest, encoding="utf-8") as f:
        for line in f:
            if "\t" in line and line.strip().split("\t") not in pairs:
                pairs.append(line.strip().split("\t"))
    adopted, refused = [], []
    for copy_path, dest in pairs:
        if not os.path.exists(os.path.join(build.ROOT, copy_path)):
            continue
        r = subprocess.run([sys.executable, os.path.abspath(__file__), "adopt", copy_path, dest, base],
                           cwd=build.ROOT, capture_output=True, text=True)
        (adopted if r.returncode == 0 else refused).append(dest)
    changed = [p for p in adopted if subprocess.run(["git", "diff", "--quiet", "--", p], cwd=build.ROOT).returncode]
    print(f"land {run}: adopted {len(changed)} changed file(s); refused {refused or 'none'}", flush=True)
    ok, back = _build_and_put_back(list(changed), "the run")
    if not ok:
        sys.exit(f"land {run}: the build does not match even with the run put back; look before going on")
    before = set(p for p in subprocess.run(["git", "diff", "--name-only", "--", "src"], cwd=build.ROOT,
                                           capture_output=True, text=True).stdout.split())
    tc = subprocess.run([sys.executable, os.path.abspath(__file__), "twin-copy"], cwd=build.ROOT,
                        capture_output=True, text=True)
    with open(os.path.join(os.path.dirname(manifest), "twin-copy.log"), "w", encoding="utf-8", newline="\n") as f:
        f.write(tc.stdout + tc.stderr)
    copied = [p for p in subprocess.run(["git", "diff", "--name-only", "--", "src"], cwd=build.ROOT,
                                        capture_output=True, text=True).stdout.split() if p not in before]
    ok2, back2 = _build_and_put_back(list(copied), "twin-copy")
    # a file the build rejected: its kept copies are failures from now on
    kept_path = os.path.join(build.ROOT, "permuter", "twincopy", "kept.txt")
    if back2 and os.path.exists(kept_path):
        with open(kept_path, encoding="utf-8") as f:
            keys = [l.rstrip("\n").split("\t") for l in f if "\t" in l]
        with open(os.path.join(build.ROOT, "permuter", "twincopy", "failed.txt"), "a", encoding="utf-8",
                  newline="\n") as f:
            for c_file, key in keys:
                if c_file in back2:
                    f.write(key + "\n")
    kept = [p for p in changed if p not in back]
    print(f"land {run}: kept {len(kept)} file(s) from the run, put back {back or 'none'}; twin-copy touched "
          f"{len(copied)} more file(s), put back {back2 or 'none'}; both hashes {'MATCH' if ok2 else 'DIFFER'}")
    print("land: now report_progress, the CHANGELOG entry and the gated commit")
    return 0 if ok2 else 1


def adopt(copy_path, dest, base):
    """a working copy put in place of DEST, whole. Work done on copies
    reaches src/ this way (or by a tool). It refuses when DEST has
    changed since BASE (the commit the copy was made from: DEST as it was then),
    so adopting can never undo later work; graft takes one function when it has."""
    if subprocess.run(["git", "cat-file", "-e", base + "^{commit}"], cwd=build.ROOT,
                      capture_output=True).returncode:
        sys.exit(f"adopt: {base} is not a commit")
    full = os.path.join(build.ROOT, dest)
    if os.path.exists(full):
        tracked = not subprocess.run(["git", "ls-files", "--error-unmatch", dest], cwd=build.ROOT,
                                     capture_output=True).returncode
        if not tracked:
            sys.exit(f"adopt: {dest} exists but is not tracked: refusing to overwrite it")
        if subprocess.run(["git", "diff", "--quiet", base, "--", dest], cwd=build.ROOT).returncode:
            sys.exit(f"adopt: REFUSED, {dest} changed since {base}; graft the copy's functions instead")
    with open(os.path.join(build.ROOT, copy_path), encoding="utf-8") as f:
        text = f.read()
    if not _write_retry(full, text):
        sys.exit(f"adopt: could not write {dest}")
    print(f"adopt: {copy_path} -> {dest} (unchanged since {base}); now check its functions and build")
    return 0


def graft(from_path, to_path, func):
    """func's definition (with its note right above it) taken from one C file
    (a working copy) into another, replacing the one there; nothing else in either file moves.
    Working copies may predate other changes to the same files (data blocks, names)."""
    with open(os.path.join(build.ROOT, from_path), encoding="utf-8") as f:
        src = f.read()
    with open(os.path.join(build.ROOT, to_path), encoding="utf-8") as f:
        dst = f.read()
    a, b = _func_span(src, func), _func_span(dst, func)
    if not a or not b:
        sys.exit(f"graft: {func} not found in {from_path if not a else to_path}")
    out = dst[:b[0]] + src[a[0]:a[1]] + dst[b[1]:]
    if not _write_retry(os.path.join(build.ROOT, to_path), out):
        sys.exit(f"{to_path}: locked")
    print(f"graft: {func} from {from_path} into {to_path}")
    return 0


def globals_plan(n_groups=16):
    """every address-named global (D_XXXXXXXX[_seg]) in src/*.c and include/
    given ONE owner, the C file that defines it, else the first file that declares it (else the
    first that uses it), so no two groups name the same symbol. Files are packed into n_groups
    groups by symbol count; permuter/names_globals/g<k>.txt holds "<symbol> <file> <how>" lines."""
    sym = re.compile(r"\bD_[0-9A-F]{8}(?:_(?:main|ovl_\d+))?\b")
    paths = sorted(glob.glob(os.path.join(build.ROOT, "src", "*.c"))) + \
        sorted(glob.glob(os.path.join(build.ROOT, "include", "*.h")))
    defined, declared, used = {}, {}, {}
    for p in paths:
        name = os.path.relpath(p, build.ROOT).replace("\\", "/")
        with open(p, encoding="utf-8") as f:
            text = re.sub(r"/\*.*?\*/", lambda m: " " * len(m.group(0)), f.read(), flags=re.S)
        for line in text.split("\n"):
            for s in sym.findall(line):
                if line.startswith("extern"):
                    declared.setdefault(s, name)
                elif re.match(r"^[A-Za-z_][^;(]*\b%s\b\s*(\[[^\]]*\])*\s*(=|;)" % re.escape(s), line) \
                        and not line.startswith(("#", "typedef")):
                    defined.setdefault(s, name)
                else:
                    used.setdefault(s, name)
    owner = {}
    for s in set(defined) | set(declared) | set(used):
        f = defined.get(s) or declared.get(s) or used.get(s)
        owner[s] = (f, "defined" if s in defined else "extern" if s in declared else "used")
    by_file = {}
    for s, (f, how) in sorted(owner.items()):
        by_file.setdefault(f, []).append((s, how))
    groups = [[] for _ in range(n_groups)]
    load = [0] * n_groups
    units = [(f, items[i:i + 220]) for f, items in by_file.items() for i in range(0, len(items), 220)]
    for f, items in sorted(units, key=lambda u: -len(u[1])):  # a big file's symbols in chunks
        k = load.index(min(load))
        groups[k].append((f, items))
        load[k] += len(items)
    out = os.path.join(build.ROOT, "permuter", "names_globals")
    os.makedirs(out, exist_ok=True)
    for k, g in enumerate(groups):
        with open(os.path.join(out, f"g{k}.txt"), "w", encoding="utf-8", newline="\n") as fh:
            for f, items in g:
                for s, how in items:
                    fh.write(f"{s} {f} {how}\n")
    print(f"{len(owner)} globals in {len(by_file)} files, groups of {min(load)}-{max(load)}")
    return 0


GCC_SYNTAX = ["-fsyntax-only", "-std=gnu89", "-w", "-fno-builtin", "-mabi=32", "-march=vr4300", "-EB",
              "-I", "include", "-I", ".", "-I", "lib/ultralib/include", "-I", "lib/ultralib/include/PR",
              "-D_MIPS_SZLONG=32", "-DBUILD_VERSION=VERSION_I", "-DF3DEX_GBI_2", "-D_FINALROM", "-DNDEBUG",
              "-DGLOBAL_ASM(...)=", "-D__sgi"]


def _gcc_errors():
    """Every C file through GCC's -fsyntax-only (the vendored mips64-elf-gcc): IDO reports a
    missing member once per file, GCC every occurrence with its column. [(file, line, col, msg)]"""
    gcc = build.tool_path(os.path.join(build.BINUTILS, "mips64-elf-gcc" + build.EXE))
    out = []
    for p in sorted(glob.glob(os.path.join(build.ROOT, "src", "*.c"))):
        rel = os.path.relpath(p, build.ROOT).replace("\\", "/")
        r = subprocess.run([gcc, *GCC_SYNTAX, rel], cwd=build.ROOT, capture_output=True, text=True)
        for m in re.finditer(r"^([^:\n]+):(\d+):(\d+): (error|note): (.*)$", r.stderr, re.M):
            where = (m.group(1).replace("\\", "/"), int(m.group(2)), int(m.group(3)))
            if m.group(4) == "error":
                out.append(where + (m.group(5), []))
            elif out and m.group(5).startswith("in expansion of macro"):
                out[-1][4].append(where)  # an error inside a macro body: where it was expanded
    return out


def fields(map_path):
    """struct fields renamed, compiler-guided. map: one "<Type> <old> <new>"
    per line (Type: the typedef name, or "struct Tag"). Each field is renamed in its struct's
    definition (include/*.h or src/*.c); then GCC (-fsyntax-only, every C file) names every use
    that broke ("'<Type>' has no member named '<old>'", with its column), each is renamed at that
    exact place, until GCC is back to the errors it had before (the baseline). A field whose
    struct cannot be found, or a broken use whose type is not in the map, stops it. Then build."""
    entries = {}
    if map_path.endswith(".jsonl"):  # a JSONL journal of field-name maps: written out for review
        import json
        rows = []
        with open(map_path, encoding="utf-8") as f:
            for line in f:
                rec = json.loads(line)
                if rec.get("type") == "result" and isinstance(rec.get("result"), dict):
                    rows += [(x["type"], x["old"], x["new"]) for x in rec["result"].get("fields", [])]
        map_path = os.path.join("permuter", "fields_map.txt")
        with open(os.path.join(build.ROOT, map_path), "w", encoding="utf-8", newline="\n") as f:
            f.write("".join(f"{t} {o} {n}\n" for t, o, n in rows))
    with open(os.path.join(build.ROOT, map_path), encoding="utf-8") as f:
        for line in f:
            parts = line.split("#", 1)[0].split()
            if len(parts) == 4 and parts[0] == "struct":
                parts = ["struct " + parts[1], parts[2], parts[3]]
            if len(parts) == 3:
                was = entries.setdefault(parts[0], {}).setdefault(parts[1], parts[2])
                if was != parts[2]:  # two groups, two names: the evidence decides, not the order
                    sys.exit(f"fields: {parts[0]}.{parts[1]} is {was} and {parts[2]} in the map; nothing changed")
    keywords = {"auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else",
                "enum", "extern", "float", "for", "goto", "if", "int", "long", "register", "return",
                "short", "signed", "sizeof", "static", "struct", "switch", "typedef", "union",
                "unsigned", "void", "volatile", "while"}
    bad = []
    for typ, ren in entries.items():
        news = list(ren.values())
        bad += [f"{typ}: {n} twice" for n in set(news) if news.count(n) > 1]
        bad += [f"{typ}: {n} is a keyword or not an identifier" for n in news
                if n in keywords or not re.match(r"^[A-Za-z_]\w*$", n)]
    if bad:
        print("\n".join(bad[:40]))
        sys.exit("fields: nothing changed")
    if "dry" in sys.argv:
        print(f"fields: {sum(len(r) for r in entries.values())} fields in {len(entries)} structs; map ok")
        return 0
    base = {(e[0], e[3]) for e in _gcc_errors()}
    heads = sorted(glob.glob(os.path.join(build.ROOT, "include", "*.h"))) + \
        sorted(glob.glob(os.path.join(build.ROOT, "src", "*.c")))
    texts = {}
    for h in heads:
        with open(h, encoding="utf-8") as f:
            texts[h] = f.read()

    def definitions(typ):
        """Every (file, start, end) of typ's body. Two files may each define their own struct of
        one name (beffect.c's and fade.c's FadeWork): each field goes to the one declaring it."""
        tag, out = typ.split()[-1], []
        for h, text in texts.items():
            if not typ.startswith("struct "):
                for m in re.finditer(r"\}\s*%s\s*;" % re.escape(tag), text):
                    depth, i, start = 0, m.start(), None
                    while i > 0:  # the body's own opening brace (nested structs inside)
                        i -= 1
                        if text[i] == "}":
                            depth += 1
                        elif text[i] == "{":
                            if depth == 0:
                                start = i
                                break
                            depth -= 1
                    if start is not None:
                        out.append((h, start, m.start()))
            if typ.startswith("struct ") or not out:
                for m in re.finditer(r"\bstruct\s+%s\s*\{" % re.escape(tag), text):
                    out.append((h, m.end() - 1, text.find("\n}", m.end())))
        return out

    edits = {}  # file -> {(start, end): {old: new}}; every field placed BEFORE any file is written
    for typ, ren in entries.items():
        defs = definitions(typ)
        if not defs:
            sys.exit(f"fields: struct {typ} not found; nothing changed")
        for old, new in ren.items():
            pat = r"\b%s\b(?=\s*(\[|;|:|,))" % re.escape(old)
            where = [d for d in defs if re.search(pat, texts[d[0]][d[1]:d[2]])]
            # a file's partial view of the struct pads the bytes it does not use (geography.c's
            # StageData: u8 unkC[0x54] over coll and the rails): a pad never takes a field's name
            pad = r"\b[us]8\s+%s\s*\[" % re.escape(old)
            where = [d for d in where if not re.search(pad, texts[d[0]][d[1]:d[2]])] or where
            # one struct seen by several files, each its own partial view (CollTri in seven,
            # SndJingle in six): unkXX names its offset, so the views' unk24 is one field when
            # every view declares it with the same type
            decl = r"([\w\s\*]+?)\s*\b%s\b\s*(\[[^\]]*\])?\s*;" % re.escape(old)
            bodies = set()
            for h, s, e in where:
                m = re.search(decl, re.sub(r"/\*.*?\*/|//[^\n]*", " ", texts[h][s:e], flags=re.S))
                bodies.add((" ".join(m.group(1).split()[-2:]), m.group(2)) if m else (h, s))
            if not where or len(bodies) != 1:
                sys.exit(f"fields: {typ}.{old} is declared in {len(where)} different definitions of {typ}"
                         f" ({', '.join(os.path.basename(d[0]) for d in defs)}); nothing changed")
            for h, start, end in where:
                edits.setdefault(h, {}).setdefault((start, end), {})[old] = new
    for h, spans in edits.items():
        text = texts[h]
        for (start, end), ren in sorted(spans.items(), reverse=True):
            body = text[start:end]
            for old, new in ren.items():
                body = re.sub(r"\b%s\b(?=\s*(\[|;|:|,))" % re.escape(old), new, body, count=1)
            text = text[:start] + body + text[end:]
        _write_retry(h, text)
    for _ in range(6):
        errs = [e for e in _gcc_errors() if (e[0], e[3]) not in base]
        fixes = {}
        for path, line, col, msg, expansions in errs:
            m = re.match(r"'([^']+)' has no member named '(\w+)'", msg)
            if m and expansions:
                # a member named by a macro argument (BLEND(w, pair, c)): the error is in the
                # macro's body, the name at the expansion that spells it
                for p, l, c in expansions:
                    with open(os.path.join(build.ROOT, p), encoding="utf-8") as f:
                        s = f.read().split("\n")[l - 1]
                    if re.search(r"\b%s\b" % re.escape(m.group(2)), s.encode("utf-8")[c - 1:].decode("utf-8", errors="ignore")):
                        path, line, col = p, l, c
                        break
            # GCC says 'struct CamView' for a use through the tag, the map says CamView (or the
            # reverse): one type
            typ = m and next((t for t in (m.group(1), m.group(1).replace("struct ", ""),
                                          "struct " + m.group(1)) if m.group(2) in entries.get(t, {})), None)
            if not typ:
                sys.exit(f"fields: {path}:{line}:{col}: {msg}")
            fixes.setdefault(path, []).append((line, col, m.group(2), entries[typ][m.group(2)]))
        if not fixes:
            print(f"fields: {sum(len(r) for r in entries.values())} fields renamed; GCC as before")
            return 0
        for path, items in fixes.items():
            full = os.path.join(build.ROOT, path)
            with open(full, encoding="utf-8") as f:
                lines = f.read().split("\n")
            # a macro argument used twice (CAM_MOVE) is reported twice at one place: once each
            for line, col, old, new in sorted(set(items), key=lambda t: (t[0], -t[1])):
                s = lines[line - 1]
                # GCC counts columns in bytes: a Japanese string before the use moves it
                col = len(s.encode("utf-8")[:col - 1].decode("utf-8", errors="ignore")) + 1
                # GCC's column is the member's, or the -> / . before it; at a macro's expansion,
                # the macro name's (the argument follows)
                m = (re.compile(r"(->|\.)?\s*\b%s\b" % re.escape(old)).match(s, col - 1)
                     or re.compile(r"(?:(?<=->)|(?<=\.)|(?<=[(,\s]))%s\b" % re.escape(old)).search(s, col - 1))
                if not m:
                    sys.exit(f"fields: {path}:{line}:{col} is not '{old}'")
                at = m.end() - len(old)
                lines[line - 1] = s[:at] + new + s[m.end():]
            _write_retry(full, "\n".join(lines))
    sys.exit("fields: GCC still reports broken uses after 6 passes")


TLB_HEADERS = ["common.h", "object.h", "game.h", "player.h", "enemy.h", "boss.h", "camera.h",
               "snd.h", "file.h", "pic.h", "acoll.h", "contpak.h", "scheduler.h"]
TYPE_WORDS = {"void", "char", "short", "int", "long", "float", "double", "signed", "unsigned",
              "const", "volatile", "struct", "union", "enum", "extern", "static", "register"}


def _balanced(text, i):
    """The index just past the parenthesis that closes the one at text[i]."""
    depth = 0
    for j in range(i, len(text)):
        depth += {"(": 1, ")": -1}.get(text[j], 0)
        if depth == 0:
            return j + 1
    return -1


def tlb_header(apply=False):
    """include/tlb.h for the TLB code files (src/tlb/). Code at 0x08000000
    cannot jal into main or the overlays (another 256 MB region): the game calls each of their
    functions through its address (lui/addiu + jalr t9), which is IDO's code for (*f)(...). The
    header declares every main/overlay function and object the TLB code refers to (from the asm:
    %hi/%lo and .word), each with its definition's types (a type main keeps in its .c file is
    copied when it is one plain typedef, else a pointer to it becomes void *), then makes each
    function's name that call: "#define f (*f)" (a macro is not expanded inside itself). Writes
    permuter/tlb/tlbh.txt (what could not be declared); with apply, include/tlb.h."""
    asm = glob.glob(os.path.join(build.ROOT, "asm", "*.s"))
    for top in (os.path.join("asm", "data"), os.path.join("asm", "nonmatchings", "tlb")):
        asm += [os.path.join(d, n) for d, _, ns in os.walk(os.path.join(build.ROOT, top))
                for n in ns if n.endswith(".s")]
    refs = set()
    for p in asm:
        with open(p, encoding="utf-8", errors="replace") as f:
            t = f.read()
        refs |= set(re.findall(r"%(?:hi|lo)\(([A-Za-z_]\w*)\)", t)) | set(re.findall(r"\.word\s+([A-Za-z_]\w*)", t))
    objdump = os.path.join(os.path.dirname(build.ROOT), "vendor", "mips64", "bin", "mips64-elf-objdump.exe")
    kind = {}
    for line in subprocess.run([objdump, "-t", os.path.join(build.ROOT, "build", "goemonsgreatadv.elf")],
                               capture_output=True, text=True, check=True).stdout.splitlines():
        p = line.split()
        if len(p) == 6 and p[1] == "g" and p[2] in ("F", "O") and p[3] != "*ABS*" and "tlb" not in p[3]:
            kind[p[5]] = p[2]
    funcs = sorted(n for n in refs if kind.get(n) == "F")
    objs = sorted(n for n in refs if kind.get(n) == "O")
    srcs = {}
    for p in sorted(glob.glob(os.path.join(build.ROOT, "src", "*.c"))):
        with open(p, encoding="utf-8") as f:
            srcs[os.path.basename(p)] = f.read()
    # the types the headers already give
    known = set()
    heads = [os.path.join(d, n) for top in ("include", os.path.join("lib", "ultralib", "include"))
             for d, _, ns in os.walk(os.path.join(build.ROOT, top)) for n in ns if n.endswith(".h")]
    htext = {}
    for h in heads:
        with open(h, encoding="utf-8", errors="replace") as f:
            htext[h] = f.read()
        known |= set(re.findall(r"\}\s*(\w+)\s*;", htext[h])) | set(re.findall(r"typedef\s[^;{}]*?\b(\w+)\s*(?:\[[^\]]*\])?\s*;", htext[h]))
        known |= set(re.findall(r"typedef\s[^;{}]*\(\s*\*\s*(\w+)\s*\)", htext[h]))
    known |= {"Gfx", "Mtx", "Vtx", "Lights7", "LookAt", "Hilite", "Vp", "OSMesg", "OSMesgQueue", "Acmd"}
    copies, copied = [], set()
    # one pass over main's .c files: its typedefs, function definitions, file-scope declarations
    tdefs, fdefs, odefs = {}, {}, {}
    for t in srcs.values():
        for m in re.finditer(r"^typedef\s+(?:struct|union)\s*\w*\s*\{", t, re.M):
            end = t.find("\n}", m.end())
            n = re.match(r"\n\}\s*(\w+)\s*;", t[end:end + 80]) if end > 0 else None
            if n:
                body = re.sub(r"/\*.*?\*/", "", t[m.start():end + n.end()], flags=re.S)
                tdefs.setdefault(n.group(1), []).append(re.sub(r"[ \t]+\n", "\n", body))
        for m in re.finditer(r"^typedef\s[^;{}\n()]*\b(\w+)\s*;", t, re.M):
            tdefs.setdefault(m.group(1), []).append(m.group(0))
        for m in re.finditer(r"^typedef\s[^;{}\n]*\(\s*\*\s*(\w+)\s*\)\s*\([^;{}\n]*\)\s*;", t, re.M):
            tdefs.setdefault(m.group(1), []).append(m.group(0))  # a function pointer type (TaskProc)
        for m in re.finditer(r"^(?!static\b|typedef\b|extern\b|return\b|else\b)([A-Za-z_][\w \t\*]*?)\b([A-Za-z_]\w*)\s*\(", t, re.M):
            end = _balanced(t, m.end() - 1)
            if end > 0 and re.match(r"\s*\{", t[end:end + 8]) and m.group(2) not in fdefs:
                fdefs[m.group(2)] = re.sub(r"\s+", " ", t[m.start():end]).strip()
        for m in re.finditer(r"^(?!static\b|typedef\b|return\b)(?:extern\s+)?((?:const\s+)?(?:struct\s+)?[A-Za-z_]\w*(?:\s*\*)*)"
                             r"\s*\b([A-Za-z_]\w*)((?:\s*\[[^\]]*\])*)\s*(?:=|;)", t, re.M):
            odefs.setdefault(m.group(2), f"extern {m.group(1).strip()} {m.group(2)}{re.sub(r'\s+', '', m.group(3))};")

    picks = []

    def typedef_in_src(name):
        """name's typedef in main's .c files. Main keeps many structs as each file's own view
        (PlStat in five files, SndSe in seven): the most common text, on a tie the most
        detailed; every such pick is listed in permuter/tlb/tlbh.txt."""
        from collections import Counter
        found = Counter(tdefs.get(name, []))
        if not found:
            return None
        body = max(found, key=lambda b: (found[b], len(b)))
        if len(found) > 1:
            picks.append(f"type {name}: {len(found)} different views in main's .c files; took one of"
                         f" {found[body]} with {len(body)} characters")
        return body

    def resolve(text, own=None):
        """text with each unknown type made known (copied) or, as a pointer, void; None if not.
        own: the declared name (a game function named Ride is not a type)."""
        for word in sorted(set(re.findall(r"\b[A-Za-z_]\w*\b", text)) - TYPE_WORDS - {own}, key=len, reverse=True):
            if word in known or word in copied or re.search(r"\bstruct\s+%s\b" % word, text):
                continue
            if not re.match(r"^[A-Z]", word) and not re.search(r"\b%s\s*\*" % word, text):
                continue  # a parameter name, not a type
            if re.match(r"^[A-Z][A-Z0-9_]+$", word):
                continue  # a macro (MAXCONTROLLERS)
            body = typedef_in_src(word)
            if body and resolve(body, word) == body:
                copies.append(body)
                copied.add(word)
            elif re.search(r"\b%s\s*\*" % word, text):
                text = re.sub(r"\b%s(?=\s*\*)" % word, "void", text)
            else:
                return None
        return text

    protos, externs, missing, defined = [], [], [], set()
    allheads = "\n".join(htext.values())
    for name in funcs:
        if re.search(r"\b%s\s*\(" % re.escape(name), allheads):
            defined.add(name)  # a header declares it already (libultra's, object.h's)
            continue
        sig = fdefs.get(name)
        sig = sig and resolve(sig, name)
        if sig:
            protos.append(sig + ";")
            defined.add(name)
        else:
            missing.append(f"function {name}: no definition in src/*.c with known types")
    for name in objs:
        if re.match(r"^D_[0-9A-F]+", name) or re.search(r"\b%s\b\s*(\[[^\]]*\])*\s*;" % re.escape(name), allheads):
            continue  # an address inside another variable (named by the field), or declared
        decl = odefs.get(name)
        decl = decl and resolve(decl, name)
        if decl:
            externs.append(decl)
        else:
            missing.append(f"object {name}: no file-scope declaration in src/*.c with known types")
    out = ["#ifndef TLB_H", "#define TLB_H", "",
           "/* The TLB code files (archive files 16-327, src/tlb/) are linked at 0x08000000 and cannot jal",
           " * into main or the overlays (0x80000000 and up, another 256 MB region). The game calls each",
           " * such function through its address: lui/addiu + jalr, which is IDO's code for (*f)(...). This",
           " * header declares what the TLB code uses of main and the overlays, then makes each function's",
           " * name that call (a macro is not expanded inside itself). Generated by",
           " * canon-decompilation-tools/match_function.py tlbh from the asm and main's definitions. */", ""]
    out += [f'#include "{h}"' for h in TLB_HEADERS] + [""]
    out += ["/* Types main keeps in its .c files. */"] + copies + [""] if copies else []
    out += ["/* Main's and the overlays' variables. */"] + externs + [""]
    out += ["/* Main's and the overlays' functions. */"] + protos + [""]
    out += [f"#define {n} (*{n})" for n in sorted(defined)] + ["", "#endif", ""]
    with open(os.path.join(build.ROOT, "permuter", "tlb", "tlbh.txt"), "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(missing + sorted(set(picks))) + "\n")
    print(f"tlbh: {len(funcs)} functions ({len(protos)} declared), {len(objs)} objects ({len(externs)} declared),"
          f" {len(copies)} types copied; {len(missing)} not (permuter/tlb/tlbh.txt)")
    if apply:
        _write_retry(os.path.join(build.ROOT, "include", "tlb.h"), "\n".join(out))
    return 0


HDR_KEYWORDS = {"void", "char", "short", "int", "long", "float", "double", "signed", "unsigned", "const",
                "volatile", "struct", "union", "enum", "extern", "static", "register", "typedef", "return",
                "else", "case", "if", "while", "for", "switch", "do", "sizeof", "goto", "inline", "__inline",
                "break", "continue", "default"}
# an old-style (K&R) definition: "void f(a, b)" then its parameters' declarations, each ending in ';'
HDR_KR_OPEN = re.compile(r"^[A-Za-z_][\w\s*]*?\b[A-Za-z_]\w*\s*\(\s*[A-Za-z_]\w*(?:\s*,\s*[A-Za-z_]\w*)*\s*\)"
                         r"\s*[A-Za-z_][^=(){}]*$")
HDR_KR_DEF = re.compile(r"^[A-Za-z_][\w\s*]*?\b[A-Za-z_]\w*\s*\(\s*[A-Za-z_]\w*(?:\s*,\s*[A-Za-z_]\w*)*\s*\)"
                        r"\s*(?:[^;=(){}]+;\s*)+$")


def _hdr_blank(text, directives=False):
    """text with comments, string and character literals (and, unless directives, preprocessor
    lines with their continuations) turned to spaces; newlines kept, so offsets and line numbers
    hold."""
    out = list(text)
    i, n, line_start = 0, len(text), True

    def wipe(a, b):
        for k in range(a, b):
            if out[k] != "\n":
                out[k] = " "
    while i < n:
        c = text[i]
        if line_start and c in " \t":
            i += 1
            continue
        if line_start and c == "#" and not directives:
            j = i
            while j < n and text[j] != "\n":
                if text[j] == "\\" and j + 1 < n and text[j + 1] == "\n":
                    j += 2
                elif text.startswith("/*", j):
                    k = text.find("*/", j + 2)
                    j = n if k < 0 else k + 2
                else:
                    j += 1
            wipe(i, j)
            i = j
            continue
        line_start = False
        if text.startswith("/*", i):
            k = text.find("*/", i + 2)
            k = n if k < 0 else k + 2
            wipe(i, k)
            i = k
            continue
        if text.startswith("//", i):
            k = text.find("\n", i)
            k = n if k < 0 else k
            wipe(i, k)
            i = k
            continue
        if c in "\"'":
            j = i + 1
            while j < n and text[j] != c and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            wipe(i + 1, min(j, n))
            i = j + 1
            continue
        if c == "\n":
            line_start = True
        i += 1
    return "".join(out)


def _hdr_close(b, i):
    """The index of the '}' closing the '{' at b[i]."""
    d = 0
    for j in range(i, len(b)):
        if b[j] == "{":
            d += 1
        elif b[j] == "}":
            d -= 1
            if d == 0:
                return j
    return len(b) - 1


def _hdr_func(head):
    """A declaration's or definition's head as one function: dict(name, ret, params, static, kr,
    sig) for `RET NAME(PARAMS)` (the name before the first '(', that '(' closing at the end, or
    an old-style definition's parameter declarations after it); None for anything else (a
    function pointer, an array of them, two declarators). sig is the declaration as written up
    to its ')', whitespace runs as one space; for an old-style definition, `RET NAME()`."""
    h = " ".join(head.split())
    i = h.find("(")
    m = re.search(r"([A-Za-z_]\w*)\s*$", h[:i]) if i > 0 else None
    if not m:
        return None
    name, ret = m.group(1), h[:m.start()].strip()
    words = ret.replace("*", " ").split()
    if not words or name in HDR_KEYWORDS or words[0] in HDR_KEYWORDS - {
            "void", "char", "short", "int", "long", "float", "double", "signed", "unsigned", "const",
            "volatile", "struct", "union", "enum", "extern", "static", "register"}:
        return None
    if not re.fullmatch(r"[A-Za-z_][\w\s*]*", ret):
        return None
    depth, j = 0, i
    for j in range(i, len(h)):
        depth += {"(": 1, ")": -1}.get(h[j], 0)
        if depth == 0:
            break
    rest = h[j + 1:].strip()
    kr = bool(rest) and bool(HDR_KR_DEF.match(h) or HDR_KR_OPEN.match(h))
    if rest and not kr:
        return None
    params = h[i + 1:j].strip()
    sig = (h[:i] + "()") if kr else h[:j + 1]
    sig = re.sub(r"^(?:extern|static)\s+", "", sig)
    return {"name": name, "ret": ret, "params": "" if kr else params, "static": "static" in words, "kr": kr,
            "sig": sig}


def _hdr_items(text):
    """Every top-level item of a C file or header: dicts with kind ('def', a function definition;
    'decl', a declaration ending in ';'), start and end (offsets; end past the ';' or the body's
    '}'), line (1-based, of start), head (the blanked text before the body or the ';') and fn
    (_hdr_func of the head, or None)."""
    b = _hdr_blank(text)
    items, i, n, start, depth = [], 0, len(b), None, 0
    while i < n:
        c = b[i]
        if start is None:
            if c.isspace():
                i += 1
                continue
            start = i
        if c in "([":
            depth += 1
        elif c in ")]":
            depth -= 1
        elif c == ";" and depth == 0:
            head = b[start:i]
            if not HDR_KR_OPEN.match(" ".join(head.split())):
                items.append({"kind": "decl", "start": start, "end": i + 1, "head": head,
                              "line": text.count("\n", 0, start) + 1, "fn": _hdr_func(head)})
                start = None
        elif c == "}" and depth == 0:  # the close of an extern "C" block
            start = None
        elif c == "{" and depth == 0:
            head = b[start:i]
            h = " ".join(head.split())
            if re.fullmatch(r'extern\s*"\s*"', h):  # extern "C" { ... }: its declarations are top level
                start = None
                i += 1
                continue
            j = _hdr_close(b, i)
            if "=" not in h and not re.match(r"(?:typedef|struct|union|enum)\b", h) and \
                    (h.endswith(")") or HDR_KR_DEF.match(h)):
                items.append({"kind": "def", "start": start, "end": j + 1, "head": head,
                              "line": text.count("\n", 0, start) + 1, "fn": _hdr_func(head)})
                start = None
            i = j + 1
            continue
        i += 1
    return items


def _hdr_ptype(p):
    """One parameter's type, its name taken out, spaced one way ("Obj *", "void (*)(Obj *)")."""
    p = " ".join(p.split())
    m = re.match(r"^(.*?)\(\s*\*\s*[A-Za-z_]?\w*\s*\)\s*\((.*)\)\s*$", p)
    if m:  # a function pointer: its name out, and its own parameters' names
        inner = m.group(2).strip()
        inner = inner if inner in ("", "void") else ", ".join(_hdr_ptype(x) for x in _hdr_split(inner))
        p = "%s(*)(%s)" % (m.group(1), inner)
    elif "(" in p:
        p = re.sub(r"\(\s*\*\s*[A-Za-z_]\w*\s*\)", "(*)", p, count=1)
    else:
        toks = re.findall(r"[A-Za-z_]\w*|\*|\[[^\]]*\]|\S", p)
        idents = [k for k, t in enumerate(toks) if re.match(r"[A-Za-z_]", t)]
        if len(idents) >= 2:
            last = idents[-1]
            before = toks[idents[-2]]
            if toks[last] not in HDR_KEYWORDS and before not in ("struct", "union", "enum") \
                    and all(not re.match(r"[A-Za-z_]", t) for t in toks[last + 1:]):
                toks = toks[:last] + toks[last + 1:]
                if last < len(toks) and toks[last].startswith("["):  # an array parameter is a pointer
                    toks[last] = "*"
        p = " ".join(toks)
    p = re.sub(r"\s*\*\s*", " *", p)
    p = re.sub(r"\s+", " ", p).strip()
    return p.replace("* *", "**").replace("* *", "**")


def _hdr_types(fn):
    """(return type, parameter types) of a _hdr_func, names out; parameter types None for an
    unprototyped declaration (an empty list), () for (void)."""
    ret = " ".join(w for w in fn["ret"].split() if w not in ("extern", "static"))
    ret = re.sub(r"\s*\*\s*", " *", ret).strip().replace("* *", "**")
    if fn["kr"] or fn["params"] == "":
        return ret, None
    if fn["params"] == "void":
        return ret, ()
    parts, depth, cur = [], 0, ""
    for ch in fn["params"]:
        if ch == "," and depth == 0:
            parts.append(cur)
            cur = ""
            continue
        depth += {"(": 1, ")": -1}.get(ch, 0)
        cur += ch
    parts.append(cur)
    return ret, tuple(_hdr_ptype(x) for x in parts)


HDR_SCALAR = {"u8": "u8", "s8": "s8", "char": "s8", "u16": "u16", "s16": "s16", "short": "s16",
              "u32": "u32", "s32": "s32", "int": "s32", "long": "s32", "unsigned": "u32",
              "unsigned int": "u32", "unsigned long": "u32", "u64": "u64", "s64": "s64",
              "f32": "f32", "float": "f32", "f64": "f64", "double": "f64", "void": "void"}


def _hdr_class(t, typedefs):
    """A parameter or return type's class for code: 'ptr' (any pointer, a function pointer type),
    a scalar's name (u8 ... f64, void), or the text itself when unknown. typedefs: {name: body}."""
    t = " ".join(w for w in t.split() if w not in ("const", "volatile", "register", "signed")) or "s32"
    if "*" in t or t.endswith(")"):
        return "ptr"
    if t in HDR_SCALAR:
        return HDR_SCALAR[t]
    seen = set()
    while t in typedefs and t not in seen:
        seen.add(t)
        body = typedefs[t]
        if "(*" in body:
            return "ptr"
        m = re.match(r"typedef\s+([\w\s*]+?)\s*\b%s\s*;$" % re.escape(t), body)
        if not m:
            return t
        t = " ".join(m.group(1).split())
        if "*" in t:
            return "ptr"
        if t in HDR_SCALAR:
            return HDR_SCALAR[t]
    return t


def _hdr_matters(a, b, typedefs):
    """Whether two declarations of one function can compile a call differently: the return
    class or void-ness, the parameter count or prototype, or a parameter of another class (a
    pointer for a pointer, or for a 32-bit integer, passes the same)."""
    ra, pa = _hdr_types(a)
    rb, pb = _hdr_types(b)
    ca, cb = _hdr_class(ra, typedefs), _hdr_class(rb, typedefs)
    if ca != cb and not ({ca, cb} <= {"ptr", "u32", "s32"} and "ptr" in {ca, cb}):
        return True
    if (pa is None) != (pb is None) or (pa is not None and len(pa) != len(pb)):
        return True
    for x, y in zip(pa or (), pb or ()):
        cx, cy = _hdr_class(x, typedefs), _hdr_class(y, typedefs)
        if cx != cy and not ({cx, cy} <= {"ptr", "u32", "s32"}):
            return True
    return False


def _hdr_module(rel):
    """(module, is_tlb) for a src/ C file: src/plmain.c is plmain, src/tlb/tlb_0017_2.c is tlb_0017
    (its archive file)."""
    m = re.match(r"src/tlb/(tlb_\d{4})(?:_\w+)?\.c$", rel)
    if m:
        return m.group(1), True
    m = re.match(r"src/(\w+)\.c$", rel)
    return (m.group(1), False) if m else (None, False)


def _hdr_header_of(module, tlb):
    """A module's header: its path under the tree, and its #include line's name. A name a library
    header has (string.c: zlib's zutil.h includes <string.h>, and -I include comes first) takes
    "_c": string_c.h."""
    if tlb:
        return "include/tlb/%s.h" % module, "tlb/%s.h" % module
    lib_dirs = (build.ULTRALIB, build.ULTRALIB + "/PR", build.ULTRALIB + "/compiler/ido", "lib/zlib")
    if any(os.path.exists(os.path.join(build.ROOT, d, module + ".h")) for d in lib_dirs):
        module += "_c"
    return "include/%s.h" % module, "%s.h" % module


def _hdr_closure(root, includes, cache):
    """Every header (tree-relative path) the include names reach, following each header's own
    #include lines: include/ first, then lib/ultralib/include (and its PR/), as the build's -I."""
    out, todo = [], list(includes)
    dirs = ["include", build.ULTRALIB, build.ULTRALIB + "/PR", build.ULTRALIB + "/compiler/ido"]
    while todo:
        name = todo.pop(0)
        path = None
        for d in dirs:
            p = d + "/" + name
            if os.path.exists(os.path.join(root, p)) or (p.startswith("lib/") and
                                                          os.path.exists(os.path.join(build.ROOT, p))):
                path = p
                break
        if path is None or path in out:
            continue
        out.append(path)
        if path not in cache:
            base = root if os.path.exists(os.path.join(root, path)) else build.ROOT
            with open(os.path.join(base, path), encoding="utf-8", errors="replace") as f:
                cache[path] = f.read()
        todo += re.findall(r'^\s*#\s*include\s+[<"]([^>"]+)[>"]', cache[path], re.M)
    return out


def _hdr_scan(root):
    """The tree read: {rel: dict(text, items, includes)} for src/**/*.c, and the same for
    include/**/*.h; rel paths with '/'."""
    files, heads = {}, {}
    for top, ext, into in (("src", ".c", files), ("include", ".h", heads)):
        for d, _, ns in os.walk(os.path.join(root, top)):
            for n in ns:
                if n.endswith(ext):
                    p = os.path.join(d, n)
                    rel = os.path.relpath(p, root).replace(os.sep, "/")
                    with open(p, encoding="utf-8") as f:
                        text = f.read()
                    into[rel] = {"text": text, "items": _hdr_items(text),
                                 "includes": re.findall(r'^\s*#\s*include\s+[<"]([^>"]+)[>"]', text, re.M)}
    return files, heads


def _hdr_typenames(text):
    """The typedef names and struct/union/enum tags a C text declares."""
    b = _hdr_blank(text)
    out = set(re.findall(r"\}\s*(\w+)\s*;", b))
    out |= set(re.findall(r"\btypedef\s[^;{}]*?\b(\w+)\s*(?:\[[^\]]*\]\s*)*;", b))
    out |= set(re.findall(r"\btypedef\s[^;{}]*\(\s*\*\s*(\w+)\s*\)", b))
    out |= set(re.findall(r"\b(?:struct|union|enum)\s+(\w+)\s*\{", b))
    return out


def _hdr_typedefs(text, items):
    """{type name: (item, body)} for a C text's top-level type definitions (typedefs, tagged
    struct/union/enum definitions); body is the definition with comments out and whitespace runs
    one space, a function pointer typedef's parameter names out (two spellings of one type)."""
    b = _hdr_blank(text)
    out = {}
    for it in items:
        if it["kind"] != "decl" or it["fn"]:
            continue
        norm = " ".join(b[it["start"]:it["end"]].split())
        if not re.match(r"(?:typedef|struct|union|enum)\b", norm):
            continue
        m = re.match(r"^typedef\s+(.*?)\(\s*\*\s*(\w+)\s*\)\s*\((.*)\)\s*;$", norm)
        if m and "{" not in norm:
            body = "typedef %s(*%s)(%s);" % (m.group(1), m.group(2), ", ".join(
                _hdr_ptype(x) for x in _hdr_split(m.group(3))))
            out[m.group(2)] = (it, body)
            continue
        names = set(re.findall(r"\}\s*(\w+)\s*;$", norm)) | set(re.findall(r"^typedef\s[^{}]*?\b(\w+)\s*;$", norm))
        names |= set(re.findall(r"^(?:typedef\s+)?(?:struct|union|enum)\s+(\w+)\s*\{", norm))
        for n in names:
            out[n] = (it, norm)
    return out


def _hdr_split(params):
    """A parameter list split at its top-level commas."""
    parts, depth, cur = [], 0, ""
    for ch in params:
        if ch == "," and depth == 0:
            parts.append(cur)
            cur = ""
            continue
        depth += {"(": 1, ")": -1}.get(ch, 0)
        cur += ch
    return parts + [cur] if cur.strip() or parts else []


def _hdr_uses(text):
    """The identifiers a C text uses outside comments and strings (directives kept), not as a
    field (after . or ->)."""
    return set(re.findall(r"(?<![\w.])(?<!->)([A-Za-z_]\w*)", _hdr_blank(text, directives=True)))


def _hdr_sig_types(fn):
    """The type names a function's signature uses."""
    ret, ps = _hdr_types(fn)
    return set(re.findall(r"[A-Za-z_]\w*", ret + " " + " ".join(ps or ()))) - HDR_KEYWORDS


def _hdr_lines(text):
    """[(start, end)] offsets of each line (end past its newline)."""
    out, s = [], 0
    for line in text.split("\n"):
        out.append((s, s + len(line) + 1))
        s += len(line) + 1
    return out


def _hdr_span(text, b2, start, end):
    """The text to cut for an item at [start, end): its whole lines (with a trailing comment, run
    on to the line its comment closes) when nothing else is on them, else the item alone."""
    ls = text.rfind("\n", 0, start) + 1
    j = end
    while j < len(text) and text[j] != "\n" and b2[j] in " \t":
        j += 1
    if b2[ls:start].strip() or (j < len(text) and text[j] != "\n"):
        return start, end
    tail = text[end:j]
    if tail.count("/*") > tail.count("*/"):  # a comment opened on the line runs on
        k = text.find("*/", j)
        j = text.find("\n", k) if k >= 0 else len(text)
        j = len(text) if j < 0 else j
    return ls, min(j + 1, len(text))


def _hdr_lead(text, b2, start):
    """Where the comment block just above the line at start begins (start's line start if
    none): lines that hold only a comment, the last of them right above."""
    ls = text.rfind("\n", 0, start) + 1
    at = ls
    while at > 0:
        ps = text.rfind("\n", 0, at - 1) + 1
        line = text[ps:at - 1]
        if not line.strip() or b2[ps:at - 1].strip() or line.lstrip().startswith("#"):
            break
        at = ps
    return at


TLB_HEADER_SET = {"include/" + h for h in TLB_HEADERS}


def _hdr_plan(root, force_omit=None):
    """The headers mode's plan for the tree at root; nothing is written. A dict:
    files/heads (the scan), defs (name -> (file, item) of each non-static definition), mods
    (module -> its header: path, include name, functions [(name, sig)], types moved in, the
    includes it needs, whether it exists), omit (function -> why its header leaves it out),
    tlbh_add/tlbh_drop (prototypes tlb.h gains or loses as a duplicate), typecopies (file ->
    type names whose identical copy goes, the header bringing the type), per file the headers to
    include and the prototypes to remove, and notes. force_omit: {function: why} no header may
    declare (the check found a file that already includes its header and cannot take it)."""
    import collections
    files, heads = _hdr_scan(root)
    cache = {}
    P = {"root": root, "files": files, "heads": heads, "notes": [], "omit": dict(force_omit or {})}
    defs, statics = {}, collections.defaultdict(set)
    for rel, f in sorted(files.items()):
        # a function declared static anywhere in its file is static (a later definition without
        # the word keeps the internal linkage)
        declared_static = {it["fn"]["name"] for it in f["items"] if it["fn"] and it["fn"]["static"]}
        for it in f["items"]:
            fn = it["fn"]
            if it["kind"] == "def" and fn:
                if fn["static"] or fn["name"] in declared_static:
                    statics[rel].add(fn["name"])
                elif fn["name"] in defs:
                    P["notes"].append(f"{fn['name']}: defined in {defs[fn['name']][0]} and {rel}")
                else:
                    defs[fn["name"]] = (rel, it)
    P["defs"] = defs
    hdecl = {}  # name -> (header, item): what include/ declares already (tlb.h apart)
    for hrel, h in sorted(heads.items()):
        if hrel == "include/tlb.h":
            continue
        for it in h["items"]:
            if it["kind"] == "decl" and it["fn"]:
                hdecl.setdefault(it["fn"]["name"], (hrel, it))
    tlbh = heads.get("include/tlb.h")
    tlb_decl = {it["fn"]["name"]: it for it in (tlbh["items"] if tlbh else [])
                if it["kind"] == "decl" and it["fn"]}
    far = set(re.findall(r"^#define\s+(\w+)\s+\(\*\1\)\s*$", tlbh["text"], re.M)) if tlbh else set()
    tdefs = {rel: _hdr_typedefs(f["text"], f["items"]) for rel, f in files.items()}
    htdefs = {rel: _hdr_typedefs(h["text"], h["items"]) for rel, h in heads.items()}
    uses = {rel: _hdr_uses(f["text"]) for rel, f in files.items()}
    P["typedefs"] = {}  # a type name -> a definition of it (for _hdr_class)
    for d in list(htdefs.values()) + list(tdefs.values()):
        for t, (it, body) in d.items():
            P["typedefs"].setdefault(t, body)

    def closure_of(names):
        return _hdr_closure(root, names, cache)

    def known_types(names):
        out = set()
        for h in closure_of(names):
            out |= _hdr_typenames(cache[h])
        return out

    fmacros = {}

    def file_macros(rel):
        """{macro: [its #define line(s) as an item (start, end)]} of a C file."""
        if rel not in fmacros:
            text, out = files[rel]["text"], collections.defaultdict(list)
            for m_ in re.finditer(r"^#define\s+(\w+)(?:[^\n]*\\\n)*[^\n]*", text, re.M):
                out[m_.group(1)].append({"start": m_.start(), "end": m_.end(), "kind": "macro"})
            fmacros[rel] = out
        return fmacros[rel]

    hfuncs = {}  # header path -> names of the functions it declares (with a prototype or not)

    def declared_in(hpath):
        if hpath not in hfuncs:
            closure_of([hpath.split("/", 1)[1]] if hpath.startswith("include/") else [hpath])
            text = cache.get(hpath) or (heads[hpath]["text"] if hpath in heads else "")
            hfuncs[hpath] = {it["fn"]["name"] for it in _hdr_items(text) if it["kind"] == "decl" and it["fn"]}
        return hfuncs[hpath]

    # the modules and the functions each header will declare
    mods = {}
    for rel in sorted(files):
        mod, tlb = _hdr_module(rel)
        if mod is None:
            continue
        path, inc = _hdr_header_of(mod, tlb)
        m = mods.setdefault(mod, {"path": path, "inc": inc, "tlb": tlb, "files": [], "funcs": [],
                                  "types": [], "exists": path in heads, "hincs": []})
        m["files"].append(rel)
    mods_by_path = {m["path"]: m for m in mods.values()}
    refs = {}
    for rel in files:
        refs[rel] = {n for n in uses[rel] if n in defs and defs[n][0] != rel}
    exported = set()  # TLB functions another file uses
    for rel in files:
        mod, tlb = _hdr_module(rel)
        for n in refs[rel]:
            dm, dt = _hdr_module(defs[n][0])
            if dt and (dm != mod or not tlb or defs[n][0] != rel):
                exported.add(n)
    for name, (rel, it) in sorted(defs.items(), key=lambda kv: (kv[1][0], kv[1][1]["start"])):
        mod, tlb = _hdr_module(rel)
        if (tlb and name not in exported) or name in P["omit"]:
            continue
        if not tlb and name in hdecl:  # declared once already, in a header: left there
            h, hit = hdecl[name]
            if _hdr_types(hit["fn"]) != _hdr_types(it["fn"]):
                P["notes"].append(f"{name}: {h} declares `{hit['fn']['sig']}`, {rel} defines `{it['fn']['sig']}`")
            continue
        mods[mod]["funcs"].append((name, it["fn"]["sig"] + ";", rel, it))

    # the types each header's signatures need: known through its includes, or moved in from the
    # defining file when every other definition of that name (in src/, include/, and for a
    # header tlb.h includes, tlb.h and the TLB files) is the same text
    def views(t, path):
        """{definition text: [files]} of type t where a file that may include the header at path
        has it: every main file and include/ header; a TLB file only for a header tlb.h includes
        (and tlb.h itself then) or for its own archive file's."""
        out = collections.defaultdict(list)
        arch = re.match(r"include/tlb/(tlb_\d{4})\.h$", path)
        for rel, td in tdefs.items():
            if t not in td:
                continue
            if rel.startswith("src/tlb/") and path not in TLB_HEADER_SET and not (
                    arch and _hdr_module(rel)[0] == arch.group(1)):
                continue
            out[td[t][1]].append(rel)
        for hrel, td in htdefs.items():
            if t in td and (hrel != "include/tlb.h" or path in TLB_HEADER_SET):
                out[td[t][1]].append(hrel)
        return out

    P["typecopies"] = {}  # file -> {type: header}: its identical copy goes where that header is seen
    tlbh_drop_types = set()
    tlb_types = known_types(["tlb.h"]) | (_hdr_typenames(heads["include/tlb.h"]["text"])
                                          if "include/tlb.h" in heads else set())
    home = {}  # a type moved into a header -> that header (each type moves once; a header needing
    # one that an earlier header holds includes that header, so the includes never form a cycle)
    header_known = {}
    for mod, m in sorted(mods.items()):
        if not m["funcs"]:
            continue
        first = m["files"][0]
        if m["exists"]:
            base_incs = heads[m["path"]]["includes"]
        elif m["tlb"]:
            base_incs = ["common.h"]
        else:
            base_incs = [i for i in files[first]["includes"] if not i.endswith(".inc.c")]
        known = known_types(base_incs)
        if m["exists"]:
            known |= _hdr_typenames(heads[m["path"]]["text"])
        moved, keep = {}, []
        for name, sig, rel, it in m["funcs"]:
            need, why, todo = {}, None, list(_hdr_sig_types(it["fn"]))
            while todo and not why:
                t = todo.pop()
                if t in known or t in moved or t in need:
                    continue
                if m["tlb"] and t in tlb_types:
                    # an archive file's header takes it where its parts do: the first of tlb.h's
                    # headers that has it, else tlb.h (never a main module's header)
                    inc = next((i for i in TLB_HEADERS if t in known_types([i])), "tlb.h")
                    if inc not in m["hincs"]:
                        m["hincs"].append(inc)
                    known |= known_types([inc])
                    continue
                if t in home and home[t] != m["path"] and mods_by_path[home[t]]["tlb"] == m["tlb"]:
                    # an earlier header holds it
                    inc = mods_by_path[home[t]]["inc"]
                    if inc not in m["hincs"]:
                        m["hincs"].append(inc)
                    known |= header_known[home[t]]
                    continue
                td = tdefs[rel].get(t)
                hx = next((h for h, d in sorted(htdefs.items()) if h != "include/tlb.h" and h != m["path"]
                           and t in d and td is not None and d[t][1] == td[1]), None)
                if hx is not None:  # a header has this very type already: that header is included
                    inc = hx.split("/", 1)[1]
                    if inc not in m["hincs"]:
                        m["hincs"].append(inc)
                    known |= known_types([inc])
                    continue
                if td is None:
                    # a header the file includes has it; for an archive file's header, the one of
                    # tlb.h's headers that has it (tlb.h itself only for a type it alone has: a
                    # header that brings tlb.h, and its far calls, is for the TLB files only)
                    incs = (TLB_HEADERS + files[rel]["includes"]) if m["tlb"] else files[rel]["includes"]
                    src_known = known_types(incs)
                    if t in src_known:
                        inc = next(i for i in incs if t in known_types([i]))
                        if inc not in m["hincs"] and inc != m["inc"]:
                            m["hincs"].append(inc)
                        known |= known_types([inc])
                        continue
                    why = f"type {t} is defined neither in {rel} nor a header it includes"
                    break
                v = views(t, m["path"])
                if len(v) > 1:
                    why = (f"no header can declare it: its type {t} is defined {len(v)} different ways ("
                           + ", ".join(os.path.basename(x[0]) + (f" +{len(x) - 1}" if len(x) > 1 else "")
                                       for x in v.values()) + ")")
                    break
                need[t] = (rel, td[0])
                body_ids = set(re.findall(r"[A-Za-z_]\w*", td[1])) - HDR_KEYWORDS - {t}
                todo += [x for x in body_ids if (x in tdefs[rel] or x in home) and x not in known]
                fmac = file_macros(rel)
                for x in sorted(body_ids & set(fmac)):
                    if len(fmac[x]) != 1:
                        why = f"type {t} uses the macro {x}, which {rel} defines {len(fmac[x])} times"
                    else:
                        need["#" + x] = (rel, fmac[x][0])
            if why:
                P["omit"][name] = why
                continue
            for t, v in need.items():
                moved[t] = v
            keep.append((name, sig, rel, it))
        m["funcs"] = keep
        # moved types in their file's order, each with the macros its body uses from that file
        for t, (rel, it) in sorted(moved.items(), key=lambda kv: (kv[1][0], kv[1][1]["start"])):
            home[t] = m["path"]
            if any(x[1] is it for x in m["types"]):
                continue
            m["types"].append((t, it, rel))
        header_known[m["path"]] = known | {t for t in moved if not t.startswith("#")}
        for t, it, rel in m["types"]:
            if t.startswith("#"):
                continue
            if "include/tlb.h" in views(t, m["path"]).get(tdefs[rel][t][1], []):
                tlbh_drop_types.add(t)  # tlb.h's copy of a type its header now holds
    P["tlbh_drop_types"] = tlbh_drop_types

    # tlb.h: a header it includes may now declare a function tlb.h declares too
    P["tlbh_drop"], P["tlbh_add"] = set(), {}
    for mod, m in mods.items():
        if m["path"] not in TLB_HEADER_SET:
            continue
        keep = []
        for name, sig, rel, it in m["funcs"]:
            t = tlb_decl.get(name)
            if t is not None and _hdr_types(t["fn"]) != _hdr_types(it["fn"]):
                P["omit"][name] = f"tlb.h, which includes {m['path']}, declares it `{t['fn']['sig']}` for the TLB files"
                continue
            if t is not None:
                P["tlbh_drop"].add(name)
            keep.append((name, sig, rel, it))
        m["funcs"] = keep
    # every header's #include names after the plan (a module header's: its base and the headers
    # holding the types it takes from them)
    P["hdr_incs"] = {p: list(h["includes"]) for p, h in heads.items()}
    for mod, m in mods.items():
        if not m["funcs"]:
            continue
        if m["exists"]:
            P["hdr_incs"][m["path"]] = list(heads[m["path"]]["includes"]) + m["hincs"]
        else:
            P["hdr_incs"][m["path"]] = (["common.h"] if m["tlb"] else [
                i for i in files[m["files"][0]]["includes"] if not i.endswith(".inc.c")]) + m["hincs"]
    decl_header = {}  # function -> the header that declares it after the plan
    for name, (h, it) in hdecl.items():
        if name in defs:
            decl_header[name] = h
    for mod, m in mods.items():
        for name, sig, rel, it in m["funcs"]:
            decl_header[name] = m["path"]
    P["mods"], P["decl_header"] = mods, decl_header

    def graph_closure(paths):
        """The headers (tree paths) the given ones reach through their #includes after the plan."""
        seen, todo = set(), list(paths)
        while todo:
            h = todo.pop()
            if h not in seen:
                seen.add(h)
                todo += ["include/" + i for i in P["hdr_incs"].get(h, [])]
        return seen

    def header_types(h, moved_only=False):
        """{type: definition text} a header has after the plan (moved_only: the types moved in)."""
        out = {} if moved_only else {t: body for t, (it, body) in htdefs.get(h, {}).items()
                                     if not (h == "include/tlb.h" and t in tlbh_drop_types)}
        m = mods_by_path.get(h)
        for t, it, rel in (m["types"] if m and m["funcs"] else []):
            if not t.startswith("#"):
                out[t] = tdefs[rel][t][1]
        return out

    # per file: the headers to include and the prototypes to remove
    tlb_known = known_types(["tlb.h"])
    P["edits"] = {}
    for rel, f in sorted(files.items()):
        mod, tlb = _hdr_module(rel)
        old_closure = set(closure_of(f["includes"]))
        add = []
        names = set(refs[rel])
        for it in f["items"]:
            if it["kind"] == "decl" and it["fn"] and not it["fn"]["static"]:
                names.add(it["fn"]["name"])
        own = mods[mod]["path"] if mod and mods[mod]["funcs"] else None
        if own and own not in old_closure and (not tlb or any(
                x[2] == rel or x[0] in names for x in mods[mod]["funcs"])):
            add.append(own)  # its own header (an archive file's: when it defines or uses one there)
        for n in sorted(names):
            h = decl_header.get(n)
            if not h or n not in defs:
                continue
            dm, dt = _hdr_module(defs[n][0])
            if tlb and not dt:
                continue  # a TLB file calls main through tlb.h
            if not tlb and h == "include/tlb.h":
                continue
            if h not in old_closure and h not in add:
                add.append(h)
        # the types the headers bring that this file defines too: an identical copy goes, a
        # different one keeps the header that brings it out of this file
        old_seen = graph_closure(["include/" + i for i in f["includes"]])
        dropped = {}
        if not tlb:  # a main file never sees tlb.h: its #define f (*f) would make main's calls far
            for a in [a for a in add if "include/tlb.h" in graph_closure([a])]:
                add.remove(a)
                dropped[a] = "it brings tlb.h, whose far-call macros are for the TLB files"
        while True:
            new_seen = graph_closure(old_seen | set(add))
            copies, clash = {}, {}
            for h in sorted(new_seen):
                for t, body in header_types(h, moved_only=h in old_seen).items():
                    if t in tdefs[rel]:
                        (copies if tdefs[rel][t][1] == body else clash)[t] = h
            gone = False
            for t, h in sorted(clash.items()):
                for a in [a for a in add if h in graph_closure([a])]:
                    add.remove(a)
                    dropped[a] = f"this file's own {t} is not {h.split('/', 1)[1]}'s"
                    gone = True
                if h in old_seen:
                    P["notes"].append(f"{rel}: its own {t} clashes with {h}'s, which it includes already")
            if not gone:
                break
        P["typecopies"][rel] = copies
        new_closure = set(closure_of(f["includes"] + [h.split("/", 1)[1] for h in add])) | graph_closure(
            old_seen | set(add))
        remove, keepers = [], []
        for it in f["items"]:
            fn = it["fn"]
            if it["kind"] != "decl" or not fn or fn["static"]:
                continue
            n = fn["name"]
            if n in statics[rel]:
                continue  # a static's forward declaration
            if n in defs and defs[n][0] == rel and decl_header.get(n) not in new_closure:
                continue  # a forward declaration of its own function that no header has: fine
            covered, view = None, None
            if n in defs:
                dm, dt = _hdr_module(defs[n][0])
                if tlb and not dt:
                    if n in tlb_decl:
                        covered, view = "include/tlb.h", tlb_decl[n]["fn"]
                    elif not (_hdr_sig_types(defs[n][1]["fn"]) - tlb_known):
                        # tlb.h makes it a far call without a prototype, or does not have it:
                        # it gets both, from the definition
                        covered, view = "include/tlb.h", defs[n][1]["fn"]
                        P["tlbh_add"][n] = (defs[n][1]["fn"]["sig"] + ";", n not in far)
                    else:
                        keepers.append((it, "tlb.h cannot declare it: its types are main's (%s)" % ", ".join(
                            sorted(_hdr_sig_types(defs[n][1]["fn"]) - tlb_known))))
                        continue
                elif decl_header.get(n) in new_closure:
                    covered, view = decl_header[n], defs[n][1]["fn"]
                    if decl_header[n] != P["mods"].get(dm, {}).get("path"):
                        view = hdecl[n][1]["fn"]
                elif decl_header.get(n) in dropped:
                    keepers.append((it, f"{decl_header[n].split('/', 1)[1]} is not included here: "
                                        + dropped[decl_header[n]]))
                    continue
                else:
                    keepers.append((it, P["omit"].get(n, "no header declares it")))
                    continue
            else:  # outside src/: libultra's headers, or include/
                for h in new_closure:
                    if n in declared_in(h):
                        covered = h
                        hit = next(x for x in _hdr_items(cache.get(h) or heads[h]["text"])
                                   if x["kind"] == "decl" and x["fn"] and x["fn"]["name"] == n)
                        view = hit["fn"]
                        break
                if not covered:
                    keepers.append((it, "defined outside src/ and declared by no header this file includes"))
                    continue
            remove.append((it, covered, _hdr_types(fn) != _hdr_types(view), view["sig"]))
        P["edits"][rel] = {"add": add, "remove": remove, "keep": keepers}
    return P


def _hdr_render_header(P, mod):
    """The text of a module's header after the plan (None when it has nothing to declare)."""
    m = P["mods"][mod]
    files, heads = P["files"], P["heads"]
    if not m["funcs"]:
        return None
    blocks = []
    for t, it, rel in m["types"]:
        text = files[rel]["text"]
        b2 = _hdr_blank(text, directives=True)
        s, e = _hdr_span(text, b2, it["start"], it["end"])
        s = _hdr_lead(text, b2, s)
        blocks.append(text[s:e].rstrip("\n"))
    protos = [sig for name, sig, rel, it in m["funcs"]]
    if m["exists"]:
        text = heads[m["path"]]["text"]
        have = {it["fn"]["name"] for it in heads[m["path"]]["items"] if it["kind"] == "decl" and it["fn"]}
        protos = [p for (name, *_), p in zip(m["funcs"], protos) if name not in have]
        add = ""
        incs = [i for i in m["hincs"] if i not in heads[m["path"]]["includes"]]
        if incs:  # after the header's last #include
            last = list(re.finditer(r'^#include\s+["<][^">]+[">].*\n', text, re.M))
            at = last[-1].end() if last else text.find("\n", text.find("#define")) + 1
            text = text[:at] + "".join(f'#include "{i}"\n' for i in incs) + text[at:]
        if blocks:
            add += "\n" + "\n\n".join(blocks) + "\n"
        if protos:
            add += "\n" + "\n".join(protos) + "\n"
        end = text.rfind("#endif")
        return text[:end].rstrip("\n") + "\n" + add + "\n" + text[end:]
    guard = re.sub(r"\W", "_", m["inc"]).upper()
    if m["tlb"]:
        incs = ["common.h"] + m["hincs"]
    else:
        incs = [i for i in files[m["files"][0]]["includes"] if not i.endswith(".inc.c")] + m["hincs"]
    out = [f"#ifndef {guard}", f"#define {guard}", ""]
    out += [f'#include "{i}"' if not i.startswith("<") else f"#include {i}" for i in incs] + [""]
    if blocks:
        out += ["\n\n".join(blocks), ""]
    out += protos + ["", "#endif", ""]
    return "\n".join(out)


def _hdr_render_file(P, rel, exclude=None):
    """A C file's text after the plan. exclude: {header: why} left out here, the prototypes and
    type copies they would have replaced kept. Each prototype kept of a function defined in
    another file gets a FAKE mark with its reason (report_progress counts them)."""
    exclude = exclude or {}
    f = P["files"][rel]
    text = f["text"]
    b2 = _hdr_blank(text, directives=True)
    e = P["edits"][rel]
    cuts = []
    marks = []  # (offset just past a kept prototype's ';', reason)
    for it, why in e["keep"]:
        marks.append((it["end"], why))
    for it, covered, differ, view in e["remove"]:
        own_fn = "fn:" + it["fn"]["name"]
        if own_fn in exclude:  # this one prototype stays (its header is one the file cannot leave)
            marks.append((it["end"], exclude[own_fn]))
            continue
        if covered in exclude:
            if P["defs"].get(it["fn"]["name"], ("",))[0] != rel:
                why = exclude[covered]
                m = re.match(r"(\w+): ", why)
                if m and m.group(1) != it["fn"]["name"]:  # left out for another function's sake
                    why = f"see {m.group(1)}"
                marks.append((it["end"], f"{covered.split('/', 1)[1]} is not included here: {why}"))
            continue
        s, t = _hdr_span(text, b2, it["start"], it["end"])
        n = it["fn"]["name"]
        if covered == "include/tlb.h":
            # a TLB file's #undef/#define around its prototype of a far call into main: tlb.h
            # has both now (a far call within the archive file keeps its #define)
            ps = text.rfind("\n", 0, s - 1) + 1 if s > 0 else 0
            if s > 0 and re.fullmatch(r"#undef\s+%s\s*" % re.escape(n), text[ps:s - 1]):
                s = ps
            ne = text.find("\n", t)
            if re.fullmatch(r"#define\s+%s\s+\(\*%s\)\s*(?:/\*.*\*/\s*)?" % (re.escape(n), re.escape(n)),
                            text[t:ne if ne >= 0 else len(text)]):
                t = ne + 1 if ne >= 0 else len(text)
        cuts.append((s, t))
    mod, tlb = _hdr_module(rel)
    own = P["mods"].get(mod, {})
    # the headers this file will see: its includes and the ones it gains, and theirs
    seen, todo = set(), ["include/" + i for i in f["includes"]] + [h for h in e["add"] if h not in exclude]
    while todo:
        h = todo.pop()
        if h in seen:
            continue
        seen.add(h)
        todo += ["include/" + i for i in P["hdr_incs"].get(h, [])]
    for t_name, hpath in P["typecopies"].get(rel, {}).items():
        if hpath not in seen:
            continue  # its identical copy stays: the header that now holds the type is not seen here
        it = _hdr_typedefs(text, f["items"])[t_name][0]
        s, t = _hdr_span(text, b2, it["start"], it["end"])
        cuts.append((_hdr_lead(text, b2, s), t))
    if own.get("funcs") and own["path"] in seen:  # the types moved out of this file
        for t_name, it, trel in own["types"]:
            if trel == rel:
                s, t = _hdr_span(text, b2, it["start"], it["end"])
                cuts.append((_hdr_lead(text, b2, s), t))
    # a comment that headed only removed lines goes with them
    cuts = sorted(set(cuts))
    lines = _hdr_lines(text)
    cut_line = set()
    for s, t in cuts:
        for k, (a, z) in enumerate(lines):
            if a >= s and z <= t + (0 if t < len(text) else 1):
                cut_line.add(k)
    for k, (a, z) in enumerate(lines):
        if k in cut_line or not text[a:z].strip() or b2[a:z].strip() or text[a:z].lstrip().startswith("#"):
            continue
        if k > 0 and lines[k - 1][0] < lines[k - 1][1] and text[lines[k - 1][0]:lines[k - 1][1]].strip() and \
                k - 1 not in cut_line:
            continue  # not the first line of its block
        j = k
        while j < len(lines) and text[lines[j][0]:lines[j][1]].strip() and not b2[lines[j][0]:lines[j][1]].strip() \
                and not text[lines[j][0]:lines[j][1]].lstrip().startswith("#"):
            j += 1  # the comment's lines
        group = []
        while j < len(lines) and text[lines[j][0]:lines[j][1]].strip():
            group.append(j)
            j += 1
        if group and all(g in cut_line for g in group):
            cuts.append((a, lines[group[0]][0]))
    out, at = [], 0
    edits = sorted([(s, t, "") for s, t in cuts] + [(m, m, " /* FAKE: %s */" % why.replace("*/", "* /"))
                                                     for m, why in marks])
    for s, t, ins in edits:
        if s < at:
            s = at
        if t < s or (t == s and not ins):
            continue
        out.append(text[at:s] + ins)
        if not ins and t < len(text) and text[t] == "\n" and "".join(out[-2:]).endswith("\n\n"):
            t += 1  # a cut between two blank lines leaves one
        at = t
    out.append(text[at:])
    text = "".join(out)
    # the includes, after the file's first block of #include lines
    add = [h for h in e["add"] if h not in exclude]
    if add:
        names = [h.split("/", 1)[1] for h in add]
        incs = list(re.finditer(r'^#include\s+"[^"]+\.h".*\n', text, re.M))
        block_end = None
        for m_ in incs:
            if block_end is None or m_.start() == block_end:
                block_end = m_.end()
            else:
                break
        at = block_end if block_end is not None else 0
        text = text[:at] + "".join(f'#include "{n}"\n' for n in sorted(names)) + text[at:]
    return text


def _hdr_render_tlbh(P):
    """include/tlb.h after the plan: the prototypes it gains (far calls it made without one), the
    ones a header it includes now declares as it does, and the type copies such a header now
    holds, taken out. Its other declarations stay as they are."""
    h = P["heads"].get("include/tlb.h")
    if not h:
        return None
    text = h["text"]
    b2 = _hdr_blank(text, directives=True)
    cuts = []
    for it in h["items"]:
        if it["kind"] == "decl" and it["fn"] and it["fn"]["name"] in P["tlbh_drop"]:
            cuts.append(_hdr_span(text, b2, it["start"], it["end"]))
    for t, (it, body) in _hdr_typedefs(text, h["items"]).items():
        if t in P["tlbh_drop_types"]:
            cuts.append(_hdr_span(text, b2, it["start"], it["end"]))
    out, at = [], 0
    for s, t in sorted(set(cuts)):
        out.append(text[at:max(s, at)])
        at = max(t, at)
    out.append(text[at:])
    text = "".join(out)
    if P["tlbh_add"]:
        # each in name order among the prototypes, and its far-call macro among the macros, as
        # tlbh writes them
        lines = text.split("\n")
        first = next(i for i, l in enumerate(lines) if l.startswith("/* Main's and the overlays' functions."))
        for name, (sig, define) in sorted(P["tlbh_add"].items()):
            last = next(i for i, l in enumerate(lines) if re.match(r"#define\s+(\w+)\s+\(\*\1\)", l)) - 1
            at = next((i for i in range(first + 1, last)
                       if re.match(r"[A-Za-z_][\w\s*]*?\b(\w+)\s*\(", lines[i])
                       and re.match(r"[A-Za-z_][\w\s*]*?\b(\w+)\s*\(", lines[i]).group(1) > name), last)
            lines.insert(at, sig)
            if define:
                macros = [i for i, l in enumerate(lines) if re.match(r"#define\s+(\w+)\s+\(\*\1\)", l)]
                at = next((i for i in macros if re.match(r"#define\s+(\w+)", lines[i]).group(1) > name),
                          macros[-1] + 1)
                lines.insert(at, f"#define {name} (*{name})")
        text = "\n".join(lines)
    return text


def _hdr_objkey(path):
    """An object's code and data for comparison: .text/.data/.rodata bytes, their relocations
    (offset, type, symbol name or section), .bss size, and each defined symbol's section and
    value. Two objects with equal keys link to the same bytes."""
    import struct
    with open(path, "rb") as f:
        d = f.read()
    shoff, = struct.unpack_from(">I", d, 0x20)
    shnum, shstrndx = struct.unpack_from(">HH", d, 0x30)
    secs = [struct.unpack_from(">IIIIIIIIII", d, shoff + i * 40) for i in range(shnum)]
    nm = lambda off, base: d[base + off:d.index(b"\0", base + off)].decode(errors="replace")
    names = [nm(s[0], secs[shstrndx][4]) for s in secs]
    syms = []
    for i, s in enumerate(secs):
        if s[1] == 2:
            for k in range(s[5] // 16):
                st, val, size, info, other, shndx = struct.unpack_from(">IIIBBH", d, s[4] + k * 16)
                syms.append((nm(st, secs[s[6]][4]), val, shndx, info & 15))
    key = {}
    for i, s in enumerate(secs):
        if names[i] in (".text", ".data", ".rodata"):
            key[names[i]] = d[s[4]:s[4] + s[5]] if s[1] != 8 else s[5]
        if names[i] == ".bss":
            key[".bss"] = s[5]
    for i, s in enumerate(secs):
        if s[1] == 9 and names[s[7]] in (".text", ".data", ".rodata"):
            rel = []
            for k in range(s[5] // 8):
                off, info = struct.unpack_from(">II", d, s[4] + k * 8)
                n, val, shndx, typ = syms[info >> 8]
                rel.append((off, info & 0xFF, names[shndx] if typ == 3 else n))
            key["rel" + names[s[7]]] = sorted(rel)
    key["syms"] = sorted((n, names[shndx] if shndx < len(names) else shndx, val)
                         for n, val, shndx, typ in syms if n and shndx != 0 and typ in (0, 1, 2))
    return key


def _hdr_compile_all(root, texts, out_dir, jobs=2):
    """Each text compiled as the build does, from a copy at out_dir/<key>/<rel> (so the tree is not
    touched) with cwd root. texts: {rel: text} or {(key, rel): text}. Returns {same key: (object
    path or None, compiler output)}. Two at a time, below normal priority, no console window."""
    import concurrent.futures
    flags = 0x00004000 | 0x08000000 if os.name == "nt" else 0

    def one(item):
        k, text = item
        sub, rel = k if isinstance(k, tuple) else ("", k)
        src = os.path.join(out_dir, sub, rel)
        obj = src[:-2] + ".o"
        os.makedirs(os.path.dirname(src), exist_ok=True)
        _write_retry(src, text)
        if os.path.exists(obj):
            os.remove(obj)
        cmd = build.compile_c(fwd(src), fwd(obj), extra=["-I", os.path.dirname(rel)])
        r = subprocess.run(cmd, cwd=root, capture_output=True, text=True, env=build.ENV,
                           creationflags=flags, encoding="utf-8", errors="replace")
        return k, (obj if r.returncode == 0 and os.path.exists(obj) else None), r.stdout + r.stderr
    out = {}
    with concurrent.futures.ThreadPoolExecutor(jobs) as pool:
        for k, obj, log in pool.map(one, sorted(texts.items(), key=lambda kv: str(kv[0]))):
            out[k] = (obj, log)
    return out


def _hdr_errors(log, detail=False):
    """A compile's error lines, short: "line N: message" each; with detail, (file, line, message)."""
    out = []
    for line in log.splitlines():
        m = re.search(r"cfe: Error: (.*?), line (\d+): (.*)$", line)
        if m:
            msg = m.group(3).split(";")[0]
            out.append((m.group(1).replace("\\", "/"), int(m.group(2)), msg) if detail
                       else f"line {m.group(2)}: {msg}")
    return out


def headers(apply=False, root=None):
    """The headers mode: each function declared once, in a header, as sm64 does. A dry run writes the plan
    to permuter/headers/headers.txt; apply writes the headers and the C files, and proves each
    object identical to its compile before (a file that is not gets back the prototypes of the
    headers that change it, marked FAKE)."""
    import collections
    root = root or build.ROOT
    out_dir = os.path.join(root, "permuter", "headers")
    os.makedirs(out_dir, exist_ok=True)
    report, not_identical = [], []
    exclude = collections.defaultdict(dict)  # file -> {header it does not include: why}
    force_omit = {}  # function -> why no header declares it (a file includes its header already)

    def plan():
        P = _hdr_plan(root, force_omit)
        new_h = {m["path"]: _hdr_render_header(P, mod) for mod, m in P["mods"].items() if m["funcs"]}
        new_h = {p: t for p, t in new_h.items() if t is not None}
        tlbh = _hdr_render_tlbh(P)
        if tlbh is not None and tlbh != P["heads"]["include/tlb.h"]["text"]:
            new_h["include/tlb.h"] = tlbh
        return P, new_h
    P, new_h = plan()
    n_remove = sum(len(e["remove"]) for e in P["edits"].values())
    n_differ = sum(1 for e in P["edits"].values() for x in e["remove"] if x[2])
    print(f"headers: {sum(1 for p in new_h if p not in P['heads'])} new, "
          f"{sum(1 for p in new_h if p in P['heads'])} extended; {n_remove} local prototypes to go "
          f"({n_differ} with other types than the header's); {len(P['omit'])} functions no header can "
          f"declare; {len(P['tlbh_add'])} prototypes tlb.h gains, {len(P['tlbh_drop'])} it loses")
    if apply:
        texts0 = {rel: f["text"] for rel, f in P["files"].items()}
        heads0 = {p: h["text"] for p, h in P["heads"].items()}
        before = _hdr_compile_all(root, texts0, os.path.join(out_dir, "before"))
        bad0 = [rel for rel, (obj, log) in before.items() if obj is None]
        if bad0:
            print(f"headers: {len(bad0)} file(s) do not compile before any change: {bad0[:5]}; nothing written")
            return 1
        keys0 = {rel: _hdr_objkey(obj) for rel, (obj, log) in before.items()}

        def write_headers(new_h):
            for p, t in new_h.items():
                os.makedirs(os.path.dirname(os.path.join(root, p)), exist_ok=True)
                _write_retry(os.path.join(root, p), t)
            for p in list(written):  # a header written before and not now: back as it was
                if p not in new_h:
                    if p in heads0:
                        _write_retry(os.path.join(root, p), heads0[p])
                    else:
                        os.remove(os.path.join(root, p))
                    written.discard(p)
            written.update(new_h)
        written = set()
        write_headers(new_h)

        def verdict(rel, res):
            obj, log = res
            if obj is None:
                errs = _hdr_errors(log)
                return "it does not compile (%s)" % ("; ".join(errs[:2]) or log.strip()[-200:])
            return None if _hdr_objkey(obj) == keys0[rel] else "the object differs"

        def culprits(rel, res, text):
            """The removed prototypes this file's failure points at: on an error's line, or (the
            object differs) those whose types differ from the header's in a way a call can
            compile differently (_hdr_matters). [(remove entry, why)]"""
            obj, log = res
            rem = [x for x in P["edits"][rel]["remove"] if "fn:" + x[0]["fn"]["name"] not in exclude[rel]
                   and x[1] not in exclude[rel]]
            out = []
            if obj is None:
                lines = text.split("\n")
                for path, n, msg in _hdr_errors(log, detail=True):
                    if path.endswith(rel.split("/")[-1]) and 0 < n <= len(lines):
                        ids = set(re.findall(r"[A-Za-z_]\w*", lines[n - 1]))
                    else:
                        ids = set(re.findall(r"'(\w+)'", msg))
                    for x in rem:
                        if x[0]["fn"]["name"] in ids and x not in [o for o, _ in out]:
                            out.append((x, f"used here as this file declares it ({msg}); "
                                           f"{x[1].split('/', 1)[1]} has `{x[3]}`"))
            else:
                for x in rem:
                    view = _hdr_func(x[3])
                    if x[2] and view and _hdr_matters(x[0]["fn"], view, P["typedefs"]):
                        out.append((x, f"the game's code calls it as this file declares it; "
                                       f"{x[1].split('/', 1)[1]} has `{x[3]}`"))
            return out

        def act(rel, entry, because, omit_now):
            """Keep one culprit prototype out of a header's reach: the file does not include a
            header it gained; a prototype tlb.h covers stays in the file; a function whose header
            the file included already is left out of that header. True when something changed."""
            (it, covered, differ, view) = entry
            name = it["fn"]["name"]
            module_header = P["decl_header"].get(name) in [m["path"] for m in P["mods"].values()]
            # leaving the header out of this file keeps all its prototypes here; leaving the
            # function out of its header keeps its prototype in every file that has one: the
            # fewer kept wins
            keep_here = sum(1 for x in P["edits"][rel]["remove"] if x[1] == covered)
            keep_all = sum(1 for e in P["edits"].values() for x in e["remove"] + e["keep"]
                           if x[0]["fn"]["name"] == name)
            if covered in P["edits"][rel]["add"] and not (module_header and name not in force_omit
                                                          and keep_all < keep_here):
                if covered in exclude[rel]:
                    return False
                exclude[rel][covered] = f"{name}: {because}"
                report.append(f"EXCLUDE {rel}: {covered}: {exclude[rel][covered]}")
            elif covered in P["edits"][rel]["add"]:
                omit_now[name] = f"in no header: {because} ({rel})"
                report.append(f"OMIT {name}: {omit_now[name]}")
            elif covered == "include/tlb.h" or name not in P["defs"]:
                exclude[rel]["fn:" + name] = f"not from {covered.split('/', 1)[1]}: {because}"
                report.append(f"KEEP {rel}: {name}: {exclude[rel]['fn:' + name]}")
            elif name not in force_omit and P["decl_header"].get(name) in [m["path"] for m in P["mods"].values()]:
                omit_now[name] = f"in no header: {rel} includes {covered.split('/', 1)[1]} already and {because}"
                report.append(f"OMIT {name}: {omit_now[name]}")
            else:
                return False
            return True

        todo = sorted(P["files"])
        unresolved = {}
        for rnd in range(16):
            got = _hdr_compile_all(root, {rel: _hdr_render_file(P, rel, exclude[rel]) for rel in todo},
                                   os.path.join(out_dir, "after"))
            bad = {rel: w for rel, w in ((rel, verdict(rel, got[rel])) for rel in todo) if w}
            print(f"headers: round {rnd + 1}: {len(todo)} compiled, {len(bad)} not identical", flush=True)
            if not bad:
                break
            nxt, omit_now, trials, found = [], {}, {}, {}
            for rel, why in sorted(bad.items()):
                found[rel] = culprits(rel, got[rel], _hdr_render_file(P, rel, exclude[rel]))
                if got[rel][0] is None:
                    continue  # a compile error names its culprit: no trial needed
                # the object differs: each gained header that holds a culprit (or, with none,
                # every gained header) left out alone; the first that makes it identical is the one
                hs = {x[1] for x, _ in found[rel] if x[1] in P["edits"][rel]["add"]} or \
                    set(P["edits"][rel]["add"])
                for h in sorted(hs - set(exclude[rel])):
                    trials[(re.sub(r"\W", "_", h), rel)] = _hdr_render_file(P, rel, {**exclude[rel], h: "?"})
            tried = _hdr_compile_all(root, trials, os.path.join(out_dir, "trial")) if trials else {}
            for rel, why in sorted(bad.items()):
                ok = [k[0] for k in tried if k[1] == rel and verdict(rel, tried[k]) is None]
                if ok:
                    h = next(h for h in P["edits"][rel]["add"] if re.sub(r"\W", "_", h) == ok[0])
                    named = [(x, b) for x, b in found[rel] if x[1] == h]
                    one = [x for x in P["edits"][rel]["remove"] if x[1] == h and x[2]]
                    if not named and len(one) == 1:  # the one prototype here that differs from h's
                        named = [(one[0], f"the game's code calls it as this file declares it; "
                                          f"{h.split('/', 1)[1]} has `{one[0][3]}`")]
                    if named:
                        for x, b in named:
                            act(rel, x, b, omit_now)
                    else:
                        diffs = [x for x in P["edits"][rel]["remove"] if x[1] == h and x[2]]
                        exclude[rel][h] = f"with it {why}" + (
                            f"; here `{diffs[0][0]['fn']['sig']}`, there `{diffs[0][3]}`"
                            + (f" (and {len(diffs) - 1} more)" if len(diffs) > 1 else "") if diffs else "")
                        report.append(f"EXCLUDE {rel}: {h}: {exclude[rel][h]}")
                    continue
                acted = False
                for x, because in found[rel]:
                    acted = act(rel, x, because, omit_now) or acted
                if not acted:
                    cands = [h for h in P["edits"][rel]["add"] if h not in exclude[rel]]
                    if cands:  # nothing named: the last gained header goes, and the next round sees
                        exclude[rel][cands[-1]] = f"with it {why}"
                        report.append(f"EXCLUDE {rel}: {cands[-1]}: with it {why} (not alone enough)")
                        acted = True
                if acted:
                    nxt.append(rel)
                else:
                    unresolved[rel] = why
                    report.append(f"UNRESOLVED {rel}: {why}")
            if omit_now:  # a header changes: plan again (from the headers as they were), compile all
                force_omit.update(omit_now)
                write_headers({})
                P, new_h = plan()
                write_headers(new_h)
                todo = sorted(P["files"])
            else:
                todo = sorted(set(nxt))
            if not todo:
                break
        final = {rel: _hdr_render_file(P, rel, exclude[rel]) for rel in sorted(P["files"])}
        for rel, text in final.items():
            if text != P["files"][rel]["text"]:
                _write_retry(os.path.join(root, rel), text)
        # the proof: every file as written, compiled again, against its object before
        got = _hdr_compile_all(root, final, os.path.join(out_dir, "final"))
        not_identical = sorted(rel for rel in final if verdict(rel, got[rel]))
        print(f"headers: {len(final) - len(not_identical)} of {len(final)} objects identical to before"
              + (f"; NOT: {not_identical}" if not_identical else ""))
        report.append(f"PROOF {len(final) - len(not_identical)} of {len(final)} objects identical to before"
                      + (f"; NOT: {' '.join(not_identical)}" if not_identical else ""))
    # the report
    lines = [f"headers mode on {root}", ""]
    for p, t in sorted(new_h.items()):
        lines.append(f"{'EXTENDED' if p in P['heads'] else 'NEW'} {p}")
    for name, why in sorted(P["omit"].items()):
        lines.append(f"OMIT {name}: {why}")
    for n, (sig, define) in sorted(P["tlbh_add"].items()):
        lines.append(f"TLBH ADD {sig}" + (f" and #define {n} (*{n})" if define else ""))
    for n in sorted(P["tlbh_drop"]):
        lines.append(f"TLBH DROP {n} (its header declares it so)")
    for rel, e in sorted(P["edits"].items()):
        for h in e["add"]:
            lines.append(f"INCLUDE {rel}: {h}" + (f" (left out: {exclude[rel][h]})" if h in exclude[rel] else ""))
        for it, covered, differs, view in e["remove"]:
            kept = covered in exclude[rel] or "fn:" + it["fn"]["name"] in exclude[rel]
            lines.append(f"{'KEEP' if kept else 'REMOVE'} {rel}:{it['line']} {it['fn']['sig']}"
                         + (f"  [{covered} has `{view}`]" if differs else ""))
        for it, why in e["keep"]:
            lines.append(f"KEEP {rel}:{it['line']} {it['fn']['sig']}: {why}")
    lines += P["notes"] + report
    _write_retry(os.path.join(out_dir, "headers.txt"), "\n".join(lines) + "\n")
    print(f"headers: written {os.path.join(out_dir, 'headers.txt')}")
    return 1 if not_identical else 0


ENUM_LITERAL = re.compile(r"[+-]?(?:0[xX][0-9A-Fa-f]+|[1-9][0-9]*|0)[uUlL]*")


def _enum_values(path):
    """An enum header's {value: name}: each `NAME = <integer>,` line (the first name a value has)."""
    with open(path, encoding="utf-8") as f:
        text = _hdr_blank(f.read())
    vals = {}
    for m in re.finditer(r"^\s*([A-Za-z_]\w*)\s*=\s*(" + ENUM_LITERAL.pattern + r")\s*,?\s*$", text, re.M):
        vals.setdefault(int(m.group(2).rstrip("uUlL"), 0), m.group(1))
    return vals


def _arg_spans(blank, a, b):
    """The top-level argument spans [(start, end)] of the call whose parentheses are blank[a]
    and blank[b - 1], each trimmed of white space."""
    out, depth, start = [], 0, a + 1
    for i in range(a + 1, b - 1):
        c = blank[i]
        if c in "([{":
            depth += 1
        elif c in ")]}":
            depth -= 1
        elif c == "," and depth == 0:
            out.append((start, i))
            start = i + 1
    out.append((start, b - 1))
    trimmed = []
    for s, e in out:
        while s < e and blank[s].isspace():
            s += 1
        while e > s and blank[e - 1].isspace():
            e -= 1
        trimmed.append((s, e))
    return trimmed


def enum_args(header, specs, apply=False, root=None):
    """The enum-args mode: each call
    FUNC(...) in root's src/*.c whose argument POS is a bare integer literal gets header's name
    for that value. specs: [(func, pos)]. Dry run: the sites to permuter/enumargs/enumargs.txt.
    apply: the C files written, then every src/*.c compiled and compared with build/src/<file>.o."""
    root = root or build.ROOT
    header_path = header if os.path.isabs(header) else os.path.join(root, header)
    names = _enum_values(header_path)
    if not names:
        print(f"enum-args: no `NAME = value,` lines in {header_path}")
        return 1
    want = dict(specs)
    fields = {f[:-2]: p for f, p in specs if f.endswith("{}")}  # TYPE{}:POS (897)
    alt = "|".join(re.escape(f) for f in want if not f.endswith("{}")) or r"(?!)"
    call = re.compile(r"(?:\b(" + alt + r")|\(\s*\*\s*(" + alt + r")\s*\))\s*\(")  # f( or (*f)( (886)

    def brace_end(blank, i):
        """The index just past the brace that closes the one at blank[i]."""
        depth = 0
        for j in range(i, len(blank)):
            depth += {"{": 1, "}": -1}.get(blank[j], 0)
            if depth == 0:
                return j + 1
        return -1

    def field_sites(blank):
        """(spec, element start, its field spans) for every element of every TYPE array."""
        for typ in fields:
            for m in re.finditer(r"^(?:static\s+)?" + re.escape(typ) + r"\s+\w+\s*\[[^\]]*\]\s*=\s*\{", blank, re.M):
                end = brace_end(blank, m.end() - 1)
                i = m.end()
                while 0 < i < end - 1:
                    if blank[i] == "{":
                        e = brace_end(blank, i)
                        yield typ + "{}", i, _arg_spans(blank, i, e)
                        i = e
                    else:
                        i += 1
    out_dir = os.path.join(root, "permuter", "enumargs")
    os.makedirs(out_dir, exist_ok=True)
    rels = sorted(os.path.relpath(p, root).replace("\\", "/")
                  for p in glob.glob(os.path.join(root, "src", "**", "*.c"), recursive=True))
    counts = {f: {"calls": 0, "named": 0, "already": 0, "unnamed": 0, "other": 0, "decls": 0} for f in want}
    sites, others, unnamed, new_texts = [], [], [], {}
    for rel in rels:
        with open(os.path.join(root, rel), encoding="utf-8") as f:
            text = f.read()
        blank = _hdr_blank(text)
        edits = []
        for m in call.finditer(blank):
            func = m.group(1) or m.group(2)
            pre = blank[:m.start()].rstrip()
            word = re.search(r"(\w+)$", pre)
            if (word and word.group(1) not in ("return", "else", "case", "sizeof")) or \
                    re.search(r"\w\s*\*+$", pre):
                counts[func]["decls"] += 1  # `Obj *eneMakeEx(` or `s32 f(`: not a call
                continue
            end = _balanced(blank, m.end() - 1)
            if end < 0:
                continue
            args = _arg_spans(blank, m.end() - 1, end)
            counts[func]["calls"] += 1
            line = text.count("\n", 0, m.start()) + 1
            where = f"{rel}:{line}"
            pos = want[func]
            if pos >= len(args):
                others.append(f"{where} {func}: no argument {pos}")
                counts[func]["other"] += 1
                continue
            s, e = args[pos]
            arg = text[s:e]
            if arg != blank[s:e] or not ENUM_LITERAL.fullmatch(arg):
                if arg in names.values():
                    counts[func]["already"] += 1
                else:
                    counts[func]["other"] += 1
                    others.append(f"{where} {func}: argument {pos} is `{' '.join(arg.split())}`")
                continue
            value = int(arg.rstrip("uUlL"), 0)
            if value not in names:
                counts[func]["unnamed"] += 1
                unnamed.append(f"{where} {func}: {arg} has no name in {header}")
                continue
            edits.append((s, e, names[value]))
            counts[func]["named"] += 1
            sites.append(f"{where} {func}: {arg} -> {names[value]}")
        for spec, el, args in field_sites(blank):
            counts[spec]["calls"] += 1
            where = f"{rel}:{text.count(chr(10), 0, el) + 1}"
            pos = want[spec]
            if pos >= len(args):
                others.append(f"{where} {spec}: no field {pos}")
                counts[spec]["other"] += 1
                continue
            s, e = args[pos]
            arg = text[s:e]
            if arg != blank[s:e] or not ENUM_LITERAL.fullmatch(arg):
                if arg in names.values():
                    counts[spec]["already"] += 1
                else:
                    counts[spec]["other"] += 1
                    others.append(f"{where} {spec}: field {pos} is `{' '.join(arg.split())}`")
                continue
            value = int(arg.rstrip("uUlL"), 0)
            if value not in names:
                counts[spec]["unnamed"] += 1
                unnamed.append(f"{where} {spec}: {arg} has no name in {header}")
                continue
            edits.append((s, e, names[value]))
            counts[spec]["named"] += 1
            sites.append(f"{where} {spec}: {arg} -> {names[value]}")
        if edits:
            for s, e, name in sorted(edits, reverse=True):
                text = text[:s] + name + text[e:]
            new_texts[rel] = text
    total = sum(c["named"] for c in counts.values())
    head = [f"enum-args on {root}: {header}, " + ", ".join(f"{f}:{p}" for f, p in specs), ""]
    for f, c in counts.items():
        head.append(f"{f}: {c['calls']} calls; {c['named']} bare numbers named, {c['unnamed']} bare numbers "
                    f"with no name, {c['already']} named already, {c['other']} not a bare number "
                    f"({c['decls']} declarations skipped)")
    head.append(f"total: {total} sites in {len(new_texts)} files")
    report = head + ["", "SITES"] + sites + ["", "NO NAME"] + unnamed + ["", "NOT A BARE NUMBER"] + others
    for line in head:
        print(f"enum-args: {line}" if line else "", flush=True)
    rc = 0
    if apply:
        for rel, text in new_texts.items():
            _write_retry(os.path.join(root, rel), text)
        texts = {}
        for rel in rels:
            with open(os.path.join(root, rel), encoding="utf-8") as f:
                texts[rel] = f.read()
        got = _hdr_compile_all(root, texts, os.path.join(out_dir, "after"))
        bad = []
        for rel in rels:
            obj, log = got[rel]
            ref = os.path.join(root, "build", rel[:-2] + ".o")
            if obj is None:
                bad.append(f"{rel} (does not compile: {'; '.join(_hdr_errors(log)[:2]) or log.strip()[-200:]})")
            elif not os.path.exists(ref):
                bad.append(f"{rel} (no {os.path.relpath(ref, root)})")
            elif _hdr_objkey(obj) != _hdr_objkey(ref):
                bad.append(f"{rel} (the object differs from {os.path.relpath(ref, root)})")
        proof = (f"PROOF {len(rels) - len(bad)} of {len(rels)} objects identical to build/src/"
                 f" ({len(new_texts)} files changed)" + (f"; NOT: {' '.join(bad)}" if bad else ""))
        print(f"enum-args: {proof}")
        report += ["", proof]
        rc = 1 if bad else 0
    _write_retry(os.path.join(out_dir, "enumargs.txt"), "\n".join(report) + "\n")
    print(f"enum-args: written {os.path.join(out_dir, 'enumargs.txt')}")
    return rc


def journal(path, out_dir):
    """A JSONL results journal written out readably: one text file per
    result in out_dir, each field as its own section with its real newlines."""
    import json
    os.makedirs(out_dir, exist_ok=True)
    k = 0
    with open(path, encoding="utf-8") as f:
        for line in f:
            rec = json.loads(line)
            r = rec.get("result")
            if rec.get("type") != "result" or not isinstance(r, dict):
                continue
            k += 1
            parts = []
            for key, val in r.items():
                text = val if isinstance(val, str) else json.dumps(val, ensure_ascii=False, indent=1)
                parts.append(f"## {key}\n{text}\n")
            tag = re.sub(r"[^\w.-]+", "_", str(r.get("segment") or r.get("file") or k))[:40]
            with open(os.path.join(out_dir, f"{k:02d}_{tag}.txt"), "w", encoding="utf-8", newline="\n") as g:
                g.write("\n".join(parts))
    print(f"{k} results written to {out_dir}")
    return 0


def _rename_scoped(entries, dry):
    """rename's file-scoped entries (Changelog (854)): `D_XXXXXXX@path name`, the segment address
    replaced in that file only and the name defined at the same address in undefined_syms.txt."""
    defined = set()
    for name in ("symbol_addrs.txt", "undefined_syms.txt"):
        with open(os.path.join(build.ROOT, name), encoding="utf-8") as f:
            defined.update(re.findall(r"^\s*([A-Za-z_]\w*)\s*=", f.read(), re.M))
    addr_of, per_file, problems = {}, {}, []
    for o, n in entries:
        sym, _, rel = o.partition("@")
        m = re.match(r"^D_([0-9A-Fa-f]{7})$", sym)
        path = os.path.join(build.ROOT, rel)
        if not m:
            problems.append(f"{o}: not a 7-digit segment address")
        elif not re.match(r"^[A-Za-z_]\w{0,62}$", n):
            problems.append(f"{o}: invalid name {n}")
        elif not os.path.isfile(path):
            problems.append(f"{o}: no file {rel}")
        elif n in defined:
            problems.append(f"{o}: {n} already defined")
        elif n in addr_of and addr_of[n] != m.group(1).upper():
            problems.append(f"{o}: {n} already names 0x0{addr_of[n]}")
        else:
            addr_of[n] = m.group(1).upper()
            per_file.setdefault(path, []).append((sym, n))
    for p in problems[:40]:
        print(f"left out: {p}")
    changed = 0
    for path, subs in sorted(per_file.items()):
        with open(path, encoding="utf-8") as f:
            text = f.read()
        new = text
        for sym, n in subs:
            new = re.sub(r"\b%s\b" % re.escape(sym), n, new)
        if new != text:
            changed += 1
            if not dry and not _write_retry(path, new):
                sys.exit(f"{path}: locked")
    if not dry and addr_of:
        with open(os.path.join(build.ROOT, "undefined_syms.txt"), "a", encoding="utf-8", newline="\n") as f:
            f.write("\n/* file-scoped segment names (match_function.py rename, Changelog (854)) */\n"
                    + "\n".join(f"{n} = 0x0{a};" for n, a in sorted(addr_of.items())) + "\n")
    print(f"rename (file-scoped): {sum(len(v) for v in per_file.values())} entries, {len(addr_of)} names, "
          f"{changed} files {'would change' if dry else 'changed'}; {len(problems)} left out")
    return 0


def rename(map_path, dry=False, short=None):
    """address names to real ones. map_path: one "func_XXXXXXXX_<seg> name"
    per line (# comments). Refuses a map with a name twice, an invalid identifier, or a name
    already defined (symbol_addrs.txt, the last link's map, libultra's headers). Replaces whole
    words in src/*.c, src/*.s and include/**/*.h, and pins each new name in symbol_addrs.txt
    (with its segment, as overlays share addresses) so splat names every reference in the asm
    and data on the next split. Then: delete asm/ (generated) and run build_rom.py."""
    pairs = []
    if map_path.endswith(".jsonl"):
        # a JSONL results journal of name maps: every result's
        # names, written out as permuter/names_map.txt for review
        import json
        with open(map_path, encoding="utf-8") as f:
            for line in f:
                rec = json.loads(line)
                if rec.get("type") == "result" and isinstance(rec.get("result"), dict):
                    for n in rec["result"].get("names", []):
                        pair = (n.get("func") or n["symbol"], n["name"])  # functions or globals
                        if pair not in pairs:  # two maps naming one shared symbol alike: once
                            pairs.append(pair)
        with open(os.path.join(build.ROOT, "permuter", "names_map.txt"), "w", encoding="utf-8",
                  newline="\n") as f:
            f.write("".join(f"{o} {n}\n" for o, n in pairs))
    else:
        with open(map_path, encoding="utf-8") as f:
            for line in f:
                parts = line.split("#", 1)[0].split()
                if len(parts) >= 2:
                    pairs.append((parts[0], parts[1]))
    scoped = [(o, n) for o, n in pairs if "@" in o]
    if scoped:
        pairs = [(o, n) for o, n in pairs if "@" not in o]
        r = _rename_scoped(scoped, dry)
        if not pairs:
            return r
    old = dict(pairs)
    bad = [n for _, n in pairs if not re.match(r"^[A-Za-z_]\w{0,62}$", n)]
    from collections import Counter
    dup = [n for n, k in Counter(n for _, n in pairs).items() if k > 1]
    known = set()  # names DEFINED somewhere (not words in comments or object file names)
    p = os.path.join(build.ROOT, "symbol_addrs.txt")
    with open(p, encoding="utf-8") as f:
        known.update(re.findall(r"^\s*([A-Za-z_]\w*)\s*=", f.read(), re.M))
    p = os.path.join(build.ROOT, "build", "goemonsgreatadv.map")
    if os.path.exists(p):
        with open(p, encoding="utf-8", errors="replace") as f:
            known.update(re.findall(r"^\s+0x[0-9a-f]+\s+([A-Za-z_]\w*)\s*$", f.read(), re.M))
    heads = [os.path.join(d, n) for top in ("include", os.path.join("lib", "ultralib", "include"))
             for d, _, ns in os.walk(os.path.join(build.ROOT, top)) for n in ns if n.endswith(".h")]
    for h in heads:
        with open(h, encoding="utf-8", errors="replace") as f:
            t = re.sub(r"/\*.*?\*/|//[^\n]*", " ", f.read(), flags=re.S)
        known.update(re.findall(r"#\s*define\s+([A-Za-z_]\w*)", t))
        known.update(re.findall(r"\b([A-Za-z_]\w*)\s*\(", t))
        known.update(re.findall(r"\b([A-Za-z_]\w*)\s*(?:\[[^\]]*\])?\s*[;,=]", t))
    clash = sorted({n for _, n in pairs if n in known and n not in old})
    # a short alias of the same address (D_800D50C for 0x0800D50C), which
    # main's C uses for a TLB function it starts after loading that file (dmenu.c). Pinning the
    # new name in symbol_addrs makes the next split drop the alias, and the link fails. The alias
    # cannot be renamed blindly (overlaid TLB files share addresses): it is resolved by hand first.
    aliases, held = [], set()
    srcs = [os.path.join(d, n) for top in ("src", "include") for d, _, ns in os.walk(os.path.join(build.ROOT, top))
            for n in ns if n.endswith((".c", ".h", ".s"))]
    shorts = {}
    for o, _ in pairs:
        a = re.match(r"^(?:func|D)_0([0-9A-F]{7})_", o)
        if a:
            shorts.setdefault(f"D_{a.group(1)}", []).append(o)
    # an alias pinned by hand (undefined_syms.txt, symbol_addrs.txt) stays whatever the split does
    pinned_aliases = set()
    for name in ("symbol_addrs.txt", "undefined_syms.txt"):
        with open(os.path.join(build.ROOT, name), encoding="utf-8") as f:
            pinned_aliases.update(re.findall(r"^\s*(D_[0-9A-F]{7})\s*=", f.read(), re.M))
    shorts = {s: o for s, o in shorts.items() if s not in pinned_aliases}
    if shorts:
        sp = re.compile(r"\b(" + "|".join(map(re.escape, shorts)) + r")\b")
        for path in srcs:
            with open(path, encoding="utf-8", errors="replace") as f:
                for s in sorted(set(sp.findall(f.read()))):
                    aliases.append(f"{s} ({'/'.join(shorts[s])}'s address) in {os.path.relpath(path, build.ROOT)}")
                    held.update(shorts[s])
    if aliases:
        # held back, not the whole batch: the alias is resolved by hand, the rest go ahead
        print(f"held back (a short alias of the same address, resolve by hand first): {' '.join(aliases[:40])}"
              f"{' ...' if len(aliases) > 40 else ''}")
        pairs = [(o, n) for o, n in pairs if o not in held]
        old = dict(pairs)
    if bad or dup or clash:
        # listed and left out; the rest go ahead (one clash once refused a whole map)
        for title, items in (("left out, invalid", bad), ("left out, given twice", dup),
                             ("left out, already defined", clash)):
            if items:
                print(f"{title}: {' '.join(items[:40])}{' ...' if len(items) > 40 else ''}")
        drop = set(bad) | set(dup) | set(clash)
        pairs = [(o, n) for o, n in pairs if n not in drop]
        old = dict(pairs)
        if not pairs:
            sys.exit("rename: nothing changed")
    pinned = set()  # names an existing symbol_addrs / undefined_syms line defines
    for name in ("symbol_addrs.txt", "undefined_syms.txt"):
        with open(os.path.join(build.ROOT, name), encoding="utf-8") as f:
            pinned.update(re.findall(r"^\s*([A-Za-z_]\w*)\s*=", f.read(), re.M))
    lines, skipped, short_pins = [], [], []
    with open(os.path.join(build.ROOT, "symbol_addrs.txt"), encoding="utf-8") as f:
        sym_text = f.read()
    for o, n in pairs:  # every name checked BEFORE any file is written
        m = re.match(r"^func_([0-9A-F]{8})_(main|ovl_\d+|tlb_\d{4})$", o)
        d = re.match(r"^D_([0-9A-F]{8})(?:_(main|ovl_\d+|tlb_\d{4}))?$", o)
        if not m and not d and re.search(r"^%s\s*=" % re.escape(o), sym_text, re.M):
            pass  # a name given by hand, pinned: its own line is renamed in place below
        elif m and o in pinned:
            pass  # pinned already (with its size): its own line is renamed in place below
        elif m:
            lines.append(f"{n} = 0x{m.group(1)}; // type:func segment:{m.group(2)}")
        elif d and o in pinned:
            pass  # its own line is renamed in place below
        elif d:
            lines.append(f"{n} = 0x{d.group(1)};" + (f" // segment:{d.group(2)}" if d.group(2) else ""))
        elif short and re.match(r"^D_(8[0-9A-F]{6})$", o):
            # a short alias splat made for an address outside every segment (scenario.c's
            # D_8000040, a scenario file's variable): pinned by its address in undefined_syms.txt
            short_pins.append(f"{n} = 0x0{o[2:]};")
        else:
            skipped.append(o)  # not an address name (a segment address such as D_300023C): listed
    if skipped:
        print(f"skipped (not a func_/D_ address name; named by hand): {' '.join(skipped)}")
        pairs = [(o, n) for o, n in pairs if o not in skipped]
        old = dict(pairs)
    pattern = re.compile(r"\b(" + "|".join(map(re.escape, sorted(old, key=len, reverse=True))) + r")\b")
    # src/ with its subfolders (src/tlb/: the TLB code files, all in one link, so their names
    # must be unique across them too: the clash check above sees every one)
    files = [os.path.join(d, n) for d, _, ns in os.walk(os.path.join(build.ROOT, "src"))
             for n in ns if n.endswith((".c", ".s"))]
    files += [os.path.join(d, n) for d, _, ns in os.walk(os.path.join(build.ROOT, "include"))
              for n in ns if n.endswith(".h")]
    # the yaml too: a bin subsegment named after its symbol is .incbin'd by that name (logo.c)
    files += [os.path.join(build.ROOT, n) for n in ("symbol_addrs.txt", "undefined_syms.txt",
                                                    "goemonsgreatadv.yaml")]
    changed = 0
    for path in files:
        with open(path, encoding="utf-8") as f:
            text = f.read()
        new = pattern.sub(lambda m: old[m.group(1)], text)
        if new != text:
            changed += 1
            if not dry and not _write_retry(path, new):
                sys.exit(f"{path}: locked")
    if not dry and lines:
        with open(os.path.join(build.ROOT, "symbol_addrs.txt"), "a", encoding="utf-8", newline="\n") as f:
            f.write("// Names\n" + "\n".join(lines) + "\n")
    if not dry and short_pins:
        with open(os.path.join(build.ROOT, "undefined_syms.txt"), "a", encoding="utf-8", newline="\n") as f:
            f.write(f"\n/* {short} */\n" + "\n".join(short_pins) + "\n")
    print(f"rename: {len(pairs)} names, {changed} files {'would change' if dry else 'changed'}")
    parked = [n for n in old.values() if any(re.search(r"GLOBAL_ASM\(.*/%s\.s" % re.escape(n), open(p, encoding="utf-8").read())
                                             for p in files if p.endswith(".c"))] if not dry else []
    if parked:
        print(f"parked, so build with --split (their .s takes the new name): {' '.join(parked)}")
    return 0


GFX_DEF = re.compile(r"^((?:static )?Gfx\s+(\w+)\s*\[\s*\d*\s*\]\s*=\s*\{)(.*?)(\}\s*;)", re.M | re.S)
GFX_U32_DEF = re.compile(r"^((?:static )?)(u32\s+(\w+)\s*\[\s*\d*\s*\]\s*=\s*\{)(.*?)(\}\s*;)", re.M | re.S)
GFX_PAIR = re.compile(r"\{\s*(0x[0-9A-Fa-f]+|\(u32\)\s*&?\w+)\s*,\s*(0x[0-9A-Fa-f]+|\(u32\)\s*&?\w+)\s*\}")


def _gfx_macros(body):
    """A Gfx array's raw { w0, w1 } pairs as gbi.h macro lines (libgfxd, F3DEX2), or None when the
    body holds anything but such pairs (and comments). A symbol word goes through a placeholder
    address that each address callback turns back into the symbol."""
    import struct
    import pygfxd as g
    text = re.sub(r"/\*.*?\*/", "", body, flags=re.S)
    pairs = GFX_PAIR.findall(text)
    if not pairs or GFX_PAIR.sub("", text).replace(",", "").strip():
        return None
    tops = {int(w, 16) >> 24 for a, b in pairs for w in (a, b) if w.startswith("0x")}
    free = next((t for t in range(0x0F, 0x80) if t not in tops), None)  # a top byte no word has
    if free is None:
        return None
    syms, words = {}, b""
    for a, b in pairs:
        for w in (a, b):
            if w.startswith("0x"):
                words += struct.pack(">I", int(w, 16))
            else:
                ph = (free << 24) + 8 * len(syms)
                syms[ph] = re.sub(r"^\(u32\)\s*", "", w)
                words += struct.pack(">I", ph)
    out = []

    def macro():
        g.gfxd_puts("    ")
        g.gfxd_macro_dflt()
        g.gfxd_puts(",\n")
        return 0

    def addr(a, *rest):
        if a in syms:
            g.gfxd_printf(syms[a])
            return 1
        return 0

    o = 0
    while o < len(words):  # libgfxd stops at each G_ENDDL: one run per display list
        e = o
        while e < len(words) and words[e] != 0xDF:
            e += 8
        e = min(e + 8, len(words))
        g.gfxd_input_buffer(words[o:e])
        outb = bytes((e - o) * 80 + 4096)
        outbuf = g.gfxd_output_buffer(outb, len(outb))
        g.gfxd_target(g.gfxd_f3dex2)
        g.gfxd_endian(g.GfxdEndian.big, 4)
        g.gfxd_macro_fn(macro)
        for cb in (g.gfxd_dl_callback, g.gfxd_timg_callback, g.gfxd_cimg_callback, g.gfxd_zimg_callback,
                   g.gfxd_mtx_callback, g.gfxd_lookat_callback, g.gfxd_light_callback, g.gfxd_vtx_callback,
                   g.gfxd_vp_callback, g.gfxd_tlut_callback):
            cb(addr)
        g.gfxd_execute()
        out.append(g.gfxd_buffer_to_string(outbuf).rstrip("\n"))
        o = e
    return "\n" + "\n".join(out) + "\n"


def _obj_data(obj):
    """An object's .data and .rodata bytes and its .data relocations (offset, type, symbol)."""
    objcopy = build.tool_path(os.path.join(build.BINUTILS, "mips64-elf-objcopy" + build.EXE))
    objdump = build.tool_path(os.path.join(build.BINUTILS, "mips64-elf-objdump" + build.EXE))
    got = []
    for sec in (".data", ".rodata", ".text"):
        b = obj + sec + ".bin"
        subprocess.run([objcopy, "-O", "binary", "-j", sec, fwd(obj), fwd(b)], cwd=build.ROOT, env=build.ENV)
        with open(b, "rb") as f:
            got.append(f.read())
    for sec in (".data", ".text", ".rodata"):
        rel = subprocess.run([objdump, "-r", "-j", sec, fwd(obj)], cwd=build.ROOT, env=build.ENV,
                             capture_output=True, text=True).stdout
        got.append(re.findall(r"^([0-9a-f]{8})\s+(\S+)\s+(\S+)\s*$", rel, re.M))
    return got


def gfx_c(target, apply=False):
    """gfx-c: each Gfx array a C file still holds as raw { w0, w1 } words, as gbi.h macros. A file
    is rewritten only where its compiled .data, .rodata and .data relocations stay the same; an
    array that changes them is put back (each tried alone)."""
    files = [target] if target != "all" else sorted(
        os.path.join(d, n) for d, _, ns in os.walk(os.path.join(build.ROOT, "src")) for n in ns if n.endswith(".c"))
    scratch = os.path.join(build.ROOT, "permuter", "gfxc")
    os.makedirs(scratch, exist_ok=True)
    total_kept = total_back = 0
    # names some file declares `extern Gfx NAME[]`: drawn as a display list, so a u32 array of that
    # name is one even with no G_ENDDL of its own (it runs on into the next object's)
    gfx_named = set()
    for d, _, ns in os.walk(os.path.join(build.ROOT, "src")):
        for n in ns:
            if n.endswith(".c"):
                with open(os.path.join(d, n), encoding="utf-8") as f:
                    gfx_named.update(re.findall(r"^extern\s+Gfx\s+(\w+)\s*\[", f.read(), re.M))
    for c_file in files:
        with open(c_file, encoding="utf-8") as f:
            src = f.read()
        cands = []
        for m in GFX_DEF.finditer(src):
            try:
                body = _gfx_macros(m.group(3))
            except Exception as ex:  # a libgfxd failure: this array stays as it is, said why
                print(f"gfx-c: {m.group(2)}: {type(ex).__name__}: {ex}")
                continue
            if body:
                cands.append((m.start(3), m.end(3), m.group(2), body))
        for m in GFX_U32_DEF.finditer(src):
            words = re.findall(r"0x[0-9A-Fa-f]+|\(u32\)\s*&?\w+", re.sub(r"/\*.*?\*/", "", m.group(4), flags=re.S))
            if len(words) < 2 or len(words) % 2 or words[-2:] != ["0xDF000000", "0x00000000"] and \
                    "0xDF000000" not in words[::2] and m.group(3) not in gfx_named:
                continue
            try:
                body = _gfx_macros(" ".join(f"{{ {words[i]}, {words[i + 1]} }}," for i in range(0, len(words), 2)))
            except Exception as ex:
                print(f"gfx-c: {m.group(3)}: {type(ex).__name__}: {ex}")
                continue
            if body:
                cands.append((m.start(2), m.end(4), m.group(3), "Gfx" + m.group(2)[3:] + body))
        if not cands:
            continue
        base_o = os.path.join(scratch, "base.o")
        if compile_one(c_file, base_o) != 0:
            print(f"gfx-c: {os.path.relpath(c_file, build.ROOT)} does not compile as it is; skipped")
            continue
        want = _obj_data(base_o)

        def with_(chosen):
            t = src
            for s, e, n, body in sorted(chosen, key=lambda c: -c[0]):
                t = t[:s] + body + t[e:]
            for s, e, n, body in chosen:
                if body.startswith("Gfx"):  # was u32: its declarations in the file take Gfx
                    t = re.sub(r"^extern\s+(?:u32|u8)\s+%s\s*\[\s*\d*\s*\]\s*;" % re.escape(n),
                               f"extern Gfx {n}[];", t, flags=re.M)
            return t

        def same(text):
            if not _write_retry(c_file, text):
                return False
            o = os.path.join(scratch, "try.o")
            ok = compile_one(c_file, o) == 0 and _obj_data(o) == want
            return ok

        kept = cands if same(with_(cands)) else [c for c in cands if same(with_([c]))]
        final = with_(kept)
        if not apply or not kept:
            _write_retry(c_file, src)
        else:
            _write_retry(c_file, final)
        total_kept += len(kept)
        total_back += len(cands) - len(kept)
        print(f"gfx-c: {os.path.relpath(c_file, build.ROOT)}: {len(kept)} of {len(cands)} arrays as macros"
              f"{'' if apply else ' (dry)'}"
              + (f"; put back: {', '.join(c[2] for c in cands if c not in kept)}" if len(kept) < len(cands) else ""))
    print(f"gfx-c: {total_kept} arrays as macros, {total_back} put back")
    return 0


def _c_string(raw):
    """Bytes (no terminator) as a C string literal's inside: EUC-JP text as itself (asm-processor
    encodes the C file back to EUC-JP), escapes for the rest; a \\x escape never runs into a
    following hex digit (the literal is split there)."""
    out, i = [], 0
    esc = {0x0A: "\\n", 0x09: "\\t", 0x0D: "\\r", 0x22: '\\"', 0x5C: "\\\\", 0x00: "\\0"}
    while i < len(raw):
        b = raw[i]
        if b in esc:
            out.append(esc[b])
            i += 1
        elif 0x20 <= b < 0x7F:
            if out and out[-1].startswith("\\x") and chr(b) in "0123456789abcdefABCDEF":
                out.append('" "')
            out.append(chr(b))
            i += 1
        else:
            ch = None
            for n in (2, 3):
                try:
                    c = raw[i:i + n].decode("euc_jp")
                    if len(c) == 1 and c.encode("euc_jp") == raw[i:i + n]:
                        ch = (c, n)
                        break
                except UnicodeDecodeError:
                    pass
            if ch:
                out.append(ch[0])
                i += ch[1]
            else:
                out.append(f"\\x{b:02X}")
                i += 1
    return "".join(out)


def _rodata_c_text(s_path, seg, pad_free=True):
    """A rodata D_*.s (strings only, or plain words only) as C definitions, else None. pad_free:
    a string's zero bytes after its terminator are left to the compiler's alignment."""
    import struct
    with open(s_path, encoding="utf-8") as f:
        if ".section .rodata" not in f.read():
            return None
    labels = _data_items(s_path)
    if len(labels) != 1:
        return None
    name, items = labels[0]
    kinds = {k for _, k, _, _ in items}
    if kinds == {"asciz"}:
        defs = []
        for n, (a, k, t, r) in enumerate(items):
            if not r or r[-1] != 0 or (n and a % 4):
                return None
            nm = name if n == 0 else f"D_{a:08X}_{seg}"
            body = r[:-1].rstrip(b"\0") if pad_free else r[:-1]  # zeros after the text: alignment
            defs.append((nm, "const char", f'const char {nm}[] = "{_c_string(body)}";'))
        return defs
    if kinds == {"word"}:
        plain = [re.fullmatch(r"-?0x[0-9A-Fa-f]+|-?\d+", t) for _, _, t, _ in items]
        if all(plain):
            words = ", ".join(f"0x{struct.unpack('>I', r)[0]:08X}" for *_, r in items)
            return [(name, "const u32", f"const u32 {name}[] = {{ {words} }};")]
        # symbols among the words: as pointers, an address initializer IDO can keep in .rodata
        words = ", ".join(f"(void *) 0x{struct.unpack('>I', r)[0]:08X}" if p else t
                          for p, (_, _, t, r) in zip(plain, items))
        return [(name, "void *const", f"void *const {name}[] = {{ {words} }};")]
    return None


def rodata_c(target, apply=False):
    """rodata-c: each .rodata block a C file includes as assembly, as C (see _rodata_c_text); kept
    where the file's compiled sections and relocations stay the same (each tried alone else)."""
    files = [target] if target != "all" else sorted(
        os.path.join(d, n) for d, _, ns in os.walk(os.path.join(build.ROOT, "src")) for n in ns if n.endswith(".c"))
    scratch = os.path.join(build.ROOT, "permuter", "gfxc")
    os.makedirs(scratch, exist_ok=True)
    pat = re.compile(r'^#pragma GLOBAL_ASM\("(asm/nonmatchings/[^"]*/(D_\w+)\.s)"\)[ \t]*$', re.M)
    total_kept = total_back = 0
    for c_file in files:
        with open(c_file, encoding="utf-8") as f:
            src = f.read()
        seg = re.search(r"(tlb_\d{4}|ovl_\d{4}|main)", os.path.basename(c_file))
        seg = seg.group(1) if seg else "main"
        cands, full = [], {}
        for m in pat.finditer(src):
            p = os.path.join(build.ROOT, m.group(1))
            defs = _rodata_c_text(p, seg) if os.path.exists(p) else None
            if defs:
                cands.append((m.start(), m.end(), m.group(2), defs))
                full[m.group(2)] = (m.start(), m.end(), m.group(2), _rodata_c_text(p, seg, pad_free=False))
        if not cands:
            continue
        base_o = os.path.join(scratch, "base.o")
        if compile_one(c_file, base_o) != 0:
            print(f"rodata-c: {os.path.relpath(c_file, build.ROOT)} does not compile as it is; skipped")
            continue
        want = _obj_data(base_o)

        def with_(chosen):
            t = src
            for s, e, n, defs in sorted(chosen, key=lambda c: -c[0]):
                text = "\n".join(d[2] for d in defs)
                # a symbol word's symbol, declared ahead when nothing above declares it
                above = re.sub(r"/\*.*?\*/", "", t[:s], flags=re.S)
                need = [y for y in dict.fromkeys(re.findall(r"\(u32\) (\w+)|[{,]\s*([A-Za-z_]\w*)\s*(?=[,}])", text))
                        for y in [y[0] or y[1]] if y]
                need = [y for y in need
                        if not re.search(r"\b%s\b" % re.escape(y), above) and y not in _header_names(src)]
                t = t[:s] + "".join(f"extern u8 {y}[];\n" for y in need) + text + t[e:]
            for s, e, n, defs in chosen:
                for nm, typ, _ in defs:  # the file's declarations of it take its type
                    t = re.sub(r"^extern\s+[^;(]*?\b%s\s*\[\s*\d*\s*\]\s*;" % re.escape(nm),
                               f"extern {typ} {nm}[];", t, flags=re.M)
            return t

        def same(text, why=None):
            if not _write_retry(c_file, text):
                return False
            o = os.path.join(scratch, "try.o")
            if compile_one(c_file, o) != 0:
                if why:
                    print(f"rodata-c: {why}: does not compile")
                return False
            got = _obj_data(o)
            if got != want and why:
                parts = (".data", ".rodata", ".text", ".data relocations", ".text relocations", ".rodata relocations")
                print(f"rodata-c: {why}: differs in {', '.join(p for p, g, w in zip(parts, got, want) if g != w)}")
            return got == want

        if same(with_(cands)):
            kept = cands
        else:
            kept = []
            for c in cands:
                if same(with_([c]), why=c[2]):
                    kept.append(c)
                elif full[c[2]][3] != c[3] and same(with_([full[c[2]]])):
                    kept.append(full[c[2]])  # its zeros written out
            if kept and not same(with_(kept)):
                kept = []  # each alone, not together: put all back
        _write_retry(c_file, with_(kept) if apply and kept else src)
        total_kept += len(kept)
        total_back += len(cands) - len(kept)
        kept_names = {c[2] for c in kept}
        print(f"rodata-c: {os.path.relpath(c_file, build.ROOT)}: {len(kept)} of {len(cands)} blocks as C"
              f"{'' if apply else ' (dry)'}"
              + (f"; put back: {', '.join(c[2] for c in cands if c[2] not in kept_names)}"
                 if len(kept) < len(cands) else ""))
    print(f"rodata-c: {total_kept} blocks as C, {total_back} put back")
    return 0


ADDR_NAME = re.compile(r"\b(func|D)_([0-9A-F]{6,8})(?:_(main|ovl_\d+))?\b")
# Address names kept on purpose: the real name's header prototype cannot express the game's call.
ALIAS_KEEP = {
    "func_8001D120_main",  # contpak.c: libultra's osPfsNumFiles, called with its first argument only
}


def alias(apply=False):
    """address names the source still uses for something that already HAS a
    name (a libultra function called by address, an overlay 9 function main calls across the
    slot, libultra's alGlobals), pointed at that name. Each func_/D_ token in src/ and include/
    (code and comments) is looked up in the last link's ELF: the one function or object starting
    at exactly that address in its segment's sections (.main/.main_bss, .ovl_00NN/_bss; with no
    suffix, the one of overlays 8, 9 and 10 that has one). None or more than one: left as is,
    listed. Writes permuter/alias_map.txt ("old new  # where") and, with apply, replaces the
    tokens. Then build both hashes: a prototype that differs from a header's is the compiler's
    to report."""
    objdump = os.path.join(os.path.dirname(build.ROOT), "vendor", "mips64", "bin", "mips64-elf-objdump.exe")
    out = subprocess.run([objdump, "-t", os.path.join(build.ROOT, "build", "goemonsgreatadv.elf")],
                         capture_output=True, text=True, check=True).stdout
    at = {}  # address -> [(section, kind, name)]
    for line in out.splitlines():
        p = line.split()
        if (len(p) == 6 and p[2] in ("F", "O") and p[3] != "*ABS*" and not ADDR_NAME.fullmatch(p[5])
                and "." not in p[5]):  # not asm-processor's name.NON_MATCHING twins
            at.setdefault(int(p[0], 16), []).append((p[3].lstrip("."), p[2], p[5]))
    files = glob.glob(os.path.join(build.ROOT, "src", "*.c")) + glob.glob(os.path.join(build.ROOT, "src", "*.s"))
    files += [os.path.join(d, n) for d, _, ns in os.walk(os.path.join(build.ROOT, "include"))
              for n in ns if n.endswith(".h")]
    files.append(os.path.join(build.ROOT, "undefined_syms.txt"))  # its notes name other symbols
    import report_progress
    fseg = report_progress.file_segments()
    slot = ["ovl_0008", "ovl_0009", "ovl_0010"]

    def context(path):
        """An unsuffixed slot address in a file of overlay 8, 9 or 10 is that overlay's."""
        seg = fseg.get(os.path.splitext(os.path.basename(path))[0]) if path.endswith(".c") else None
        return seg if seg in slot else None

    tokens = {}  # (context, token) -> match
    for path in files:
        with open(path, encoding="utf-8") as f:
            for m in ADDR_NAME.finditer(f.read()):
                if m.group(0) not in ALIAS_KEEP:
                    tokens.setdefault((None if m.group(3) else context(path), m.group(0)), m)
    found, none, many = {}, [], []
    for (ctx, tok), m in sorted(tokens.items(), key=lambda kv: (kv[0][1], kv[0][0] or "")):
        addr = int(m.group(2), 16)
        segs = [m.group(3)] if m.group(3) else [ctx] if ctx else slot
        if not m.group(3) and not 0x801738A0 <= addr < 0x801738A0 + 0x47000:
            segs = ["main", "ovl_0006"]  # an old unsuffixed name outside the slot: one segment has it
        cands = [c for c in at.get(addr, []) if c[0] in segs or c[0] in [s + "_bss" for s in segs]]
        want = "F" if m.group(1) == "func" else "O"
        same = [c for c in cands if c[1] == want] or cands  # D_ names of functions point at them too
        if len(same) == 1:
            found[(ctx, tok)] = same[0]
        elif same:
            many.append(f"{tok}: {' '.join(c[2] + '@' + c[0] for c in same)}")
        else:
            none.append(tok + (f" (in {ctx})" if ctx else ""))
    with open(os.path.join(build.ROOT, "permuter", "alias_map.txt"), "w", encoding="utf-8", newline="\n") as f:
        f.write("".join(f"{t} {c[2]}  # {c[0]} {c[1]}{' in ' + x if x else ''}\n" for (x, t), c in found.items()))
        f.write("".join(f"# ambiguous {x}\n" for x in many) + "".join(f"# no symbol {x}\n" for x in none))
    print(f"alias: {len(tokens)} address names (per overlay context); {len(found)} have a name, "
          f"{len(many)} ambiguous, {len(none)} none (permuter/alias_map.txt)")
    if not apply or not found:
        return 0
    changed = 0
    for path in files:
        with open(path, encoding="utf-8") as f:
            text = f.read()
        ctx = context(path)

        def sub(m):
            hit = found.get((None if m.group(3) else ctx, m.group(0)))
            return hit[2] if hit else m.group(0)
        new = ADDR_NAME.sub(sub, text)
        if new != text:
            changed += 1
            if not _write_retry(path, new):
                sys.exit(f"{path}: locked")
    print(f"alias: {changed} files changed")
    return 0


ONE_STMT = re.compile(r"^\s*(?!return\b|break\b|continue\b|goto\b|case\b|default\b|else\b|if\b|for\b"
                      r"|while\b|do\b|switch\b)[^{}]*;\s*$")


def _taken(rows):
    """The sp offsets a function takes the address of (addiu reg,sp,N): its address-taken locals."""
    return sorted({int(m.group(1)) for r in rows for m in [re.match(r"addiu \w+,sp,(\d+)$", r)] if m})


def _slot_move(base, got):
    """Two sorted slot lists: "" when equal, "k +d" for the commonest move of the slots taken in
    order when both have as many, else "n to m" (a slot gained or lost)."""
    from collections import Counter
    if base == got:
        return ""
    if len(base) != len(got):
        return f"{len(base)} to {len(got)}"
    moves = Counter(g - b for b, g in zip(base, got) if g != b)
    delta, k = moves.most_common(1)[0]
    return f"{k} {'+' if delta > 0 else '-'}0x{abs(delta):X}"


def drops(c_file, func):
    """A diagnostic, not a find (each variant computes less than the game): every one-line
    statement of a parked function removed in turn, compiled as `check` does, and its frame, its
    address-taken locals and its stack slots compared with the C as written. The locals sit on
    top of the compiler temps (IDO_NOTES 243), so where this C has temp words the game lacks
    (plchain.c func_80175968: every local 0xC high), a statement whose removal moves the locals
    down is one whose expressions make a temp. A statement that is a local's only use moves them
    too (its word goes): read each line listed."""
    with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
        text = f.read()
    if not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), text):
        sys.exit(f"{func} is not parked in {c_file}")
    m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), text, re.M)
    if not m:
        sys.exit(f"{func}: no definition in {c_file}")
    lines = text[m.end():].split("\n")
    end = next(i for i, l in enumerate(lines) if l.startswith("}"))
    d = os.path.join(build.ROOT, "permuter", "drops", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)
    if _variant_lines(d, func, want, text) is None:
        sys.exit(f"{func}: the C as written does not compile")
    frame0, slots0 = _stack_map(VARIANT_ROWS[d])
    taken0 = _taken(VARIANT_ROWS[d])
    gframe, gslots = _stack_map(want)
    print(f"as written: frame 0x{frame0:X}, {len(slots0)} slots, address-taken "
          f"{' '.join('%X' % s for s in taken0) or '-'}; the game's: frame 0x{gframe:X}, "
          f"{len(gslots)} slots, address-taken {' '.join('%X' % s for s in _taken(want)) or '-'}",
          flush=True)
    found = 0
    for j in range(end):
        code = re.sub(r"/\*.*?\*/|//.*$", "", lines[j])
        if not ONE_STMT.match(code) or DECL.match(lines[j]) or code.count("(") != code.count(")"):
            continue
        VARIANT_ROWS.pop(d, None)
        c = _variant_lines(d, func, want, text[:m.end()] + "\n".join(lines[:j] + lines[j + 1:]))
        if c is None or d not in VARIANT_ROWS:
            continue
        frame, slots = _stack_map(VARIANT_ROWS[d])
        taken, moved = _slot_move(taken0, _taken(VARIANT_ROWS[d])), _slot_move(slots0, slots)
        if frame == frame0 and not taken and not moved:
            continue
        found += 1
        print(f"line {j + 1} removed: frame 0x{frame:X}" + (f", address-taken {taken}" if taken else "")
              + (f", slots {moved}" if moved else "") + f"  '{lines[j].strip()[:60]}'", flush=True)
    print(f"{func}: {found} statements move the frame or the slots")
    return 0


COMMUTATIVE = ("+", "*", "&", "|", "^", "==", "!=")


def _top_ops(expr):
    """The top-level binary operators of a C expression (outside (), [] and strings), as
    (position, operator): + * & | ^ == != and also the others, so a mixed level can be refused."""
    ops, depth, i, prev = [], 0, 0, ""  # prev: the last non-space character seen at depth 0
    while i < len(expr):
        c = expr[i]
        if c == '"':
            j = expr.index('"', i + 1)
            i, prev = j + 1, '"'
            continue
        if c in "([":
            depth += 1
        elif c in ")]":
            depth -= 1
        elif depth == 0 and not c.isspace():
            two = expr[i:i + 2]
            operand_before = prev and (prev.isalnum() or prev in "_)]\"'.")
            if two in ("==", "!=", "<=", ">=", "&&", "||", "<<", ">>", "->", "++", "--", "+=", "-=",
                       "*=", "/=", "&=", "|=", "^="):
                if operand_before and two not in ("->", "++", "--"):
                    ops.append((i, two))
                i, prev = i + 2, two[-1] if two not in ("->",) else "."
                continue
            if c in "+-*/%&|^<>?:" and operand_before:
                ops.append((i, c))
        if not c.isspace():
            prev = c
        i += 1
    return ops


def _commute(expr):
    """expr with the operands of its last top-level commutative operator swapped (the left part
    parenthesised when it has operators of its own), or None when the top level is not one
    commutative operator throughout."""
    ops = _top_ops(expr)
    if not ops or len({o for _, o in ops}) != 1 or ops[0][1] not in COMMUTATIVE:
        return None
    if ops[0][1] in ("==", "!=") and len(ops) > 1:
        return None
    pos, op = ops[-1]
    left, right = expr[:pos].strip(), expr[pos + len(op):].strip()
    # an operand with a call, an assignment or ++/-- has a side effect the swap would reorder
    side = re.compile(r"\b(?!sizeof\b)[A-Za-z_]\w*\s*\(|\(\s*\*\s*\w+\s*\)\s*\(|\+\+|--|[^=!<>]=[^=]")
    if side.search(left) or side.search(right):
        return None
    if len(ops) > 1:
        left = f"({left})"
    return f"{right} {op} {left}"


ASSIGN_RHS = re.compile(r"^(\s*[^=;]*?[^=!<>+\-*/%&|^]\s=\s)(.+);(\s*(?:/\*.*\*/)?\s*)$")
RETURN = re.compile(r"^(\s*return\s+)(.+);(\s*(?:/\*.*\*/)?\s*)$")
IFTEST = re.compile(r"^(\s*(?:\}\s*else\s+)?if \()(.*)(\) \{\s*(?:/\*.*\*/)?\s*)$")


def commutes(c_file, func, quiet=False):
    """Operand order as a search: each assignment's right side, return value and one-line if test
    whose top level is a single commutative operator (+ * & | ^, or one == / !=) with its
    operands swapped at the last occurrence, compiled as `check` does. IDO evaluates and colours
    by operand order (particle.c func_800F53B0 matched with "y = p->y + (p->vy >> 2)" for the
    other order, IDO_NOTES 259). A float sum of three or more changes its rounding when
    regrouped: read a find before keeping it. quiet (the sweep): (base, results) or None."""
    with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
        text = f.read()
    if not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), text):
        if quiet:
            return None
        sys.exit(f"{func} is not parked in {c_file}")
    m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), text, re.M)
    if not m:
        if quiet:
            return None
        sys.exit(f"{func}: no definition in {c_file}")
    lines = text[m.end():].split("\n")
    end = next(i for i, l in enumerate(lines) if l.startswith("}"))
    variants = []
    for j in range(end):
        line = lines[j]
        if DECL.match(line) or line.strip().startswith(("/*", "*", "#")) or "FAKE" in line:
            continue
        for pat in (ASSIGN_RHS, RETURN, IFTEST):
            w = pat.match(line)
            if not w:
                continue
            try:
                new = _commute(w.group(2))
            except ValueError:
                new = None
            if new:
                closing = ";" if pat is not IFTEST else ""
                variants.append((f"line {j + 1}: '{w.group(2).strip()[:40]}' -> '{new[:40]}'",
                                 lines[:j] + [w.group(1) + new + closing + w.group(3)] + lines[j + 1:]))
            break
    d = os.path.join(build.ROOT, "permuter", "commutes", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)
    base = _variant_lines(d, func, want, text)
    base_len = VARIANT_LEN.get(d)
    if not quiet:
        print(f"{base}  as written; {len(variants)} swaps", flush=True)
    results = []
    for where, variant in variants:
        c = _variant_lines(d, func, want, text[:m.end()] + "\n".join(variant))
        if c is not None and base is not None and c < base:
            where += _len_note(d, want, base_len)
            results.append((c, where))
            VARIANT_SOURCES[where] = text[:m.end()] + "\n".join(variant)
            if not quiet:
                print(f"{c:5d}  {where}", flush=True)
    results.sort()
    if quiet:
        return base, results
    print(f"{func}: {base} as written; {len(variants)} operand swaps tried; best:")
    for c, where in results[:8]:
        print(f"  {c:5d}  {where}")
    return 0


def _chain_parts(stmt):
    """'a = b = K' -> ['a', 'b', 'K'] (split at top-level plain '=' only), else None."""
    parts, depth, start, i = [], 0, 0, 0
    while i < len(stmt):
        c = stmt[i]
        if c in "([":
            depth += 1
        elif c in ")]":
            depth -= 1
        elif c == "=" and depth == 0 and stmt[i + 1:i + 2] != "=" and \
                stmt[i - 1:i] not in ("=", "!", "<", ">", "+", "-", "*", "/", "%", "&", "|", "^"):
            parts.append(stmt[start:i].strip())
            start = i + 1
        i += 1
    parts.append(stmt[start:].strip())
    return parts if len(parts) >= 3 else None


CHAIN = re.compile(r"^(\s*)([^;{}]*=[^;{}]*=[^;{}]*);(\s*(?:/\*.*\*/)?\s*)$")
SIMPLE_STORE = re.compile(r"^(\s*)([^=;{}]*[^=!<>+\-*/%&|^\s])\s=\s(-?(?:0x[0-9A-Fa-f]+|\d+(?:\.\d*)?f?|NULL|FALSE|TRUE));\s*$")


def rechains(c_file, func, quiet=False):
    """Chained assignments as a search: each "a = b = ... = v;" with its targets in every other
    order (all orders up to four targets, else only reversed), and split into separate
    statements (in order, and reversed), compiled as `check` does. A chain spends one temporary
    and stores its innermost target first (camera.c func_800CAEF8: "[1] = [0] = 0" where "[0] =
    [2] = 0" stored in the wrong order, IDO_NOTES 261). Targets of different types convert the
    value differently in another order: read a find before keeping it. quiet: (base, results)."""
    import itertools
    with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
        text = f.read()
    if not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), text):
        if quiet:
            return None
        sys.exit(f"{func} is not parked in {c_file}")
    m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), text, re.M)
    if not m:
        if quiet:
            return None
        sys.exit(f"{func}: no definition in {c_file}")
    lines = text[m.end():].split("\n")
    end = next(i for i, l in enumerate(lines) if l.startswith("}"))
    variants = []
    for j in range(end):
        w = CHAIN.match(lines[j])
        if not w or DECL.match(lines[j]) or "FAKE" in lines[j] or lines[j].strip().startswith(("if", "for", "while")):
            continue
        parts = _chain_parts(w.group(2))
        if not parts:
            continue
        targets, value = parts[:-1], parts[-1]
        orders = itertools.permutations(targets) if len(targets) <= 4 else [tuple(reversed(targets))]
        for order in orders:
            if list(order) != targets:
                new = " = ".join(order) + f" = {value};"
                variants.append((f"line {j + 1} as '{new[:60]}'", lines[:j] + [w.group(1) + new + w.group(3)] + lines[j + 1:]))
        for name, seq in (("split", list(reversed(targets))), ("split reversed", targets)):
            stmts = [f"{w.group(1)}{t} = {value};" for t in seq]
            variants.append((f"line {j + 1} {name}: '{stmts[0].strip()[:40]}' ...", lines[:j] + stmts + lines[j + 1:]))
    # Two adjacent one-line stores of the same value joined into a chain, both ways round: a chain
    # spends a temporary the separate stores do not (IDO_NOTES 255, 261).
    for j in range(end - 1):
        a, b = SIMPLE_STORE.match(lines[j]), SIMPLE_STORE.match(lines[j + 1])
        if not a or not b or a.group(1) != b.group(1) or a.group(3) != b.group(3) or "FAKE" in lines[j] + lines[j + 1]:
            continue
        for first, second in ((a.group(2), b.group(2)), (b.group(2), a.group(2))):
            new = f"{a.group(1)}{second} = {first} = {a.group(3)};"
            variants.append((f"lines {j + 1}-{j + 2} as '{new.strip()[:60]}'", lines[:j] + [new] + lines[j + 2:]))
    d = os.path.join(build.ROOT, "permuter", "rechains", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)
    base = _variant_lines(d, func, want, text)
    base_len = VARIANT_LEN.get(d)
    if not quiet:
        print(f"{base}  as written; {len(variants)} chain variants", flush=True)
    results = []
    for where, variant in variants:
        c = _variant_lines(d, func, want, text[:m.end()] + "\n".join(variant))
        if c is not None and base is not None and c < base:
            where += _len_note(d, want, base_len)
            results.append((c, where))
            VARIANT_SOURCES[where] = text[:m.end()] + "\n".join(variant)
            if not quiet:
                print(f"{c:5d}  {where}", flush=True)
    results.sort()
    if quiet:
        return base, results
    print(f"{func}: {base} as written; {len(variants)} chain variants tried; best:")
    for c, where in results[:8]:
        print(f"  {c:5d}  {where}")
    return 0


SCALAR_DECL = re.compile(r"^(\s+)((?:u8|s8|u16|s16|u32|s32|f32|f64|[A-Z]\w*\s*\*+|\w+\s*\*+)\s*\**\s*\w+"
                         r"\s*(?:=[^;]*)?;)(.*)$")


LVALUE = r"[A-Za-z_][\w.\[\]]*(?:->[\w.\[\]]+)*"  # a plain name, field or element, no side effects
ATOM = r"(?:[A-Za-z_][\w.\[\]]*(?:->[\w.\[\]]+)*|\d[\w.]*|\([^()]*\))"  # one operand, no operator
COMPOUND_OP = r"(<<|>>|[-+*/%&|^])"
TO_COMPOUND = re.compile(r"^(\s*)(%s)\s*=\s*\2\s*%s\s*(%s)\s*;(\s*(?:/\*.*\*/)?)$" % (LVALUE, COMPOUND_OP, ATOM))
FROM_COMPOUND = re.compile(r"^(\s*)(%s)\s*%s=(?!=)\s*([^;]+?)\s*;(\s*(?:/\*.*\*/)?)$" % (LVALUE, COMPOUND_OP))


def compounds(c_file, func, quiet=False):
    """Each one-line "x = x op y;" written "x op= y;" and each "x op= y;" written "x = x op (y);",
    compiled as `check` does; the C computes the same (y is one operand, or parenthesised). The
    two forms colour registers differently: "cur -= step" gives cur the lower float register,
    "cur = cur - step" gives it to step (camera.c func_800CAEF8, 130 to 11, IDO_NOTES 271).
    quiet (the sweep): (base, results) or None."""
    with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
        text = f.read()
    if not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), text):
        if quiet:
            return None
        sys.exit(f"{func} is not parked in {c_file}")
    m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), text, re.M)
    if not m:
        if quiet:
            return None
        sys.exit(f"{func}: no definition in {c_file}")
    lines = text[m.end():].split("\n")
    end = next(i for i, l in enumerate(lines) if l.startswith("}"))
    d = os.path.join(build.ROOT, "permuter", "compounds", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)
    base = _variant_lines(d, func, want, text)
    base_len = VARIANT_LEN.get(d)
    if not quiet:
        print(f"{base}  as written", flush=True)
    variants = []
    for j in range(end):
        t = TO_COMPOUND.match(lines[j])
        if t:
            new = f"{t.group(1)}{t.group(2)} {t.group(3)}= {t.group(4)};{t.group(5)}"
            variants.append((f"line {j + 1} as '{new.strip()[:50]}'", lines[:j] + [new] + lines[j + 1:]))
            continue
        c = FROM_COMPOUND.match(lines[j])
        if c and not lines[j].lstrip().startswith(("if", "for", "while", "return")):
            y = c.group(4) if re.fullmatch(ATOM, c.group(4)) else f"({c.group(4)})"
            new = f"{c.group(1)}{c.group(2)} = {c.group(2)} {c.group(3)} {y};{c.group(5)}"
            variants.append((f"line {j + 1} as '{new.strip()[:50]}'", lines[:j] + [new] + lines[j + 1:]))
    results = []
    for where, variant in variants:
        cnt = _variant_lines(d, func, want, text[:m.end()] + "\n".join(variant))
        if not quiet:
            print(f"{'-' if cnt is None else cnt:>5}  {where}", flush=True)
        if cnt is not None and base is not None and cnt < base:
            where += _len_note(d, want, base_len)
            results.append((cnt, where))
            VARIANT_SOURCES[where] = text[:m.end()] + "\n".join(variant)
    results.sort()
    if quiet:
        return base, results
    print(f"{func}: {base} as written; {len(variants)} forms tried; best:")
    for cnt, where in results[:8]:
        print(f"  {cnt:5d}  {where}")
    return 0


def scopes(c_file, func, quiet=False):
    """Each scalar or pointer local (one declarator, no initialiser) declared in a block instead of
    at the function's top: (a) right after the opening brace of the innermost existing block that
    holds all its uses, (b) in a new bare block from the top-level statement of its first use to
    the body's end. The C computes the same. Block-scoped locals take the frame words just above
    the compiler temps, function-scope ones the words from the top (IDO_NOTES 272: sndtrack.c
    func_8000B5D0; vram.c func_800E3630's bz/bx moved down a word), so this moves a local's home.
    quiet (the sweep): (base, results) or None."""
    with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
        text = f.read()
    if not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), text):
        if quiet:
            return None
        sys.exit(f"{func} is not parked in {c_file}")
    m = re.search(r'^[A-Za-z][^\n;]*\b%s\(([^;{]*)\)\s*\{\n' % re.escape(func), text, re.M)
    if not m:
        if quiet:
            return None
        sys.exit(f"{func}: no definition in {c_file}")
    lines, end, n, _, _, starts, straddled = _body_scan(m, text)
    # each line's open blocks: (the line of its "{", the line of its "}")
    opens, stack = [], []
    for k in range(end):
        code = re.sub(r'/\*.*?\*/|//.*$|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', "", lines[k])
        opens.append(list(stack))
        for ch in code:
            if ch == "{":
                stack.append([k, None])
            elif ch == "}" and stack:
                stack.pop()[1] = k
    d = os.path.join(build.ROOT, "permuter", "scopes", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)
    base = _variant_lines(d, func, want, text)
    base_len = VARIANT_LEN.get(d)
    if not quiet:
        print(f"{base}  as written", flush=True)
    indent = next((re.match(r"\s*", lines[k]).group(0) for k in range(n, end) if lines[k].strip()), "    ")
    variants = []
    for j in range(n):
        dm = SCALAR_DECL.match(lines[j])
        if not dm or "=" in dm.group(2) or "," in dm.group(2):
            continue
        name = re.search(r"(\w+)\s*;", dm.group(2)).group(1)
        decl = lines[j].strip()
        uses = [k for k in range(n, end) if re.search(r"(?<![\w.>])%s\b" % re.escape(name),
                                                      re.sub(r'/\*.*?\*/|"(?:\\.|[^"\\])*"', "", lines[k]))]
        if not uses:
            continue
        rest = lines[:j] + lines[j + 1:]  # every index past j moves up one
        # (a) the innermost existing block holding every use
        holders = [b for b in opens[uses[0]] if b[1] is not None and b[1] >= uses[-1]
                   and all(b in opens[k] for k in uses)]
        if holders:
            b = holders[-1][0]
            nxt = lines[b + 1].strip() if b + 1 < end else ""
            if lines[b].rstrip().endswith("{") and not nxt.startswith(("case ", "default")):
                inner = re.match(r"\s*", lines[b + 1]).group(0) or indent
                at = b + 1 - 1  # the line after b's "{", in rest
                variants.append((f"'{decl}' into the block of line {b + 1}",
                                 rest[:at] + [inner + decl] + rest[at:]))
        # (b) a new block from its first use's top-level statement to the body's end
        s = max((t for t in starts if t <= uses[0]), default=None)
        if s is not None and s not in straddled:
            at, close = s - 1, end - 1
            block = rest[:at] + [indent + "{", indent + "    " + decl] + rest[at:close] + [indent + "}"] + rest[close:]
            variants.append((f"'{decl}' in a block from line {s + 1} to the end", block))
    results = []
    for where, variant in variants:
        c = _variant_lines(d, func, want, text[:m.end()] + "\n".join(variant))
        if not quiet:
            print(f"{'-' if c is None else c:>5}  {where}", flush=True)
        if c is not None and base is not None and c < base:
            where += _len_note(d, want, base_len)
            results.append((c, where))
            VARIANT_SOURCES[where] = text[:m.end()] + "\n".join(variant)
    results.sort()
    if quiet:
        return base, results
    print(f"{func}: {base} as written; {len(variants)} scopes tried; best:")
    for c, where in results[:8]:
        print(f"  {c:5d}  {where}")
    return 0


def volatiles(c_file, func, quiet=False):
    """Each scalar or pointer local (one declarator to a line, no array) declared volatile in
    turn, compiled as `check` does (FAKE: the game stores such a variable and reloads it at once,
    sndloc.c func_800DA52C's a, sys.c func_80004880's lines). quiet (the sweep): (base, results)
    or None."""
    with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
        text = f.read()
    if not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), text):
        if quiet:
            return None
        sys.exit(f"{func} is not parked in {c_file}")
    m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), text, re.M)
    if not m:
        if quiet:
            return None
        sys.exit(f"{func}: no definition in {c_file}")
    lines = text[m.end():].split("\n")
    end = next(i for i, l in enumerate(lines) if l.startswith("}"))
    variants = []
    for j in range(end):
        w = SCALAR_DECL.match(lines[j])
        if not w or "volatile" in lines[j] or "," in w.group(2) or "[" in w.group(2):
            continue
        variants.append((f"line {j + 1} volatile: '{lines[j].strip()[:50]}'",
                         lines[:j] + [f"{w.group(1)}volatile {w.group(2)} /* FAKE: stored and "
                                      f"reloaded (volatiles) */"] + lines[j + 1:]))
    d = os.path.join(build.ROOT, "permuter", "volatiles", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)
    base = _variant_lines(d, func, want, text)
    base_len = VARIANT_LEN.get(d)
    if not quiet:
        print(f"{base}  as written; {len(variants)} locals", flush=True)
    results = []
    for where, variant in variants:
        c = _variant_lines(d, func, want, text[:m.end()] + "\n".join(variant))
        if not quiet:
            print(f"{c if c is not None else 'fails':>5}  {where}", flush=True)
        if c is not None and base is not None and c < base:
            where += _len_note(d, want, base_len)
            results.append((c, where))
            VARIANT_SOURCES[where] = text[:m.end()] + "\n".join(variant)
    results.sort()
    if quiet:
        return base, results
    print(f"{func}: {base} as written; {len(variants)} volatile locals tried; best:")
    for c, where in results[:8]:
        print(f"  {c:5d}  {where}")
    return 0


NARROW_DECL = re.compile(r"^(\s+)(u8|s8|u16|s16)(\s+\w+\s*(?:=[^;]*)?;.*)$")


def widens(c_file, func, quiet=False):
    """Each narrow scalar local (u8, s8, u16, s16) of a parked function declared s32, then u32,
    in turn, compiled as `check` does. A byte read once and used in tests and a store is an s32
    in the game's source: a u8 local kept a copy in another register (plmain.c func_8018A270,
    39 to a match, IDO_NOTES 260). The C can compute differently (a narrow local truncates):
    read a find before keeping it. quiet (the sweep): return (base, results sorted) or None."""
    with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
        text = f.read()
    if not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), text):
        if quiet:
            return None
        sys.exit(f"{func} is not parked in {c_file}")
    m = re.search(r'^[A-Za-z][^\n;]*\b%s\([^;{]*\)\s*\{\n' % re.escape(func), text, re.M)
    if not m:
        if quiet:
            return None
        sys.exit(f"{func}: no definition in {c_file}")
    lines = text[m.end():].split("\n")
    end = next(i for i, l in enumerate(lines) if l.startswith("}"))
    variants = []
    for j in range(end):
        w = NARROW_DECL.match(lines[j])
        if w:
            for t in ("s32", "u32"):
                variants.append((f"line {j + 1} as {t}: '{lines[j].strip()[:50]}'",
                                 lines[:j] + [w.group(1) + t + w.group(3)] + lines[j + 1:]))
    d = os.path.join(build.ROOT, "permuter", "widens", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)
    base = _variant_lines(d, func, want, text)
    base_len = VARIANT_LEN.get(d)
    if not quiet:
        print(f"{base}  as written; {len(variants)} widenings", flush=True)
    results = []
    for where, variant in variants:
        c = _variant_lines(d, func, want, text[:m.end()] + "\n".join(variant))
        if c is not None and base is not None and c < base:
            where += _len_note(d, want, base_len)
            results.append((c, where))
            VARIANT_SOURCES[where] = text[:m.end()] + "\n".join(variant)
            if not quiet:
                print(f"{c:5d}  {where}", flush=True)
    results.sort()
    if quiet:
        return base, results
    print(f"{func}: {base} as written; {len(variants)} widenings tried; best:")
    for c, where in results[:8]:
        print(f"  {c:5d}  {where}")
    return 0


PARAM = re.compile(r"^(.*?[\s*])([A-Za-z_]\w*)$")  # "GAniFrame *f": the type, then the name
SCALARS = {"s8", "u8", "s16", "u16", "s32", "u32", "s64", "u64", "f32", "f64", "int", "char",
           "short", "long", "float", "double", "unsigned", "signed"}


def copies(c_file, func, quiet=False):
    """IDO_NOTES 241 as a search: each parameter (and each pointer or scalar local, from the
    line that first sets it) of a parked function read through a copy ("T *p_cp = p;") from
    each top-level statement on, every later use renamed, compiled as `check` does. The game keeps a parameter it uses over a stretch in a saved register and
    reaches it there through a copy of its own (polygon.c func_800D60BC matched that way, the
    gani draw func_800D66B8 went 124 to 31). The copy sits at the body's outer level, so it
    runs before every line after it and the C computes the same, unless a goto jumps back over
    it: read a find before keeping it. quiet (the sweep): return (base, results sorted) or None."""
    with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
        text = f.read()
    if not re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), text):
        if quiet:
            return None
        sys.exit(f"{func} is not parked in {c_file}")
    m = re.search(r'^[A-Za-z][^\n;]*\b%s\(([^;{]*)\)\s*\{\n' % re.escape(func), text, re.M)
    if not m:
        if quiet:
            return None
        sys.exit(f"{func}: no definition in {c_file}")
    lines, end, n, params, first_set, starts, straddled = _body_scan(m, text)
    d = os.path.join(build.ROOT, "permuter", "copies", func)
    import_function(c_file, func, d)
    want = disassemble(os.path.join(d, "target.o"), func)
    base = _variant_lines(d, func, want, text)
    base_len = VARIANT_LEN.get(d)
    if not quiet:
        print(f"{base}  as written; variables: {', '.join(name for _, name in params)}", flush=True)
    indent = "    "
    results = []
    tried = 0
    for ptype, name in params:
        use = re.compile(r"(?<!->)(?<!\.)(?<![\w])%s(?!\w)" % re.escape(name))
        cp = f"{name}_cp"
        for j in starts:
            if j <= first_set[name] or j in straddled:
                continue
            if not any(use.search(l) for l in lines[j:end]):
                break
            # renamed up to the function's closing brace only (the rest of the file once had its
            # own parameters of that name renamed too: polygon.c func_800D6FDC)
            body = (lines[n:j] + [f"{indent}{cp} = {name};"]
                    + [use.sub(cp, l) for l in lines[j:end]] + lines[end:])
            decl = f"{indent}{ptype}{'' if ptype.endswith('*') else ' '}{cp};"
            # declared last, and declared first: a register-only local still takes a frame word
            # in declaration order, and first it moves every compiler temp up one (polygon.c
            # func_800D6BE0 matched with its copy declared first)
            for placement, variant in (("", lines[:n] + [decl] + body),
                                       (" (declared first)", [decl] + lines[:n] + body)):
                tried += 1
                c = _variant_lines(d, func, want, text[:m.end()] + "\n".join(variant))
                if c is not None and base is not None and c < base:
                    where = f"{name} through a copy from line {j + 1}{placement}, '{lines[j].strip()[:50]}'"
                    where += _len_note(d, want, base_len)
                    results.append((c, where))
                    VARIANT_SOURCES[where] = text[:m.end()] + "\n".join(variant)
                    if not quiet:
                        print(f"{c:5d}  {where}", flush=True)
    results.sort()
    if quiet:
        return base, results
    print(f"{func}: {base} as written; {tried} copies tried; best:")
    for c, where in results[:8]:
        print(f"  {c:5d}  {where}")
    return 0


# unfakes first: an old FAKE can be what blocks the next step (IDO_NOTES 260)
GREEDY_MODES = ("unfakes", "pads", "joins", "copies", "inlines", "empties", "splits", "rechains",
                "commutes", "compounds", "volatiles", "moves", "scopes")
GREEDY_ESCAPES = int(os.environ.get("GREEDY_ESCAPES", "3"))  # hills tried when a round is stuck


def greedy(c_file, func):
    """The modes that keep what the C computes (GREEDY_MODES: casts and swaps can change it),
    stacked unattended on one parked function. Each round runs them in turn; their finds are
    tried best first in the real source (written to c_file, measured as `check` measures), and
    the first that is BETTER is kept: fewer differing lines with the instruction-mix distance not
    rising, or as many lines with a lower mix (a form closer in its instructions). Every source
    tried is remembered, so nothing is visited twice.
    When a round keeps nothing, greedy is at the bottom of its hill, and that hill can be the
    wrong one (bmove.c func_8017CDF8: the twins' form was 310 against 241, and the way down from
    it led to a match). So it ESCAPES: the variants that round compiled which are closer to the
    game (by mix, or as close in mix with a closer stack map or a smaller t-register shift; see
    _Collected) are ranked by (mix, stack distance, t-shift, lines), and for each of the best
    GREEDY_ESCAPES it steps onto that variant even if its lines rise, descends from
    it as far as the rounds go, and keeps the whole path only if it ends better than where it was
    stuck; otherwise it steps back. It stops when no hill leads lower.
    It works on a copy (permuter/greedy/<func>.c), so the sweeps reading src/ never see a trial;
    at the end, if a step was kept, the copy goes back into c_file (FAKE lines carry their FAKE
    comment). Read the diff, build, then commit."""
    global COLLECT
    real = os.path.join(build.ROOT, c_file)
    os.makedirs(os.path.join(build.ROOT, "permuter", "greedy"), exist_ok=True)
    work = os.path.join("permuter", "greedy", func + ".c")
    path = os.path.join(build.ROOT, work)
    with open(real, encoding="utf-8") as f:
        original = f.read()
    d = os.path.join(build.ROOT, "permuter", "greedy", func)

    stack_now = [0, 0]  # the stack distance and t-shift of the copy as last measured

    def measure():
        diff, want, got, _ = _compare(work, func, d)
        stack_now[0], stack_now[1] = _stack_distance(want, got), _tshift_size(want, got)
        return len(diff), _mix_distance(want, got)

    def write(target, source):
        if not _write_retry(target, source):
            sys.exit(f"{target}: could not write")

    def read():
        with open(path, encoding="utf-8") as f:
            return f.read()

    def better(l2, m2, l1, m1):
        return (l2 < l1 and m2 <= m1) or (l2 == l1 and m2 < m1)

    seen = {original}

    def one_round(lines, mix, collected=None):
        """The first better find of one round, applied to the copy: (mode, where, lines, mix),
        or None (the copy unchanged). collected, if given, is offered every variant compiled."""
        global COLLECT
        for mode in GREEDY_MODES:
            VARIANT_SOURCES.clear()
            if collected is not None:
                collected.mode = mode
            COLLECT = collected
            try:
                r = globals()[mode](c_file, func, quiet=True)
            finally:
                COLLECT = None
            if not r:
                continue
            for c, where in r[1]:
                source = VARIANT_SOURCES.get(where)
                if source is None or c > lines or source in seen:
                    continue
                old = read()
                write(path, source)
                seen.add(source)
                l2, m2 = measure()
                if better(l2, m2, lines, mix):
                    return (mode, where, l2, m2)
                write(path, old)
        return None

    def descend(lines, mix):
        """Rounds from the copy as it is until none is better: the steps and where they end."""
        steps = []
        while lines:
            k = one_round(lines, mix)
            if not k:
                break
            steps.append(k)
            lines, mix = k[2], k[3]
        return steps, lines, mix

    def escape(lines, mix, collected):
        """Step onto each of the best hills in turn and go down from it; the first path that
        ends better than (lines, mix) is kept: [(mode, where, lines, mix), ...], else None."""
        tried = 0
        for hm, hs, ht, hl, hmode, source in collected.best():
            if tried >= GREEDY_ESCAPES:
                break
            if source in seen:
                continue
            tried += 1
            old = read()
            write(path, source)
            seen.add(source)
            l1, m1 = measure()
            hill = ("escape", f"onto a {hmode} variant ({l1} lines, mix distance {m1}, stack "
                              f"distance {stack_now[0]}, t-shift {stack_now[1]})", l1, m1)
            steps, l2, m2 = descend(l1, m1)
            if better(l2, m2, lines, mix):
                return [hill] + steps
            print(f"      (a hill: {hmode} variant at {l1} lines, mix {m1}, led to {l2}/{m2}: not lower)",
                  flush=True)
            write(path, old)
        return None

    write(path, original)
    c_file = work
    lines, mix = measure()
    print(f"{lines:5d} lines, mix distance {mix}: as written", flush=True)
    rounds = 0
    while lines:
        measure()  # the copy as it stands: its stack distance for the escape's filter
        collected = _Collected(lines, mix, stack_now[0], stack_now[1]) if GREEDY_ESCAPES else None
        kept = one_round(lines, mix, collected)
        path_kept = [kept] if kept else (escape(lines, mix, collected) if collected else None)
        if not path_kept:
            break
        for mode, where, l2, m2 in path_kept:
            rounds += 1
            print(f"{l2:5d} lines, mix distance {m2}: + {mode}: {where}", flush=True)
        lines, mix = path_kept[-1][2], path_kept[-1][3]
    note = ""
    if rounds:
        with open(real, encoding="utf-8") as f:
            now = f.read()
        if now == original:
            with open(path, encoding="utf-8") as f:
                write(real, f.read())
            note = f"; written to {os.path.relpath(real, build.ROOT)}"
        else:  # edited meanwhile (by hand, or another greedy): never overwrite it
            note = f"; NOT written ({os.path.relpath(real, build.ROOT)} changed meanwhile): the result is {work}"
    print(f"{func}: {lines} lines, mix distance {mix} after {rounds} kept step(s)" + note)
    return 0


def greedy_sweep(start=0, tag=""):
    """greedy over every parked function, closest first by the last checkall, from the start-th
    on; each finished function goes into permuter/greedy_done<tag>.txt, read again before each
    one, so a restart or a second sweep (started further down) skips those. A new tag is a new
    pass over everything; names written into its list beforehand (functions being worked
    on elsewhere) are skipped. It writes each improved file back into src/: read and commit as the lines
    "written to" appear."""
    done_path = os.path.join(build.ROOT, "permuter", f"greedy_done{tag}.txt")

    def done():
        if not os.path.exists(done_path):
            return set()
        with open(done_path, encoding="utf-8") as f:
            return {l.split("\t")[0].strip() for l in f if l.strip()}

    rank = {}
    table = os.path.join(build.ROOT, "permuter", "checkall.txt")
    if os.path.exists(table):
        with open(table, encoding="utf-8") as f:
            for i, line in enumerate(f):
                parts = line.split("\t")
                if len(parts) > 2:
                    rank[parts[2]] = i
    order = sorted(parked_functions(), key=lambda p: rank.get(p[1], len(rank)))
    for c_file, func in order[start:] + order[:start]:
        if func in done():
            continue
        with open(done_path, "a", encoding="utf-8") as f:
            f.write(func + "\n")  # claimed before it starts, so a second sweep passes it by
        print(f"== {c_file} {func}", flush=True)
        greedy(c_file, func)
    return 0


def _sweep(mode, fn):
    """`mode` over every parked function, closest first by the last checkall, one line each
    where it helps; each function finished goes into permuter/<mode>_done.txt (a restart skips
    those) and the finds into permuter/<mode>.txt."""
    done_path = os.path.join(build.ROOT, "permuter", f"{mode}_done.txt")
    done = {}
    if os.path.exists(done_path):
        with open(done_path, encoding="utf-8") as f:
            for line in f:
                line = line.rstrip("\r\n")
                if line.strip():
                    done[line.split("\t")[0].strip()] = line if "\t" in line else line + "\t"
    rank = {}
    table = os.path.join(build.ROOT, "permuter", "checkall.txt")
    if os.path.exists(table):
        with open(table, encoding="utf-8") as f:
            for i, line in enumerate(f):
                parts = line.split("\t")
                if len(parts) > 2:
                    rank[parts[2]] = i
    for c_file, func in sorted(parked_functions(), key=lambda p: rank.get(p[1], len(rank))):
        if func in done:
            continue
        r = fn(c_file, func, quiet=True)
        line = ""
        if r is not None:
            base, results = r
            if results:
                line = f"{base - results[0][0]:5d}  {c_file} {func}: {base} -> {results[0][0]}, {results[0][1]}"
                print(line, flush=True)
        done[func] = f"{func}\t{line}"
        with open(done_path, "a", encoding="utf-8") as f:
            f.write(done[func] + "\n")
    out = [v.split("\t", 1)[1] for v in done.values() if v.split("\t", 1)[1:] and v.split("\t", 1)[1]]
    with open(os.path.join(build.ROOT, "permuter", f"{mode}.txt"), "w", encoding="utf-8") as f:
        f.write("\n".join(sorted(out, reverse=True)) + "\n")
    return 0


def casts_sweep():
    """`casts` over every parked function, closest first by the last checkall, one line each
    where a cast helps; each function finished goes into permuter/casts_done.txt (a restart
    skips those) and the finds into permuter/casts.txt."""
    done_path = os.path.join(build.ROOT, "permuter", "casts_done.txt")
    done = {}
    if os.path.exists(done_path):
        with open(done_path, encoding="utf-8") as f:
            for line in f:
                line = line.rstrip("\r\n")
                if line.strip():
                    done[line.split("\t")[0].strip()] = line if "\t" in line else line + "\t"
    rank = {}
    table = os.path.join(build.ROOT, "permuter", "checkall.txt")
    if os.path.exists(table):
        with open(table, encoding="utf-8") as f:
            for i, line in enumerate(f):
                parts = line.split("\t")
                if len(parts) > 2:
                    rank[parts[2]] = i
    for c_file, func in sorted(parked_functions(), key=lambda p: rank.get(p[1], len(rank))):
        if func in done:
            continue
        r = casts(c_file, func, quiet=True)
        line = ""
        if r is not None:
            base, results = r
            if results:
                line = f"{base - results[0][0]:5d}  {c_file} {func}: {base} -> {results[0][0]}, {results[0][1]}"
                print(line, flush=True)
        done[func] = f"{func}\t{line}"
        with open(done_path, "a", encoding="utf-8") as f:
            f.write(done[func] + "\n")
    out = [v.split("\t", 1)[1] for v in done.values() if v.split("\t", 1)[1:] and v.split("\t", 1)[1]]
    with open(os.path.join(build.ROOT, "permuter", "casts.txt"), "w", encoding="utf-8") as f:
        f.write("\n".join(sorted(out, reverse=True)) + "\n")
    return 0


def bases():
    """Every parked function whose last checkall diff (permuter/checkall/chk_<func>.log) has %hi
    addresses on one side only: the game's alone and the C's alone. Only a base apart (demo.c:
    the game 0x80088420 + 0xA2, the C 0x80088338 + 0x18A) is a symbol the game named at its own
    address (undefined_syms.txt). Three or more of the game's within 0x100 of each other, with
    the C's base instead, is one absolute load per element: a loop IDO unrolled fully (stage.c
    func_800DB8C4). Report only; run checkall first for fresh logs."""
    hi = re.compile(r'%hi\((0x[0-9A-Fa-f]+)\)')
    base = os.path.join(build.ROOT, "permuter", "checkall")
    for c_file, func in parked_functions():
        log = os.path.join(base, f"chk_{func}.log")
        if not os.path.exists(log):
            continue
        game, c = set(), set()
        with open(log, encoding="utf-8") as f:
            for line in f:
                side = game if line.startswith("-") else c if line.startswith("+") else None
                if side is not None:
                    side.update(int(a, 16) for a in hi.findall(line))
        only_game, only_c = sorted(game - c), sorted(c - game)
        if not only_c:
            continue
        near = max((sum(1 for b in only_game if 0 <= b - a < 0x100) for a in only_game), default=0)
        print(f"{c_file} {func}: the game {' '.join('%X' % a for a in only_game) or '-'}; "
              f"the C {' '.join('%X' % a for a in only_c)}{'  unrolled?' if near >= 3 else ''}",
              flush=True)
    return 0


def unroll_sweep():
    """Every parked function compiled at limits 160 and 640 beside the default: the ones whose
    size there equals the game's while the default's does not have a loop unrolled otherwise
    than the game's (run `unroll src/<f>.c <func>` on each)."""
    for c_file, func in parked_functions():
        d = os.path.join(build.ROOT, "permuter", "unroll", func)
        try:
            import_function(c_file, func, d)
        except SystemExit as e:  # a file mid-edit: report it and go on
            print(f"{c_file} {func}: skipped ({e})", flush=True)
            continue
        obj = os.path.join(d, "unroll.o")
        want = len(disassemble(os.path.join(d, "target.o"), func))
        got = {}
        for limit in (None, 160, 640):
            extra = [] if limit is None else [f"-Wo,-unrolllimit,{limit}"]
            got[limit] = -1 if compile_one(os.path.join(d, "full.c"), obj, extra) else \
                len(disassemble(obj, func))
        if got[None] != want and want in (got[160], got[640]):
            print(f"{c_file} {func}: the game {want}; default {got[None]}, limit 160 {got[160]}, "
                  f"limit 640 {got[640]}", flush=True)
    return 0


def unroll(c_file, func):
    """func's instruction count against uopt's -unrolllimit: the steps tell how heavy IDO finds
    each loop body. uopt unrolls a loop of body B by 4 while 4B <= the limit and by 2 while
    2B <= it, so a step at limit L means a body of about L/4 (or L/2). This build's default limit
    is 320-323 (audio.c compiles alike only there). The game's count says which unrolling it
    has; the step says how many units the C's body must gain or lose to get it at 320."""
    import_function(c_file, func)
    d = os.path.join(build.ROOT, "permuter", func)
    with open(os.path.join(build.ROOT, c_file), encoding="utf-8") as f:
        parked = re.search(r'GLOBAL_ASM\("[^"]*/%s\.s"\)' % re.escape(func), f.read()) is not None
    src = os.path.join(d, "full.c") if parked else os.path.join(build.ROOT, c_file)
    obj = os.path.join(d, "unroll.o")
    sizes = {}

    def size(limit):
        if limit not in sizes:
            extra = [] if limit is None else [f"-Wo,-unrolllimit,{limit}"]
            sizes[limit] = -1 if compile_one(src, obj, extra) else len(disassemble(obj, func))
        return sizes[limit]

    print(f"{func}: the game {len(disassemble(os.path.join(d, 'target.o'), func))} instructions, "
          f"this C {size(None)} at the default limit", flush=True)
    coarse = list(range(16, 1281, 32))
    for lo, hi in zip(coarse, coarse[1:]):
        if size(lo) == size(hi):
            continue
        while hi - lo > 1:  # the count only grows with the limit; bisect to the step
            mid = (lo + hi) // 2
            lo, hi = (mid, hi) if size(mid) == size(lo) else (lo, mid)
        print(f"  limit {lo} -> {hi}: {size(lo)} -> {size(hi)} instructions "
              f"(a body of about {hi // 4} unrolled by 4, or {hi // 2} by 2)", flush=True)
    return 0


def best():
    """For every permuter/<func>/ the decomp-permuter has written output-<score>-<n> into: its
    base score and last iteration (from the permuter/run_*.log that names it), the best score, and
    the best output's changed lines. Read-only."""
    root = os.path.join(build.ROOT, "permuter")
    runs = {}
    for log in glob.glob(os.path.join(root, "run_*.log")):
        with open(log, "rb") as f:
            head = f.read(4096).decode("utf-8", "replace")
            f.seek(max(0, os.path.getsize(log) - 2048))
            tail = f.read().decode("utf-8", "replace")
        m = re.search(r"\[(func_\w+)\] base score = (\d+)", head)
        if m:
            its = re.findall(r"iteration (\d+)", tail)
            runs[m.group(1)] = (int(m.group(2)), its[-1] if its else "?")
    parked = {func for _, func in parked_functions()}
    for d in sorted(glob.glob(os.path.join(root, "func_*"))):
        if os.path.basename(d) not in parked:
            continue
        # A find made on an older base rarely carries over to the C as it is now (file.c
        # func_800047EC's and gameproc.c func_800C7B10's did not), and an import keeps the old
        # output dirs: count only outputs written after base.c.
        import time
        base_mtime = os.path.getmtime(os.path.join(d, "base.c"))
        outs = []
        stale = 0
        for o in glob.glob(os.path.join(d, "output-*-*")):
            m = re.match(r"output-(\d+)-\d+$", os.path.basename(o))
            if m:
                if os.path.getmtime(o) < base_mtime:
                    stale += 1
                    continue
                outs.append((int(m.group(1)), o))
        if not outs:
            continue
        score, o = min(outs)
        func = os.path.basename(d)
        base, it = runs.get(func, ("?", "?"))
        when = time.strftime("%m-%d %H:%M", time.localtime(base_mtime))
        old = f", {stale} older outputs skipped" if stale else ""
        print(f"{func}: best {score} (base {base}, iteration {it}, base.c {when}{old}), "
              f"{os.path.basename(o)}")
        diff = os.path.join(o, "diff.txt")
        if os.path.exists(diff):
            with open(diff, encoding="utf-8", errors="replace") as f:
                lines = [l.rstrip() for l in f if l[:1] in "+-" and not l.startswith(("+++", "---"))]
            minus = [l for l in lines if l[0] == "-"]
            plus = [l for l in lines if l[0] == "+"]
            if len(minus) == len(plus) and any(len(l) > 150 for l in lines):
                # joined macro lines (sameline): show only the tokens that changed
                import difflib
                for a, b in zip(minus, plus):
                    sm = difflib.SequenceMatcher(None, a[1:].split(), b[1:].split())
                    for op, i1, i2, j1, j2 in sm.get_opcodes():
                        if op != "equal":
                            print(f"    {op}: -[{' '.join(a[1:].split()[i1:i2])[:70]}] "
                                  f"+[{' '.join(b[1:].split()[j1:j2])[:70]}]")
            else:
                for l in lines[:6]:
                    print("    " + l.strip()[:150])
    return 0


SCREEN_OPS = ("mul.s", "div.s", "add.s", "sub.s", "neg.s", "abs.s", "sqrt.s", "mul.d", "add.d",
              "cvt.s.w", "cvt.w.s", "cvt.d.s", "cvt.s.d", "trunc.w.s", "mult", "multu", "div", "divu",
              "sll", "sra", "srl", "andi", "xori", "slti", "sltiu", "lb", "lbu", "lh", "lhu", "lw",
              "lwc1", "sb", "sh", "sw", "swc1", "jal", "jalr")
SCREEN_BRANCH = re.compile(r"^(?:b|j|jr|beq|bne|beqz|bnez|blez|bgez|bltz|bgtz|bc1[tf])l?$")


def _reloads(rows):
    """Loads of a field just stored: a load from the offset and base of a store at most 12
    instructions before it, with no branch or call between (one block), the stack aside. IDO
    forwards a stored value within a block, so the game's having more of these than the C means
    a block boundary the C lacks (IDO_NOTES 232)."""
    store = re.compile(r"(?:sb|sh|sw|swc1|sdc1) \S+,(-?\w+)\((\w+)\)$")
    load = re.compile(r"(?:lb|lbu|lh|lhu|lw|lwc1|ldc1) \S+,(-?\w+)\((\w+)\)$")
    n = 0
    for i, r in enumerate(rows):
        m = store.match(r)
        if not m or m.group(2) == "sp":
            continue
        for r2 in rows[i + 1:i + 13]:
            op = r2.split()[0]
            if SCREEN_BRANCH.match(op) or op in ("jal", "jalr"):
                break
            m2 = load.match(r2)
            if m2 and m2.groups() == m.groups():
                n += 1
                break
    return n


TREGS = ["t6", "t7", "t8", "t9", "t0", "t1", "t2", "t3", "t4", "t5"]  # IDO's temporary order


def _frame(rows):
    """The frame size and the callee-saved registers the prologue stores (s0-s8, fp, $f20-$f30).
    The game saving a register the C does not (or a bigger frame) means a variable the C lacks:
    a pointer shared across switch cases that the game kept apart, or locals declared between
    two homed ones (particle.c func_800F69FC; IDO_NOTES 240)."""
    size, saves = 0, set()
    save = re.compile(r"(?:sw|sd|sdc1) (\$?(?:s[0-8]|fp|f2[02468]|f30)),-?\w+\(sp\)$")
    for r in rows[:40]:
        m = re.match(r"addiu sp,sp,-(\d+)$", r)
        if m and not size:
            size = int(m.group(1))
        m = save.match(r)
        if m:
            saves.add(m.group(1).lstrip("$"))
        if r.split()[0] in ("jal", "jalr") or SCREEN_BRANCH.match(r.split()[0]):
            break
    return size, saves


def _tshift(want, got):
    """For a function of the game's length: (delta, first index, pairs) when most differing
    instruction pairs differ only in t-registers, each shifted by the same step along TREGS,
    or None. A shift of +1 means this C allocates one temporary more before that point, -1 one
    fewer: the place to spend (or drop) one (IDO_NOTES 234, 235)."""
    from collections import Counter
    if len(want) != len(got):
        return None
    deltas, first = Counter(), {}
    pairs = 0
    for k, (a, b) in enumerate(zip(want, got)):
        if a == b:
            continue
        pairs += 1
        ta, tb = re.split(r"[ ,()]+", a), re.split(r"[ ,()]+", b)
        if len(ta) != len(tb) or ta[0] != tb[0]:
            continue
        ds = set()
        for x, y in zip(ta, tb):
            if x == y:
                continue
            if x in TREGS and y in TREGS:
                ds.add((TREGS.index(y) - TREGS.index(x)) % len(TREGS))
            else:
                ds = None
                break
        if ds and len(ds) == 1:
            d = ds.pop()
            d = d - len(TREGS) if d > len(TREGS) // 2 else d
            deltas[d] += 1
            first.setdefault(d, k)
    if not deltas:
        return None
    d, n = deltas.most_common(1)[0]
    return (d, first[d], n, pairs) if n * 2 >= pairs else None


def screen():
    """For every parked function with a checkall object (run checkall first): the differences
    that register allocation cannot make. Length, the counts of SCREEN_OPS, the 2^32 fix-ups of
    unsigned-to-float conversions (lui 0x4F80), the sequence of branches and jumps, the order
    of the calls' targets, and (where the game has more) reloads of a field just stored, game
    against C. A function that agrees on all of them differs only in registers and scheduling
    (the permuter's work); one that does not has a source difference to read (IDO_NOTES 220-226
    and 232 came from these)."""
    from collections import Counter
    base = os.path.join(build.ROOT, "permuter", "checkall")
    for c_file, func in parked_functions():
        d = os.path.join(base, func)
        obj = next((os.path.join(d, o) for o in ("full.o", "build.o")
                    if os.path.exists(os.path.join(d, o))), None)
        if obj is None or not os.path.exists(os.path.join(d, "target.o")):
            continue
        want, got = disassemble(os.path.join(d, "target.o"), func), disassemble(obj, func)
        ops = lambda rows: [r.split()[0] for r in rows]
        cw, cg = Counter(ops(want)), Counter(ops(got))
        notes = []
        if len(want) != len(got):
            notes.append(f"length {len(want)}/{len(got)}")
        notes += [f"{op} {cw[op]}/{cg[op]}" for op in SCREEN_OPS if cw[op] != cg[op]]
        fix = lambda rows: sum(1 for r in rows if re.match(r"lui \w+,0x4f80$", r))
        if fix(want) != fix(got):
            notes.append(f"2^32 fix-ups {fix(want)}/{fix(got)}")
        bw = [o for o in ops(want) if SCREEN_BRANCH.match(o)]
        bg = [o for o in ops(got) if SCREEN_BRANCH.match(o)]
        if bw != bg:
            k = next((i for i, (a, b) in enumerate(zip(bw, bg)) if a != b), min(len(bw), len(bg)))
            notes.append(f"branches differ from #{k + 1} ({' '.join(bw[k:k + 3])} / "
                         f"{' '.join(bg[k:k + 3])})")
        # by address: a libultra name and splat's func_ name for one function are the same call
        # (geography.c func_800E0C44: guAlignF, func_80011A40_main)
        cl = lambda rows: [_call_address(r.split()[-1]) for r in rows if r.startswith("jal ")]
        if cl(want) != cl(got):
            k = next((i for i, (a, b) in enumerate(zip(cl(want), cl(got))) if a != b),
                     min(len(cl(want)), len(cl(got))))
            notes.append(f"calls differ from #{k + 1}")
        if _reloads(want) > _reloads(got):
            notes.append(f"reloads {_reloads(want)}/{_reloads(got)}")
        ts = _tshift(want, got)
        if ts:
            notes.append(f"t-shift {ts[0]:+d} from instruction {ts[1] + 1} ({ts[2]} of {ts[3]} "
                         f"differing pairs)")
        fw, fg = _frame(want), _frame(got)
        if fw[0] != fg[0]:
            notes.append(f"frame {fw[0]:#x}/{fg[0]:#x}")
        if fw[1] != fg[1]:
            notes.append(f"saves {' '.join(sorted(fw[1] - fg[1])) or '-'}"
                         f"/{' '.join(sorted(fg[1] - fw[1])) or '-'}")
        if notes:
            print(f"{c_file} {func}: " + "; ".join(notes))
    return 0


def main():
    args = sys.argv[1:]
    if len(args) == 1 and args[0] == "screen":
        return screen()
    if len(args) == 1 and args[0] == "best":
        return best()
    if len(args) == 1 and args[0] == "twins":
        return twins()
    if args[:2] == ["twins", "all"]:
        return twins_all(listing=args[2:] == ["--list"])
    if args and args[0] == "ida-starts":
        return ida_starts(write="--write" in args[1:],
                          only=next((a for a in args[1:] if not a.startswith("--")), None))
    if args[:3] == ["twins", "near", "control"]:
        return twins_near(control=True)
    if args[:2] == ["twins", "near"]:
        rest = args[2:]
        num = [a for a in rest if re.fullmatch(r"\d*\.\d+", a)]
        return twins_near(float(num[0]) if num else 0.8, next((a for a in rest if a not in num), None))
    if args[:2] == ["twins", "files"]:
        return twins_files(show_all=args[2:] == ["--all"])
    if args and args[0] == "objsame" and len(args) == 2:
        c_file = os.path.abspath(args[1])
        rel = os.path.relpath(c_file, build.ROOT).replace(os.sep, "/")
        ref = os.path.join(build.ROOT, "build", os.path.splitext(rel)[0] + ".o")
        out = os.path.join(build.ROOT, "permuter", "objsame", rel.replace("/", "_") + ".o")
        os.makedirs(os.path.dirname(out), exist_ok=True)
        if not os.path.exists(ref) or compile_one(c_file, out) != 0:
            print(f"objsame: {rel}: {'no build object' if not os.path.exists(ref) else 'does not compile'}")
            return 1
        k0, k1 = _hdr_objkey(ref), _hdr_objkey(out)
        diff = sorted(k for k in set(k0) | set(k1) if k0.get(k) != k1.get(k))
        print(f"objsame: {rel}: {'IDENTICAL' if not diff else 'DIFFERS in ' + ', '.join(diff)}")
        return 0 if not diff else 1
    if args and args[0] == "data-c" and len(args) == 3 and args[2] == "--unapply":
        return data_unapply(os.path.abspath(args[1]))
    if args and args[0] == "data-c" and len(args) >= 5 and args[2] == "--move":
        return data_move(os.path.abspath(args[1]), os.path.abspath(args[3]), args[4:])
    if args[:2] == ["data-c", "--retype-list"] and len(args) == 3:
        with open(args[2], encoding="utf-8") as f:
            rows = [l.split() for l in f if l.strip() and not l.startswith("#")]
        return data_retype([(os.path.join(build.ROOT, r[0]), r[1], r[2]) for r in rows])
    if args and args[0] == "data-c" and len(args) >= 4 and args[2] == "--retype":
        return data_retype([(args[1],) + tuple(a.split("=", 1)) for a in args[3:]])
    if args and args[0] == "data-c" and len(args) == 3 and args[2] == "--verify":
        return data_verify(os.path.abspath(args[1]))
    if args and args[0] == "data-c" and len(args) == 3 and args[2] == "--flip":
        if data_verify(os.path.abspath(args[1])) != 0:  # (909): never flip a file whose C lacks its data
            print(f"data-c --flip: REFUSED, {args[1]}'s .data is not its .s (did --apply put it back?)")
            return 1
        return 0 if _data_flip(os.path.abspath(args[1])) else 1
    if args and args[0] == "data-c" and len(args) in (2, 3):
        return data_c(os.path.abspath(args[1]), apply=args[2:] == ["--apply"])
    if args and args[0] == "twin-copy":
        return twin_copy(set(args[1:]) or None)
    if args and args[0] == "m2c-drafts":
        opt = {a.split("=")[0]: a.split("=")[1] for a in args[1:] if a.startswith("--") and "=" in a}
        return m2c_drafts([a for a in args[1:] if not a.startswith("--")] or None,
                          int(opt["--limit"]) if "--limit" in opt else None, apply="--apply" in args,
                          min_size=int(opt.get("--min", "8"), 0),
                          max_size=int(opt["--max"], 0) if "--max" in opt else None)
    if args == ["folded-empties"]:
        return fold_empties()
    if len(args) == 1 and args[0] == "dprintf":
        return dprintf()
    if len(args) == 3 and args[0] == "dprintf":
        return dprintf(args[1], args[2])
    if len(args) == 3 and args[0] == "unroll":
        return unroll(args[1], args[2])
    if len(args) == 1 and args[0] == "unroll":
        return unroll_sweep()
    if len(args) == 1 and args[0] == "protos":
        return protos()
    if len(args) == 1 and args[0] == "calls":
        return calls()
    if len(args) == 1 and args[0] == "bases":
        return bases()
    if len(args) == 3 and args[0] == "pads":
        return pads(args[1], args[2])
    if len(args) == 1 and args[0] == "pads":
        return _sweep("pads", pads)
    if len(args) == 3 and args[0] == "splits":
        return splits(args[1], args[2])
    if len(args) == 4 and args[0] == "splits" and args[3] == "greedy":
        return splits(args[1], args[2], greedy=True)
    if len(args) == 4 and args[0] == "splits" and args[3] == "pairs":
        return splits(args[1], args[2], pairs=True)
    if len(args) == 3 and args[0] == "casts":
        return casts(args[1], args[2])
    if len(args) == 1 and args[0] == "casts":
        return casts_sweep()
    if len(args) == 3 and args[0] == "swaps":
        return swaps(args[1], args[2])
    if len(args) == 3 and args[0] == "hoist":
        r = hoist(args[1], args[2])
        return 0 if r is not None else 1
    if len(args) == 1 and args[0] == "hoist":
        return _sweep("hoist", hoist)
    if len(args) == 5 and args[0] == "move-lines":
        return move_lines(args[1], int(args[2]), int(args[3]), int(args[4]))
    if len(args) == 3 and args[0] == "unpark":
        return unpark(args[1], args[2])
    if len(args) == 3 and args[0] == "macros":
        r = macros(args[1], args[2])
        return 0 if r is not None else 1
    if len(args) == 1 and args[0] == "macros":
        return _sweep("macros", macros)
    if len(args) == 3 and args[0] == "results":
        r = results(args[1], args[2])
        return 0 if r is not None else 1
    if len(args) == 1 and args[0] == "results":
        return _sweep("results", results)
    if len(args) == 3 and args[0] == "spellings":
        r = spellings(args[1], args[2])
        return 0 if r is not None else 1
    if len(args) == 1 and args[0] == "spellings":
        return _sweep("spellings", spellings)
    if len(args) == 1 and args[0] == "swaps":
        return _sweep("swaps", swaps)
    if len(args) == 3 and args[0] == "greedy" and args[1].isdigit():
        return greedy_sweep(int(args[1]), args[2])  # "greedy START TAG": a pass of its own
    if len(args) == 3 and args[0] == "greedy":
        return greedy(args[1], args[2])
    if len(args) == 1 and args[0] == "greedy":
        return greedy_sweep()
    if len(args) == 2 and args[0] == "greedy" and args[1].isdigit():
        return greedy_sweep(int(args[1]))
    if len(args) == 3 and args[0] == "reprotos":
        return reprotos(args[1], args[2])
    if len(args) == 1 and args[0] == "reprotos":
        return _sweep("reprotos", reprotos)
    if len(args) == 3 and args[0] == "rechains":
        return rechains(args[1], args[2])
    if len(args) == 1 and args[0] == "rechains":
        return _sweep("rechains", rechains)
    if len(args) == 3 and args[0] == "commutes":
        return commutes(args[1], args[2])
    if len(args) == 1 and args[0] == "commutes":
        return _sweep("commutes", commutes)
    if len(args) == 3 and args[0] == "widens":
        return widens(args[1], args[2])
    if len(args) == 1 and args[0] == "widens":
        return _sweep("widens", widens)
    if len(args) == 3 and args[0] == "unfakes":
        return unfakes(args[1], args[2])
    if len(args) == 1 and args[0] == "unfakes":
        return _sweep("unfakes", unfakes)
    if len(args) == 4 and args[0] == "unfakes" and args[3] == "apply":
        return 0 if unfakes_apply(args[1], args[2]) else 1
    if len(args) == 2 and args[0] == "unfakes" and args[1] == "apply":
        return unfakes_apply_sweep()
    if len(args) == 3 and args[0] == "journal":
        return journal(args[1], args[2])
    if len(args) in (2, 3) and args[0] == "fields":
        return fields(args[1])  # "dry" as a third argument: validate the map only
    if len(args) == 1 and args[0] == "globals":
        return globals_plan()
    if len(args) == 2 and args[0] == "globals" and args[1].isdigit():
        return globals_plan(int(args[1]))
    if len(args) == 4 and args[0] == "graft":
        return graft(args[1], args[2], args[3])
    if len(args) == 4 and args[0] == "adopt":
        return adopt(args[1], args[2], args[3])
    if len(args) == 3 and args[0] == "copyout":
        return copyout(args[1], args[2])
    if len(args) == 3 and args[0] == "land":
        return land(args[1], args[2])
    if len(args) == 2 and args[0] == "rename":
        return rename(args[1])
    if len(args) == 3 and args[0] == "rename" and args[2] == "dry":
        return rename(args[1], dry=True)
    if len(args) == 5 and args[0] == "ugen-edit":
        return ugen_edit(args[1], int(args[2]), int(args[3]), args[4])
    if len(args) == 3 and args[0] == "trace":
        return trace(args[1], args[2])
    if len(args) in (3, 4, 5) and args[0] == "idiom":  # idiom <c_file> <func> [<index>] [strict]
        rest = args[3:]
        ix = None
        if rest and re.fullmatch(r"\d+(,\d+)+", rest[0]):
            ix = [int(v) for v in rest[0].split(",")]
        elif rest and rest[0].isdigit():
            ix = int(rest[0])
        return idiom(args[1], args[2], ix, strict="strict" in rest)
    if len(args) >= 3 and args[0] == "force":  # force <func> none | <range> <reg> ...
        return force(args[1], args[2:])
    if len(args) == 3 and args[0] == "bsswrap":
        return bsswrap(args[1], int(args[2]))
    if len(args) in (3, 4) and args[0] == "probe":
        return probe(args[1], args[2].split(","), args[3] if len(args) == 4 else None)
    if len(args) == 4 and args[0] == "alias-merge":
        return alias_merge(args[1], args[2], args[3].split(","))
    if len(args) in (4, 5) and args[0] == "alias-uses":
        return alias_uses(args[1], args[2], args[3].split(","), undo=len(args) == 5 and args[4] == "undo")
    if len(args) >= 2 and args[0] == "swaps-apply":
        # swaps-apply <sweep output> [dry] [skip=a,b,...]
        skip = next((a[5:].split(",") for a in args[2:] if a.startswith("skip=")), [])
        return swaps_apply(args[1], dry="dry" in args[2:], skip=set(skip))
    if len(args) in (2, 3) and args[0] == "rodata-c":
        return rodata_c(args[1] if args[1] == "all" else os.path.join(build.ROOT, args[1]),
                        apply=len(args) == 3 and args[2] == "apply")
    if len(args) in (2, 3) and args[0] == "gfx-c":
        return gfx_c(args[1] if args[1] == "all" else os.path.join(build.ROOT, args[1]),
                     apply=len(args) == 3 and args[2] == "apply")
    if len(args) in (4, 5) and args[0] == "rename" and args[2] == "short":
        # rename <map> short "<what they are>" [dry]: short aliases (D_8000040) too, pinned in
        # undefined_syms.txt under that comment
        return rename(args[1], dry=len(args) == 5 and args[4] == "dry", short=args[3])
    if args in (["tlbh"], ["tlbh", "apply"]):
        return tlb_header(apply=args[1:] == ["apply"])
    if args and args[0] == "headers" and len(args) <= 3:  # headers [TREE] [apply]
        rest = args[1:]
        tree = next((a for a in rest if a != "apply"), None)
        return headers(apply="apply" in rest, root=os.path.abspath(tree) if tree else None)
    if len(args) >= 3 and args[0] == "enum-args":  # enum-args HEADER FUNC:POS ... [TREE] [apply]
        rest = args[2:]
        specs = [(a.split(":")[0], int(a.split(":")[1])) for a in rest if re.fullmatch(r"\w+(\{\})?:\d+", a)]
        tree = next((a for a in rest if a != "apply" and not re.fullmatch(r"\w+(\{\})?:\d+", a)), None)
        if not specs:
            sys.exit(__doc__)
        return enum_args(args[1], specs, apply="apply" in rest, root=os.path.abspath(tree) if tree else None)
    if args in (["alias"], ["alias", "apply"]):
        return alias(apply=args[1:] == ["apply"])
    if len(args) == 3 and args[0] == "drops":
        return drops(args[1], args[2])
    if len(args) == 3 and args[0] == "volatiles":
        return volatiles(args[1], args[2])
    if len(args) == 1 and args[0] == "volatiles":
        return _sweep("volatiles", volatiles)
    if len(args) == 3 and args[0] == "reloads":
        return reloads(args[1], args[2])
    if len(args) == 1 and args[0] == "reloads":
        return _sweep("reloads", reloads)
    if len(args) == 3 and args[0] == "empties":
        return empties(args[1], args[2])
    if len(args) == 1 and args[0] == "empties":
        return _sweep("empties", empties)
    if len(args) == 3 and args[0] == "inlines":
        return inlines(args[1], args[2])
    if len(args) == 1 and args[0] == "inlines":
        return _sweep("inlines", inlines)
    if len(args) == 3 and args[0] == "joins":
        return joins(args[1], args[2])
    if len(args) == 1 and args[0] == "joins":
        return _sweep("joins", joins)
    if len(args) == 3 and args[0] == "scopes":
        return scopes(args[1], args[2])
    if len(args) == 1 and args[0] == "scopes":
        return _sweep("scopes", scopes)
    if len(args) == 3 and args[0] == "compounds":
        return compounds(args[1], args[2])
    if len(args) == 1 and args[0] == "compounds":
        return _sweep("compounds", compounds)
    if len(args) == 3 and args[0] == "moves":
        return moves(args[1], args[2])
    if len(args) == 1 and args[0] == "moves":
        return _sweep("moves", moves)
    if len(args) == 3 and args[0] == "copies":
        return copies(args[1], args[2])
    if len(args) == 1 and args[0] == "copies":
        return _sweep("copies", copies)
    if len(args) == 1 and args[0] == "splits":
        return splits_sweep()
    if len(args) in (1, 2) and args[0] == "checkall":
        return checkall(args[1] if len(args) == 2 else os.path.join(build.ROOT, "permuter", "checkall.txt"))
    if len(args) in (2, 3) and args[0] == "basecheck":
        return basecheck(args[1], args[2] if len(args) == 3 else None)
    if len(args) == 4 and args[0] == "import":
        return import_function(args[1], args[2], args[3])
    if len(args) == 3 and args[0] == "import":
        import_function(args[1], args[2])
        return 0
    if len(args) in (3, 4) and args[0] == "check":
        return check(args[1], args[2], args[3] if len(args) == 4 else None)
    if len(args) >= 4 and args[0] == "compile" and args[2] == "-o":
        return compile_one(args[1], args[3], args[4:])  # extra flags, e.g. -DNON_MATCHING
    sys.exit(__doc__)


if __name__ == "__main__":
    sys.exit(main())
