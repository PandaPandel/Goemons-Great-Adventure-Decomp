"""Decompilation progress: functions (and their bytes) in C vs. still assembly, per segment.

    python canon-decompilation-tools/report_progress.py

A function is counted from splat's `nonmatching NAME, SIZE` directives, in the files the CURRENT
linker script uses: asm/X.s for every build/asm/X.o, and asm/nonmatchings/NAME/ for every
build/src/NAME.o. (splat never deletes the old asm/X.s when a file becomes C, so walking asm/
double-counts.) A function is "in C" when its C file no longer pulls it in with
#pragma GLOBAL_ASM. Everything else is still assembly.

A libultra object the linker script takes from build/lib/libultra_rom.a is C (lib/ultralib's
source): its functions are the code symbols in its .text. A zlib object from
build/lib/libz.a likewise (lib/zlib's source, canon-decompilation-tools/build_zlib.py).

The denominator is NOT taken on trust from the yaml. Before the percentage, every byte of the
working ROM is read for code (survey_rom.code_runs): each counted segment's code read is set
against its counted bytes, and code anywhere the count does not reach is listed as NOT COUNTED.
The final line is out of all the code in the ROM. Then what "fully done" also needs: address
names, data still assembly, FAKE lines, parked functions, raw assets.

Changelog: first public release (2026-10-20). Earlier history is kept privately.
"""

import collections
import os
import re
import subprocess

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
NONMATCHING = re.compile(r"^nonmatching\s+(\w+),\s*(0x[0-9A-Fa-f]+|\d+)", re.M)
SEGMENT = re.compile(r"_(main|ovl_\d+|tlb)_?\d*$")


def segment_of(name):
    """main, ovl_00NN, or tlb: the archive's TLB code files (tlb_0016..tlb_0327), one row for all 311."""
    m = SEGMENT.search(name)
    return m.group(1) if m else "main"


def file_segments():
    """Each source file's segment, read off the yaml (functions with real names carry no
    _main/_ovl suffix to tell it)."""
    seg, out = None, {}
    with open(os.path.join(ROOT, "goemonsgreatadv.yaml"), encoding="utf-8") as f:
        for line in f:
            m = re.match(r"^  - name:\s*(\w+)", line)
            if m:
                seg = "tlb" if m.group(1).startswith("tlb_") else m.group(1)  # one row for all 311
                continue
            m = re.match(r"^\s*- \[0x[0-9A-Fa-f]+,\s*(?:c|hasm|\.data|\.rodata|\.bss),\s*([\w/]+)", line)
            if m and seg:
                out.setdefault(m.group(1), seg)
            m = re.match(r"^\s*- \{.*\bname:\s*([\w/]+)", line)
            if m and seg:
                out.setdefault(m.group(1), seg)
    return out


CODE_TYPES = ("c", "asm", "hasm", "lib")  # lib: libultra/zlib objects, counted from their .text


def yaml_pieces():
    """[(rom start, rom end, segment, subsegment type, subsegment name)] over the whole working
    ROM, from the yaml: a segment without subsegments is one piece of its own type. A lib row
    names its section (5th field, or section:); one that is not .text has type lib.data,
    lib.rodata or lib.bss, which is not code."""
    import yaml
    with open(os.path.join(ROOT, "goemonsgreatadv.yaml"), encoding="utf-8") as f:
        segs = yaml.safe_load(f)["segments"]
    tops = []
    for s in segs:
        if isinstance(s, dict):
            tops.append((s["start"], s.get("name"), s.get("type"), s.get("subsegments") or []))
        elif isinstance(s, list) and s:
            tops.append((s[0], None, None, []))
    pieces = []
    for (start, name, kind, subs), nxt in zip(tops, tops[1:]):
        end = nxt[0]
        if not subs:
            pieces.append((start, end, name, kind, name))
            continue
        rows = []
        for sub in subs:
            if isinstance(sub, dict):
                if "start" in sub:  # a bss subsegment has no ROM bytes, so no start
                    t, sec = sub.get("type"), sub.get("section")
                    rows.append((sub["start"], t + sec if t == "lib" and sec not in (None, ".text") else t,
                                 sub.get("name", "")))
            else:
                t = sub[1] if len(sub) > 1 else "bin"
                if t == "lib" and len(sub) > 4 and sub[4] != ".text":
                    t += sub[4]
                rows.append((sub[0], t, sub[2] if len(sub) > 2 else ""))
        for (s0, t, n), after in zip(rows, rows[1:] + [(end, None, None)]):
            pieces.append((s0, after[0], name, t, n))
    return pieces


