"""Build Goemon's Great Adventure (USA) from the split sources and check it against the baserom.

    python canon-decompilation-tools/build_rom.py            assemble, link, objcopy, pack, compare both SHA-1s
    python canon-decompilation-tools/build_rom.py --split    unpack baserom.us.z64 (canon-decompilation-tools/nisitenma_archive.py) and run splat on the
                                     decompressed ROM first (asm/, assets/ and the decompressed ROM
                                     are generated from the ROM and never committed)
    python canon-decompilation-tools/build_rom.py --clean    remove build/ first
    python canon-decompilation-tools/build_rom.py --setup    fetch the toolchain into tools/, each tool at
                                     a pinned version, and the lib/ submodules (once, after cloning;
                                     run again after a pin changes). It ends by compiling a 40 KB
                                     file with both IDOs.

Pipeline: --split extracts the game's assets into assets/ in real formats (asset_build.py:
PNG, model and table C, scenario text, bank JSON and sample AIFC); every build makes the
archive and audio bins from them (asset_build.py build); the linker produces the DECOMPRESSED
ROM (checked against the yaml's sha1), then canon-decompilation-tools/nisitenma_archive.py
packs its files back into the ROM (checked against goemonsgreatadv.us.sha1).

Toolchain (override with environment variables; the defaults are what --setup puts in tools/):
    BINUTILS       directory holding mips64-elf-as/ld/objcopy/ar (default tools/mips64/bin:
                   libdragon's GNU binutils for Windows. Elsewhere, when that is absent, the
                   folder of a mips64-elf-as on PATH)
    IDO            the IDO 7.1 compiler driver (cc)             (default tools/ido-7.1/cc:
                   decompals/ido-static-recomp's 7.1, built from source on Windows). The game is
                   7.1: it truncates short/char values in place (sll s0,s0,16; sra s0,s0,16),
                   which IDO 5.3 never does.
    IDO53          the IDO 5.3 cc, for libultra (canon-decompilation-tools/build_libultra.py)
                   (default tools/ido-5.3/cc: decompals/ido-static-recomp's 5.3 release).
                   libultra 2.0I was built with 5.3.
    ASM_PROCESSOR  asm-processor's build.py                     (default tools/asm-processor/build.py)
    Target: GNU binutils assembling o32 big-endian (-EB -32 -march=vr4300), linked as elf32ebmip.
    C: IDO through asm-processor, so a function not yet decompiled stays `#pragma GLOBAL_ASM`.
    -mips2: the ROM's code uses branch-likely instructions, which IDO emits only under -mips2.
    Headers: libultra 2.0I from the ultralib submodule (lib/ultralib; `git submodule update
    --init`), with ultralib's own IDO defines, and the F3DEX2 GBI.
    zlib 1.1.3's inflate side is built from the zlib submodule (lib/zlib, upstream at v1.1.3)
    by canon-decompilation-tools/build_zlib.py, with the game's IDO 7.1.
    IDO_EXTRA      extra IDO flags, space-separated, for one measurement only (e.g. a uopt
                   option through -Wo,...); never set for the build itself.
    MSYS_RUNTIME   (Windows) an MSYS2 usr/bin holding a good msys-2.0.dll, put first on PATH for
                   the IDO recomp .exes (default: the first of C:/devkitPro/msys2/usr/bin and
                   C:/msys64/usr/bin that has one). With Git for Windows' msys-2.0.dll, cfe stops
                   after about 32 KB of preprocessed text ("Unexpected End-of-file"), which every
                   file including ultra64.h exceeds. --setup also builds IDO 7.1 with its gcc,
                   g++ and make.

Changelog: first public release (2026-10-20). Earlier history is kept privately.
"""

import concurrent.futures
import hashlib
import os
import re
import shutil
import subprocess
import sys
import threading
import time

HERE = os.path.dirname(os.path.abspath(__file__))  # canon-decompilation-tools/
ROOT = os.path.dirname(HERE)
NAME = "goemonsgreatadv"
EXE = ".exe" if os.name == "nt" else ""
TOOLS = os.path.join(ROOT, "tools")  # the toolchain --setup fetches; ignored by git (tools/.gitignore)


def tool_path(p):
    """Absolute, with forward slashes. asm-processor runs the assembler through os.system after
    shlex-quoting it; on Windows a backslashed path gets single quotes that cmd.exe rejects."""
    return os.path.abspath(p).replace("\\", "/")


