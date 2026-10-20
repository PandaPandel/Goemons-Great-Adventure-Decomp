"""The game's audio region (ROM 0x3F910-0x582530), extracted and rebuilt to the same bytes.

    python canon-decompilation-tools/audio_codec.py check            rebuild everything in memory, compare
                                                                     (and each song on its own)
    python canon-decompilation-tools/audio_codec.py extract OUTDIR   banks as JSON, samples as AIFC, songs
    python canon-decompilation-tools/audio_codec.py build INDIR OUT  the region's bytes from an extraction

Three parts, back to back (the asset survey, permuter/assets/results/12_12.txt):
  - 4 sound banks, libultra ALBankFile "B1" (.ctl): one shared envelope and keymap, then per
    instrument its ALSound, its sample's ALWaveTable (with loop and codebook) on first use, and
    its ALInstrument; the ALBank last; every object 8-aligned, the file 16-aligned. Bank b's
    sample offsets are into wave table b.
  - 4 wave tables (.tbl): VADPCM frames (9 bytes: scale/predictor, 16 nibbles) per sample, in
    first-use order, each 8-aligned; a sample's length is its frames rounded up to even (one
    zero byte), the table zero-padded to 16. A sample extracts as an AIFC (VAPC: the frames as
    they are, a VADPCMCODES book, a VADPCMLOOPS loop), the container sm64 uses; it is rebuilt by
    copying the frames, so the bytes never depend on an encoder.
  - 179 songs (ids 0x10-0xC2), Konami-LZ blocks padded to 16: each as a script of the Konami
    driver's bytecode, as its own player reads it (SONG_OPS: the handler table and each
    handler's reads, cited). Per channel one line per command, by its handler's name with its
    operands (decimal; signed where the handler sign-extends; keys and masks hex), and per note
    `note`/`drum` KEY [len=] [gate=] vel=; a label after each loop start, mark and repeat start,
    and on each jump where the channel's run took it. A song file starts with one u16 offset
    per channel, then the length and 16-bit sum sndCheckBlock checks, both computed. Assembled
    and packed back with the game's own encoder (nisitenma_archive.py konami_lz_pack); check
    proves every song on its own. Every song parses whole.
Every range comes from the game's own tables in audio.c (gSndBankRomStart/End,
gSndWaveTableRom, gSndMusicRomStart/End).

Changelog: first public release (2026-10-20). Earlier history is kept privately.
"""

import json
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import nisitenma_archive as arc  # noqa: E402

ROOT = arc.ROOT
CTL = [(0x3F910, 0x48D30), (0x48D30, 0x51320), (0x51320, 0x5A650), (0x5A650, 0x60890)]
TBL = [0x60890, 0x285020, 0x35AC00, 0x419BF0, 0x531370]
MUSIC = (0x531370, 0x582530)
INST_FIELDS = ("volume", "pan", "priority", "flags", "tremType", "tremRate", "tremDepth",
               "tremDelay", "vibType", "vibRate", "vibDepth", "vibDelay")


def rom():
    with open(arc.BASEROM, "rb") as f:
        return f.read()


def parse_bank(data, n):
    """A .ctl as a dict: the constants, then each instrument (its fields, its sound's fields,
    its sample's name), and the samples by name (table offset, length, type, book, loop)."""
    u32 = lambda o: struct.unpack_from(">I", data, o)[0]  # noqa: E731
    s16 = lambda o: struct.unpack_from(">h", data, o)[0]  # noqa: E731
    bo = u32(4)
    bank = {"revision": data[0:2].decode(), "bankCount": s16(2), "flags": data[bo + 2],
            "pad": data[bo + 3], "sampleRate": struct.unpack_from(">i", data, bo + 4)[0],
            "percussion": u32(bo + 8), "instruments": []}
    samples, names = {}, {}
    for i in range(s16(bo)):
        io = u32(bo + 12 + 4 * i)
        so = u32(io + 16)
        eo, ko, wo = u32(so), u32(so + 4), u32(so + 8)
        if wo not in names:
            names[wo] = f"bank{n}_sample{len(names):03d}"
            lp, bk = u32(wo + 12), u32(wo + 16)
            order, npred = struct.unpack_from(">ii", data, bk)
            samples[names[wo]] = {
                "base": u32(wo), "len": struct.unpack_from(">i", data, wo + 4)[0],
                "type": data[wo + 8], "flags": data[wo + 9],
                "book": data[bk:bk + 8 + 16 * order * npred],
                "loop": data[lp:lp + 44] if lp else None}
        inst = data[io:io + 16]
        bank["instruments"].append({
            **{k: inst[j] for j, k in enumerate(INST_FIELDS)},
            "bendRange": struct.unpack_from(">h", inst, 12)[0],
            "samplePan": data[so + 12], "sampleVolume": data[so + 13], "soundFlags": data[so + 14],
            "envelope": data[eo:eo + 14].hex(), "keymap": data[ko:ko + 6].hex(),
            "sample": names[wo]})
    return bank, samples