def ida_code_rom(work_offsets):
    """IDA's code as sorted (start, end) ranges of the working ROM, from every export
    (ida_dump_funcs.py export:), placed by where each database's slice sits (an archive file at
    its work offset, main at ROM 0x1000). The code is each IDA function's bytes from its first
    instruction that is not a nop (a function of nops only is alignment padding), and each word
    outside every function that is not a nop.
    None without exports."""
    import json
    d = os.path.join(os.path.dirname(ROOT), "ida", "goemons_great_adventure", "export")
    if not os.path.isdir(d):
        return None
    out = []
    for n in os.listdir(d):
        if not n.endswith(".json"):
            continue
        with open(os.path.join(d, n), encoding="utf-8") as f:
            exp = json.load(f)
        m = re.match(r"^(\d{4})_([0-9A-F]{8})\.bin\.i64$", exp.get("idb", ""))
        if m and int(m.group(1)) in work_offsets:
            base_r, base_v = work_offsets[int(m.group(1))], int(m.group(2), 16)
        elif exp.get("idb") == "main.bin.i64":
            base_r, base_v = 0x1000, 0x80000400
        else:
            continue
        for fn in exp["functions"]:
            real = [ea for ea, _, m, _, _ in fn["ins"] if m != "nop"]
            if fn["start"] is None:  # the code outside every function: its non-nop words
                out += [(base_r + ea - base_v, base_r + ea + 4 - base_v) for ea in real]
            elif real:  # from its first instruction that is not a nop (leading nops are padding)
                out.append((base_r + min(real) - base_v, base_r + fn["end"] - base_v))
    return sorted(out)


def text_ranges():
    """{database: [(vram lo, vram hi)]}: the code subsegments (CODE_TYPES) of entry, main and each
    overlay in memory, adjacent ones merged; entry and main share main.bin.i64."""
    import yaml
    with open(os.path.join(ROOT, "goemonsgreatadv.yaml"), encoding="utf-8") as f:
        segs = {s["name"]: s for s in yaml.safe_load(f)["segments"] if isinstance(s, dict) and "vram" in s}
    out = collections.defaultdict(list)
    for start, end, seg, kind, _ in yaml_pieces():
        if seg not in segs or kind not in CODE_TYPES or end <= start:
            continue
        s = segs[seg]
        db = "main" if seg in ("entry", "main") else "%04d" % int(seg[len("ovl_"):])
        lo, hi = s["vram"] + start - s["start"], s["vram"] + end - s["start"]
        r = out[db]
        if r and r[-1][1] == lo:
            r[-1] = (r[-1][0], hi)
        else:
            r.append((lo, hi))
    return dict(out)


IDA_DIR = os.path.join(os.path.dirname(ROOT), "ida", "goemons_great_adventure")


def print_text_gaps(decode=False, only=()):
    """--text-ranges: each database's code ranges, and the bytes in them its export does not
    have as code, as the ida_dump_funcs.py decode:A:B arguments that would decode them.
    decode (--decode-gaps [NAME...]): run those, and export: again, for every database with a
    gap (or only the NAMEs: main, 0006, 0123)."""
    import json
    d = os.path.join(IDA_DIR, "export")
    jobs = []
    for db, ranges in sorted(text_ranges().items()):
        p = os.path.join(d, db + ".json")
        words = []
        if os.path.exists(p):
            with open(p, encoding="utf-8") as f:
                exp = json.load(f)
            words = sorted(map(tuple, exp["code_words"]))
        gaps = []
        for lo, hi in ranges:
            at = lo
            for a, b in words:
                if b <= at or a >= hi:
                    continue
                if a > at:
                    gaps.append((at, a))
                at = max(at, b)
            if at < hi:
                gaps.append((at, hi))
        size = sum(hi - lo for lo, hi in ranges)
        print(f"{db}: code {size} bytes in {len(ranges)} range(s) "
              + " ".join(f"{lo:08X}..{hi:08X}" for lo, hi in ranges)
              + (f"; export missing" if not words else
                 f"; not code in the export: {sum(b - a for a, b in gaps)} bytes in {len(gaps)} gap(s)"))
        if words and gaps:
            args = [f"decode:{a:08X}:{b:08X}" for a, b in gaps]
            print("  " + " ".join(args))
            idb = os.path.join(IDA_DIR, exp["idb"]) if exp["idb"] == "main.bin.i64" else \
                os.path.join(IDA_DIR, "files", exp["idb"])
            jobs.append((db, idb, args + ["export:" + p]))
    if decode and jobs:
        decode_gaps([j for j in jobs if not only or j[0] in only])