def default_binutils():
    """tools/mips64/bin (--setup fetches it on Windows); elsewhere, when that is absent, the folder
    of a mips64-elf-as on PATH."""
    here = os.path.join(TOOLS, "mips64", "bin")
    found = shutil.which("mips64-elf-as")
    return os.path.dirname(found) if os.name != "nt" and not os.path.isdir(here) and found else here


BINUTILS = os.environ.get("BINUTILS", default_binutils())
AS = tool_path(os.path.join(BINUTILS, "mips64-elf-as" + EXE))
LD = tool_path(os.path.join(BINUTILS, "mips64-elf-ld" + EXE))
OBJCOPY = tool_path(os.path.join(BINUTILS, "mips64-elf-objcopy" + EXE))
AS_FLAGS = ["-EB", "-32", "-march=vr4300", "-G0", "-I", "include"]
IDO = tool_path(os.environ.get("IDO", os.path.join(TOOLS, "ido-7.1", "cc" + EXE)))
ASM_PROCESSOR = tool_path(os.environ.get("ASM_PROCESSOR", os.path.join(TOOLS, "asm-processor", "build.py")))
ULTRALIB = "lib/ultralib/include"  # forward slashes: asm-processor quotes a backslashed path for cmd.exe
CFLAGS = ["-c", "-G", "0", "-non_shared", "-Xfullwarn", "-Xcpluscomm", "-signed", "-nostdinc",
          "-mips2", "-O2", "-Olimit", "3000", "-32", "-Wab,-r4300_mul", "-I", "include", "-I", ".",
          "-I", ULTRALIB, "-I", ULTRALIB + "/compiler/ido", "-I", ULTRALIB + "/PR",
          "-D_MIPS_SZLONG=32", "-DBUILD_VERSION=VERSION_I", "-DF3DEX_GBI_2", "-D_FINALROM",
          "-DNDEBUG"] + os.environ.get("IDO_EXTRA", "").split()


# (Windows) devkitPro's MSYS2, then MSYS2's own default install: the first with an msys-2.0.dll
MSYS_RUNTIMES = ("C:/devkitPro/msys2/usr/bin", "C:/msys64/usr/bin")
MSYS_RUNTIME = os.environ.get("MSYS_RUNTIME") or next(
    (d for d in MSYS_RUNTIMES if os.path.isfile(os.path.join(d, "msys-2.0.dll"))), MSYS_RUNTIMES[0])
ENV = dict(os.environ)
if os.name == "nt" and os.path.isfile(os.path.join(MSYS_RUNTIME, "msys-2.0.dll")):
    ENV["PATH"] = MSYS_RUNTIME + os.pathsep + ENV.get("PATH", "")


def compile_c(src, obj, extra=()):
    """One C file through asm-processor and IDO. match_function.py compiles through this too, and
    canon-decompilation-tools/build_zlib.py (with `extra` flags). The game's strings are EUC-JP; splat writes them into
    the .s files as UTF-8, so asm-processor converts them back."""
    return [sys.executable, ASM_PROCESSOR, "--input-enc", "utf-8", "--output-enc", "euc-jp",
            IDO, "--", AS, *AS_FLAGS, "--", *CFLAGS, *extra, "-o", obj, src]


TIMES = {}  # step kind -> [seconds, count], printed at the end of a build
TIMES_LOCK = threading.Lock()  # run() is called from parallel()'s threads
JOBS = int(os.environ.get("BUILD_JOBS", os.cpu_count() or 1))


def run(cmd):
    t = time.time()
    r = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True, env=ENV)
    # the kind: a script's name and mode (asset_build.py build), else the program's name
    kind = os.path.basename(cmd[1]) + (" " + cmd[2] if len(cmd) > 2 and not cmd[2].startswith("-") else "") \
        if os.path.basename(cmd[0]).lower().startswith("python") else os.path.basename(cmd[0])
    if any("asm_processor" in c or "asm-processor" in c for c in cmd):
        kind = "compile C"
    with TIMES_LOCK:
        k = TIMES.setdefault(kind, [0.0, 0])
        k[0] += time.time() - t
        k[1] += 1
    if r.returncode:
        sys.exit(f"FAILED: {' '.join(cmd)}\n{r.stdout}{r.stderr}")
    return r


