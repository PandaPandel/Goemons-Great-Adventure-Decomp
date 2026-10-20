# Canon decompilation tools

These are the tools written for, and used on, this decompilation of Goemon's Great Adventure
(N64, USA). It was the first game decompiled this way. They are kept together here as the canon
set: the next game extends them in place, one tool per job, with no copies and no `tool2`.

Everything else the build uses is someone else's work, used as published: splat, IDO
(ido-static-recomp), asm-processor, GNU binutils, decomp-permuter, m2c, asm-differ, n64sym, and
libultra and zlib from their own sources. `CREDITS` names their authors.

## Building the ROM

The build turns the split sources back into the game's exact ROM and checks both SHA-1s. It
needs your own copy of the game.

### Prerequisites

- Python 3 (the matching build was made with 3.13) and Git.
- **Windows** (where this decomp was built and proven): MSYS2, for IDO 7.1.
  - Every Windows release of IDO 7.1 stops at startup, so setup builds it from source with
    MSYS2's gcc, g++ and make.
  - The IDO programs then run on MSYS2's `msys-2.0.dll`. With Git for Windows' copy of that
    DLL, IDO stops reading a file after about 32 KB.
  - Use either one:
    - devkitPro's MSYS2 at `C:/devkitPro/msys2`, as installed (the one proven here);
    - MSYS2 from https://www.msys2.org at `C:/msys64`, then `pacman -S --needed make gcc` in its
      shell.
  - An MSYS2 anywhere else: set `MSYS_RUNTIME` to its `usr/bin`.
- **macOS** (proven: both hashes, the first build took 7 minutes): IDO 5.3 and 7.1 come as
  release builds.
  - GNU binutils 2.45 for `mips64-elf`, built from GNU's source with the system zlib:
    `./configure --target=mips64-elf --with-system-zlib --disable-nls --disable-werror`, then
    `make` and `make install`. Put it on PATH, or name its `bin` folder in `BINUTILS`. The
    `tehzz/n64-dev` brew tap's 2.37, and 2.45 built with its own bundled zlib, both fail on the
    current macOS SDK.
  - Check the repository out to a path with **no spaces**. IDO's cc exits 0 and writes no object
    for a path with a space.
  - If `--setup`'s downloads fail on certificates (a Python without root certificates), set
    `SSL_CERT_FILE` to a certificate bundle, for example the one `certifi` installs.
  - Install the current Command Line Tools (`xcode-select --install`).
- **Linux** (not yet tried): IDO 5.3 and 7.1 come as release builds. You need GNU binutils for
  mips with the `mips64-elf-` prefix, on PATH or in a folder named by `BINUTILS` (Arch:
  `mips64-elf-binutils` from the AUR).

### The game

Put Goemon's Great Adventure (USA) in the repository root as `baserom.us.z64`. It must be the
big-endian `.z64` with sha1 `9f9e860816f9e2a68bb5f4f56086e0c7450c64d7`, the hash in
`goemonsgreatadv.us.sha1`. It is never committed (`.gitignore` lists ROM images).

### Setup, once

    git clone -c core.autocrlf=false -c core.eol=lf <this repository> Goemons-Great-Adventure-Decomp
    cd Goemons-Great-Adventure-Decomp
    python -m venv .venv
    .venv/Scripts/python -m pip install -r requirements.txt
    .venv/Scripts/python canon-decompilation-tools/build_rom.py --setup

On Linux and macOS the venv's Python is `.venv/bin/python`.

The two `-c` options keep the files' LF line ends. Git for Windows otherwise checks text out with
CRLF (its default `core.autocrlf=true`), and IDO's cfe reads a `\` line continuation before CRLF
as an error. Setup refuses such a checkout and prints this clone command. It checks the
submodules out with LF itself.

`--setup` fetches each tool at a pinned version into `tools/`. Setup writes `tools/.gitignore`,
so git ignores the folder. Every download and clone runs without prompting.

