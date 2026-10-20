"""The game's assets from their real formats: extracted at split, rebuilt by every build.

    python canon-decompilation-tools/asset_build.py extract    write assets/ from the decompressed ROM
    python canon-decompilation-tools/asset_build.py build      rebuild the archive and audio bins from assets/
    python canon-decompilation-tools/asset_build.py check      build in memory, compare with the ROM

As the decomps before this one, the game's assets are never committed: build_rom.py --split
extracts them from the user's own ROM into assets/ (git-ignored), each in a real format, and
every build regenerates the bytes from them:
  assets/ptx/XXXX.png        PIC sub-files (pic_codec); XXXX.tex.png the raw ones whose format a
                             model's display list gives (texture_codec); the rest XXXX.bin for now
  assets/models/geo_NNNN.c   the model files, scenery and objects (model_codec.source; compiled,
                             cached by date)
  assets/stages/stage_NNNN.c the stage files: rails, collision, geography (stage_codec; the same)
  assets/scenario/*.txt      the scenario scripts (scenario_codec)
  assets/tables/*.c          archive files 11-15 (table_codec)
  assets/textures/*          the font, BG tile pictures, UI and glyph sheets (texture_codec); each
                             BG picture's C, bg_NNNN.c, written from its PNGs by the build
  assets/audio/              banks (JSON), samples (AIFC), songs (audio_codec)
  assets/logo_png/           the boot logos' art (texture_codec), from and back to splat's bins
  assets/archive/NNNN.bin    every other archive file, raw, until its format has a codec
The yaml's archive bins (files_0001, files_0011, files_0329: the files between the code
segments, in nisitenma.tsv's layout, plus the ROM tail) and its audio bin are reassembled from
these, so the linker sees the same bytes, now made from the real formats.

Changelog: first public release (2026-10-20). Earlier history is kept privately.
"""

import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import audio_codec  # noqa: E402
import model_codec  # noqa: E402
import nisitenma_archive as arc  # noqa: E402
import pic_codec  # noqa: E402
import scenario_codec  # noqa: E402
import stage_codec  # noqa: E402
import table_codec  # noqa: E402
import texture_codec  # noqa: E402

ROOT = arc.ROOT
ASSETS = os.path.join(ROOT, "assets")
BINS = ("files_0001", "files_0011", "files_0329")


def layout():
    """[(file id, work offset, work size)] from nisitenma.tsv, and the yaml's bin ranges."""
    rows = []
    with open(os.path.join(ROOT, "nisitenma.tsv"), encoding="utf-8") as f:
        for line in f.read().splitlines()[1:]:
            p = line.split("\t")
            if p[0].isdigit():
                rows.append((int(p[0]), int(p[2], 16), int(p[3], 16)))
    with open(os.path.join(ROOT, "goemonsgreatadv.yaml"), encoding="utf-8") as f:
        yaml = f.read()
    starts = [int(m.group(1), 16) for m in re.finditer(r"^    start: (0x[0-9A-F]+)", yaml, re.M)]
    end = int(re.findall(r"^  - \[(0x[0-9A-F]+)\]", yaml, re.M)[-1], 16)
    ranges = {}
    for name in BINS + ("audio",):
        s = int(re.search(r"^  - name: %s\n    type: bin\n    start: (0x[0-9A-F]+)" % name, yaml, re.M).group(1), 16)
        ranges[name] = (s, min([x for x in starts if x > s] + [end]))
    return rows, ranges


def _write(path, blob):
    """Written only when it changes, so a re-extraction keeps the build's caches (a model's
    compile is redone only when its C is newer than its bytes)."""
    os.makedirs(os.path.dirname(path), exist_ok=True)
    raw = blob.encode() if isinstance(blob, str) else blob
    if os.path.exists(path):
        with open(path, "rb") as f:
            if f.read() == raw:
                return
    mode = "w" if isinstance(blob, str) else "wb"
    with open(path, mode, **({"encoding": "utf-8", "newline": "\n"} if mode == "w" else {})) as f:
        f.write(blob)


JOBS = int(os.environ.get("BUILD_JOBS", os.cpu_count() or 1))