def parallel(jobs):
    """Each job (a function of no arguments) in a pool of JOBS threads; the work is in the child
    processes they start, so threads are enough. Waits for all of them; a job that fails (run()'s
    sys.exit) stops the build with its message once the running ones are done. Returns their
    results in the jobs' order."""
    if len(jobs) <= 1 or JOBS <= 1:
        return [j() for j in jobs]
    with concurrent.futures.ThreadPoolExecutor(max_workers=JOBS) as ex:
        futures = [ex.submit(j) for j in jobs]
        return [f.result() for f in futures]


def print_times():
    print("times: " + ", ".join(f"{k} {v[0]:.0f}s" + (f" ({v[1]}x)" if v[1] > 1 else "")
                                for k, v in sorted(TIMES.items(), key=lambda kv: -kv[1][0])))


MTIMES = {}  # a source's mtime, read once per build (907): every C file lists every header


def stale(obj, sources):
    """Rebuild when the object is missing or older than any of its sources (incremental)."""
    out = os.path.join(ROOT, obj)
    if not os.path.exists(out):
        return True
    t = os.path.getmtime(out)
    for s in sources:
        if s not in MTIMES:
            p = os.path.join(ROOT, s)
            MTIMES[s] = os.path.getmtime(p) if os.path.exists(p) else None
        if MTIMES[s] is not None and MTIMES[s] > t:
            return True
    return False


HEADERS = []  # every header, walked once per build (907): it was walked once per C file


def c_sources(rel):
    """A C file's inputs: itself, every header, and every asm file it pulls in by GLOBAL_ASM."""
    with open(os.path.join(ROOT, rel + ".c"), encoding="utf-8") as f:
        text = f.read()
    asm = re.findall(r'GLOBAL_ASM\("([^"]+)"\)', text)
    asm += re.findall(r'\.incbin\s+"([^"]+)"', text)  # art its inline GLOBAL_ASM blocks pull in
    asm += re.findall(r'#include\s+"([^"]+\.inc\.c)"', text)  # generated C arrays it includes (logo.c)
    if not HEADERS:
        HEADERS.extend(os.path.relpath(os.path.join(d, n), ROOT) for top in ("include", ULTRALIB)
                       for d, _, ns in os.walk(os.path.join(ROOT, top)) for n in ns if n.endswith(".h"))
    return [rel + ".c", *asm, *HEADERS]


def assemble(rel, obj):
    """GNU as takes a .s file's bytes as they are, and splat writes the game's EUC-JP strings
    as UTF-8: a file with any is converted back, into build/, before it is assembled."""
    src = rel + ".s"
    with open(os.path.join(ROOT, src), "rb") as f:
        data = f.read()
    if not data.isascii():
        src = obj[:-len(".o")] + ".eucjp.s"
        with open(os.path.join(ROOT, src), "wb") as f:
            f.write(data.decode("utf-8").encode("euc-jp"))
    run([AS, *AS_FLAGS, "-o", obj, src])


def objects_from_ld():
    """The objects splat's linker script names: build/asm/*.o and build/assets/*.o."""
    with open(os.path.join(ROOT, f"{NAME}.ld")) as f:
        return sorted(set(re.findall(r"(build/[\w/.-]+\.o)\(", f.read())))


def remove_stale_copies(split_start):
    """After a split, a function's .s in both asm/matchings/ and asm/nonmatchings/: keep the copy
    this split wrote (the newer) and delete the other, so every function has one .s. Then delete
    every .s under asm/ the split did not write (older than split_start, a time.time())."""
    old = 0
    for dirpath, _, files in os.walk(os.path.join(ROOT, "asm")):
        for name in files:
            p = os.path.join(dirpath, name)
            if name.endswith(".s") and os.path.getmtime(p) < split_start - 2:
                os.remove(p)
                old += 1
    print(f"build_rom: removed {old} .s files this split did not write (old names, old folders)")
    mdir, ndir = os.path.join(ROOT, "asm", "matchings"), os.path.join(ROOT, "asm", "nonmatchings")
    removed = 0
    for dirpath, _, files in os.walk(mdir):
        for name in files:
            if not name.endswith(".s"):
                continue
            m = os.path.join(dirpath, name)
            n = os.path.join(ndir, os.path.relpath(m, mdir))
            if os.path.isfile(n):
                os.remove(n if os.path.getmtime(n) < os.path.getmtime(m) else m)
                removed += 1
    print(f"build_rom: removed {removed} stale .s copies (a function in both asm/matchings and asm/nonmatchings)")