def build_bank(bank, samples, frames):
    """The .ctl and .tbl bytes for a bank, in the game's order. frames: name -> sample bytes."""
    out, tbl = bytearray(), bytearray()

    def align(a, buf=out):
        buf += bytes(-len(buf) % a)
    out += bank["revision"].encode() + struct.pack(">hI", bank["bankCount"], 0)
    align(8)
    first = bank["instruments"][0]
    env_off = len(out)
    out += bytes.fromhex(first["envelope"])
    align(8)
    key_off = len(out)
    out += bytes.fromhex(first["keymap"])
    align(8)
    wave_at, inst_offs = {}, []
    for it in bank["instruments"]:
        so = len(out)
        out += struct.pack(">III", env_off, key_off, 0) + bytes([it["samplePan"], it["sampleVolume"], it["soundFlags"], 0])
        align(8)
        name = it["sample"]
        if name not in wave_at:
            s = samples[name]
            align(8, tbl)
            base = len(tbl)
            tbl += frames[name]
            wo = len(out)
            out += struct.pack(">IiBBH", base, s["len"], s["type"], s["flags"], 0) + bytes(8)
            align(8)
            lpo = 0
            if s["loop"]:
                lpo = len(out)
                out += s["loop"]
                align(8)
            bko = len(out)
            out += s["book"] + bytes(8)  # each book is followed by 8 zero bytes
            align(8)
            struct.pack_into(">II", out, wo + 12, lpo, bko)
            wave_at[name] = wo
        struct.pack_into(">I", out, so + 8, wave_at[name])
        io = len(out)
        out += bytes(it[k] for k in INST_FIELDS) + struct.pack(">hhI", it["bendRange"], 1, so)
        align(8)
        inst_offs.append(io)
    bo = len(out)
    out += struct.pack(">hBBiI", len(inst_offs), bank["flags"], bank["pad"], bank["sampleRate"], bank["percussion"])
    for io in inst_offs:
        out += struct.pack(">I", io)
    align(16)
    struct.pack_into(">I", out, 4, bo)
    align(16, tbl)
    return bytes(out), bytes(tbl)


def _ext80(rate):
    """An 80-bit IEEE extended float, as AIFF's COMM wants the sample rate."""
    import math
    m, e = math.frexp(rate)
    return struct.pack(">HQ", e + 16382, int(m * (1 << 64)))


def to_aifc(sample, data, rate):
    """A sample as AIFC (VAPC): COMM, the book (APPL stoc VADPCMCODES), the loop (VADPCMLOOPS),
    and the frames (SSND) without the even-length pad byte."""
    nfr = sample["len"] // 9
    order, npred = struct.unpack_from(">ii", sample["book"], 0)

    def chunk(t, body):
        return t + struct.pack(">I", len(body)) + body + bytes(len(body) & 1)
    comm = struct.pack(">hIh", 1, nfr * 16, 16) + _ext80(rate) + b"VAPC" + b"\x0bVADPCM ~4-1"
    codes = b"stoc" + b"\x0bVADPCMCODES" + struct.pack(">hhh", 1, order, npred) + sample["book"][8:]
    body = chunk(b"COMM", comm) + chunk(b"APPL", codes)
    if sample["loop"]:
        body += chunk(b"APPL", b"stoc" + b"\x0bVADPCMLOOPS" + struct.pack(">hh", 1, 1) + sample["loop"])
    body += chunk(b"SSND", struct.pack(">II", 0, 0) + data[:nfr * 9])
    return b"FORM" + struct.pack(">I", 4 + len(body)) + b"AIFC" + body


