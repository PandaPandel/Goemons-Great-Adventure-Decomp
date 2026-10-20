"""The game's plain texture files as PNG (and tile maps as text), back to the same bytes.

    python canon-decompilation-tools/texture_codec.py check            every file through PNG and back
    python canon-decompilation-tools/texture_codec.py extract OUTDIR   the PNGs and maps
    python canon-decompilation-tools/texture_codec.py logo-inc [OUTDIR] the logo's .inc.c from assets/logo_png/
                                                                       (default assets/logo/, where logo.c
                                                                       #includes them)

Each file is a list of regions, laid out as the code that draws it proves (the asset survey,
results/03_3.txt, 09_9.txt, 10_10.txt, 13_13.txt):
  - file 1: the debug ASCII font, CI4 128x32 (asciiDraw: gDPLoadTextureBlock_4b 128x32); its
    palette is main's gAsciiFontTlut (sys.c), so the PNG shows it with that palette;
  - the BG tile pictures (cambg.c's gBgTable in file 15, and file 704 for Bbg): 1-5 palettes
    of 256 RGBA5551 colours (dawn, day, dusk, night), 16x16 CI8 tiles (bgDraw loads 8 at a
    time as a 16x128 block), a 32x16 map of u16 cells (the tile in the low 14 bits; bit 15
    mirrors S, bit 14 T). A tile sheet PNG (the tiles stacked, in palette 0), one 16x16 swatch
    PNG per palette, the map as text ("12", "12s", "12t", "12st");
  - file 2576: the message cursor and "next"-mark sheets, RGBA16 32x64 and 32x16 (msgfrm.c);
  - file 2521: a 4-bit glyph sheet, 81 cells of 16x16 (I4, as measured; no reader found).

Changelog: first public release (2026-10-20). Earlier history is kept privately.
"""

import io
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import model_codec as mc  # noqa: E402
import nisitenma_archive as arc  # noqa: E402

ROOT = mc.ROOT
FONT_TLUT = 0x800379C8  # gAsciiFontTlut (sys.c)


# file id -> regions: (kind, offset, ...)
SPECS = {
    1: [("ci4font", 0, 128, 32)],
    704: [("pal", 0, 0x200), ("ci8tiles", 0x200, 61), ("map", 0x3F00, 32, 16)],
    709: [("pal", 0, 0x200), ("ci8tiles", 0x200, 64), ("map", 0x4200, 32, 16)],
    757: [("pal", 0, 0x600), ("ci8tiles", 0x600, 0x35), ("map", 0x3B00, 32, 16)],
    758: [("pal", 0, 0xA00), ("ci8tiles", 0xA00, 0x5A), ("map", 0x6400, 32, 16)],
    759: [("pal", 0, 0xA00), ("ci8tiles", 0xA00, 0x79), ("map", 0x8300, 32, 16)],
    760: [("pal", 0, 0x800), ("ci8tiles", 0x800, 0x5F), ("map", 0x6700, 32, 16)],
    761: [("pal", 0, 0x200), ("ci8tiles", 0x200, 0x100), ("map", 0x10200, 32, 16)],
    2521: [("i4", 0, 16, 16 * 81)],
    2576: [("rgba16", 0, 32, 64), ("rgba16", 0x1000, 32, 16)],
}


def _rgba(v):
    c = lambda x: (x << 3) | (x >> 2)  # noqa: E731
    return (c(v >> 11), c((v >> 6) & 31), c((v >> 1) & 31), 255 if v & 1 else 0)


def _5551(r, g, b, a):
    return ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1) | (1 if a else 0)


def _png(w, h, rows, **kw):
    import png
    buf = io.BytesIO()
    png.Writer(w, h, **kw).write(buf, rows)
    return buf.getvalue()


def _read(blob):
    import png
    w, h, rows, info = png.Reader(bytes=blob).read()
    return w, h, [list(r) for r in rows], info