def main():
    if "--setup" in sys.argv:
        return setup()
    import build_libultra
    missing = [p for p in (AS, LD, OBJCOPY, IDO, build_libultra.IDO53, ASM_PROCESSOR) if not os.path.isfile(p)]
    if missing:
        sys.exit("FAILED: missing " + ", ".join(missing) + "\nRun `python canon-decompilation-tools/"
                 "build_rom.py --setup` first (or set BINUTILS, IDO, IDO53, ASM_PROCESSOR).")
    if "--clean" in sys.argv and os.path.isdir(os.path.join(ROOT, "build")):
        shutil.rmtree(os.path.join(ROOT, "build"))
    # The linker script is splat's, made from the yaml, symbol_addrs.txt and reloc_addrs.txt. One older than any of
    # them (an update that changed one) lists the old segments or symbols, and the build looks for C files that are
    # no longer there or comes out different. Split again then, as a fresh clone does.
    ld = os.path.join(ROOT, f"{NAME}.ld")
    splat_inputs = [f"{NAME}.yaml", "symbol_addrs.txt", "reloc_addrs.txt"]  # what splat reads (the yaml's paths)
    newer = [n for n in splat_inputs if os.path.isfile(ld) and os.path.isfile(os.path.join(ROOT, n))
             and os.path.getmtime(os.path.join(ROOT, n)) > os.path.getmtime(ld)]
    ld_stale = not os.path.isfile(ld) or bool(newer)
    if ld_stale and os.path.isdir(os.path.join(ROOT, "asm")) and "--split" not in sys.argv:
        why = f"{', '.join(newer)} newer than {NAME}.ld" if newer else f"no {NAME}.ld"
        print(f"build_rom: {why}: splitting again")
    if "--split" in sys.argv or not os.path.isdir(os.path.join(ROOT, "asm")) or ld_stale:
        run([sys.executable, os.path.join(HERE, "nisitenma_archive.py"), "unpack"])
        split_start = time.time()
        run([sys.executable, "-m", "splat", "split", f"{NAME}.yaml"])
        remove_stale_copies(split_start)
        # the assets in real formats (PNG, C, JSON, AIFC, text), from the user's own ROM
        run([sys.executable, os.path.join(HERE, "asset_build.py"), "extract"])
    # the archive and audio bins, made from those real formats
    run([sys.executable, os.path.join(HERE, "asset_build.py"), "build"])
    jobs = []  # each stale object's step, run side by side (906)
    for obj in objects_from_ld():
        out = os.path.join(ROOT, obj)
        os.makedirs(os.path.dirname(out), exist_ok=True)
        rel = obj[len("build/"):-len(".o")]
        if rel.startswith("asm/"):
            if stale(obj, [rel + ".s", "include/macro.inc"]):
                jobs.append(lambda rel=rel, obj=obj: assemble(rel, obj))
        elif rel.startswith("src/") and os.path.exists(os.path.join(ROOT, rel + ".s")):
            # the original's hand-written assembly, kept as source (splat's hasm in src/)
            if stale(obj, [rel + ".s", "include/macro.inc"]):
                jobs.append(lambda rel=rel, obj=obj: assemble(rel, obj))
        elif rel.startswith("src/"):
            if stale(obj, c_sources(rel)):
                jobs.append(lambda rel=rel, obj=obj: run(compile_c(rel + ".c", obj)))
        elif rel.startswith("assets/"):
            if stale(obj, [rel + ".bin"]):
                jobs.append(lambda rel=rel, obj=obj: run([OBJCOPY, "-I", "binary", "-O", "elf32-bigmips", "-B",
                                                          "mips", rel + ".bin", obj]))
        else:
            sys.exit(f"FAILED: no rule for {obj}")
    t = time.time()
    parallel(jobs)
    TIMES["objects (wall)"] = [time.time() - t, len(jobs)]
    import build_libultra as ultralib  # libultra's objects from lib/ultralib's source
    import build_zlib as libz  # zlib 1.1.3's inflate objects from lib/zlib's source
    elf, dec, rom = f"build/{NAME}.elf", f"build/{NAME}.decompressed.z64", f"build/{NAME}.us.z64"
    run([LD, "-m", "elf32ebmip", "--no-check-sections", "-Map", f"build/{NAME}.map",
         "-T", "undefined_syms.txt",
         "-T", "undefined_syms_auto.txt", "-T", "undefined_funcs_auto.txt", "-T", f"{NAME}.ld",
         *ultralib.link_inputs(), *libz.link_inputs(), "-o", elf])
    run([OBJCOPY, "-O", "binary", elf, dec])
    with open(os.path.join(ROOT, f"{NAME}.yaml")) as f:
        want_dec = re.search(r"^sha1:\s*([0-9a-f]{40})", f.read(), re.M).group(1)
    ok = compare(dec, want_dec, "baserom.us.decompressed.z64")
    run([sys.executable, os.path.join(HERE, "nisitenma_archive.py"), "pack", dec, rom])
    with open(os.path.join(ROOT, f"{NAME}.us.sha1")) as f:
        want = f.read().split()[0]
    ok = compare(rom, want, "baserom.us.z64") and ok
    print_times()
    return 0 if ok else 1