def from_aifc(blob):
    """(frames with the pad byte back, book, loop) from to_aifc's AIFC."""
    pos, frames, book, loop = 12, b"", None, None
    while pos < len(blob):
        t, n = blob[pos:pos + 4], struct.unpack_from(">I", blob, pos + 4)[0]
        body = blob[pos + 8:pos + 8 + n]
        if t == b"SSND":
            frames = body[8:]
        elif t == b"APPL" and body[4:16] == b"\x0bVADPCMCODES":
            order, npred = struct.unpack_from(">hh", body, 18)
            book = struct.pack(">ii", order, npred) + body[22:]
        elif t == b"APPL" and body[4:16] == b"\x0bVADPCMLOOPS":
            loop = body[20:]
        pos += 8 + n + (n & 1)
    return frames + bytes(len(frames) & 1), book, loop


# The Konami driver's song bytecode, as the game's own player reads it (the matched C; IDA's
# main.bin.i64 agrees: the table at 0x80038290 and every handler's reads). trkNextNote
# (src/sndtrack.c:415-518) takes a channel's bytes: 0xD0-0xFF go through music's handler table
# gSndMusicCmdTable[cmd - 0xD0] (sndtrack.c:429-444, the table sndtrack.c:859-872; tracks 0-15 are
# music's), F2/F3 end the step and FF the track (sndtrack.c:435-442). Anything below 0xD0 is a note.
# A row: opcode -> (name, operands, source). The name is the handler's (sndCmd... without the
# prefix, in snake case); the operands follow the command byte in the order the handler reads them:
#   u8   a byte read as is (lbu)
#   s8   a byte the handler sign-extends (lb, an (s8) cast, or <<8 into an s16 field)
#   key  a note key (a u8; 0x3C plays the sample at its own rate, gSndPitchTable)
#   u16  two bytes, the high one first (`*p++ << 8` then `+= *p++`)
# The sound effects' table (0x80038350, sndtrack.c:873-886) skips the music-only commands' bytes
# with sndCmdSkip1/2/3: the same counts, except D7 (it skips 3) and DB (it skips 2); songs play
# on music's tracks, so music's table is the one here.
SONG_OPS = {
    0xD0: ("tempo", (("tempo", "u8"),), "sndcmd.c:52 sndCmdTempo: the tempo's high byte (8.8)"),
    0xD1: ("tempo_slide", (("ticks", "u8"), ("target", "u8")), "sndcmd.c:59 sndCmdTempoSlide"),
    0xD2: ("instrument", (("inst", "u8"),), "sndcmd.c:91 sndCmdInstrument"),
    0xD3: ("vol_instrument", (("volume", "u8"), ("inst", "u8")),
           "sndcmd.c:174 sndCmdVolInstrument: sndCmdVolume, then sndCmdInstrument"),
    0xD4: ("vol_instrument_pan", (("volume", "u8"), ("inst", "u8"), ("pan", "u8")),
           "sndcmd.c:180 sndCmdVolInstrumentPan: sndCmdVolume, sndCmdInstrument, sndCmdPan"),
    0xD5: ("volume", (("volume", "u8"),), "sndcmd.c:187 sndCmdVolume"),
    0xD6: ("volume_slide", (("ticks", "u8"), ("target", "u8")), "sndcmd.c:201 sndCmdVolumeSlide"),
    0xD7: ("env_attack_decay", (("attack", "u8"), ("decay", "u8")),
           "sndcmd.c:231 sndCmdEnvAttackDecay: a rate; the decay's time (high nibble) and level (low)"),
    0xD8: ("env_sustain", (("sustain", "u8"),), "sndcmd.c:244 sndCmdEnvSustain (0: held)"),
    0xD9: ("env_release", (("release", "u8"),), "sndcmd.c:267 sndCmdEnvRelease"),
    0xDA: ("fx_params", (("ffcoef", "u8"), ("gain", "u8")),
           "sndcmd.c:278 sndCmdFxParams: reverb parameters 3 (x40) and 4 (<<7), 0 their defaults"),
    0xDB: ("end_db", (), "sndtrack.c:862 sndCmdEnd (sndcmd.c:955) in DB's slot"),
    0xDC: ("fade_hold", (("level", "u8"),), "sndcmd.c:304 sndCmdFadeHold (0xFF lets go)"),
    0xDD: ("pan", (("pan", "u8"),), "sndcmd.c:317 sndCmdPan"),
    0xDE: ("pan_slide", (("ticks", "u8"), ("target", "u8")), "sndcmd.c:328 sndCmdPanSlide"),
    0xDF: ("transpose", (("semitones", "s8"),),
           "sndcmd.c:363 sndCmdTranspose: the byte <<8 into the s16 transpose"),
    0xE0: ("detune", (("detune", "s8"),), "sndcmd.c:369 sndCmdDetune: (s8), in 1/64 semitones"),
    0xE1: ("vibrato", (("delay", "u8"), ("speed", "u8"), ("depth", "u8")), "sndcmd.c:376 sndCmdVibrato"),
    0xE2: ("vibrato_rise", (("ticks", "u8"),), "sndcmd.c:469 sndCmdVibratoRise"),
    0xE3: ("wobble", (("speed", "u8"), ("mask", "u16")), "sndcmd.c:481 sndCmdWobble"),
    0xE4: ("portamento", (("delay", "u8"), ("ticks", "u8"), ("key", "key")),
           "sndcmd.c:532 sndCmdPortamentoCheck right after a note or F2/F3 (sndtrack.c:377-378); "
           "elsewhere sndcmd.c:507 sndCmdPortamentoSkip skips the same 3 bytes"),
    0xE5: ("pitch_bend", (("delay", "u8"), ("ticks", "u8"), ("semitones", "s8")),
           "sndcmd.c:585 sndCmdPitchBend"),
    0xE6: ("glide", (("rate", "u8"),), "sndcmd.c:608 sndCmdGlide"),
    0xE7: ("loop_start", (), "sndcmd.c:645 sndCmdLoopStart: the loop returns to the byte after it"),
    0xE8: ("loop_end", (("count", "u8"), ("velocity", "s8"), ("detune", "s8")),
           "sndcmd.c:661 sndCmdLoopEnd: count (0: forever); each pass adds a velocity and a detune (1/32s)"),
    0xE9: ("loop2_start", (), "sndcmd.c:678 sndCmdLoop2Start"),
    0xEA: ("loop2_end", (("count", "u8"), ("velocity", "s8"), ("detune", "s8")), "sndcmd.c:686 sndCmdLoop2End"),
    0xEB: ("mark", (), "sndcmd.c:703 sndCmdMark: the byte after it"),
    0xEC: ("goto_mark", (), "sndcmd.c:708 sndCmdGotoMark"),
    0xED: ("repeat_start", (), "sndcmd.c:713 sndCmdRepeatStart: the byte after it"),
    0xEE: ("repeat", (), "sndcmd.c:720 sndCmdRepeat: 1st arms, 2nd jumps back keeping its place, 3rd returns"),
    0xEF: ("fx_mix", (("mix", "u8"),), "sndcmd.c:740 sndCmdFxMix"),
    0xF0: ("bank", (("bank", "u8"),), "sndcmd.c:763 sndCmdBank"),
    0xF1: ("state_mute", (("mode", "u8"),), "sndcmd.c:773 sndCmdStateMute"),
    0xF2: ("rest", (("ticks", "u8"),), "sndcmd.c:803 sndCmdRest; ends the step"),
    0xF3: ("tie", (("ticks", "u8"), ("gate", "u8")), "sndcmd.c:814 sndCmdTie; ends the step"),
    0xF4: ("end_f4", (), "sndtrack.c:869 sndCmdEnd in F4's slot"),
    0xF5: ("end_f5", (), "sndtrack.c:869 sndCmdEnd in F5's slot"),
    0xF6: ("end_f6", (), "sndtrack.c:869 sndCmdEnd in F6's slot"),
    0xF7: ("end_f7", (), "sndtrack.c:869 sndCmdEnd in F7's slot"),
    0xF8: ("scale_base", (("kind", "u8"), ("scale", "u8"), ("base", "s8")),
           "sndcmd.c:837 sndCmdScaleBase (base is the s8 base[kind], snd.h:109)"),
    0xF9: ("random_variation", (("group", "u8"), ("chance", "u8")), "sndcmd.c:854 sndCmdRandomVariation"),
    0xFA: ("event", (("id", "u8"),), "sndcmd.c:899 sndCmdEvent"),
    0xFB: ("event_arg", (("id", "u8"), ("arg", "u8")), "sndcmd.c:904 sndCmdEventArg"),
    0xFC: ("event_kind2", (("id", "u8"), ("arg", "u8"), ("value", "s8")),
           "sndcmd.c:914 sndCmdEventKind2 -> sndcmd.c:943 sndCmdEventValue ((s8) value << 8)"),
    0xFD: ("event_kind3", (("id", "u8"), ("arg", "u8"), ("value", "s8")), "sndcmd.c:918 sndCmdEventKind3"),
    0xFE: ("event_kind4", (("id", "u8"), ("arg", "u8"), ("value", "s8")), "sndcmd.c:922 sndCmdEventKind4"),
    0xFF: ("end", (), "sndcmd.c:955 sndCmdEnd; trkNextNote returns (sndtrack.c:440)"),
}
SONG_OP_BYTE = {name: op for op, (name, _, _) in SONG_OPS.items()}
SONG_ENDS = (0xDB, 0xF4, 0xF5, 0xF6, 0xF7, 0xFF)  # sndCmdEnd: the track stops (id 0)
# the control flow (sndcmd.c:645-735): a start command and what returns to the byte after it
SONG_STARTS = {0xE7: "loop", 0xE9: "loop2", 0xEB: "mark", 0xED: "repeat"}