def decode_gaps(jobs):
    """Each job (name, database, dumper arguments) through ida_dump_funcs.py with the system
    python (idalib), two at a time, below normal priority and with no console window."""
    import concurrent.futures
    dumper = os.path.join(ROOT, "canon-decompilation-tools", "ida_dump_funcs.py")
    flags = 0x00004000 | 0x08000000  # BELOW_NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW

    def one(job):
        name, idb, args = job
        r = subprocess.run(["py", dumper] + args, env=dict(os.environ, IDB=idb), capture_output=True,
                           text=True, encoding="utf-8", errors="replace", creationflags=flags)
        made = re.findall(r"decode \w+\.\.\w+: (\d+) instruction\(s\), (\d+) function", r.stdout)
        return name, r.returncode, sum(int(a) for a, _ in made), sum(int(b) for _, b in made), \
            (r.stdout + r.stderr).strip().splitlines()[-1:] if r.returncode else []

    with concurrent.futures.ThreadPoolExecutor(2) as pool:
        for name, rc, ins, fns, tail in pool.map(one, jobs):
            print(f"{name}: rc {rc}, {ins} instruction(s) and {fns} function(s) created"
                  + (f"  FAILED: {tail}" if rc else ""), flush=True)


def code_inventory(tbytes, tfile=None):
    """Every byte of code in the working ROM, found by READING it (survey_rom.code_runs), set
    against what the table counts. tfile: counted bytes per TLB archive file,
    set against IDA's per file. Returns the whole ROM's code bytes."""
    import bisect
    import survey_rom
    with open(os.path.join(ROOT, "baserom.us.decompressed.z64"), "rb") as f:
        rom = f.read()
    files = []  # (work offset, end, archive file id), to name the file a run sits in
    with open(os.path.join(ROOT, "nisitenma.tsv"), encoding="utf-8") as f:
        for line in f.read().splitlines()[1:]:
            p = line.split("\t")
            if p[0].isdigit() and int(p[3], 16):
                files.append((int(p[2], 16), int(p[2], 16) + int(p[3], 16), int(p[0])))
    files.sort()
    fstarts = [a for a, _, _ in files]

    def file_at(off):
        i = bisect.bisect_right(fstarts, off) - 1
        return files[i][2] if i >= 0 and files[i][0] <= off < files[i][1] else None

    ida_spans = ida_code_rom({fid: a for a, _, fid in files})
    ida_starts = [a for a, _ in ida_spans] if ida_spans else []

    def ida_bytes_in(lo, hi):
        """Bytes of IDA's code inside [lo, hi) of the working ROM."""
        n, i = 0, max(bisect.bisect_right(ida_starts, lo) - 1, 0)
        while ida_spans and i < len(ida_spans) and ida_spans[i][0] < hi:
            a, b = ida_spans[i]
            n += max(0, min(b, hi) - max(a, lo))
            i += 1
        return n

    read, ida, uncounted = collections.Counter(), collections.Counter(), []
    ida_file = collections.Counter()
    for start, end, seg, kind, name in yaml_pieces():
        if kind in CODE_TYPES and seg and ida_spans:
            n = ida_bytes_in(start, end)
            ida["tlb" if seg.startswith("tlb_") else seg] += n
            if seg.startswith("tlb_"):
                ida_file[int(seg[4:8])] += n
        for s, e, r in survey_rom.code_runs(rom[start:end]):
            if kind in CODE_TYPES and seg:
                read["tlb" if seg.startswith("tlb_") else seg] += e - s
            else:
                fid = file_at(start + s)
                uncounted.append((e - s, r, start + s, seg, kind, name, fid))
    print("\nthe whole ROM read for code (IDA's code words from its exports; rabbitizer's runs beside "
          "them), against the count above:")
    print(f"{'segment':<10} {'IDA code':>10} {'rabbitizer':>10} {'counted':>10}")
    whole = 0
    for seg in sorted(set(read) | set(tbytes) | set(ida), key=lambda s: (s != "main", s)):
        found = ida[seg] if ida_spans else read[seg]
        flag = "  <-- code the count misses" if found > tbytes[seg] + 64 else ""
        print(f"{seg:<10} {(ida[seg] if ida_spans else '-'):>10} {read[seg]:>10} {tbytes[seg]:>10}{flag}")
        whole += max(found, tbytes[seg])
    if tfile and ida_file:
        fids = sorted(set(ida_file) | set(tfile))
        more = [(fid, ida_file[fid], tfile[fid]) for fid in fids if ida_file[fid] > tfile[fid]]
        less = [fid for fid in fids if ida_file[fid] < tfile[fid]]
        print(f"TLB files where IDA finds MORE code than the count (IDA/counted): "
              + (", ".join(f"{fid} {a}/{b}" for fid, a, b in more) if more else "none")
              + f"; files where it finds less (splat's sizes keep trailing alignment nops): {len(less)}")
    nc = sum(u[0] for u in uncounted)
    if uncounted:
        print(f"NOT COUNTED: {nc} bytes of code in {len(uncounted)} runs where the count reaches none:")
        for size, r, off, seg, kind, name, fid in sorted(uncounted, reverse=True)[:25]:
            where = f"{seg} [{kind}{', ' + name if name and name != seg else ''}]"
            tiny = "  (under 16 bytes: may be data)" if size < 16 else ""
            print(f"  ROM 0x{off:07X}  {size:>7} bytes  {r:>4} returns  {where}"
                  f"{f'  archive file {fid}' if fid else ''}{tiny}")
        if len(uncounted) > 25:
            print(f"  ... and {len(uncounted) - 25} more runs")
    else:
        print("NOT COUNTED: none (every run of code in the ROM lies in a counted segment)")
    return whole + nc