def compare(path, want, reference):
    with open(os.path.join(ROOT, path), "rb") as f:
        data = f.read()
    got = hashlib.sha1(data).hexdigest()
    if got == want:
        print(f"{path}: {len(data)} bytes, sha1 {got}  MATCH")
        return True
    ref = open(os.path.join(ROOT, reference), "rb").read()
    first = next((i for i in range(min(len(ref), len(data))) if ref[i] != data[i]), None)
    print(f"{path}: sha1 {got}  DIFFERS from {want}: "
          + (f"first differing byte at 0x{first:X}" if first is not None
             else f"sizes {len(data)} vs {len(ref)}"))
    return False


# --- --setup: the toolchain, each tool at a pinned version, into tools/ ------------------------
# A fresh clone builds after --setup, as sm64's does.
# An archive is checked by its sha256 and a clone by its commit, and nothing may
# ask for input: every git runs with no credential helper, GIT_TERMINAL_PROMPT=0 and GCM_INTERACTIVE=never
# (a wrong repository name once opened a sign-in window).

IDO_RELEASE = "https://github.com/decompals/ido-static-recomp/releases/download/v1.2/ido-{}-recomp-{}.tar.gz"
IDO_RELEASE_SHA256 = {
    ("5.3", "windows"): "2268f64cab973750e78bdf7aa2ff3cccf78ad1d181507da6faeb61f3d22c2e1e",
    ("5.3", "linux"): "ab5c741561f80913d58c8b074771f23941a3edd312505a8ebed6d1dfeb65e506",
    ("5.3", "linux-arm"): "b6844c986fa65d7de4ce10b27583769a5ec7a88ae37a151f6dc66c929d3e1c92",
    ("5.3", "macos"): "5b1ca006ee4b158ffba0422fc3f00b9b330f9036f279bae2ea1b4703317ad9c0",
    ("7.1", "linux"): "0d411696e178fcca34c31c3bf02011b928d7fd9c1fa7f8bf45070e0781b58e15",
    ("7.1", "linux-arm"): "d74443a65b1e7c92c7853c60a80de97b40c7b374eb0565d67b754d86bbf5f123",
    ("7.1", "macos"): "7f75570ed1ca14e6161b84b10743d440c685f27291bfbfaf87c6a82236553bc6",
}
# Windows 7.1: every release (v1.0, v1.1, v1.2) stops at startup on memory_allocate's assert
# ("start == TRUNC_PAGE(start)", libc_impl.c line 247), whichever msys-2.0.dll is loaded. It is
# built from source with the two __CYGWIN__ hunks of patches/ido-static-recomp-msys2.patch.
IDO_SOURCE = ("https://github.com/decompals/ido-static-recomp.git", "d5aec59932034b8a20f23f471297a55c45dcbc45")
IDO_PATCH = os.path.join(HERE, "patches", "ido-static-recomp-msys2.patch")
ASM_PROCESSOR_SOURCE = ("https://github.com/simonlindholm/asm-processor.git",
                        "f3b2f85a5bc7e8ae2030ea5a83ea8aa0bcbede03")
# GNU binutils for Windows: libdragon's toolchain zip. It is a prerelease libdragon replaces in
# place, so its sha256 is the pin and a replaced archive is refused.
BINUTILS_ZIP = ("https://github.com/DragonMinded/libdragon/releases/download/toolchain-continuous-prerelease/"
                "gcc-toolchain-mips64-win64.zip", "bd201c35afee4aceb9d2a80c6dc9d25eb2a4d426b1b26ef38e33881641b4d1d0")
NO_PROMPT = {"GIT_TERMINAL_PROMPT": "0", "GCM_INTERACTIVE": "never"}


def host():
    """This machine, as ido-static-recomp's releases name it."""
    if os.name == "nt":
        return "windows"
    if sys.platform == "darwin":
        return "macos"
    import platform
    return "linux-arm" if platform.machine().lower() in ("aarch64", "arm64") else "linux"