def _song_items(data, p, end):
    """A channel's bytes as trkNextNote reads them, as [(offset, size, command or None, values)],
    and where the reading stopped. A note's values are (key, length, gate, velocity byte, escaped):
    its key byte is 0x00-0x67 with a length byte after it, or 0x68-0xCF (key + 0x68) reusing the
    last length; a key of 0x48 and up is a drum, and 0x67 takes the drum's key from the next byte
    (before the length); then a gate if the byte is below 0x80, then the velocity (its low 7 bits)
    (sndtrack.c:445-487)."""
    items = []
    while p < end:
        b = data[p]
        if b >= 0xD0:
            vals, q = [], p + 1
            for _, kind in SONG_OPS[b][1]:
                w = 2 if kind == "u16" else 1
                if q + w > end:
                    return items, p
                v = int.from_bytes(data[q:q + w], "big")
                vals.append(v - 0x100 if kind == "s8" and v >= 0x80 else v)
                q += w
            items.append((p, q - p, b, vals))
            p = q
            continue
        q, key = p + 1, (b if b < 0x68 else b - 0x68)
        esc = key == 0x67
        need = (1 if esc else 0) + (1 if b < 0x68 else 0) + 1
        if q + need > end:
            break
        if esc:
            key, q = data[q], q + 1
        length = None
        if b < 0x68:
            length, q = data[q], q + 1
        gate = None
        if data[q] < 0x80:
            gate, q = data[q], q + 1
            if q >= end:
                break
        items.append((p, q + 1 - p, None, (key, length, gate, data[q], esc)))
        p = q + 1
    return items, p