# The boot logos' art (logo.c incbins these splat bins, assets/logo/): CI8 textures and their
# RGBA16 palettes, the sizes as logo.c loads them. A palette is a 16x16 swatch PNG; a texture an
# indexed PNG shown in its palette (its indices are what it rebuilds from).
LOGO = {
    "gLogoFirstTlut": ("tlut",), "gLogoSecondTlut": ("tlut",), "gLogoBackdropTlut": ("tlut",),
    "gLogoFirstTex0": (64, 32, "gLogoFirstTlut"), "gLogoFirstTex1": (72, 24, "gLogoFirstTlut"),
    "gLogoFirstTex2": (64, 32, "gLogoFirstTlut"), "gLogoFirstTex3": (128, 16, "gLogoFirstTlut"),
    "gLogoFirstTex4": (64, 16, "gLogoFirstTlut"), "gLogoFirstTex5": (128, 16, "gLogoFirstTlut"),
    "gLogoSecondTex0": (32, 32, "gLogoSecondTlut"), "gLogoSecondTex1": (128, 16, "gLogoSecondTlut"),
    "gLogoSecondTex2": (112, 16, "gLogoSecondTlut"), "gLogoSecondTex3": (112, 16, "gLogoSecondTlut"),
    "gLogoSecondTex4": (128, 16, "gLogoSecondTlut"), "gLogoSecondTex5": (128, 16, "gLogoSecondTlut"),
    "gLogoSecondTex6": (128, 16, "gLogoSecondTlut"), "gLogoSecondTex7": (128, 16, "gLogoSecondTlut"),
    "gLogoBackdropTex": (32, 32, "gLogoBackdropTlut"),
}


def logo_to_png(bins):
    """{name.png: bytes} for the logo art, from {name: bin bytes}."""
    out = {}
    for name, spec in LOGO.items():
        if spec[0] == "tlut":
            pal = [struct.unpack_from(">H", bins[name], 2 * i)[0] for i in range(256)]
            rows = [[v for i in range(16) for v in _rgba(pal[16 * y + i])] for y in range(16)]
            out[name + ".png"] = _png(16, 16, rows, greyscale=False, alpha=True)
        else:
            w, h, tl = spec
            pal = [_rgba(struct.unpack_from(">H", bins[tl], 2 * i)[0]) for i in range(256)]
            rows = [list(bins[name][y * w:(y + 1) * w]) for y in range(h)]
            out[name + ".png"] = _png(w, h, rows, palette=pal, bitdepth=8)
    return out


def logo_from_png(pngs):
    """{name: bin bytes} for the logo art, from logo_to_png's PNGs."""
    out = {}
    for name, spec in LOGO.items():
        w, h, rows, _ = _read(pngs[name + ".png"])
        if spec[0] == "tlut":
            out[name] = b"".join(struct.pack(">H", _5551(*r[4 * i:4 * i + 4])) for r in rows for i in range(16))
        else:
            out[name] = bytes(v for r in rows for v in r[:w])
    return out


def logo_inc_c(name, blob):
    """A logo piece's bytes (logo_from_png's) as the C initializer logo.c #includes inside its
    array's braces: a palette as u16 values (8 a line), a texture as u8 values (16 a line)."""
    if LOGO[name][0] == "tlut":
        vals = [f"0x{v:04X}," for v in struct.unpack(f">{len(blob) // 2}H", blob)]
        per = 8
    else:
        vals = [f"0x{v:02X}," for v in blob]
        per = 16
    return "".join(" ".join(vals[k:k + per]) + "\n" for k in range(0, len(vals), per))


def logo_inc_write(outdir):
    """Every logo piece's .inc.c in outdir, from assets/logo_png/."""
    pdir = os.path.join(ROOT, "assets", "logo_png")
    pngs = {}
    for n in os.listdir(pdir):
        with open(os.path.join(pdir, n), "rb") as f:
            pngs[n] = f.read()
    os.makedirs(outdir, exist_ok=True)
    for name, blob in logo_from_png(pngs).items():
        with open(os.path.join(outdir, name + ".inc.c"), "w", encoding="utf-8", newline="\n") as f:
            f.write(logo_inc_c(name, blob))
    return len(LOGO)