def setup_run(cmd, cwd=None, data=None, env=None):
    """One setup step: nothing asks for input; on Windows below-normal priority and no console window."""
    flags = 0
    if os.name == "nt":
        flags = subprocess.BELOW_NORMAL_PRIORITY_CLASS | subprocess.CREATE_NO_WINDOW
    r = subprocess.run(cmd, cwd=cwd, input=data, capture_output=True, creationflags=flags,
                       env={**(env or os.environ), **NO_PROMPT})
    if r.returncode:
        out = (r.stdout + r.stderr).decode("utf-8", "replace")
        sys.exit(f"setup FAILED: {' '.join(cmd)}\n{out[-3000:]}")
    return r


def git(*args, cwd=None, data=None, lf=True):
    """git with no credential helper; lf: check out with the commit's LF line ends (a tool's
    Makefile and scripts)."""
    return setup_run(["git", "-c", "credential.helper=", *(LF if lf else ()), *args], cwd=cwd, data=data)


def rmtree(path):
    """shutil.rmtree, also through git's read-only object files on Windows."""
    def unlock(func, p, _):
        os.chmod(p, 0o700)
        func(p)
    if os.path.isdir(path):
        shutil.rmtree(path, **({"onexc": unlock} if sys.version_info >= (3, 12) else {"onerror": unlock}))


def pinned(dest, pin):
    """dest holds pin's version: setup writes dest/.pin last, after a good install."""
    p = os.path.join(dest, ".pin")
    return os.path.isfile(p) and open(p).read().strip() == pin


def mark(dest, pin):
    with open(os.path.join(dest, ".pin"), "w") as f:
        f.write(pin + "\n")


def file_sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for block in iter(lambda: f.read(1 << 20), b""):
            h.update(block)
    return h.hexdigest()


def fetch(url, sha256, hint=""):
    """The archive at url, in tools/downloads/, checked against its pinned sha256."""
    import urllib.request
    d = os.path.join(TOOLS, "downloads")
    os.makedirs(d, exist_ok=True)
    path = os.path.join(d, url.rsplit("/", 1)[1])
    if not (os.path.isfile(path) and file_sha256(path) == sha256):
        print(f"setup: downloading {url}", flush=True)
        with urllib.request.urlopen(url, timeout=300) as r, open(path + ".part", "wb") as f:
            shutil.copyfileobj(r, f, 1 << 20)
        os.replace(path + ".part", path)
        got = file_sha256(path)
        if got != sha256:
            sys.exit(f"setup FAILED: {url}\n  sha256 {got}\n  pinned {sha256}\n{hint}")
    return path


def checkout(source, dest):
    """A clean checkout of one commit (fetched alone, by its hash) into dest."""
    url, commit = source
    rmtree(dest)
    os.makedirs(dest)
    git("init", "-q", cwd=dest)
    git("fetch", "-q", "--depth", "1", url, commit, cwd=dest)
    git("checkout", "-q", "FETCH_HEAD", cwd=dest)


def unpack(archive, dest):
    """A release archive (.tar.gz or .zip) extracted into an emptied dest."""
    import tarfile
    import zipfile
    rmtree(dest)
    os.makedirs(dest)
    if archive.endswith(".zip"):
        with zipfile.ZipFile(archive) as z:
            z.extractall(dest)
    else:
        with tarfile.open(archive) as t:
            t.extractall(dest, **({"filter": "data"} if hasattr(tarfile, "data_filter") else {}))


SUBMODULES = ("lib/ultralib", "lib/zlib")
LF = ("-c", "core.autocrlf=false", "-c", "core.eol=lf")
CLONE = "git clone -c core.autocrlf=false -c core.eol=lf <repository> Goemons-Great-Adventure-Decomp"


def converted(repo):
    """The files git checked out with CRLF line ends where the commit has LF (`ls-files --eol`).
    Git for Windows does so by default (core.autocrlf=true), and IDO's cfe reads a `\\` line
    continuation before CRLF as an error (IDO_NOTES 181)."""
    r = git("ls-files", "--eol", cwd=repo, lf=False)
    return [line.split("\t", 1)[1] for line in r.stdout.decode("utf-8", "replace").splitlines()
            if "\t" in line and line.split()[:2] == ["i/lf", "w/crlf"]]