def _song_flow(items):
    """A channel's control flow run as the handlers run it (sndcmd.c:645-735; the pointers are not
    reset when a song starts, audio.c:1466-1542, so a jump before its start command has no target):
    {command offset: set of target offsets} for the jumps taken, {repeat offset: states seen}, the
    offsets reached, and whether it ran out of steps. Notes and the other commands only move on; it
    stops at an end or when its whole state repeats."""
    at = {it[0]: i for i, it in enumerate(items)}
    jumps, rstates, seen, reached = {}, {}, set(), set()
    lp = l2p = rs = 0
    ls = l2s = mk = rst = rret = None
    i, steps = 0, 0
    while i < len(items) and steps < 2000000:
        steps += 1
        off, size, op, vals = items[i]
        reached.add(off)
        nxt, target, jumped = off + size, None, False
        if op in SONG_ENDS:
            break
        if op == 0xE7:
            lp, ls = 0, nxt
        elif op == 0xE9:
            l2p, l2s = 0, nxt
        elif op in (0xE8, 0xEA):
            if op == 0xE8:
                lp = (lp + 1) & 0xFF
                jumped, target = vals[0] == 0 or vals[0] != lp, ls
            else:
                l2p = (l2p + 1) & 0xFF
                jumped, target = vals[0] == 0 or vals[0] != l2p, l2s
        elif op == 0xEB:
            mk = nxt
        elif op == 0xEC:
            jumped, target = True, mk
        elif op == 0xED:
            rs, rst = 0, nxt
        elif op == 0xEE:
            rstates.setdefault(off, set()).add(rs)
            if rs == 0:
                rs = 1
            elif rs == 1:
                rs, rret, jumped, target = 2, nxt, True, rst
            elif rs == 2:
                rs, jumped, target = 1, True, rret
        if not jumped:
            i += 1
            continue
        jumps.setdefault(off, set()).add(target)
        state = (target, lp, ls, l2p, l2s, mk, rs, rst, rret)
        if target not in at or state in seen:
            break
        seen.add(state)
        i = at[target]
    return jumps, rstates, reached, steps >= 2000000