FMT = {0: "rgba", 2: "ci", 3: "ia", 4: "i"}
BITS = {0: 4, 1: 8, 2: 16, 3: 32}


def n64_to_png(data, fmt, siz, w, h):
    """A raw N64 texture (G_IM_FMT/G_IM_SIZ as the display list loads it; a CI texture's TLUT
    right after its pixels) as PNG, the format and the bytes after the image in a tEXt chunk."""
    bits = BITS[siz]
    n = w * h * bits // 8
    px = data[:n]

    def nib(i):
        return (px[i // 2] >> (4 if i % 2 == 0 else 0)) & 15
    if FMT[fmt] == "ci":
        colors = 16 if bits == 4 else 256
        pal = [_rgba(struct.unpack_from(">H", data, n + 2 * k)[0]) for k in range(colors)]
        rows = [[nib(y * w + x) if bits == 4 else px[y * w + x] for x in range(w)] for y in range(h)]
        blob = _png(w, h, rows, palette=pal, bitdepth=bits)
        used = n + 2 * colors
    elif FMT[fmt] == "rgba" and bits == 16:
        rows = [[v for x in range(w) for v in _rgba(struct.unpack_from(">H", px, 2 * (y * w + x))[0])] for y in range(h)]
        blob, used = _png(w, h, rows, greyscale=False, alpha=True), n
    elif FMT[fmt] == "rgba" and bits == 32:
        rows = [list(px[4 * y * w:4 * (y + 1) * w]) for y in range(h)]
        blob, used = _png(w, h, rows, greyscale=False, alpha=True), n
    elif FMT[fmt] == "i":
        rows = [[nib(y * w + x) if bits == 4 else px[y * w + x] for x in range(w)] for y in range(h)]
        blob, used = _png(w, h, rows, greyscale=True, bitdepth=bits), n
    elif FMT[fmt] == "ia" and bits == 16:
        rows = [list(px[2 * y * w:2 * (y + 1) * w]) for y in range(h)]
        blob, used = _png(w, h, rows, greyscale=True, alpha=True), n
    elif FMT[fmt] == "ia":  # IA8 (4+4) and IA4 (3+1) as grey and alpha, exact both ways
        vals = [nib(y * w + x) if bits == 4 else px[y * w + x] for y in range(h) for x in range(w)]
        rows = [[c for v in vals[y * w:(y + 1) * w] for c in ((v >> 4, v & 15) if bits == 8 else (v >> 1, v & 1))]
                for y in range(h)]
        blob, used = _png(w, h, rows, greyscale=True, alpha=True, bitdepth=4 if bits == 8 else 4), n
    else:
        raise ValueError(f"fmt {fmt} siz {siz}")
    tail = data[used:]
    meta = {"N64-fmt": f"{fmt} {siz}"}
    if any(tail):
        meta["N64-tail"] = tail.hex()
    else:
        meta["N64-pad"] = str(len(tail))
    import zlib
    chunks = b""
    for k, v in meta.items():
        body = k.encode() + b"\0" + v.encode()
        chunks += struct.pack(">I", len(body)) + b"tEXt" + body + struct.pack(">I", zlib.crc32(b"tEXt" + body))
    return blob[:-12] + chunks + blob[-12:]


def png_to_n64(blob):
    """n64_to_png's PNG back to the texture's bytes."""
    import png
    w, h, rows, info = _read(blob)
    meta = {}
    for ctype, body in png.Reader(bytes=blob).chunks():
        if ctype == b"tEXt":
            k, v = body.split(b"\0", 1)
            meta[k.decode()] = v.decode()
    fmt, siz = (int(x) for x in meta["N64-fmt"].split())
    bits = BITS[siz]
    out = bytearray()

    def pack4(vals):
        return bytes((vals[i] << 4) | vals[i + 1] for i in range(0, len(vals), 2))
    if FMT[fmt] == "ci":
        flat = [v for r in rows for v in r[:w]]
        out += pack4(flat) if bits == 4 else bytes(flat)
        for c in info["palette"]:
            out += struct.pack(">H", _5551(c[0], c[1], c[2], c[3] if len(c) > 3 else 255))
    elif FMT[fmt] == "rgba" and bits == 16:
        for r in rows:
            for x in range(w):
                out += struct.pack(">H", _5551(*r[4 * x:4 * x + 4]))
    elif FMT[fmt] in ("rgba", "ia") and bits in (32, 16):
        out += bytes(v for r in rows for v in r)
    elif FMT[fmt] == "i":
        flat = [v for r in rows for v in r[:w]]
        out += pack4(flat) if bits == 4 else bytes(flat)
    else:  # IA8 / IA4
        vals = [(r[2 * x] << 4 | r[2 * x + 1]) if bits == 8 else (r[2 * x] << 1 | r[2 * x + 1]) for r in rows for x in range(w)]
        out += pack4(vals) if bits == 4 else bytes(vals)
    return bytes(out) + (bytes.fromhex(meta["N64-tail"]) if "N64-tail" in meta else bytes(int(meta["N64-pad"])))


def font_palette():
    with open(arc.DECOMPRESSED, "rb") as f:
        dec = f.read()
    at = arc.MAIN_ROM + FONT_TLUT - arc.MAIN_VRAM
    return [struct.unpack_from(">H", dec, at + 2 * i)[0] for i in range(16)]


def extract(fid, data):
    """{name: bytes} for a file's regions."""
    out = {}
    pals = []
    for spec in SPECS[fid]:
        kind, off = spec[0], spec[1]
        if kind == "pal":
            n = spec[2] // 0x200
            pals = [[struct.unpack_from(">H", data, off + 0x200 * k + 2 * i)[0] for i in range(256)] for k in range(n)]
            for k, p in enumerate(pals):
                rows = [[v for i in range(16) for v in _rgba(p[16 * y + i])] for y in range(16)]
                out[f"{fid:04d}.pal{k}.png"] = _png(16, 16, rows, greyscale=False, alpha=True)
        elif kind == "ci8tiles":
            n = spec[2]
            rows = [list(data[off + 16 * y:off + 16 * y + 16]) for y in range(16 * n)]
            out[f"{fid:04d}.tiles.png"] = _png(16, 16 * n, rows, palette=[_rgba(v) for v in pals[0]], bitdepth=8)
        elif kind == "map":
            w, h = spec[2], spec[3]
            lines = []
            for y in range(h):
                cells = []
                for x in range(w):
                    v = struct.unpack_from(">H", data, off + 2 * (y * w + x))[0]
                    cells.append(f"{v & 0x3FFF}{'s' if v & 0x8000 else ''}{'t' if v & 0x4000 else ''}")
                lines.append(" ".join(cells))
            out[f"{fid:04d}.map.txt"] = ("\n".join(lines) + "\n").encode()
        elif kind == "ci4font":
            w, h = spec[2], spec[3]
            rows = [[(data[off + (y * w + x) // 2] >> (4 if x % 2 == 0 else 0)) & 15 for x in range(w)] for y in range(h)]
            out[f"{fid:04d}.png"] = _png(w, h, rows, palette=[_rgba(v) for v in font_palette()], bitdepth=4)
        elif kind == "i4":
            w, h = spec[2], spec[3]
            rows = [[(data[off + (y * w + x) // 2] >> (4 if x % 2 == 0 else 0)) & 15 for x in range(w)] for y in range(h)]
            out[f"{fid:04d}.png"] = _png(w, h, rows, greyscale=True, bitdepth=4)
        elif kind == "rgba16":
            w, h = spec[2], spec[3]
            rows = [[v for x in range(w) for v in _rgba(struct.unpack_from(">H", data, off + 2 * (y * w + x))[0])]
                    for y in range(h)]
            out[f"{fid:04d}.{off:04X}.png"] = _png(w, h, rows, greyscale=False, alpha=True)
    return out


def build(fid, files, size):
    """A file's bytes from extract's output."""
    out = bytearray(size)
    for spec in SPECS[fid]:
        kind, off = spec[0], spec[1]
        if kind == "pal":
            for k in range(spec[2] // 0x200):
                w, h, rows, _ = _read(files[f"{fid:04d}.pal{k}.png"])
                for y in range(16):
                    for i in range(16):
                        struct.pack_into(">H", out, off + 0x200 * k + 2 * (16 * y + i), _5551(*rows[y][4 * i:4 * i + 4]))
        elif kind == "ci8tiles":
            w, h, rows, _ = _read(files[f"{fid:04d}.tiles.png"])
            out[off:off + w * h] = bytes(v for r in rows for v in r)
        elif kind == "map":
            w = spec[2]
            for y, line in enumerate(files[f"{fid:04d}.map.txt"].decode().splitlines()):
                for x, cell in enumerate(line.split()):
                    v = int(cell.rstrip("st")) | (0x8000 if "s" in cell else 0) | (0x4000 if "t" in cell else 0)
                    struct.pack_into(">H", out, off + 2 * (y * w + x), v)
        elif kind in ("ci4font", "i4"):
            w, h, rows, _ = _read(files[f"{fid:04d}.png"])
            for y in range(h):
                for x in range(0, w, 2):
                    out[off + (y * w + x) // 2] = (rows[y][x] << 4) | rows[y][x + 1]
        elif kind == "rgba16":
            w, h, rows, _ = _read(files[f"{fid:04d}.{off:04X}.png"])
            for y in range(h):
                for x in range(w):
                    struct.pack_into(">H", out, off + 2 * (y * w + x), _5551(*rows[y][4 * x:4 * x + 4]))
    return bytes(out)


# the BG tile pictures: the files with a map
BG = [fid for fid, regs in SPECS.items() if any(r[0] == "map" for r in regs)]


def bg_c(fid, files):
    """A BG tile picture as C, from extract's PNGs and map text: its palettes as u16 RGBA5551, its
    tiles as u8 (CI8, 16x16 each), its map as u16 cells written with object.h's BG_CELL bits, in
    file order. asset_build compiles it into the file (model_codec.build)."""
    regs = {r[0]: r for r in SPECS[fid]}
    _, poff, psize = regs["pal"]
    _, toff, ntiles = regs["ci8tiles"]
    _, moff, w, h = regs["map"]
    if not (poff == 0 and toff == poff + psize and moff == toff + 0x100 * ntiles):
        raise ValueError(f"{fid}: the palettes, tiles and map are not back to back")
    data = build(fid, files, moff + 2 * w * h)
    npal = psize // 0x200
    name = f"bg_{fid:04d}"

    def rows(vals, per, fmt):
        return ["        " + " ".join(fmt.format(v) + "," for v in vals[k:k + per]) for k in range(0, len(vals), per)]

    lines = [f"/* File {fid}: a BG tile picture (cambg.c's gBgTable in file 15; tilemap.c's bgDraw draws it),",
             f" * written from assets/textures/{fid:04d}.* by texture_codec.bg_c. */",
             '#include "object.h"', "",
             f"/* {npal} palette{'s' if npal > 1 else ''} of 256 RGBA5551 colours */",
             f"u16 {name}_pal[{npal}][256] = {{"]
    for k in range(npal):
        lines += ["    {"] + rows(struct.unpack_from(">256H", data, 0x200 * k), 8, "0x{:04X}") + ["    },"]
    lines += ["};", "", f"/* {ntiles} tiles, CI8 16x16 */", f"u8 {name}_tiles[{ntiles}][16 * 16] = {{"]
    for t in range(ntiles):
        lines += ["    {"] + rows(data[toff + 0x100 * t:toff + 0x100 * (t + 1)], 16, "0x{:02X}") + ["    },"]

    def cell(v):
        s = str(v & 0x3FFF)
        s += " | BG_CELL_MIRROR_S" if v & 0x8000 else ""
        return s + (" | BG_CELL_MIRROR_T" if v & 0x4000 else "")
    lines += ["};", "", f"/* the map, {w}x{h} cells: a tile, mirrored in S or T */", f"u16 {name}_map[{h}][{w}] = {{"]
    for y in range(h):
        cells = [cell(v) for v in struct.unpack_from(f">{w}H", data, moff + 2 * w * y)]
        lines += ["    {"] + ["        " + " ".join(c + "," for c in cells[k:k + 8]) for k in range(0, w, 8)] + ["    },"]
    return "\n".join(lines + ["};", ""])


def main():
    cmd = sys.argv[1:]
    files = mc.archive()
    if cmd == ["check"]:
        bad = [str(fid) for fid in SPECS if build(fid, extract(fid, files[fid]), len(files[fid])) != files[fid]]
        print(f"textures: {len(SPECS) - len(bad)} of {len(SPECS)} files through PNG and back byte-exact;"
              f" not: {' '.join(bad)}")
        scratch = os.path.join(ROOT, "permuter", "texture_codec", str(os.getpid()))
        cbad = []
        for fid in BG:  # and the BG tile pictures on through their C, compiled as the build does
            got = mc.build(bg_c(fid, extract(fid, files[fid])), scratch)
            if got + bytes(max(0, len(files[fid]) - len(got))) != files[fid]:
                cbad.append(str(fid))
        print(f"textures: {len(BG) - len(cbad)} of {len(BG)} BG tile pictures through PNG, C (IDO) and back"
              f" byte-exact; not: {' '.join(cbad)}")
        bad += cbad
        import pic_codec
        subs = dict(pic_codec.subfiles())
        raws = pic_codec.raw_textures()
        rbad = [f"0x{s:04X}" for s, (fm, sz, w, h) in raws.items() if png_to_n64(n64_to_png(subs[s], fm, sz, w, h)) != subs[s]]
        nraw = sum(1 for d in subs.values() if d[:3] != b"PIC")
        print(f"textures: {len(raws) - len(rbad)} of {len(raws)} raw PTX sub-files with a known format (of the"
              f" {nraw} raw ones) through PNG and back byte-exact; not: {' '.join(rbad[:20])}")
        bins = {}
        for name in LOGO:
            with open(os.path.join(ROOT, "assets", "logo", name + ".bin"), "rb") as f:
                bins[name] = f.read()
        back = logo_from_png(logo_to_png(bins))
        for n in LOGO:  # and through logo_inc_c's C initializer, read back as the compiler stores it
            vals = [int(v, 16) for v in logo_inc_c(n, back[n]).replace(",", " ").split()]
            back[n] = b"".join(v.to_bytes(2 if LOGO[n][0] == "tlut" else 1, "big") for v in vals)
        lbad = [n for n in LOGO if back[n] != bins[n]]
        print(f"textures: {len(LOGO) - len(lbad)} of {len(LOGO)} logo bins through PNG and C (.inc.c) and back byte-exact;"
              f" not: {' '.join(lbad)}")
        return 0 if not bad and not rbad and not lbad else 1
    if cmd[:1] == ["logo-inc"] and len(cmd) <= 2:
        outdir = cmd[1] if len(cmd) == 2 else os.path.join(ROOT, "assets", "logo")
        print(f"textures: {logo_inc_write(outdir)} logo .inc.c written to {outdir}")
        return 0
    if len(cmd) == 2 and cmd[0] == "extract":
        os.makedirs(cmd[1], exist_ok=True)
        for fid in SPECS:
            for name, blob in extract(fid, files[fid]).items():
                with open(os.path.join(cmd[1], name), "wb") as f:
                    f.write(blob)
        return 0
    sys.exit(__doc__)


if __name__ == "__main__":
    sys.exit(main())