| Tool | From | Pinned by |
|---|---|---|
| IDO 5.3 (libultra) | decompals/ido-static-recomp release v1.2, `ido-5.3-recomp-<platform>.tar.gz` | sha256 |
| IDO 7.1 (the game) | Linux and macOS: the same release, `ido-7.1-recomp-<platform>.tar.gz`. Windows: built from decompals/ido-static-recomp's source plus `patches/ido-static-recomp-msys2.patch` | sha256; commit `d5aec59` |
| GNU binutils 2.45 for mips (Windows) | DragonMinded/libdragon's `gcc-toolchain-mips64-win64.zip` | sha256 `bd201c35…` |
| asm-processor | simonlindholm/asm-processor | commit `f3b2f85` |

- libdragon publishes that zip as a prerelease and replaces it in place. If it changes, setup
  refuses the new archive and says so; set `BINUTILS` to any GNU binutils for mips instead.
- Setup also checks out `lib/ultralib` and `lib/zlib` (`git submodule update --init`), with LF
  line ends.
- It ends by compiling a 40 KB file with both IDOs, which proves the MSYS runtime.
- A tool already at its pin is not fetched again. The pins are in `build_rom.py`.

### Build

    .venv/Scripts/python canon-decompilation-tools/build_rom.py

The first build splits the ROM first: the code with splat into `asm/`, the assets into `assets/`
in their real formats. It took 37 minutes on the machine it was proven on, 28 of them
compiling the 1,475 models back to bytes. Later builds remake only what changed. A good build
ends with both hashes:

    build/goemonsgreatadv.decompressed.z64: 27059246 bytes, sha1 a0db6785e180f41b47f340d12e13ceabcbcc292d  MATCH
    build/goemonsgreatadv.us.z64: 16777216 bytes, sha1 9f9e860816f9e2a68bb5f4f56086e0c7450c64d7  MATCH

The first split comes before any link, so the scenario scripts' calls (op 11) are written as
addresses. `build_rom.py --split` after a build writes them as function names.

## Running them

- Run every tool from the repository root, with the Python environment from `requirements.txt`:
  `python canon-decompilation-tools/<tool>.py <command> ...`
- The build's tools are in `tools/`, fetched by `build_rom.py --setup` (above). Environment
  variables can point elsewhere; `build_rom.py` lists them.
- The matching tools in `match_function.py` also use decomp-permuter, m2c and two instrumented
  IDO passes beside the repository, in `../vendor/`; `match_function.py` names each one.
- The tools import each other from this folder (`build_rom` holds the compile pipeline every
  other tool uses), so they move together.