def _pmap(fn, args):
    """fn(*a) for each a in args, in a pool of JOBS processes (the work is Python), the results in
    args' order. fn is a module-level function of a codec, so the pool's processes import it."""
    if JOBS <= 1 or len(args) < 2 * JOBS:
        return [fn(*a) for a in args]
    import concurrent.futures
    with concurrent.futures.ProcessPoolExecutor(max_workers=min(JOBS, 61)) as ex:
        return list(ex.map(fn, *zip(*args), chunksize=max(1, len(args) // (JOBS * 4))))


def _write_all(items):
    """_write for each (path, blob), in JOBS threads (file I/O)."""
    import concurrent.futures
    with concurrent.futures.ThreadPoolExecutor(max_workers=JOBS) as ex:
        list(ex.map(lambda pb: _write(*pb), items))


def extract():
    """assets/ in real formats, from the decompressed ROM (after nisitenma_archive.py unpack)."""
    files = model_codec.archive()
    covered = set()
    writes = []  # (path, blob), written together at the end (907)
    # PTX containers: each sub-file a PNG (a PIC) or a .bin (raw); the parent is their concatenation
    raws = pic_codec.raw_textures()  # raw sub-files whose format a model's display list gives
    pics, texs = [], []
    for sub, parent, data in pic_codec.subfiles(with_parent=True):
        if data[:3] == b"PIC":
            pics.append((sub, data))
        elif sub in raws:
            texs.append((sub, data))
            stale = os.path.join(ASSETS, "ptx", f"{sub:04X}.bin")  # a raw .bin from before its format was known
            if os.path.exists(stale):
                os.remove(stale)
        else:
            writes.append((os.path.join(ASSETS, "ptx", f"{sub:04X}.bin"), data))
        covered.add(parent)
    for (sub, _), png in zip(pics, _pmap(pic_codec.to_png, [(d,) for _, d in pics])):
        writes.append((os.path.join(ASSETS, "ptx", f"{sub:04X}.png"), png))
    for (sub, _), png in zip(texs, _pmap(texture_codec.n64_to_png, [(d, *raws[s]) for s, d in texs])):
        writes.append((os.path.join(ASSETS, "ptx", f"{sub:04X}.tex.png"), png))
    names = scenario_codec.symbols()
    for fid, data in scenario_codec.files().items():
        writes.append((os.path.join(ASSETS, "scenario", f"scenario_{fid:04d}.txt"), scenario_codec.to_text(fid, data, names)))
        covered.add(fid)
    for fid in table_codec.TABLES:
        writes.append((os.path.join(ASSETS, "tables", f"table_{fid:04d}.c"), table_codec.to_c(fid, files[fid])))
        covered.add(fid)
    for fid in texture_codec.SPECS:
        for name, blob in texture_codec.extract(fid, files[fid]).items():
            writes.append((os.path.join(ASSETS, "textures", name), blob))
        covered.add(fid)
    objects = set(model_codec.object_files(files))
    models = []
    for fid, data in sorted(files.items()):
        if fid not in covered and (fid in objects or model_codec.dl_start(data)):
            models.append((fid, data))
            covered.add(fid)
    for (fid, _), c in zip(models, _pmap(model_codec.source, models)):
        writes.append((os.path.join(ASSETS, "models", f"geo_{fid:04d}.c"), c))
    for fid, (geo, coll, rails) in sorted(stage_codec.stage_files(files).items()):
        if fid not in covered:
            writes.append((os.path.join(ASSETS, "stages", f"stage_{fid:04d}.c"), stage_codec.to_c(fid, files[fid], geo, coll, rails)))
            covered.add(fid)
    for path, blob in audio_codec.extract_all().items():
        writes.append((os.path.join(ASSETS, "audio", path), blob))
    bins = {}
    for name in texture_codec.LOGO:  # the logo's art: splat's bins (logo.c incbins them) to PNG
        with open(os.path.join(ASSETS, "logo", name + ".bin"), "rb") as f:
            bins[name] = f.read()
    for name, blob in texture_codec.logo_to_png(bins).items():
        writes.append((os.path.join(ASSETS, "logo_png", name), blob))
    # the rest of the files the bins hold (the overlays and the TLB code files are code
    # segments, built from src/, not from here)
    rows, ranges = layout()
    in_bins = {fid for fid, wo, ws in rows if ws and any(lo <= wo < hi for name, (lo, hi) in ranges.items() if name in BINS)}
    raw = [fid for fid in files if fid in in_bins and fid not in covered]
    for fid in raw:
        writes.append((os.path.join(ASSETS, "archive", f"{fid:04d}.bin"), files[fid]))
    _write_all(writes)
    for fid in covered:  # a raw bin left from before its codec existed is not a source any more
        stale = os.path.join(ASSETS, "archive", f"{fid:04d}.bin")
        if os.path.exists(stale):
            os.remove(stale)
    print(f"assets: {len(covered & in_bins)} of the bins' {len(in_bins)} files in real formats, {len(raw)} raw;"
          " the audio region")
    return 0


def file_bytes(fid, size, ptx_parents, cache):
    """An archive file's bytes from its real-format source in assets/."""
    def read(path, mode="rb"):
        with open(path, mode, **({"encoding": "utf-8"} if mode == "r" else {})) as f:
            return f.read()
    if fid in ptx_parents:
        out = b""
        for sub in ptx_parents[fid]:
            png = os.path.join(ASSETS, "ptx", f"{sub:04X}.png")
            tex = os.path.join(ASSETS, "ptx", f"{sub:04X}.tex.png")
            if os.path.exists(png):
                out += pic_codec.to_pic(read(png))
            elif os.path.exists(tex):
                out += texture_codec.png_to_n64(read(tex))
            else:
                out += read(os.path.join(ASSETS, "ptx", f"{sub:04X}.bin"))
        return out
    p = os.path.join(ASSETS, "scenario", f"scenario_{fid:04d}.txt")
    if os.path.exists(p):
        return scenario_codec.from_text(read(p, "r"), cache.setdefault("names", scenario_codec.symbols()))
    if fid in texture_codec.SPECS:
        d = os.path.join(ASSETS, "textures")
        parts = {n: read(os.path.join(d, n)) for n in os.listdir(d) if n.startswith(f"{fid:04d}.")}
        if fid not in texture_codec.BG:
            return texture_codec.build(fid, parts, size)
        # a BG tile picture: its C (written from the PNGs and the map), compiled like a model's
        src = os.path.join(d, f"bg_{fid:04d}.c")
        text = texture_codec.bg_c(fid, parts)
        _write(src, text)
        done = os.path.join(ROOT, "build", "assets", "textures", f"bg_{fid:04d}.bin")
        if not os.path.exists(done) or os.path.getmtime(done) < os.path.getmtime(src):
            blob = model_codec.build(text, os.path.join(ROOT, "build", "assets", "scratch", str(os.getpid())))
            _write(done, blob + bytes(max(0, size - len(blob))))
        return read(done)
    for kind, name in (("tables", f"table_{fid:04d}"), ("models", f"geo_{fid:04d}"), ("stages", f"stage_{fid:04d}")):
        src = os.path.join(ASSETS, kind, name + ".c")
        if os.path.exists(src):
            done = os.path.join(ROOT, "build", "assets", kind, name + ".bin")
            if not os.path.exists(done) or os.path.getmtime(done) < os.path.getmtime(src):
                blob = model_codec.build(read(src, "r"), os.path.join(ROOT, "build", "assets", "scratch", str(os.getpid())),
                                         base=0x08000000 if kind == "tables" else model_codec.SEG3)
                _write(done, blob + bytes(max(0, size - len(blob))))
            return read(done)
    return read(os.path.join(ASSETS, "archive", f"{fid:04d}.bin"))


C_KINDS = (("tables", "table"), ("models", "geo"), ("stages", "stage"))


def _compile_stale_c(rows, ranges):
    """Every model, table and stage C newer than its .bin, compiled side by side (907), each
    thread in its own scratch folder; file_bytes then finds each .bin fresh and reads it."""
    import concurrent.futures
    import threading
    jobs = []
    for fid, wo, ws in rows:
        if not ws or not any(lo <= wo < hi for name, (lo, hi) in ranges.items() if name in BINS):
            continue
        if fid in texture_codec.SPECS or os.path.exists(os.path.join(ASSETS, "scenario", f"scenario_{fid:04d}.txt")):
            continue  # file_bytes takes these first
        for kind, stem in C_KINDS:
            src = os.path.join(ASSETS, kind, f"{stem}_{fid:04d}.c")
            if os.path.exists(src):
                done = os.path.join(ROOT, "build", "assets", kind, f"{stem}_{fid:04d}.bin")
                if not os.path.exists(done) or os.path.getmtime(done) < os.path.getmtime(src):
                    jobs.append((src, done, kind, ws))
                break

    def one(src, done, kind, size):
        with open(src, encoding="utf-8") as f:
            text = f.read()
        scratch = os.path.join(ROOT, "build", "assets", "scratch", f"{os.getpid()}_{threading.get_ident()}")
        blob = model_codec.build(text, scratch, base=0x08000000 if kind == "tables" else model_codec.SEG3)
        _write(done, blob + bytes(max(0, size - len(blob))))
    with concurrent.futures.ThreadPoolExecutor(max_workers=JOBS) as ex:
        for f in [ex.submit(one, *j) for j in jobs]:
            f.result()


def build(write=True):
    """The archive and audio bins from assets/; returns {bin name: bytes}."""
    rows, ranges = layout()
    _compile_stale_c(rows, ranges)
    ptx_parents = {}
    for sub, parent, _ in pic_codec.subfiles(with_parent=True):
        ptx_parents.setdefault(parent, []).append(sub)
    with open(arc.DECOMPRESSED, "rb") as f:
        dec = f.read()
    out, cache = {}, {}
    for name in BINS:
        lo, hi = ranges[name]
        buf = bytearray(hi - lo)
        last = lo
        for fid, wo, ws in rows:
            if ws and lo <= wo < hi:
                buf[wo - lo:wo - lo + ws] = file_bytes(fid, ws, ptx_parents, cache)[:ws]
                last = max(last, wo + ws)
        buf[last - lo:] = dec[last:hi]  # the ROM tail after the last file (files_0329)
        out[name] = bytes(buf)
    files = {}
    adir = os.path.join(ASSETS, "audio")
    for d, _, ns in os.walk(adir):
        for n in ns:
            with open(os.path.join(d, n), "rb") as f:
                files[os.path.relpath(os.path.join(d, n), adir).replace("\\", "/")] = f.read()
    out["audio"] = audio_codec.build_all(files)
    pdir = os.path.join(ASSETS, "logo_png")
    pngs = {}
    for n in os.listdir(pdir):
        with open(os.path.join(pdir, n), "rb") as f:
            pngs[n] = f.read()
    for name, blob in texture_codec.logo_from_png(pngs).items():
        out[os.path.join("logo", name)] = blob
    if write:
        for name, blob in out.items():
            _write(os.path.join(ASSETS, name + ".bin"), blob)
            os.utime(os.path.join(ASSETS, name + ".bin"), None)  # made now, changed or not (up_to_date)
        for name in texture_codec.LOGO:  # logo.c #includes these (no GLOBAL_ASM)
            inc = os.path.join(ASSETS, "logo", name + ".inc.c")
            _write(inc, texture_codec.logo_inc_c(name, out[os.path.join("logo", name)]))
            os.utime(inc, None)
        with open(ARC_STAMP, "w", encoding="utf-8", newline="\n") as f:
            f.write(_arc_used_hash() + "\n")
    return out


def check():
    rows, ranges = layout()
    with open(arc.DECOMPRESSED, "rb") as f:
        dec = f.read()
    built = build(write=False)
    bad = [name for name, blob in built.items() if name in ranges and blob != dec[ranges[name][0]:ranges[name][1]]]
    for name in texture_codec.LOGO:  # the logo's bins: against splat's own
        with open(os.path.join(ASSETS, "logo", name + ".bin"), "rb") as f:
            if built[os.path.join("logo", name)] != f.read():
                bad.append(name)
    print(f"assets: {len(built) - len(bad)} of {len(built)} bins rebuilt from the real formats byte-exact;"
          f" not: {' '.join(bad)}")
    return 0 if not bad else 1


ARC_USED = ("ROOT", "DECOMPRESSED", "BASEROM", "MAIN_ROM", "MAIN_VRAM", "MUSIC_COUNT", "MUSIC_ROM_START",
            "MUSIC_ROM_END", "be32", "konami_lz", "konami_lz_pack")


def _arc_used_hash():
    """A hash of what the bins use of nisitenma_archive.py: ARC_USED's values and functions'
    source (the cutter and the rest of that file do not make a byte of any bin)."""
    import hashlib
    import inspect
    h = hashlib.sha1()
    for name in ARC_USED:
        v = getattr(arc, name)
        h.update((name + "\0" + (inspect.getsource(v) if callable(v) else repr(v)) + "\0").encode("utf-8"))
    return h.hexdigest()


ARC_STAMP = os.path.join(ASSETS, ".arc_used.sha1")


def up_to_date():
    """True when every bin build writes is newer than every real-format source in assets/ (and
    than the codecs), and what the bins use of nisitenma_archive.py is unchanged since the last
    build (ARC_STAMP): the build then has nothing to remake."""
    outs = [os.path.join(ASSETS, n + ".bin") for n in BINS + ("audio",)]
    outs += [os.path.join(ASSETS, "logo", n + ".bin") for n in texture_codec.LOGO]
    outs += [os.path.join(ASSETS, "logo", n + ".inc.c") for n in texture_codec.LOGO]
    if not all(os.path.exists(p) for p in outs):
        return False
    oldest = min(os.path.getmtime(p) for p in outs)
    srcs = [os.path.join(d, n) for sub in ("ptx", "models", "stages", "scenario", "tables", "textures", "audio",
                                            "logo_png", "archive")
            for d, _, ns in os.walk(os.path.join(ASSETS, sub)) for n in ns]
    # only the code that makes the bins: this file and the codecs it imports (not every tool)
    srcs += [os.path.abspath(__file__)] + [m.__file__ for m in (audio_codec, model_codec, pic_codec,
                                                                scenario_codec, stage_codec, table_codec,
                                                                texture_codec)]
    if not os.path.exists(ARC_STAMP):
        return False
    with open(ARC_STAMP, encoding="utf-8") as f:
        if f.read().strip() != _arc_used_hash():
            return False
    return all(os.path.getmtime(p) < oldest for p in srcs)


if __name__ == "__main__":
    cmd = sys.argv[1:]
    if cmd == ["extract"]:
        sys.exit(extract())
    if cmd == ["build"]:
        if not up_to_date():
            build()
        sys.exit(0)
    if cmd == ["check"]:
        sys.exit(check())
    sys.exit(__doc__)
