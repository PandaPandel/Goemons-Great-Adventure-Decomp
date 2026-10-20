"""The game's scenario scripts (event and dialogue files), as text and back to the same bytes.

    python canon-decompilation-tools/scenario_codec.py check            every file through text and back
    python canon-decompilation-tools/scenario_codec.py extract OUTDIR   one .txt per scenario file
    python canon-decompilation-tools/scenario_codec.py build TXT OUT    a file's bytes from its .txt

The files are the ones overlay 6's gScenarioFiles names; each loads whole at 0x08000000
(fileRamAddrAt). Two regions, then zeros to the file's end (the asset survey, results/08_8.txt;
):
  - messages from offset 0: s16 codes, -1 ending each, zeros to the next 4 bytes. 0-95 is a
    glyph of gFontGlyphs (font.c), which starts at the space: code = ASCII - 0x20. Negative codes
    are controls (scRunMessage, _strAdd): -20 opens a window, -22 closes it, -21/-23 wait for A,
    -24 is a new line at the scenario level, -2 at the string level, and so on;
  - commands: ScCmd {u8 op; u8 pad[3]; s32 arg}, 8 bytes each, to the end. A command's first
    half-word, read as a code, is above 95: that is where the messages end.
In the text form a message is a quoted string, a control written {N} (N its code) and the
characters " { } \\ escaped with a backslash; a command is "op ARG", ARG a label when it points
into the same file (msg_XXXX a message, L_XXXX a command), a function name when it is one in
the last link's ELF (op 11 calls overlay 6's script functions), else hex. Labels are
offsets, so the text is the file in order and rebuilds exactly.

Changelog: first public release (2026-10-20). Earlier history is kept privately.
"""

import os
import re
import struct
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import nisitenma_archive as arc  # noqa: E402

ROOT = arc.ROOT
SCENARIO_FILES, SCENARIO_COUNT = 0x8010AED0, 792  # gScenarioFiles (overlay 6)
OVL6_ROM, OVL6_VRAM = 0x582D30, 0x800C7B10
BASE = 0x08000000
# the ops, as the interpreter (scenario.c) runs them; an operand command follows set/test
OPS = {0: "target", 2: "ne", 3: "eq", 4: "set", 6: "test", 8: "end", 9: "assign", 10: "jump",
       11: "call", 12: "ge", 13: "le", 14: "lt", 15: "gt", 16: "message", 17: "menu",
       18: "menuend", 19: "add", 20: "sub", 24: "or", 25: "and", 32: "flagset", 33: "flagclear",
       34: "flagjump", 35: "flagjumpnot"}
OP_NUMBER = {v: k for k, v in OPS.items()}


def files():
    """{file id: bytes} for every scenario file, from the decompressed ROM by nisitenma.tsv."""
    with open(arc.DECOMPRESSED, "rb") as f:
        dec = f.read()
    at = OVL6_ROM + SCENARIO_FILES - OVL6_VRAM
    ids = sorted({int.from_bytes(dec[at + 2 * i:at + 2 * i + 2], "big") for i in range(SCENARIO_COUNT)})
    rows = {}
    with open(os.path.join(ROOT, "nisitenma.tsv"), encoding="utf-8") as f:
        for line in f.read().splitlines()[1:]:
            p = line.split("\t")
            if p[0].isdigit():
                rows[int(p[0])] = (int(p[2], 16), int(p[3], 16))
    return {i: dec[rows[i][0]:rows[i][0] + rows[i][1]] for i in ids if i and rows[i][1]}


def symbols():
    """{address: name} of the last link's functions (for op 11's targets). Without an ELF (a
    failed link removes it) the linker map gives the same addresses: every name in it, which
    is all from_text needs (a name to its address); to_text runs after a good build."""
    elf = os.path.join(ROOT, "build", "goemonsgreatadv.elf")
    lmap = os.path.join(ROOT, "build", "goemonsgreatadv.map")
    if not os.path.exists(elf):
        names = {}
        if not os.path.exists(lmap):
            return names  # no link yet (a fresh clone): op 11's targets stay hex both ways
        with open(lmap, encoding="utf-8", errors="replace") as f:
            for m in re.finditer(r"^\s+0x([0-9a-f]{8,16})\s+([A-Za-z_]\w*)\s*$", f.read(), re.M):
                a = int(m.group(1), 16) & 0xFFFFFFFF
                if a >= 0x80000000:  # main and the overlays (op 11 never names TLB code)
                    names.setdefault(a, m.group(2))
        return names
    import build_rom  # the build's binutils (tools/, or BINUTILS)
    objdump = build_rom.tool_path(os.path.join(build_rom.BINUTILS, "mips64-elf-objdump" + build_rom.EXE))
    out = subprocess.run([objdump, "-t", elf], capture_output=True, text=True, check=True).stdout
    names = {}
    for line in out.splitlines():
        p = line.split()
        if len(p) == 6 and p[2] == "F" and p[3] != "*ABS*" and "tlb" not in p[3]:
            names.setdefault(int(p[0], 16), p[5])
    return names