def _song_value(v, kind):
    return f"0x{v:02X}" if kind == "key" else f"0x{v:04X}" if kind == "u16" else str(v)


def song_to_text(sid, data):
    """A decompressed song as a script: per channel, one command or note per line with its operands
    (SONG_OPS), a label after each loop start, mark and repeat start, and on each jump where it went
    when the channel's control flow was run (_song_flow)."""
    n = (struct.unpack_from(">H", data, 0)[0] - 4) // 2
    offs = list(struct.unpack_from(">%dH" % n, data, 0))
    blen, bsum = struct.unpack_from(">HH", data, 2 * n)
    lines = [f"; song 0x{sid:02X}: {n} channels, Konami's sound driver (audio_codec.py SONG_OPS)"]
    if blen != len(data) - 2 * n - 4:  # sndCheckBlock's length and sum, when not the computed ones
        lines.append(f".len {blen}")
    if bsum != sum(data[2 * n + 4:2 * n + 4 + blen]) & 0xFFFF:
        lines.append(f".sum 0x{bsum:04X}")
    bounds = offs + [len(data)]
    for c in range(n):
        lines.append(f"channel {c}")
        items, stop = _song_items(data, bounds[c], bounds[c + 1])
        jumps, rstates, reached, _ = _song_flow(items)
        labels, count = {}, {}
        for off, size, op, _ in items:
            if op in SONG_STARTS:
                count[op] = count.get(op, 0) + 1
                labels[off + size] = f"ch{c}_{SONG_STARTS[op]}_{count[op]}"
        for off, size, op, vals in items:
            if off in labels:
                lines.append(labels[off] + ":")
            if op is None:
                key, length, gate, vel, esc = vals
                drum = esc or key >= 0x48
                parts = ["drum" if drum else "note", f"0x{key:02X}"]
                if esc and 0x48 <= key <= 0x66:
                    parts.append("esc")  # a key the direct form could hold, escaped anyway
                if length is not None:
                    parts.append(f"len={length}")
                if gate is not None:
                    parts.append(f"gate={gate}")
                # the velocity byte's top bit is set exactly when there is no gate (or it would read
                # as one); with a gate the game leaves it clear
                parts.append(f"vel={vel & 0x7F}" if (vel >= 0x80) == (gate is None) else f"velbyte=0x{vel:02X}")
                lines.append("    " + " ".join(parts))
                continue
            name, ops, _ = SONG_OPS[op]
            line = "    " + " ".join([name] + [_song_value(v, k) for v, (_, k) in zip(vals, ops)])
            if op in (0xE8, 0xEA, 0xEC, 0xEE) and off not in reached:
                line += "  ; (not reached)"
            elif op in (0xE8, 0xEA, 0xEC) or (op == 0xEE and 1 in rstates[off]):
                to = sorted(jumps.get(off, ()), key=lambda x: (x is None, x or 0))
                line += ("  ; -> " + ", ".join(labels.get(t, "(no target)") for t in to)) if to else "  ; (never taken)"
                if op == 0xEE:
                    line += ", back here after it"
            elif op == 0xEE:
                line += "  ; end of the repeated part"
            lines.append(line)
        if stop < bounds[c + 1]:  # whatever does not parse as the player reads, kept as bytes
            lines.append("    .byte " + " ".join(f"0x{x:02X}" for x in data[stop:bounds[c + 1]]))
    return "\n".join(lines) + "\n"