def unfinished(functions):
    """What "fully done" (the sm64 standard) needs beyond functions in C, counted from the tree.
    functions: every function name the table counted."""
    data_asm = data_bytes = 0
    for start, end, seg, kind, name in yaml_pieces():
        if seg and kind in ("data", "rodata", "bss", "sbss"):
            data_asm += 1
            data_bytes += end - start
    srcs = []
    for top in ("src", "include"):
        for d, _, ns in os.walk(os.path.join(ROOT, top)):
            srcs += [os.path.join(d, n) for n in ns if n.endswith((".c", ".h"))]
    dnames, fake, parked = set(), 0, 0
    unk_decl = unk_use = argn = 0  # fields named by offset, their uses; arg<N> names
    marks = collections.Counter()  # (file, first 60 chars after FAKE) -> marks in the tree
    for p in srcs:
        with open(p, encoding="utf-8") as f:
            text = f.read()
        dnames.update(re.findall(r"\bD_[0-9A-Fa-f]{7,8}\w*", text))
        unk_use += len(re.findall(r"(?:\.|->)\s*(?:unk|field)_?[0-9A-Fa-f]+\w*", text))
        unk_decl += len(re.findall(r"^[^\n;(]*[\s*](?:unk|field)_?[0-9A-Fa-f]+\w*\s*(?:\[[^\]\n]*\]\s*)*;",
                                   text, re.M))
        argn += len(re.findall(r"\barg[0-9]+\b", text))
        rel = os.path.relpath(p, ROOT).replace(os.sep, "/")
        for m in re.finditer(r"(?://|/\*)\s*FAKE([^\n]*)", text):
            fake += 1
            marks[(rel, fake_key(m.group(1)))] += 1
        parked += len(re.findall(r"^#ifdef NON_MATCHING", text, re.M))
    kept, stale = fake_kept(marks)
    unnamed = [f for f in functions if re.match(r"func_[0-9A-Fa-f]{8}", f)]
    print("\nwhat fully done also needs (the sm64 standard), counted from the tree:")
    print(f"  functions still named by address (func_), as of the last split: {len(unnamed)}")
    print(f"  data symbols still named by address (D_), in src/ and include/: {len(dnames)}")
    print(f"  struct fields named by offset (unk<hex>, field_<hex>): {unk_decl} declared, "
          f"{unk_use} uses in the code;  arg<N> parameter/local names: {argn}")
    print(f"  data subsegments still assembly (yaml data/rodata/bss): {data_asm} ({data_bytes} bytes)")
    print(f"  FAKE lines: {fake} ({kept - FAKEMATCHES[0]} kept as sm64/OoT keep them, "
          f"{FAKEMATCHES[0]} kept fakematches, cited in fake_kept.txt; "
          f"{fake - kept} to review)    parked (NON_MATCHING) functions: {parked}")
    for line in stale[:10]:
        print(f"  fake_kept.txt: {line}")
    if len(stale) > 10:
        print(f"  fake_kept.txt: ... and {len(stale) - 10} more")
    raw = rawtex = None
    assets = os.path.join(ROOT, "assets")
    if os.path.isdir(assets):
        raw = len(os.listdir(os.path.join(assets, "archive"))) if os.path.isdir(os.path.join(assets, "archive")) else 0
        ptx = os.path.join(assets, "ptx")
        rawtex = 0
        if os.path.isdir(ptx):  # a .bin counts only when no PNG of it exists beside it
            have = set(os.listdir(ptx))
            rawtex = sum(1 for n in have if n.endswith(".bin")
                         and n[:-4] + ".tex.png" not in have and n[:-4] + ".png" not in have)
        print(f"  assets still raw bytes: {raw} archive files, {rawtex} textures (asset_build.py extract)")
    else:
        print("  assets: not extracted here (no assets/): run a --split build first")
    return {"funcs": len(functions), "unnamed": len(unnamed), "dnames": len(dnames), "fake": fake,
            "unk_decl": unk_decl, "unk_use": unk_use, "argn": argn, "kept": kept, "parked": parked, "data_asm": data_asm, "raw": raw, "rawtex": rawtex}