def split(data):
    """([(offset, [codes])] messages, commands start, [(offset, op, arg)] commands, end)."""
    msgs, q = [], 0
    while q + 2 <= len(data):
        if struct.unpack_from(">h", data, q)[0] > 95:
            break  # an op byte and its zero: the commands
        start, codes = q, []
        while True:
            v = struct.unpack_from(">h", data, q)[0]
            q += 2
            if v == -1:
                break
            codes.append(v)
        msgs.append((start, codes))
        q = (q + 3) & ~3
    cmds, c = [], q
    while c + 8 <= len(data) and data[c:].strip(b"\0"):
        cmds.append((c, data[c], data[c + 1:c + 4], struct.unpack_from(">i", data, c + 4)[0]))
        c += 8
    return msgs, q, cmds, c


def to_text(fid, data, names):
    msgs, cstart, cmds, end = split(data)
    labels = {off: f"msg_{off:04X}" for off, _ in msgs}
    labels.update({off: f"L_{off:04X}" for off, *_ in cmds})
    lines = [f"; scenario file {fid}: {len(msgs)} messages, {len(cmds)} commands, {len(data)} bytes",
             f".size 0x{len(data):X}", ""]
    for off, codes in msgs:
        text = ""
        for v in codes:
            if 0 <= v <= 95:
                ch = chr(v + 0x20)
                text += "\\" + ch if ch in '"{}\\' else ch
            else:
                text += "{%d}" % v
        lines.append(f'msg_{off:04X}: "{text}"')
    lines.append("")
    for off, op, pad, arg in cmds:
        if BASE <= arg < BASE + len(data) and (arg - BASE) in labels:
            a = labels[arg - BASE]
        elif arg & 0xFFFFFFFF in names and op == 11:
            a = names[arg & 0xFFFFFFFF]
        else:
            a = f"0x{arg & 0xFFFFFFFF:X}"
        pad_note = "" if pad == b"\0\0\0" else f" pad 0x{pad.hex()}"
        lines.append(f"L_{off:04X}: {OPS.get(op, op)} {a}{pad_note}")
    return "\n".join(lines) + "\n"


def from_text(text, names):
    by_name = {v: k for k, v in names.items()}
    size = int(re.search(r"^\.size (0x[0-9A-F]+)", text, re.M).group(1), 16)
    out = bytearray()
    msgs = re.findall(r'^msg_[0-9A-F]+: "((?:[^"\\]|\\.)*)"$', text, re.M)
    for m in msgs:
        i = 0
        while i < len(m):
            if m[i] == "\\":
                out += struct.pack(">h", ord(m[i + 1]) - 0x20)
                i += 2
            elif m[i] == "{":
                j = m.index("}", i)
                out += struct.pack(">h", int(m[i + 1:j]))
                i = j + 1
            else:
                out += struct.pack(">h", ord(m[i]) - 0x20)
                i += 1
        out += struct.pack(">h", -1)
        out += bytes(-len(out) % 4)
    cmds = re.findall(r"^L_[0-9A-F]+: (\w+) (\S+)(?: pad 0x([0-9a-f]{6}))?$", text, re.M)
    for op, a, pad in cmds:
        op = OP_NUMBER[op] if op in OP_NUMBER else int(op)
        if re.match(r"^(msg|L)_[0-9A-F]+$", a):
            arg = BASE + int(a.split("_")[1], 16)
        elif a in by_name:
            arg = by_name[a]
        else:
            arg = int(a, 16)
        out += bytes([int(op)]) + (bytes.fromhex(pad) if pad else bytes(3)) + struct.pack(">I", arg & 0xFFFFFFFF)
    return bytes(out) + bytes(size - len(out))


def main():
    cmd = sys.argv[1:]
    names = symbols()
    if cmd == ["check"]:
        ok, bad, n_msg, n_cmd = 0, [], 0, 0
        for fid, data in files().items():
            text = to_text(fid, data, names)
            n_msg += text.count('\nmsg_')
            n_cmd += len(re.findall(r"^L_", text, re.M))
            if from_text(text, names) == data:
                ok += 1
            else:
                bad.append(str(fid))
        print(f"scenario: {ok} files through text and back byte-exact, {len(bad)} not {' '.join(bad)};"
              f" {n_msg} messages, {n_cmd} commands")
        return 0 if not bad else 1
    if len(cmd) == 2 and cmd[0] == "extract":
        os.makedirs(cmd[1], exist_ok=True)
        for fid, data in files().items():
            with open(os.path.join(cmd[1], f"scenario_{fid:04d}.txt"), "w", encoding="utf-8", newline="\n") as f:
                f.write(to_text(fid, data, names))
        return 0
    if len(cmd) == 3 and cmd[0] == "build":
        with open(cmd[1], encoding="utf-8") as f:
            blob = from_text(f.read(), names)
        with open(cmd[2], "wb") as f:
            f.write(blob)
        return 0
    sys.exit(__doc__)


if __name__ == "__main__":
    sys.exit(main())