def _song_int(s):
    return int(s, 0)


def song_from_text(text):
    """song_to_text's script back to the song's bytes: the channel offsets, sndCheckBlock's length
    and 16-bit sum over the channels' bytes (audio.c:1295-1319), then the channels."""
    chans, blen, bsum = [], None, None
    for line in text.splitlines():
        t = line.split(";")[0].split()
        if not t or t[0].endswith(":"):  # a label: the bytecode's jumps carry no address
            continue
        if t[0] == ".len":
            blen = _song_int(t[1])
        elif t[0] == ".sum":
            bsum = _song_int(t[1])
        elif t[0] == "channel":
            chans.append(bytearray())
        elif t[0] == ".byte":
            chans[-1] += bytes(_song_int(x) for x in t[1:])
        elif t[0] in ("note", "drum"):
            key, kv = _song_int(t[1]), dict(x.split("=") if "=" in x else (x, "") for x in t[2:])
            if t[0] == "note" and (key >= 0x48 or "esc" in kv):
                raise ValueError(f"{line.strip()}: keys 0x48 and up are drums (sndtrack.c:451)")
            esc = t[0] == "drum" and ("esc" in kv or not 0x48 <= key <= 0x66)
            reuse = 0 if "len" in kv else 0x68
            chans[-1] += bytes([0x67 + reuse, key] if esc else [key + reuse])
            if "len" in kv:
                chans[-1].append(_song_int(kv["len"]))
            if "gate" in kv:
                chans[-1].append(_song_int(kv["gate"]))
            chans[-1].append(_song_int(kv["velbyte"]) if "velbyte" in kv
                             else _song_int(kv["vel"]) | (0 if "gate" in kv else 0x80))
        else:
            op = SONG_OP_BYTE[t[0]]
            kinds = [k for _, k in SONG_OPS[op][1]]
            if len(t) - 1 != len(kinds):
                raise ValueError(f"{line.strip()}: {t[0]} takes {len(kinds)} operands")
            chans[-1].append(op)
            for v, k in zip(t[1:], kinds):
                chans[-1] += (_song_int(v) & (0xFFFF if k == "u16" else 0xFF)).to_bytes(2 if k == "u16" else 1, "big")
    n = len(chans)
    offs, at = [], 2 * n + 4
    for ch in chans:
        offs.append(at)
        at += len(ch)
    body = b"".join(bytes(c) for c in chans)
    blen = len(body) if blen is None else blen
    bsum = sum(body[:blen]) & 0xFFFF if bsum is None else bsum
    return struct.pack(">%dH" % n, *offs) + struct.pack(">HH", blen, bsum) + body


def extract_all():
    """{path: bytes} of the whole extraction, in memory."""
    data = rom()
    files = {}
    for n, (lo, hi) in enumerate(CTL):
        bank, samples = parse_bank(data[lo:hi], n)
        files[f"bank{n}.json"] = json.dumps(bank, indent=1).encode()
        for name, s in samples.items():
            raw = data[TBL[n] + s["base"]:TBL[n] + s["base"] + s["len"]]
            files[f"samples/{name}.aifc"] = to_aifc(s, raw, bank["sampleRate"])
    for i in range(arc.MUSIC_COUNT):
        s = arc.be32(data, arc.MAIN_ROM + arc.MUSIC_ROM_START - arc.MAIN_VRAM + 4 * i)
        e = arc.be32(data, arc.MAIN_ROM + arc.MUSIC_ROM_END - arc.MAIN_VRAM + 4 * i)
        files[f"music/song_{i + 0x10:02X}.txt"] = song_to_text(i + 0x10, arc.konami_lz(data[s:e])).encode()
    return files