def local_prototypes(show=False):
    """Prototypes a .c file writes for a function it does not define (each
    function is declared once, in a header, as sm64 does). A line at column 0 ending in `);`
    that names a function is a prototype; one followed by `{` before any `;` is a definition.
    Returns (count in main files, count in TLB files, set of the functions so declared)."""
    proto = re.compile(r"^(?!typedef|#|return|else|case)[A-Za-z_][\w \t*]*?\b([A-Za-z_]\w*)\s*\(([^;{}]*)\)\s*;")
    head = re.compile(r"^(?!typedef|#|extern|struct|union|enum|if|for|while|switch|return)"
                      r"[A-Za-z_][\w \t*]*?\b([A-Za-z_]\w*)\s*\(")
    main = tlb = 0
    names = set()
    for d, _, ns in os.walk(os.path.join(ROOT, "src")):
        for n in ns:
            if not n.endswith(".c"):
                continue
            with open(os.path.join(d, n), encoding="utf-8") as f:
                lines = f.read().split("\n")
            defined, protos = set(), []
            for i, line in enumerate(lines):
                m = proto.match(line)
                if m:
                    if re.search(r"(?://|/\*)\s*FAKE", line):
                        continue  # a marked prototype: counted under FAKEs (878)
                    fp = line[m.end(1):].lstrip().startswith("(*")  # `void (*table[])(void);`
                    if "(*" not in line.split(m.group(1))[0][-3:] and not fp:  # not a function pointer
                        protos.append((m.group(1), i + 1))
                    continue
                m = head.match(line)
                if m:  # a definition when `{` comes before any `;`
                    rest = "\n".join(lines[i:i + 8])
                    b, s = rest.find("{"), rest.find(";")
                    if b >= 0 and (s < 0 or b < s):
                        defined.add(m.group(1))
            for name, lineno in protos:
                if name not in defined:
                    names.add(name)
                    if show:
                        print(f"{os.path.relpath(os.path.join(d, n), ROOT)}:{lineno}: {name}")
                    if "tlb" in os.path.relpath(d, ROOT).split(os.sep):
                        tlb += 1
                    else:
                        main += 1
    return main, tlb, names


def list_fakes():
    """--fakes: every FAKE mark fake_kept.txt does not keep, as file:line and its text."""
    kept = collections.Counter()
    path = os.path.join(ROOT, "canon-decompilation-tools", "fake_kept.txt")
    if os.path.exists(path):
        with open(path, encoding="utf-8") as f:
            for line in f:
                cols = [c.strip() for c in line.split("|")]
                if line.strip() and not line.startswith("#") and len(cols) >= 4 \
                        and re.search(r"\S+-decomp\s+\S+:\d+", cols[2]):
                    kept[(cols[0], fake_key(cols[1]))] += 1
    n = 0
    for top in ("include", "src"):
        for d, _, ns in sorted(os.walk(os.path.join(ROOT, top))):
            for name in sorted(ns):
                if not name.endswith((".c", ".h")):
                    continue
                p = os.path.join(d, name)
                rel = os.path.relpath(p, ROOT).replace(os.sep, "/")
                with open(p, encoding="utf-8") as f:
                    for i, line in enumerate(f, 1):
                        for m in re.finditer(r"(?://|/\*)\s*FAKE([^\n]*)", line):
                            key = (rel, fake_key(m.group(1)))
                            if kept[key] > 0:
                                kept[key] -= 1
                                continue
                            n += 1
                            print(f"{rel}:{i}: {fake_key(m.group(1))}")
    print(f"{n} FAKE marks to review")


def fake_key(rest):
    """A FAKE mark's key: the text after FAKE on its line, without the comment's end, leading
    punctuation or repeated spaces, cut to 60 characters."""
    rest = rest.split("*/")[0]
    return " ".join(rest.lstrip(":,. ").split())[:60]


