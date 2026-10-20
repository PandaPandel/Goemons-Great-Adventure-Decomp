"""The game's table files (archive files 11-15) as C and back to the same bytes.

    python canon-decompilation-tools/table_codec.py check            each file through C, IDO, link
    python canon-decompilation-tools/table_codec.py extract OUTDIR   one .c per table file
    python canon-decompilation-tools/table_codec.py kinds [OUT.h]    the enemy-kind enum (file 12)

Each is an array of a struct main's own C reads (the asset survey, results/03_3.txt); the C is compiled with the game's IDO and linked at 0x08000000, as file 13 is
loaded (its name pointers point into itself):
  11  FileSet[326]       s16 file[4]: a file set (models, textures); floadGetSet DMAs entry n
  12  EneKindInfo[576]   proc (a 0x08xxxxxx task handler in the kind's code file), prio, flags,
                         set, file (the kind's code file), base, from, to (include/enemy.h)
  13  StageEntry[266]    name (a string in this file), init (0x08xxxxxx in the stage's code
                         file), file; then the names (the string literals, after the entries)
  14  StageData[116]     file, objFile, geo, coll (segmented), rails[10] {s16 n; u32 points}
  15  BGEntry[7]         file, unk2, w, h, img1, img2, pal[4] (segmented, in that file)

Changelog: first public release (2026-10-20). Earlier history is kept privately.
"""

import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import model_codec as mc  # noqa: E402

ROOT = mc.ROOT


def _ints(data, fmt, count):
    size = struct.calcsize(fmt)
    return [struct.unpack_from(fmt, data, i * size) for i in range(count)]