def build_all(files):
    """The region's bytes (0x3F910-0x582530) from an extraction."""
    ctls, tbls = [], []
    for n in range(4):
        bank = json.loads(files[f"bank{n}.json"])
        names = sorted({it["sample"] for it in bank["instruments"]}, key=lambda x: int(x[-3:]))
        samples, frames = {}, {}
        for name in names:
            raw, book, loop = from_aifc(files[f"samples/{name}.aifc"])
            frames[name] = raw
            # the bank records the frames rounded up to even (the pad byte from_aifc restores);
            # every sample is type 0 (ADPCM) with flags 0 (the round trip checks it)
            samples[name] = {"len": len(raw), "type": 0, "flags": 0, "book": book, "loop": loop}
        ctl, tbl = build_bank(bank, samples, frames)
        ctls.append(ctl)
        tbls.append(tbl)
    music = b"".join(arc.konami_lz_pack(song_from_text(files[f"music/song_{i + 0x10:02X}.txt"].decode()), 16)
                     for i in range(arc.MUSIC_COUNT))
    return b"".join(ctls) + b"".join(tbls) + music


def check_songs(data):
    """Each song on its own: its script back to the decompressed bytes, and packed back to its
    ROM block. Returns the ids that differ."""
    bad = []
    for i in range(arc.MUSIC_COUNT):
        s = arc.be32(data, arc.MAIN_ROM + arc.MUSIC_ROM_START - arc.MAIN_VRAM + 4 * i)
        e = arc.be32(data, arc.MAIN_ROM + arc.MUSIC_ROM_END - arc.MAIN_VRAM + 4 * i)
        dec = arc.konami_lz(data[s:e])
        back = song_from_text(song_to_text(i + 0x10, dec))
        if back != dec or arc.konami_lz_pack(back, 16) != data[s:e]:
            bad.append(i + 0x10)
    return bad


def check():
    data = rom()
    bad = check_songs(data)
    print(f"songs: {arc.MUSIC_COUNT - len(bad)} of {arc.MUSIC_COUNT} byte-exact from their scripts"
          f" (decompressed and Konami-LZ packed){'; not: ' + ' '.join('0x%02X' % x for x in bad) if bad else ''}")
    files = extract_all()
    got = build_all(files)
    want = data[CTL[0][0]:MUSIC[1]]
    print(f"audio: {len(files)} files extracted ({sum(1 for k in files if k.endswith('.aifc'))} samples,"
          f" {sum(1 for k in files if k.startswith('music/'))} songs as text"
          f" ({sum(1 for k, v in files.items() if k.startswith('music/') and b'.byte' in v)} with raw bytes), 4 banks); rebuilt"
          f" {'BYTE-EXACT' if got == want else 'DIFFERENT'} (0x{len(got):X} of 0x{len(want):X} bytes)")
    if got != want:
        k = next((i for i in range(min(len(got), len(want))) if got[i] != want[i]), min(len(got), len(want)))
        print(f"  first difference at ROM 0x{CTL[0][0] + k:X}")
    return 0 if got == want and not bad else 1


def main():
    cmd = sys.argv[1:]
    if cmd == ["check"]:
        return check()
    if len(cmd) == 2 and cmd[0] == "extract":
        for path, blob in extract_all().items():
            full = os.path.join(cmd[1], path)
            os.makedirs(os.path.dirname(full), exist_ok=True)
            with open(full, "wb") as f:
                f.write(blob)
        return 0
    if len(cmd) == 3 and cmd[0] == "build":
        files = {}
        for d, _, ns in os.walk(cmd[1]):
            for n in ns:
                with open(os.path.join(d, n), "rb") as f:
                    files[os.path.relpath(os.path.join(d, n), cmd[1]).replace("\\", "/")] = f.read()
        with open(cmd[2], "wb") as f:
            f.write(build_all(files))
        return 0
    sys.exit(__doc__)


if __name__ == "__main__":
    sys.exit(main())