def setup_sources():
    """This checkout and the lib/ submodules (libultra and zlib from their own sources), all with
    the commits' LF line ends."""
    if not os.path.exists(os.path.join(ROOT, ".git")):
        sys.exit(f"setup FAILED: not a git checkout. Clone the repository with:\n  {CLONE}")
    crlf = converted(ROOT)
    if crlf:
        sys.exit(f"setup FAILED: git checked out {len(crlf)} files with CRLF line ends (e.g. {crlf[0]}): "
                 "core.autocrlf=true, Git for Windows' default. IDO cannot read them. Clone again with:\n"
                 f"  {CLONE}")
    for sub in SUBMODULES:  # a submodule checked out with CRLF and unmodified: check it out again
        path = os.path.join(ROOT, sub)
        if os.path.exists(os.path.join(path, ".git")) and converted(path):
            if git("status", "--porcelain", cwd=path, lf=False).stdout.strip():
                sys.exit(f"setup FAILED: {sub} has CRLF line ends and local changes; IDO cannot read it")
            git("submodule", "deinit", "-q", "-f", "--", sub, cwd=ROOT, lf=False)
    setup_run(["git", "-c", "credential.helper=", *LF, "submodule", "update", "--init"], cwd=ROOT)
    for sub in SUBMODULES:
        path = os.path.join(ROOT, sub)
        git("config", "core.autocrlf", "false", cwd=path, lf=False)
        git("config", "core.eol", "lf", cwd=path, lf=False)
        if converted(path):
            sys.exit(f"setup FAILED: {sub} is still checked out with CRLF line ends")
    for need in ("lib/ultralib/include/ultra64.h", "lib/zlib/inflate.c"):
        if not os.path.isfile(os.path.join(ROOT, need)):
            sys.exit(f"setup FAILED: {need} is missing after `git submodule update --init`")
    print("setup: this checkout, lib/ultralib and lib/zlib have LF line ends", flush=True)


def setup_ido71_windows(dest, pin):
    """IDO 7.1 built from ido-static-recomp's source with MSYS2's gcc, g++ and make:
    `make setup` first, then VERSION=7.1, RELEASE=1 (the releases' -Os
    and strip); bash run without -l and OS=Windows_NT set inside it."""
    need = [t for t in ("bash", "make", "gcc", "g++") if not os.path.isfile(os.path.join(MSYS_RUNTIME, t + ".exe"))]
    if need:
        sys.exit("setup FAILED: on Windows IDO 7.1 is built from source (every Windows 7.1 release stops "
                 f"at startup), with MSYS2. {MSYS_RUNTIME} lacks {', '.join(need)}.\n"
                 "Install MSYS2 (https://www.msys2.org) and run `pacman -S --needed make gcc` in its "
                 "shell, or set MSYS_RUNTIME to an MSYS2 usr/bin that has them.")
    import shlex
    src = os.path.join(TOOLS, "src", "ido-static-recomp")
    checkout(IDO_SOURCE, src)
    with open(IDO_PATCH, "rb") as f:
        git("apply", "-", cwd=src, data=f.read().replace(b"\r\n", b"\n"))
    print("setup: building IDO 7.1 from source with MSYS2 (several minutes)", flush=True)
    log = os.path.join(TOOLS, "src", "ido-static-recomp.log")
    script = (f'export OS=Windows_NT PATH="/usr/bin:$PATH" && cd "$(cygpath -u {shlex.quote(tool_path(src))})" '
              "&& make setup RELEASE=1 && make VERSION=7.1 RELEASE=1 -j2")
    with open(log, "wb") as f:
        r = subprocess.run([os.path.join(MSYS_RUNTIME, "bash.exe"), "-c", script], stdout=f,
                           stderr=subprocess.STDOUT, env={**ENV, **NO_PROMPT},
                           creationflags=subprocess.BELOW_NORMAL_PRIORITY_CLASS | subprocess.CREATE_NO_WINDOW)
    out = os.path.join(src, "build", "7.1", "out")
    if r.returncode or not os.path.isfile(os.path.join(out, "cc.exe")):
        with open(log, encoding="utf-8", errors="replace") as f:
            sys.exit(f"setup FAILED: building IDO 7.1 (log: {log})\n{f.read()[-3000:]}")
    rmtree(dest)
    shutil.copytree(out, dest)
    mark(dest, pin)