FAKEMATCHES = [0]  # kept lines whose fourth column starts with "fakematch" (881)


def fake_kept(marks):
    """How many of the tree's FAKE marks fake_kept.txt keeps, and its lines that keep nothing.
    marks: Counter of (file, fake_key) found in src/ and include/. A mark
    of a kind sm64/OoT keep counts as met when cited. A line without a precedent is refused."""
    path = os.path.join(ROOT, "canon-decompilation-tools", "fake_kept.txt")
    if not os.path.exists(path):
        return 0, []
    left, kept, stale = collections.Counter(marks), 0, []
    with open(path, encoding="utf-8") as f:
        for n, line in enumerate(f, 1):
            if not line.strip() or line.startswith("#"):
                continue
            cols = [c.strip() for c in line.split("|")]
            if len(cols) < 4 or not re.search(r"\S+-decomp\s+\S+:\d+", cols[2]):
                stale.append(f"line {n} refused: no precedent `<decomp> <file>:<line>` in column 3")
                continue
            key = (cols[0], fake_key(cols[1]))
            if left[key] > 0:
                left[key] -= 1
                kept += 1
                if cols[3].lower().startswith("fakematch"):
                    FAKEMATCHES[0] += 1  # kept as a fakematch, not a natural kind
            else:
                stale.append(f"line {n} matches no FAKE mark in the tree: {cols[0]} | {cols[1][:40]}")
    return kept, stale


def done_report(code_done, code_total, left):
    """The definition of done: the criteria, each its own count; DONE
    only when all are met. No blended percentage."""
    named = left["funcs"] - left["unnamed"]
    crit = [
        ("code: every function and data block in C, byte-matching",
         code_done == code_total and left["parked"] == 0 and left["data_asm"] == 0,
         f"{code_done} / {code_total} functions in C, {left['parked']} parked, "
         f"{left['data_asm']} data subsegments still assembly"),
        ("names: every name real (functions, globals, struct fields, parameters)",
         left["unnamed"] == 0 and left["dnames"] == 0 and left["unk_decl"] == 0 and left["argn"] == 0,
         f"{named} / {left['funcs']} functions named; {left['unnamed']} functions and "
         f"{left['dnames']} data symbols still named by address; {left['unk_decl']} struct fields "
         f"named by offset ({left['unk_use']} uses); {left['argn']} arg<N> names"),
        ("FAKEs: none that natural C can replace",
         left["fake"] - left["kept"] == 0,
         f"{left['fake'] - left['kept']} FAKE marks to review; {FAKEMATCHES[0]} kept fakematches, marked "
         f"as OoT keeps its own; {left['kept'] - FAKEMATCHES[0]} kept as sm64/OoT "
         f"keep them, each cited (fake_kept.txt)"),
        ("assets: in real formats",
         left["raw"] == 0 and left["rawtex"] == 0,
         f"{left['raw']} archive files and {left['rawtex']} textures still raw bytes"
         if left["raw"] is not None else "not extracted here (no assets/)"),
    ]
    outside = []  # the build's tool defaults that reach outside the repository (ROOT, "..")
    for tool in ("build_rom.py", "build_libultra.py", "build_zlib.py"):
        path = os.path.join(ROOT, "canon-decompilation-tools", tool)
        if not os.path.exists(path):
            continue
        name = None
        with open(path, encoding="utf-8") as f:
            for line in f:
                m = re.match(r"(\w+)\s*=", line)
                name = m.group(1) if m else name
                if re.search(r'ROOT,\s*"\.\."', line):
                    outside.append(f"{tool[:-3]}.{name}")
    pmain, ptlb, pnames = local_prototypes()
    crit.append(("headers: each function declared once, in a header (as sm64)",
                 pmain + ptlb == 0,
                 f"{pmain + ptlb} local prototypes of functions defined elsewhere ({pmain} in main "
                 f"files, {ptlb} in TLB files), for {len(pnames)} functions"))
    crit.append(("build: usable like sm64's, from a fresh clone",
                 not outside,
                 f"tool defaults outside the repository: {', '.join(outside)}" if outside
                 else "every tool default resolves inside the repository"))
    met = sum(1 for _, ok, _ in crit if ok)
    print(f"\nDONE: {'YES' if met == len(crit) else 'NO'} ({met} of {len(crit)} "
          f"criteria met; no blended percentage)")
    for name, ok, detail in crit:
        print(f"  [{'met' if ok else 'NOT met'}] {name}: {detail}")