| Tool | Job |
|---|---|
| `build_rom.py` | The matching build: sources to the ROM, checked against both SHA-1s |
| `match_function.py` | One function against the game: check, diagnose, search for the C that matches |
| `report_progress.py` | How much is in C, by functions and bytes, per segment |
| `build_libultra.py` | libultra compiled from its own source as Nintendo compiled it, mapped onto the ROM |
| `build_zlib.py` | zlib 1.1.3's inflate side compiled from upstream's source, mapped onto the ROM |
| `nisitenma_archive.py` | The game's file system: unpack the ROM, pack it back byte for byte (its Konami-LZ encoder too); the TLB code files' yaml, their rodata and their objects |
| `asset_build.py` | The assets: extracted at --split into assets/ in real formats, rebuilt by every build |
| `pic_codec.py` | The PIC images (X68000 PIC, both of the game's encoders) to PNG and back, byte for byte |
| `audio_codec.py` | Sound banks (JSON), samples (AIFC), songs, back to the audio region byte for byte |
| `scenario_codec.py` | The scenario scripts (dialogue and events) to text and back |
| `model_codec.py` | Scenery models to C (Vtx and gbi.h display lists), compiled with the game's IDO, back |
| `table_codec.py` | The table files (file sets, enemy kinds, stage list, stage data, backgrounds) to C and back |
| `texture_codec.py` | The font, BG tile pictures, UI and glyph sheets to PNG (maps as text) and back |
| `survey_rom.py` | A ROM's static facts: header, compiler, library names, file system |
| `ida_dump_funcs.py` | Headless IDA dumps (MIPS and PowerPC databases) |
| `patches/` | A fix to decomp-permuter that this C needs, and the fix that lets IDO 7.1 run on Windows |

## build_rom.py

The whole build, from the split sources to the shipped ROM.

    python canon-decompilation-tools/build_rom.py           assemble, compile, link, pack, check both SHA-1s
    python canon-decompilation-tools/build_rom.py --split   unpack the baserom and run splat first
    python canon-decompilation-tools/build_rom.py --clean   remove build/ first
    python canon-decompilation-tools/build_rom.py --setup   fetch the toolchain into tools/ (see Building the ROM)

- The linker makes the **decompressed** ROM, which is checked against the yaml's SHA-1. Then
  `nisitenma_archive.py` packs its files back into the shipped ROM, which is checked against
  `goemonsgreatadv.us.sha1`. "Both hashes match" means both.
- The game's C is compiled through asm-processor with IDO 7.1, `-O2 -mips2`. A function not yet
  in C stays `#pragma GLOBAL_ASM` and assembles as the game's own instructions.
- libultra comes from `build_libultra.py` (IDO 5.3) and zlib from `build_zlib.py`.
- Toolchain paths are set by `BINUTILS`, `IDO`, `IDO53` and `ASM_PROCESSOR`; each defaults to
  what `--setup` put in `tools/`, and a build with one missing says to run `--setup`. `IDO_EXTRA` adds
  IDO flags for one measurement only, never for the build. `MSYS_RUNTIME` (Windows) points at a
  good msys-2.0.dll: with Git for Windows' copy, IDO's cfe stops after about 32 KB of
  preprocessed text.
- `compile_one()` is the one compile every other tool calls, so a function is always measured
  exactly as the build compiles it.

## match_function.py

The matcher: everything done to one function between "it is in C" and "it matches". It was
first written as glue for decomp-permuter and grew one mode at a time. Every search mode
compiles each variant the way the build would, and keeps nothing on its own: it lists what got
closer. Only `greedy` writes, and only into `src/` when it ends better.

### The measure

    python canon-decompilation-tools/match_function.py check src/FILE.c FUNC [DIR]

- `check` compiles the function and diffs it alone against the game's instructions, with
  relocations resolved. It is not thrown off when an earlier function in the file changes size.
- It prints the differing lines and one of three verdicts: `MATCH`, `MATCH except own section
  addresses` (only the function's own .rodata/.data/.bss references differ, which the link
  decides), or `N differing lines (L lines); this C L2, mix distance M`.
- The **mix distance** M is the sum, over opcodes, of the difference in counts. M = 0 means the
  instructions are the game's and only registers, stack slots or schedule differ. A change that
  lowers the lines but raises M is usually the diff aligning, not progress.
- When the frames differ, a `stack:` line maps every stack slot the game touches against this
  C's.
- The keep rule used throughout: keep a change when the lines drop and the mix does not rise.

### Diagnostics (read-only)

| Command | What it tells you |
|---|---|
| `checkall OUT` | Every unmatched function, closest first (its own work directory, safe beside other runs) |
| `screen` | After checkall: what register allocation cannot change (length, operation counts, branch order, call order, frame, saved registers), so a function that agrees on all of it differs only in registers and schedule |
| `twins` | Unmatched functions whose instructions equal a matched one's: start from its C |
| `protos` | Prototypes that disagree with their C definitions (a wrong one moves argument registers) |
| `calls` | Where the game calls through a register (jalr) and the C does not: write `(*func)(...)` |
| `bases` | `%hi` addresses on one side only: a symbol at its own address, or a loop IDO unrolled |
| `unroll [FILE FUNC]` | The function's size at each uopt unroll limit: how much lighter or heavier a loop must be |
| `dprintf [FILE FUNC]` | The function checked with its debug prints written as the game's own print form |
| `drops FILE FUNC` | Each statement removed in turn: which one makes a compiler temporary the game lacks |
| `reloads [FILE FUNC]` | A dead test after each store: an invisible temporary the game spends |

### Search modes

Each takes `src/FILE.c FUNC` and lists the variants that compiled closer. With no arguments it
sweeps every unmatched function. All but `casts` and `swaps` keep what the C computes.

| Mode | What it varies |
|---|---|
| `pads` | Declarations: a pad of 1-4 words at each position, each pad removed, each pair swapped (frame slots follow declaration order) |
| `splits` | A FAKE `do {} while (0);` block boundary after each statement, alone and as a stack of 8 (`SPLIT_STACKS`). One boundary can decide which pointer gets a saved register; a stack lowers the priority of a value live only there |
| `empties` | A FAKE `if (v) {}` of each variable at each statement after it is set (keeps it live) |
| `unfakes` | Each FAKE taken out in turn: an old FAKE can block the next step |
| `joins` | Two adjacent statements on one line, or a one-statement block closed onto its line (IDO schedules by source line) |
| `moves` | Each one-line statement moved up or down, up to 6 lines, past statements it provably commutes with (where a value is computed decides its registers); a dead `PRINTF` moves past anything |
| `compounds` | Each `x = x op y;` written `x op= y;` and back (the two forms colour registers differently) |
| `scopes` | Each local declared in a block around all its uses instead of at the top (block-scoped locals take the frame words just above the compiler temps) |
| `copies` | A parameter read through a copy from each statement on (the game's saved register for it) |
| `inlines` | A local's straight-line definition substituted into its uses (fewer names move the temporaries) |
| `rechains` | Chained assignments reordered or split, adjacent equal stores chained |
| `commutes` | Each one-operator expression's operands swapped |
| `widens` | Each narrow local as s32, then u32 |
| `volatiles` | Each local declared volatile (a variable the game stores and reloads at once) |
| `reprotos` | Each disagreeing callee given its definition's prototype |
| `casts` | Each assignment's right side cast to u8/s8/u16/s16 (can change the C: read each find) |
| `swaps` | Each adjacent pair of statements swapped (can change the C: read each find) |

### greedy

    python canon-decompilation-tools/match_function.py greedy src/FILE.c FUNC
    python canon-decompilation-tools/match_function.py greedy START TAG     (a pass over everything)

- It stacks the modes that keep what the C computes, unattended, on a copy of the file
  (`permuter/greedy/`). Each round runs the modes in turn and keeps the first variant that is
  better: fewer lines with the mix not rising, or as many lines with a lower mix. No source is
  tried twice.
- When a round keeps nothing, it **escapes**. Of the variants that round compiled, it takes the
  ones closest to the game, ranked by mix distance, then stack distance, then t-register shift,
  even if their lines rise. It descends from each of the best `GREEDY_ESCAPES` (default 3), and
  keeps a path only if it ends lower.
- At the end, if it kept anything and the real file was not edited meanwhile, it writes the
  result back into `src/` (FAKE lines carry a FAKE comment). Read the diff, build to both
  hashes, then commit.
- A pass over every function claims each one in `permuter/greedy_doneTAG.txt` before starting
  it. Write a function's name there first to keep the pass off it.

### decomp-permuter glue

| Command | What it does |
|---|---|
| `import src/FILE.c FUNC [DIR]` | Sets up `permuter/FUNC/` (base.c preprocessed by IDO's own cc -E and pruned, target.o, compile.sh, settings.toml) |
| `basecheck FUNC [DIR]` | Whether base.c compiles to the same code as the real file; a base that does not is a different function to permute |
| `compile IN.c -o OUT.o` | What compile.sh runs: one file through the build's compile |
| `best` | Every permuter run's best find so far, with its changed lines |
| `unfakes [FILE FUNC] apply` | Matched functions too: each FAKE that can be taken out mechanically goes when the code stays identical row for row (never a line with a string: a print keeps its string in .rodata) |
| `rename MAP [dry]` | Address names to real ones (`func_X_seg name` per line, or a JSONL results journal of name maps): refuses duplicates and names already defined, rewrites src/ and include/, pins each in symbol_addrs.txt with its segment. Then delete asm/ and run build_rom.py --split |
| `graft FROM.c TO.c FUNC` | One function and its note from a file (a working copy) into another |
| `adopt COPY DEST BASE` | A working copy put in place of DEST, only if DEST is unchanged since BASE (the commit the copy was made from); otherwise it refuses, and `graft` takes single functions. Work done on copies reaches `src/` this way, or by a tool |
| `twins all [--list]` | Every function still in assembly against every function's shape (IDA's instructions, from the export, with references abstracted; `NO_IDA=1` uses splat's `.s`): twins already in C, and same-shape clusters |
| `twin-copy [FUNC ...]` | Each function still in assembly with a same-shape twin in C gets that C. Mapped from the twin to the target: address-named symbols (IDA's names, in instruction order), strings (by `.rodata` index, one per `.asciz`), floats (by exact bits), declarations and typedefs it needs (only those before the insertion point). The twin's comments are not copied. Kept only on a MATCH from `check`, else the file is put back exactly; known failures are cached in `permuter/twincopy/failed.txt`. "MATCH except own section addresses" is provisional: the full build and `nisitenma_archive.py yaml-diffs` decide. A parked function is tried too: its whole `#ifdef NON_MATCHING` block gives way to the twin's C, and its note is listed for rewriting |
| `m2c-drafts [FILE ...] [--limit=N] [--min=S] [--max=S] [--apply]` | Each function still in assembly, smallest first, gets m2c's draft, checked in a copy of its file under `permuter/m2c/FUNC/`. `src/` never changes, because an `M2C_FIELD` draft is not readable C. A function whose asm and C file are unchanged is not drafted again (its `verdict.txt`). Two run at a time. `permuter/m2c/summary.tsv` holds every function's verdict. `--apply` writes a match into `src/` only when its draft is plain C |
| `twins files [--all]` | Each C file still holding assembly, set against the C file most like it. A function counts as "same" when its IDA mnemonics equal a matched function's there, and "near" at a 0.9 ratio. Such a file is best matched whole, starting from its twin |
| `folded-empties` | Empty functions splat folded into the function before them (`jr ra; nop` twice), written out as their own empty functions |
| `copyout SRC COPY` | A working copy, byte-exact, made by the tool (only under `permuter/`; never overwrites); it adds the pair to the run's `permuter/runs/RUN/manifest.txt` |
| `land RUN BASE` | A run of working copies brought in: every copy in the run's manifest adopted against BASE (refusals listed), the full build, each TLB file that differs put back (`yaml-diffs`) and the build again, then `twin-copy` with the same put-back. Prints what was adopted, reverted and copied, and both hashes. The commit stays a person's |
| `journal JOURNAL.jsonl OUTDIR` | A JSONL results journal written out readably, one file per result |

Matched functions' asm is in `asm/matchings/<file>/` (the yaml's `disassemble_all`), parked ones'
in `asm/nonmatchings/<file>/`; `check` and report_progress.py read both.

Then run decomp-permuter itself: `python ../vendor/decomp-permuter/permuter.py permuter/FUNC -j1
--stack-diffs`. Check every find for what the C computes: the permuter does not.

## report_progress.py

    python canon-decompilation-tools/report_progress.py

Functions and bytes in C against assembly, per segment, counted from the current linker script.
libultra and zlib objects built from source count as C.

**The total is checked, not trusted.** The report then reads every byte of the working ROM for
code (`survey_rom.py`'s `code_runs`), segment by segment and archive file by archive file:
- each counted segment's code read is set against its counted bytes;
- any code outside a counted segment is listed as NOT COUNTED, by place and archive file;
- the last line, "ALL THE CODE IN THE ROM", is out of everything read.

This exists because the count once came from the yaml alone. It said 99.5% while 3,168
functions in the archive's TLB files, which the yaml then kept as `bin`, were never counted.

The code read has three columns:
- **IDA code:** from each database's export (`ida_dump_funcs.py export:`): every IDA function
  from its first instruction that is not a nop, plus non-nop words outside any function.
  Alignment padding is not code. The final line uses the larger of this and the count. Each
  TLB file where IDA finds more code than the count is named. A shortfall is only nops, because
  splat's sizes keep some trailing padding.
- **rabbitizer:** `survey_rom.py`'s runs, printed beside IDA's as a check.
- **counted:** the table's own count.

Two more modes keep IDA's figure whole:

    python canon-decompilation-tools/report_progress.py --text-ranges
    python canon-decompilation-tools/report_progress.py --decode-gaps [main 0006 0123 ...]

- `--text-ranges` prints each database's code ranges in memory, from the yaml. It also lists the
  bytes in them that the export does not have as code, as `decode:A:B` arguments.
- `--decode-gaps` runs `ida_dump_funcs.py` on them, `decode:` and then `export:` again. It runs
  two databases at a time, at below-normal priority and with no console window.

`idat -B` leaves code undecoded when nothing calls it directly. Before the gaps were decoded,
overlay 8 showed 5,964 bytes of code against 9,152 counted.

Then come the lines for what fully done also needs:
- functions and data still named by address;
- data still in assembly;
- FAKE lines and parked functions;
- assets still raw.

Every tool here keeps its own `Changelog` block in its header, newest first. A tool's entry is
written there before any change to it. The original's hand-written
assembly is kept as source in `src/` (splat's `hasm` with `hasm_in_src_path`: `src/entry.s`, the
entry point, and `src/rspboot.s`, libultra's RSP boot microcode, which spimdisasm had read as two
"handwritten functions"); each `glabel` there counts as done, and the report names them.
`build_rom.py` assembles a `src/` object from its `.s` when there is one.

## build_libultra.py

    python canon-decompilation-tools/build_libultra.py build | map | yaml | diff ROM FILE | near ROM END

- It compiles libultra 2.0I from decompals/ultralib's source (`lib/ultralib`) with ultralib's
  own flags and IDO 5.3, each file in its own directory, made fit for GNU ld.
- `map` matches each object's .text against the game's code, `yaml` prints the library
  subsegments, `diff` shows where an object departs from the ROM, and `near` finds the closest
  objects for code nothing matches.
- `build_rom.py` links the matched objects.

## build_zlib.py

    python canon-decompilation-tools/build_zlib.py build | map | diff ROM FILE

- It compiles the inflate half of zlib 1.1.3 from upstream's source (`lib/zlib`, unedited) with
  the game's IDO 7.1 flags, less `-signed` (zlib's char is unsigned here).
- It is not called `zlib.py`: that would shadow Python's own zlib, which `nisitenma_archive.py`
  packs the ROM with.

## nisitenma_archive.py

    python canon-decompilation-tools/nisitenma_archive.py survey | unpack | pack DEC OUT | check

- The game's file system, named after its own magic, "Nisitenma-Ichigo": 2,576 files behind a
  table of ROM offsets in main's data.
- `unpack` writes the decompressed ROM that splat and the linker work on, plus `nisitenma.tsv`.
  `pack` turns it back into the exact shipped ROM (zlib level 9, memLevel 8 reproduces every
  stream; the one Konami-LZ file is carried over). `check` does the round trip.
- **The game-specific tool:** another game gets its own archive tool beside this one.
- The TLB code files (archive files 16-327) as C files:
  - `yaml FIRST LAST [--write|--count|--rodata]` writes their splat segments. A file built from
    several objects gets one C file per object.
  - `units FID` prints a file's cut into objects, and the yaml writer's verdict with its reason.
  - `rodata-objects FIRST LAST [--compare]` prints each file's .rodata object starts, from the
    ROM alone. Use it to check a change to the cutter against every file already split.
    `--compare` cuts each file twice, once from IDA's references and once from the old decoding,
    and prints only the files where the two disagree.
  - `pad-cuts FIRST LAST` finds the object boundaries a `.rodata` cut cannot see, from IDA's
    functions: a function, then only nops, then a 16-aligned function. IDO pads no function
    inside an object, so each one is a new object. It also lists cuts that have more padding
    before them than 16-alignment gives, which need a pad row. A function just before an uncut
    boundary fails the build in C, however right its C is.
  - `recut FID [--write]` cuts them. It adds a c row at each boundary, with `.data`/`.rodata` rows
    where the new object's IDA reads begin, and a pad row for extra padding, then renumbers the
    objects. `--write` also moves each function's C or pragma into its object's C file. Then
    delete `asm/` and re-split. File 322 became its six builder objects this way, and its five
    builders that were parked for padding were unparked.
  - The cutter reads each function's data references from IDA's export when there is one
    (`../ida/goemons_great_adventure/export/NNNN.json`, beside this repository). Without an export, it decodes the
    `lui`/`addiu`/load/store pairs itself. `refs-compare FID` prints where the two disagree, per
    instruction.
  - `force-migrate FID [--write]` pins each rodata block splat left outside its one reader to
    that function, as `function_owner` in `symbol_addrs.txt`. A block no function reads goes to
    the function whose rodata surrounds it, typed f32 when it reads as floats. Then re-split.
    This is what let the last 17 files become C files.

## survey_rom.py

    python canon-decompilation-tools/survey_rom.py header | compiler | libsig | filetable | unpack | recompress | textend | codemap ...

The first look at a ROM, before any split:
- `header`: the header fields, the libultra version and the hashes.
- `compiler`: compiler fingerprints of the main code. It is how this game was found to be IDO 7.1.
- `libsig`: libultra and libc names by signature (n64sym), checkable against a reference ROM.
- `filetable`, `unpack`, `recompress`: a file system from its tables, its files decompressed, and
  which zlib settings reproduce them.
- `textend`: where an IDO code file's .text ends.
- `codemap`: every run of code in the ROM, found by reading every word. With the file system's
  tables (`--offsets --ranges --count`), each file is read decompressed. Run it before the first
  split of a new game: its total is the denominator of every progress figure.
  - Great Adventure: 2,805,584 bytes of code. Only archive files 6, 8, 9, 10 and 16-327 hold
    code; the rest is the boot code and the RSP microcode.

Instructions are decoded by rabbitizer, never by hand.

## ida_dump_funcs.py

    IDB=<database.i64> python canon-decompilation-tools/ida_dump_funcs.py <mode> [<mode> ...]

- A headless IDA (idalib) dumper with a MIPS mode added for N64 databases (PowerPC as before).
- The database and the targets are both inputs, so one dumper serves every game.
- Its modes: a bare address (the whole function), `owner:`, `range:`, `forcerange:`,
  `blocks:`, `callers:`, `callees:`, `xrefsto:`, `data:`, `jpt:`, `strs:`, `imm:`, `bytes:`,
  `hilo:`, `stores:`, `loads:`, `decomp:`. The header of the file documents each one.
- Two modes write:
  - `decode:A:B` forces the range to code, makes functions there, and saves the database.
  - `mkfuncs:A[,B..]` makes a function start at each address, ending any IDA function that
    runs over it, and saves. `match_function.py ida-starts [--write] [DB]` feeds it every
    function start the split has and IDA lacks. It also lists functions whose IDA size differs
    from the split's: splat's sizes keep some trailing alignment nops.
- `export:OUT.json` writes one database's facts for the other tools. For each function: its
  bounds and name, and each instruction's address, opcode, mnemonic, operands and data target.
  Each `lui`/`lo` pair's target goes on the instruction that completes it, and a folded macro's
  goes on its last word. It also writes the code words, including code outside any function.
  `nisitenma_archive.py`, `match_function.py` and `report_progress.py` read these exports.
- The databases, kept beside this repository (not in it): one per TLB file and overlay,
  `../ida/goemons_great_adventure/files/NNNN_<RAM>.bin.i64`,
  plus `main.bin.i64`. Each was built with `idat.exe -B -pmipsb -b<VRAM/16>`.

## patches/

`decomp-permuter-deref-function.patch`: decomp-permuter's type pass crashed on `(*func)()`.
That is the form this decomp writes for every call the game makes through a register: IDO
compiles it to a jalr, and `func()` to a jal. Apply it in a decomp-permuter checkout with
`git apply`.

`ido-static-recomp-msys2.patch`: two hunks in ido-static-recomp's `libc_impl.c`, under
`__CYGWIN__` only (the MSYS2 build). `build_rom.py --setup` applies it on Windows before it builds
IDO 7.1.
- It drops `memory_allocate`'s page assert. MSYS2 reports 64 KB pages, so 7.1's unaligned data
  start trips the assert, although the code under it already rounds the range.
- It passes `__assert`'s arguments in newlib's order (file, line, expression). GCC 15 refuses
  glibc's order there.

## Extending them for another game

- **Game-independent:** `match_function.py`, `report_progress.py`, `build_libultra.py`,
  `build_zlib.py`, `survey_rom.py` and `ida_dump_funcs.py`.
  Extend them in place.
- **Game-specific:** `nisitenma_archive.py`, and the file names, segments and versions in
  `build_rom.py`. Change those in place (or put a game's archive tool beside this one) rather
  than copying the set.
- **What was never used, and is not here:** a status-line usage logger (its file was never
  written).