def setup_smoke():
    """Both IDOs compile a 40,040-byte file. With Git for Windows' msys-2.0.dll first on PATH, cfe
    stops after about 32 KB of preprocessed text; every file including
    ultra64.h is longer."""
    import build_libultra
    d = os.path.join(TOOLS, "smoke")
    os.makedirs(d, exist_ok=True)
    src, obj = os.path.join(d, "big.c"), os.path.join(d, "big.o")
    with open(src, "w", newline="\n") as f:
        f.write("".join(f"int var{i:05d};\n" for i in range(2860)))
    for ido in (IDO, build_libultra.IDO53):
        if os.path.exists(obj):
            os.remove(obj)
        r = subprocess.run([ido, "-c", "-G", "0", "-non_shared", "-mips2", "-O2", "-32", tool_path(src),
                            "-o", tool_path(obj)], cwd=d, env=ENV, capture_output=True, text=True)
        if r.returncode or not os.path.isfile(obj):
            sys.exit(f"setup FAILED: {ido} cannot compile a 40,040-byte file\n{r.stdout}{r.stderr}\n"
                     "On Windows, MSYS_RUNTIME must hold an MSYS2 msys-2.0.dll (devkitPro's or "
                     "MSYS2's own): Git for Windows' stops cfe at about 32 KB.")
        print(f"setup: {ido} compiles a 40,040-byte file", flush=True)


def setup():
    os.makedirs(TOOLS, exist_ok=True)
    with open(os.path.join(TOOLS, ".gitignore"), "w") as f:
        f.write("# Fetched by canon-decompilation-tools/build_rom.py --setup; never committed\n*\n")
    plat = host()
    setup_sources()
    # asm-processor
    dest = os.path.join(TOOLS, "asm-processor")
    if not pinned(dest, ASM_PROCESSOR_SOURCE[1]):
        checkout(ASM_PROCESSOR_SOURCE, dest)
        mark(dest, ASM_PROCESSOR_SOURCE[1])
    print(f"setup: asm-processor {ASM_PROCESSOR_SOURCE[1][:7]}", flush=True)
    # IDO 5.3 (libultra), and 7.1 (the game) off Windows: the release
    for version in ("5.3", "7.1"):
        dest = os.path.join(TOOLS, f"ido-{version}")
        if version == "7.1" and plat == "windows":
            with open(IDO_PATCH, "rb") as f:
                pin = IDO_SOURCE[1] + "+" + hashlib.sha256(f.read().replace(b"\r\n", b"\n")).hexdigest()
            if not pinned(dest, pin):
                setup_ido71_windows(dest, pin)
            print(f"setup: IDO 7.1 built from ido-static-recomp {IDO_SOURCE[1][:7]} + "
                  f"{os.path.basename(IDO_PATCH)}", flush=True)
            continue
        sha = IDO_RELEASE_SHA256.get((version, plat))
        if sha is None:
            sys.exit(f"setup FAILED: ido-static-recomp has no {version} release for {plat}")
        if not pinned(dest, sha):
            archive = fetch(IDO_RELEASE.format(version, plat), sha)
            unpack(archive, dest)
            os.remove(archive)
            mark(dest, sha)
        print(f"setup: IDO {version} from ido-static-recomp v1.2 ({plat})", flush=True)
    # GNU binutils for mips: libdragon's zip on Windows; elsewhere a mips64-elf binutils on PATH
    if plat == "windows":
        dest = os.path.join(TOOLS, "mips64")
        if not pinned(dest, BINUTILS_ZIP[1]):
            archive = fetch(*BINUTILS_ZIP, hint=(
                "libdragon has replaced its toolchain prerelease. Set BINUTILS to a folder with GNU "
                "binutils for mips (mips64-elf-as, -ld, -objcopy, -ar), or pin the new archive in "
                "BINUTILS_ZIP once a build with it gives both hashes."))
            unpack(archive, dest)
            os.remove(archive)
            mark(dest, BINUTILS_ZIP[1])
    if not os.path.isfile(AS):
        sys.exit(f"setup FAILED: no {AS}. Install a mips64-elf binutils (Arch: mips64-elf-binutils "
                 "from the AUR; macOS: brew install tehzz/n64-dev/mips64-elf-binutils) or set BINUTILS.")
    version = subprocess.run([AS, "--version"], capture_output=True, text=True).stdout.splitlines()
    print(f"setup: {AS}: {version[0] if version else '?'}", flush=True)
    setup_smoke()
    print("setup: done. Build with: python canon-decompilation-tools/build_rom.py")
    return 0


if __name__ == "__main__":
    sys.exit(main())