def fileset(data):
    rows = _ints(data, ">4h", len(data) // 8)
    body = ",\n".join(f"    {{ {{ {', '.join(f'0x{v & 0xFFFF:X}' if v else '0' for v in r)} }} }}" for r in rows)
    return ("typedef struct FileSet {\n    /* 0x0 */ s16 file[4];\n} FileSet;\n\n"
            f"FileSet gFileSets[{len(rows)}] = {{\n{body}\n}};\n")


def enekind(data):
    rows = _ints(data, ">IBBhiBBBB", len(data) // 16)
    body = ",\n".join(f"    {{ 0x{r[0]:08X}, 0x{r[1]:X}, 0x{r[2]:X}, 0x{r[3] & 0xFFFF:X}, 0x{r[4]:X}, {r[5]}, {r[6]}, {r[7]}, {r[8]} }}"
                      for r in rows)
    return ("typedef struct EneKindRow {\n    /* 0x0 */ u32 proc;\n    /* 0x4 */ u8 prio;\n    /* 0x5 */ u8 flags;\n"
            "    /* 0x6 */ s16 set;\n    /* 0x8 */ s32 file;\n    /* 0xC */ u8 base;\n    /* 0xD */ u8 from;\n"
            "    /* 0xE */ u8 to;\n    /* 0xF */ u8 pad;\n} EneKindRow;\n\n"
            f"EneKindRow gEneKinds[{len(rows)}] = {{\n{body}\n}};\n")


def stagelist(data):
    # the entries run up to the first name (the strings follow them, 16-aligned)
    first = min(struct.unpack_from(">I", data, i * 12)[0] - 0x08000000 for i in range(len(data) // 12)
                if 0x08000000 < struct.unpack_from(">I", data, i * 12)[0] < 0x08000000 + len(data))
    n = first // 12
    rows = _ints(data, ">IIi", n)
    lines = []
    for name, init, file in rows:
        if name:
            o = name - 0x08000000
            s = data[o:data.index(b"\0", o)].decode("ascii")
            lines.append(f'    {{ "{s}", 0x{init:08X}, 0x{file:X} }}')
        else:
            lines.append(f"    {{ NULL, 0x{init:08X}, 0x{file:X} }}" if init or file else "    { 0 }")
    return ("typedef struct StageEntry {\n    /* 0x0 */ char *name;\n    /* 0x4 */ u32 init;\n"
            "    /* 0x8 */ s32 file;\n} StageEntry;\n\n"
            f"StageEntry gStageList[{n}] = {{\n" + ",\n".join(lines) + "\n};\n")


def stagedata(data):
    rows = []
    for i in range(len(data) // 0x60):
        f, of, geo, coll = struct.unpack_from(">iiII", data, i * 0x60)
        rails = [struct.unpack_from(">hHI", data, i * 0x60 + 0x10 + 8 * k) for k in range(10)]
        rs = ", ".join(f"{{ {n}, 0x{p:X} }}" if n or p else "{ 0 }" for n, pad, p in rails)
        rows.append(f"    {{ 0x{f:X}, 0x{of:X}, 0x{geo:X}, 0x{coll:X}, {{ {rs} }} }}")
    return ("typedef struct StageRailRow {\n    /* 0x0 */ s16 n;\n    /* 0x4 */ u32 points;\n} StageRailRow;\n\n"
            "typedef struct StageDataRow {\n    /* 0x00 */ s32 file;\n    /* 0x04 */ s32 objFile;\n"
            "    /* 0x08 */ u32 geo;\n    /* 0x0C */ u32 coll;\n    /* 0x10 */ StageRailRow rails[10];\n} StageDataRow;\n\n"
            f"StageDataRow gStageData[{len(rows)}] = {{\n" + ",\n".join(rows) + "\n};\n")


def bgtable(data):
    rows = _ints(data, ">HHHHII4I", len(data) // 0x20)
    body = ",\n".join(f"    {{ 0x{r[0]:X}, 0x{r[1]:X}, {r[2]}, {r[3]}, 0x{r[4]:X}, 0x{r[5]:X}, "
                      f"{{ 0x{r[6]:X}, 0x{r[7]:X}, 0x{r[8]:X}, 0x{r[9]:X} }} }}" for r in rows)
    return ("typedef struct BGRow {\n    /* 0x00 */ u16 file;\n    /* 0x02 */ u16 unk2;\n    /* 0x04 */ u16 w;\n"
            "    /* 0x06 */ u16 h;\n    /* 0x08 */ u32 img1;\n    /* 0x0C */ u32 img2;\n    /* 0x10 */ u32 pal[4];\n} BGRow;\n\n"
            f"BGRow gBgTable[{len(rows)}] = {{\n{body}\n}};\n")


def geoobj(data):
    """A stage's object table (files 398-411, StageData.objFile; geography.c's GeoObj): the
    records, then zeros to 16 (in the C as the array's own size: the last records zero)."""
    n = len(data) // 0x1C
    while n and not any(data[(n - 1) * 0x1C:n * 0x1C]) and n * 0x1C > len(data) - 16:
        n -= 1  # zero records that are only the padding
    rows = []
    for i in range(n):
        dl, kind, unk5 = struct.unpack_from(">IBB", data, i * 0x1C)
        files = struct.unpack_from(">11H", data, i * 0x1C + 6)
        rows.append(f"    {{ 0x{dl:08X}, {kind}, {unk5}, {{ {', '.join(f'0x{v:X}' if v else '0' for v in files)} }} }}")
    return ("typedef struct GeoObjRow {\n    /* 0x00 */ u32 dl;\n    /* 0x04 */ u8 kind;\n    /* 0x05 */ u8 unk5;\n"
            "    /* 0x06 */ u16 files[11];\n} GeoObjRow;\n\n"
            f"GeoObjRow gGeoObjs[{n}] = {{\n" + ",\n".join(rows) + "\n};\n")


def _symbols():
    """symbol_addrs.txt as {(segment or None, address): [names]}."""
    import re
    out = {}
    with open(os.path.join(ROOT, "symbol_addrs.txt"), encoding="utf-8") as f:
        for line in f:
            m = re.match(r"\s*(\w+)\s*=\s*(0x[0-9A-Fa-f]+)\s*;(.*)", line)
            if m:
                seg = re.search(r"segment:(\w+)", m.group(3))
                out.setdefault((seg.group(1) if seg else None, int(m.group(2), 16)), []).append(m.group(1))
    return out


def _upper_snake(name):
    """tanuKobanProc -> TANU_KOBAN, shop_tengu_proc -> SHOP_TENGU: camelCase words split, a
    closing "Proc" or "_proc" dropped."""
    import re
    for end in ("Proc", "_proc"):
        if name.endswith(end) and len(name) > len(end):
            name = name[:-len(end)]
            break
    s = re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", name)
    s = re.sub(r"([A-Z]+)([A-Z][a-z])", r"\1_\2", s)
    return re.sub(r"_+", "_", re.sub(r"\W", "_", s)).strip("_").upper()


def kinds_enum(data):
    """The enemy-kind enum (the kinds mode; the Changelog's 2026-10-17 entry): (header text,
    {"named": n, "unnamed": n, "suffixed": n})."""
    import re
    syms = _symbols()
    rows = _ints(data, ">IBBhiBBBB", len(data) // 16)
    entries = []  # (kind, base name or None, note, row)
    for k, r in enumerate(rows):
        proc, file, setn = r[0], r[4], r[3]
        if proc == 0:
            entries.append((k, None, "no proc (an empty row)" if not any(r) else "no proc", r))
            continue
        seg = f"tlb_{file:04d}" if file else None
        names = syms.get((seg, proc), []) if seg else \
            [n for (s, a), ns in syms.items() if a == proc and (s is None or not s.startswith("tlb_")) for n in ns]
        names = [n for n in names if not re.match(r"(func|D)_[0-9A-Fa-f]{8}", n)]
        if len(set(names)) != 1:
            why = (f"proc 0x{proc:08X} in file {file} has no name yet" if not names
                   else f"proc 0x{proc:08X} has {len(set(names))} names: {', '.join(sorted(set(names)))}")
            entries.append((k, None, why, r))
            continue
        entries.append((k, names[0], None, r))
    # kinds sharing a name: a suffix from each one's own row, the fields that tell them apart in
    # the row's order of meaning (set, file, hours, flags, base, prio), as few as tell them all
    # apart; else the kind itself
    fields = [  # (suffix, the evidence, the value)
        (lambda r: f"SET_{r[3] & 0xFFFF:X}", lambda r: f"set 0x{r[3] & 0xFFFF:X}", lambda r: r[3]),
        (lambda r: f"FILE_{r[4]}", lambda r: f"file {r[4]}", lambda r: r[4]),
        (lambda r: "HOURS_ANY" if r[6] == r[7] else f"HOURS_{r[6]}_{r[7]}",
         lambda r: ("from == to: at any hour" if r[6] == r[7] else f"hours {r[6]} to {r[7]}")
         + " (eneSpawnInReach)", lambda r: None if r[6] == r[7] else (r[6], r[7])),
        (lambda r: f"FLAGS_{r[2]:X}", lambda r: f"flags 0x{r[2]:X}", lambda r: r[2]),
        (lambda r: f"BASE_{r[5]}", lambda r: f"base {r[5]}", lambda r: r[5]),
        (lambda r: f"PRIO_{r[1]:X}", lambda r: f"prio 0x{r[1]:X}", lambda r: r[1]),
    ]
    groups = {}
    for e in entries:
        if e[1]:
            groups.setdefault("ENE_" + _upper_snake(e[1]), []).append(e)
    names, notes, suffixed = {}, {}, 0
    for base, es in groups.items():
        if len(es) == 1:
            names[es[0][0]] = base
            continue
        use = []
        procs = sorted({e[1] for e in es})
        # two procs giving one name are told apart by their files first (each proc's own file)
        for f in (fields if len(procs) == 1 else [fields[1], fields[0]] + fields[2:]):
            if len({f[2](e[3]) for e in es}) > 1:
                use.append(f)
                if len({tuple(u[2](e[3]) for u in use) for e in es}) == len(es):
                    break
        whole = len({tuple(u[2](e[3]) for u in use) for e in es}) == len(es)
        for e in es:
            if whole:
                names[e[0]] = base + "".join("_" + u[0](e[3]) for u in use)
                why = ", ".join(u[1](e[3]) for u in use)
            else:
                names[e[0]] = f"{base}_{e[0]:03X}"
                why = f"kind 0x{e[0]:03X} (no field of its row tells it from the others)"
            others = ", ".join(f"0x{o[0]:X}" for o in es if o is not e)
            notes[e[0]] = (f"{e[1]} (file {e[3][4]})"
                           + (f"; {', '.join(procs)} give one name" if len(procs) > 1 else "")
                           + f"; as kind{'s' if len(es) > 2 else ''} {others}; its own: {why}")
            suffixed += 1
    lines = []
    for k, name, why, r in entries:
        if name is None:
            names[k] = f"ENE_KIND_{k:03X}"
            notes[k] = why
        lines.append(f"    {names[k]} = 0x{k:03X}," + (f" /* {notes[k]} */" if k in notes else ""))
    named = sum(1 for e in entries if e[1])
    text = ("#ifndef ENEKIND_H\n#define ENEKIND_H\n\n"
            "/* The enemy kinds: a kind is its index in file 0xC's table (EneKindInfo[576], read by\n"
            " * eneMakeBench), the number eneMake, eneMakeUnder and eneMakeEx take. Each is named after its\n"
            " * proc, the task handler in its row (symbol_addrs.txt; a closing \"Proc\" or \"_proc\"\n"
            " * dropped); a suffix (_SET_, _HOURS_, _FLAGS_...) tells kinds of one name apart by their rows;\n"
            " * ENE_KIND_<kind>: no proc, or a proc with no name yet. Made by\n"
            " * canon-decompilation-tools/table_codec.py kinds. */\n"
            "enum EneKindId {\n" + "\n".join(lines) + "\n};\n\n#endif\n")
    return text, {"named": named, "unnamed": len(entries) - named, "suffixed": suffixed}


TABLES = {11: fileset, 12: enekind, 13: stagelist, 14: stagedata, 15: bgtable,
          **{fid: geoobj for fid in range(398, 412)}}


def to_c(fid, data):
    return f"#include \"ultra64.h\"\n\n/* Archive file {fid}. */\n" + TABLES[fid](data)


def main():
    cmd = sys.argv[1:]
    files = mc.archive()
    if cmd == ["check"]:
        scratch = os.path.join(ROOT, "permuter", "table_codec")
        ok, bad = 0, []
        for fid in TABLES:
            data = files[fid]
            got = mc.build(to_c(fid, data), scratch, base=0x08000000)
            got = got + bytes(len(data) - len(got)) if len(got) < len(data) else got
            if got == data:
                ok += 1
            else:
                k = next((i for i in range(min(len(got), len(data))) if got[i] != data[i]), None)
                bad.append(f"{fid}@0x{k:X}" if k is not None else f"{fid}(len {len(got):X}/{len(data):X})")
        print(f"tables: {ok} of {len(TABLES)} through C and back byte-exact; not: {' '.join(bad)}")
        return 0 if not bad else 1
    if len(cmd) == 2 and cmd[0] == "extract":
        os.makedirs(cmd[1], exist_ok=True)
        for fid in TABLES:
            with open(os.path.join(cmd[1], f"table_{fid:04d}.c"), "w", encoding="utf-8", newline="\n") as f:
                f.write(to_c(fid, files[fid]))
        return 0
    if cmd[:1] == ["kinds"] and len(cmd) <= 2:
        text, n = kinds_enum(files[12])
        print(f"kinds: {n['named'] + n['unnamed']}: {n['named']} named from their procs "
              f"({n['suffixed']} of them with a suffix: a name shared), {n['unnamed']} ENE_KIND_<hex>",
              file=sys.stderr)
        if len(cmd) == 2:
            with open(cmd[1], "w", encoding="utf-8", newline="\n") as f:
                f.write(text)
        else:
            sys.stdout.write(text)
        return 0
    sys.exit(__doc__)


if __name__ == "__main__":
    sys.exit(main())