def unverified():
    """src/ and include/ files that differ from the last commit (modified, staged or untracked):
    work no full build has proven yet. Named before any figure."""
    r = subprocess.run(["git", "status", "--porcelain", "--", "src", "include"], cwd=ROOT,
                       capture_output=True, text=True)
    return [line[3:] for line in r.stdout.splitlines() if line.strip()]


def main():
    dirty = unverified()
    if dirty:
        print(f"UNVERIFIED: {len(dirty)} file(s) in src/ or include/ differ from the last commit; the "
              f"figures below INCLUDE them and are not proven until a full build and a commit: "
              + ", ".join(dirty[:8]) + (" ..." if len(dirty) > 8 else ""))
    pulled = set()
    for d, _, names in os.walk(os.path.join(ROOT, "src")):
        for n in names:
            if n.endswith(".c"):
                with open(os.path.join(d, n), encoding="utf-8") as f:
                    pulled.update(re.findall(r'GLOBAL_ASM\("([^"]+)"\)', f.read()))
    pulled = {p.replace("\\", "/") for p in pulled}
    with open(os.path.join(ROOT, "goemonsgreatadv.ld")) as f:
        ld = f.read()
    objs = set(re.findall(r"build/((?:asm|src)/[\w/.-]+)\.o\(", ld))
    members = set(re.findall(r"build/lib/libultra_rom\.a:(\w+)\.o\(", ld))
    zmembers = set(re.findall(r"build/lib/libz\.a:(\w+)\.o\(", ld))
    files = [o + ".s" for o in objs if o.startswith("asm/")]
    for o in objs:
        if o.startswith("src/"):
            # a parked function's asm is in asm/nonmatchings/, a matched one's in asm/matchings/
            # (splat's disassemble_all); both count, a pulled-in (GLOBAL_ASM) one as not done
            for kind in ("nonmatchings", "matchings"):
                folder = os.path.join(ROOT, "asm", kind, o[len("src/"):])
                if os.path.isdir(folder):
                    files += ["asm/%s/%s/%s" % (kind, o[len("src/"):], n)
                              for n in os.listdir(folder) if n.endswith(".s")]
    total, done = collections.Counter(), collections.Counter()
    tbytes, dbytes = collections.Counter(), collections.Counter()
    tfile = collections.Counter()  # counted bytes per TLB archive file, against IDA's
    # The original's hand-written assembly kept as SOURCE in src/ (splat's hasm, the entry point
    # and rspboot): done, as in the decomps before this one. Each glabel to its endlabel.
    source_asm = []
    fseg = file_segments()
    for o in objs:
        path = os.path.join(ROOT, o + ".s")
        if o.startswith("src/") and os.path.exists(path):
            with open(path, encoding="utf-8") as f:
                text = f.read()
            for name in re.findall(r"^glabel\s+(\w+)", text, re.M):
                body = re.search(r"^glabel\s+%s\b(.*?)^endlabel\s+%s\b" % (name, name), text,
                                 re.M | re.S)
                size = 4 * len(re.findall(r"^\s+[a-z][a-z0-9.]*\b(?!:)", body.group(1), re.M)) if body else 0
                if size == 0:
                    continue  # a label on an .incbin (the microcodes' binaries): not a function
                seg = fseg.get(o[len("src/"):], segment_of(name))
                for c in (total, done):
                    c[seg] += 1
                for c in (tbytes, dbytes):
                    c[seg] += size
                source_asm.append(name)
    handwritten = []  # spimdisasm's "Handwritten function": original assembly, it stays .s
    names = set(source_asm)  # every function counted, for the address-name line
    # a re-split leaves a matched function's old .s in asm/nonmatchings/ beside its new one in
    # asm/matchings/: count each name once (matchings first, so a stale copy is the one dropped)
    # the names that still exist: a renamed function's old .s stays after a re-split
    with open(os.path.join(ROOT, "symbol_addrs.txt"), encoding="utf-8") as f:
        current = set(re.findall(r"^\s*(\w+)\s*=", f.read(), re.M))
    for d, _, ns in os.walk(os.path.join(ROOT, "src")):
        for n in ns:
            if n.endswith(".c"):
                with open(os.path.join(d, n), encoding="utf-8") as f:
                    current.update(re.findall(r"\b(\w+)\s*\(", f.read()))
    files.sort(key=lambda p: (not p.startswith("asm/matchings/"),
                              os.path.basename(p)[:-2] not in current, p))
    counted_names, dropped = set(), 0
    for path in files:
        if not os.path.exists(os.path.join(ROOT, path)):
            continue
        with open(os.path.join(ROOT, path), encoding="utf-8") as f:  # splat writes UTF-8
            text = f.read()
        funcs = NONMATCHING.findall(text)
        handwritten += re.findall(r"/\* Handwritten function \*/\s*\nnonmatching\s+(\w+),", text)
        in_c_folder = path.startswith(("asm/nonmatchings/", "asm/matchings/"))
        for name, size in funcs:
            if name.startswith(("D_", "jtbl_")) or re.search(r"^dlabel\s+%s\b" % re.escape(name), text, re.M):
                continue  # data, not a function (named data no longer starts with D_)
            # its first instruction's ROM offset: a renamed function's stale func_*.s has the same
            body = re.search(r"^nonmatching\s+%s,.*?/\*\s*([0-9A-F]{6,})\s+[0-9A-F]{8}\s" % re.escape(name),
                             text, re.M | re.S)
            rom_key = ("rom", body.group(1)) if body else None
            if name in counted_names or (rom_key and rom_key in counted_names):
                dropped += 1
                continue
            counted_names.add(name)
            if rom_key:
                counted_names.add(rom_key)
            parts = path.split("/")
            # the C file's name is everything between asm/<kind>/ and the .s (tlb/tlb_0035)
            seg = fseg.get("/".join(parts[2:-1]), segment_of(name)) if in_c_folder else segment_of(name)
            size = int(size, 0)
            names.add(name)
            total[seg] += 1
            tbytes[seg] += size
            m = re.search(r"/tlb_(\d{4})(?:_\d+)?/", path)
            if m:
                tfile[int(m.group(1))] += size
            if in_c_folder and path not in pulled:
                done[seg] += 1
                dbytes[seg] += size
    if members or zmembers:
        import build_libultra as ultralib
        import build_zlib as libz
        paths = [ultralib.obj_path(ultralib.member_rel(m)) for m in members]
        paths += [os.path.join(ROOT, libz.obj_path(m)) for m in zmembers]
        for path in paths:
            e = ultralib.read_elf(path)
            for c in (total, done):
                c["main"] += len(e["funcs"])
            for c in (tbytes, dbytes):
                c["main"] += len(e["text"])
    print(f"{'segment':<10} {'functions in C':>18} {'bytes in C':>22}")
    for seg in sorted(total, key=lambda s: (s != "main", s)):
        print(f"{seg:<10} {done[seg]:>6} / {total[seg]:<6} ({100 * done[seg] / total[seg]:5.2f}%)"
              f" {dbytes[seg]:>8} / {tbytes[seg]:<8} ({100 * dbytes[seg] / tbytes[seg]:5.2f}%)")
    T, D = sum(total.values()), sum(done.values())
    TB, DB = sum(tbytes.values()), sum(dbytes.values())
    print(f"{'counted':<10} {D:>6} / {T:<6} ({100 * D / T:5.2f}%) {DB:>8} / {TB:<8} ({100 * DB / TB:5.2f}%)")
    if dropped:
        print(f"(each function counted once: {dropped} duplicate .s copies dropped, stale from re-splits)")
    whole = code_inventory(tbytes, tfile)
    if TB > whole * 1.01:  # the count cannot hold more code than the ROM has: a counting bug
        print(f"\nCOUNT IS WRONG: {TB} bytes counted against {whole} read in the ROM; no percentage until "
              f"it is found (the 2026-10-09 duplicates were this)")
        return 1
    print(f"\ncriterion 1 of 4, code matched (NOT the done figure): {DB} / {whole} code bytes in C; "
          f"functions: {D} / {T} counted, plus any NOT COUNTED above"
          + (f"  [UNVERIFIED: includes {len(dirty)} uncommitted file(s)]" if dirty else ""))
    left = unfinished(names)
    if source_asm:
        print("hand-written assembly kept as source in src/ (counted done): "
              + ", ".join(sorted(source_asm)))
    if handwritten:
        # the original's own assembly (the entry point, coprocessor-0 routines): never C
        print(f"of the {T - D} still assembly, {len(handwritten)} are handwritten (they stay .s): "
              + ", ".join(sorted(set(handwritten))))
    done_report(D, T, left)


if __name__ == "__main__":
    import sys
    if "--text-ranges" in sys.argv or "--decode-gaps" in sys.argv:
        print_text_gaps(decode="--decode-gaps" in sys.argv,
                        only=[a for a in sys.argv[1:] if not a.startswith("--")])
    elif "--fakes" in sys.argv:
        list_fakes()
    elif "--headers" in sys.argv:
        pm, pt, pn = local_prototypes(show=True)
        print(f"{pm + pt} local prototypes ({pm} main, {pt} TLB) for {len(pn)} functions")
    else:
        main()
