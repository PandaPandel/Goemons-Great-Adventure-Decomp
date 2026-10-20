# IDO 5.3 codegen notes (learned from matches)

These are notes learned while matching this game, cited from `src/` as `IDO_NOTES <n>`.

Each rule here was **measured**: a hypothesis, a changed line, a rebuild, and asm-differ. The
source is Goemon's Great Adventure (`-O2 -mips2 -G 0 -non_shared -Xfullwarn -Xcpluscomm -signed
-32 -Wab,-r4300_mul`, through asm-processor).

⚠ **The game is IDO 7.1.** Rules 1-11 were measured under 5.3. Every function they
came from also matches under 7.1, so they stand. Rules 12 onward are measured under 7.1.

## Telling IDO 5.3 from 7.1
0. **A short or char value truncated in place means 7.1:** `addiu s0,s0,1; sll s0,s0,16;
   sra s0,s0,16` (or `andi v0,v0,0xff`). 5.3 goes through a temporary (`sll t8,s0,16;
   sra s0,t8,16`, or `andi t6,v0,0xff … move v0,t6`) under `-O1/-O2/-O3`, `-mips1`, `-g3`
   and `-Wo,-loopunroll,0`. A scratch s16 loop settles it in seconds.

## Functions and frames
1. **An empty function with N named parameters spills them to the caller's argument area with no
   frame of its own:** `sw a0,0(sp)` … `jr ra; sw a3,12(sp)`. A **variadic** `(fmt, ...)` version
   gets an 8-byte frame of its own (`addiu sp,-8` … `addiu sp,8`), so a frameless spill of all four
   registers is NOT variadic. Standalone test in `work/idotest`; `func_80000DCC_main`.
2. **An infinite `while (1)` loop still gets its epilogue**, after 1–4 `nop`s of padding.
   IDO emits it though nothing reaches it. `func_800006F4_main`, `func_80000C28_main`,
   `func_80000E3C_main`.

## Locals
3. **Stack slots go in declaration order from the top:** the first-declared local gets the
   highest address. `func_80000C28_main`: `OSMesg dummy; SwapMsg *msg; SchedClient client;` sit
   at `0x54`, `0x50`, `0x48`.

3b. **Unused locals still reserve stack.** The original frame was 16 bytes bigger than mine
   (0x68 vs. 0x58), and an unused `s32 pad[4];` declared after the used locals fixed it. This is
   the decomp community's usual `pad` idiom. `func_800008C8_main`.

## Constants and stores
4. **A 64-bit zero store under `-mips2`** loads its own pair of registers (`addiu tX,zero,0` ×2)
   and stores the LOW word first, then the high word (`sw t9,0x8E4` then `sw t8,0x8E0`). A run
   of these pairs means `u64`/`s64` fields (OSTime-like). `func_80000460_main`.
5. **A constant is shared in one register only between uses of the same type class.** The value
   `1` stored to two `u32` fields shared `v0`, while `1` stored to an `s16` got its own register.
   With the fields as `s32`, all three shared one register and it did not match. So: **if a
   repeated constant sits in two registers, the destination types differ in signedness.**
   `func_80000460_main`.
6. **A chained assignment** (`a = b = 1`) produced the same code as two separate stores here, so
   it is not a lever for register choice.
6b. **Rule 5 again, for a comparison:** a `state` compared with `1`/`2` kept those in `s7`/`fp`
   and reused `s7` as `OS_MESG_BLOCK`'s `1`. The original loads a fresh `li a2,1` for the flag, so
   `state` is **unsigned** (`u32`). `func_800008C8_main`.
6c. **A register variable assigned right AFTER a call is hoisted above it:** `osSendMesg(…);
   state = 0;` emits `li a2,1; move s1,zero` before the `jal`. Written before the call, the two
   come out in the opposite order. `func_800008C8_main`.

## Control flow
7. **`while (cond)` compiles as `if (cond) do { … } while (cond)`**, which is m2c's
   `if (…) { do { … } while (…); }`. Write it as `while`. `func_80000878_main`,
   `func_80000C28_main`.
8. **A two-case `switch` on small constants compiles to a `beq` chain** (`beq 666; beq 669;
   b loop`). `func_800006F4_main` matched as a `switch`.
9. **`if (x == 0) { A } else { B }`** puts A in the fall-through and B (here a single store) in
   the delay slot of a `bnel`, and also leaves an unreachable copy of B right before the join.
   `func_80000C28_main`.

## Addresses
10. **Pointer arithmetic on a global's address folds into the relocation; integer arithmetic does
    not.** `buf + 0x800`, `(u64 *)((u8 *)buf + 0x4000)` and `field + 0x800` all compile to a
    second `lui`/`addiu` pair on `buf+0x4000`. The original's `lui t0,%hi(buf); addiu t0,..;
    addiu t1,t0,0x4000` (one base, then an offset) came only from `(u64 *)((u32)buf + 0x4000)`.
    So **`addiu rX, rBase, N` on an address register means the source did integer arithmetic.**
    `func_80001A90_main` (the RDP output FIFO's end).
11. **A pointer difference times its element size stays as written:** `(head - start) *
    sizeof(Gfx)` compiles as `subu; sra 3; sll 3`, not a plain `subu`. `func_80001A90_main`.

## Scheduling (IDO 7.1)
12. **IDO schedules by source line.** Two statements on ONE line can come out in a different
    order from the same two on two lines. `func_80001F24_main` matched only with
    `D_80080188_main[port].error = ret; return ret;` on one line: then the store lands after the
    stack pop (`lw ra; addiu sp; sw v1,0x2a0(a1); jr ra; move v0,v1`). decomp-permuter found it.
    ⚠ Tools that re-print C (pycparser, the permuter's pruning) lose this; check such a
    function against unpruned source (`tools/permuter.py check`).
    - **A macro is one source line.** Everything it expands to is on the line that invokes it.
      Bkas_proc (tlb_0326_0, 17,430 instructions) was 12 lines from the game across three uses
      of a LANDED macro (an if/else after two stores). The game hoisted the if path's call setup
      above the stores; the macro hoisted the else path's. Written out on its own lines at each
      use, it matches. A probe written on separate lines had
      already shown the game's order. When a probe of a macro's body gives the game's order and
      the function doesn't, expand the macro. 2026-10-12.
    - **The reverse holds inside loops.** pseleCheck (tlb_0018_6) matched with its choices
      loop's body joined onto one line:
      `if (w->choice[i] > 0) { ...chara = i; } w->choice[i] = 100;`
      That puts the hoisted 100 before the 10 at the first test, as the game has. The if alone
      on one line did not change it. Found with a probe written the same way.
      CHANGELOG (742).
    - Tool: `match_function.py macros <file> <func>` writes out each macro use on its own
      lines, one at a time, and keeps the best.
    - **The reverse of a hand-unrolled loop:** four stores to a global array's elements
      (`gPlayerStats[0..3].wpLv = 0`) share one base register when written as four statements.
      Written as a loop of four, IDO unrolls it into four separate %hi/%lo stores, as the game
      has them (utaDemoProc).
13. **A store written through the global array can move past the `ra` reload; one through a
    pointer local cannot.** `pak->error = f(pak)` keeps `sw` before `lw ra`, while
    `D_80080188_main[port].error = f(pak)` puts it in the `jr ra` delay slot. The assembler knows
    the global is not stack memory. `func_80002898_main`.
14. **A value spilled across a call goes into a local's home when it IS that local; otherwise it
    gets a compiler slot above the locals.** `func_8000297C_main`: with a `status` local, the
    spill sits at `0x18` (its home); with none, at `0x1C`.
15. **A loop over a small constant count is unrolled, one `lui`/`sh` per element**, where four
    written-out stores share one base register (`lui v0; addiu v0; sh zero,0x2A8(v0);
    sh zero,0x564(v0) …`). Original per-element `lui at` pairs mean a loop. `func_80002E04_main`.
16. **Statement order decides which pointer gets which register.** In `func_80001880_main`,
    `D_8004F6AC_main[i] = 2;` written before `task = &D_8004F6B8_main[i];` gives the original's
    `a0` = task, `v1` = message; the other order swaps them.

## Locals and the frame (IDO 7.1)
17. **Where a local lands is not "one slot per declaration, top down".** Measured on one
    function body (`func_800022FC_main`'s) with 16 declaration sets (scratch `frame.c`): the
    frame and every slot moved with both the ORDER and any unused `pad`. The original (frame
    0x38; `offset` 0x20, `check` 0x24, `ret` 0x34) came only from
    `s32 ret; s32 pad[2]; s32 offset; u8 *check;`. Neither `ret, check, offset` + any pad nor
    an 8-byte local did it. **Method: when only slots differ, compile the body under several
    declaration sets side by side and pick the one that reproduces the original's offsets. Don't
    reason it out.**
18. **A comparison stored in a variable is materialised** (`xor; sltu rX,zero,rX; beqz rX`);
    written inside the `if` it is a plain `bne`. The original's `xor/sltu` before a branch means
    `differ = *p++ != *q++; if (differ) …`. `func_800025A8_main`.
19. **`!x` and `x == 0` are not the same to IDO.** They give different register allocation around
    the test. `func_800013F0_main` matched only with `if (!D_800266FC_main)`
    (decomp-permuter).
20. **Two stores to one global:** `G = size; … G = size - 1;` drops the first store as dead.
    `G = size; G2 = G; … G--;` keeps both and still computes `size - 1` from the register (the
    stored value is forwarded). `func_80002F40_main`'s prologue.
21. **Branch polarity follows the source.** `if (p != NULL) { A } else { B }` puts A in the
    fall-through; `if (p == NULL) { B } else { A }` puts B there. When everything matches except a
    swapped `beqz`/`bnez` and two blocks trading places, flip the condition.
    `func_800034D4_main`, `func_80003744_main`.
22. **A constant used as a register (`subu t8,t7,t6` with `t6 = 0x2000`) is a value forwarded
    from a store**, not a local: `G = 0x2000; H -= G;`. A `size` local with the constant folds
    to `addiu -0x2000`. `func_8000354C_main`.

## Arrays and indexing (IDO 7.1)
23. **`tab[n - 1]` folds the -8 into the loads (`lw t8,-4(v1)`) when `n` is signed or `tab` is
    an array of structs. The original's `sll; addiu -8; addu; lw 4(v1)` shape, with temporaries
    starting at `t7`, came only from a FLAT u32 array indexed `tab[(n - 1) * 2 + 1]` and
    `tab[(n - 1) * 2]`.** An unsigned `n` stops the folding but keeps `t6`. Scratch `idx.c`, 17
    shapes. `func_800044C8_main`, `func_800045D4_main` (the RAM-range table).
24. **A descending clear that tests the OLD pointer against `&a[1]`, materialised**
    (`sltu v1,v0,a0; xori v1,v1,1; addiu v0,-4; bnez v1; sw zero,4(v0)`), is
    `p = &a[31]; do { *p = 0; } while (p-- >= &a[1]);`. A `for (i = 31; i >= 0; i--)` gives a
    plain `sltu; beqz` against `&a[0]`. Scratch `idx.c`, 9 shapes. `func_8000479C_main`.
    **Corrected 2026-10-01 (rule 188):** the pointer form builds `&a[31]` before `&a[1]`; the
    game's order (and the match) is the INDEX form `i = 31; do { a[i] = 0; } while (i-- > 0);`,
    which IDO strength-reduces to the same loop.

## Loops, parameters and returns (IDO 7.1)
25. **`for` and `while` loops are rotated** (the test is copied to the bottom and the first copy
    constant-folded, e.g. `bltzl v0` for `count() >= 0`); **`while (TRUE) { if (!cond) break; … }`
    is not.** The original's shape, with the test at the top and `i++` in the back branch's delay
    slot, came only from the `while (TRUE)` form. `func_800075C0_main` and its three siblings in
    `fileres.c`.
26. **A u16 return kept in an s32 variable is copied raw (`or s1,v0,zero`) and masked where it is
    passed to a u16 parameter (`andi a0,s1,0xFFFF`).** Kept in a u16 variable, it is masked on
    assignment (`andi s1,v0,0xFFFF`). The same four functions.
27. **A narrow parameter that is live across a call lives in its home and is reloaded with
    `lhu`/`lbu`; one used before any call is `andi`'d from `a0`.** A 32-bit parameter passed on to
    a u16 parameter after a call is an `lhu` of its home's low half (`func_80003E04_main`).
    `func_800079F4_main` does `sw a0; lhu a0,0x1A(sp)` with no call before it. u16, s32, K&R,
    and an unprototyped callee all give `andi`; decomp-permuter matched it only with a
    `volatile u16` parameter (kept, marked FAKE). Also from the permuter: `func_80007434_main`
    matched with `if (file)` for `if (file != 0)`, as in rule 19.
28. **`li v0,-1` on a range check means the function returns an int**, even when every caller
    masks the result to u8. Declared u8, it gives `li v0,255`. `func_80007518_main`.
29. **`(*p).f` and `p->f` do not compile the same.** In `func_800056D0_main` only
    `heap = &G; … (*heap).base` loads the heap's size before its base, as the original does;
    `heap->base`, `(*&G).base` and `G.base` do not. decomp-permuter found it, and it is a FAKE
    match.
30. **A comparison's operand order carries into the register assignment.**
    `func_80007B94_main` matched only as `(u16) (list->file & 0x0FFFFFFF) == file`; with
    `file == …` the two values traded `v0`/`v1`.
31. **`n = f() - 1; while (n != 0)` with a signed `n` is folded to `li s8,1; beq v0,s8`.** The
    original's `addiu s2,v0,-1; beqzl s2` came only from an **unsigned** `n`. `for`, `do/while`,
    and `--n` all fold the same way. Scratch `rl_*.c`. `func_8000619C_main`.
32. **A switch's case constant shares a register with an equal argument constant of the same
    type.** `switch (f(2))` put the call's 2 in the case-2 register (`move a0,s6`). The
    original's `li a0,2` came from `u32 code = f(2); switch (code)`: an unsigned case constant
    is a different constant. The function's return type is not the fix, because
    `func_80005F50_main` needs `0x1A` shared between a u8 test and the call's result.
    `func_80006314_main`, `func_800065A0_main`.
33. **An OR chain of one type is re-associated; a mixed one keeps its grouping.**
    `(r << 11 | g << 6 | b << 1) | f(1)` came out as `f(1) | r<<11 | …` until the call was cast
    `(u32)`. An unsigned local in place of the cast put it on the wrong side of the `or`.
    `func_80005E64_main`.
34. **An address put in a register for a single store** (`lui v0; addiu v0; sw a1,0(v0)`) means
    the global is read back later: `G = dst; pic->dst = G;` (the read is forwarded, but its
    address stays). `func_80005820_main` (`D_80088574 = *D_80088570`), `func_80005CC0_main`.
35. **The fingerprint of an if/else that assigns one variable:** the else's one instruction
    scheduled above the test, and a `b` from the then-block to the very next instruction:
    `andi s4,v0,0xFFFE; bne s7,t8,L; nop; b L; or s4,v0,zero`. It is
    `if (c) { x = v; } else { x = v & m; }`. Written as `x = v & m; if (c) x = v;` it gives a
    `bnel`. `func_8000619C_main`.
36. **A result that goes through a temporary on the way out** (`lh a0,…; sll a0,a0,1; jr ra;
    move v0,a0`, or `move v1,zero … move v0,v1`) is a ternary `return c ? a : b;`.
    `func_80006900_main`, `func_80006F6C_main`.
37. **Arguments written as `f(pic, --x, ++y, c)`** let IDO fill the switch's branch delay slots
    with `or a0,s2,zero`. The same steps as separate statements before the call do not.
    `func_800065A0_main`. Also: `pos = pos + len - 1` with `len` a variable keeps `pos` first in
    the `addu`; every inline form put it second (`func_80006458_main`).

## Statics, flags and delay loops (IDO 7.1)
38. **A global read and then written in another block keeps its address in a register when it is
    `extern`** (`lui v0; addiu v0; lw t7,0(v0) … sw zero,0(v0)`). As a **`static`** it takes a
    fresh `lui` for each access (`lui t7; lw t7,%lo; … lui at; sw zero,%lo(at)`). The original's
    shape therefore marks a static, here the SDK's function-local `static int firstTime = 1`.
    `volatile`, an array, and a nested `if` all keep the register. Such a function waits for its
    file's `.data` to be migrated. `func_8000833C_main`, scratch `am_*.c`.
39. **A flag set to 1 with its own `li s2,1`, while a register already holds 1** (for
    `OS_MESG_BLOCK` and case 1), is a **u8** flag: its constant is a different type.
    `func_80008028_main` (`u8 done`). Its frame (8 bytes larger, the two address-taken locals
    4 higher) needed one unused `s32 pad;` declared first (rule 17's method).
40. **An empty countdown `li s1,16; addiu s1,s1,-1; L: bnel s1,zero,L; addiu s1,s1,-1`** is
    `i = 16; do { i--; } while (i != 0);`. `while (--i != 0)` folds the first decrement to
    `li s1,15`. `func_80007D10_main`.
41. **`G = &a[i]; if (G->f)` reads through the pointer just stored,** which IDO forwards, and
    the forwarding costs one temporary. Written as `a[i].f`, every later temporary shifts by one
    (`lw t7` for the original's `lw t8`). `func_8000B1A0_main`, `func_8000B224_main`.
42. **Chained assignments to globals:** `a = b = 0` with `b` an **s8** stores `b`, reloads it
    with `lb` and stores that to `a`. With `b` a **u8** the zero is forwarded (no reload; a u16
    `a` gets `andi t,zero,0xFF`). Every global in the chain still has its address in a register.
    The types in `func_80008844_main` came from this.
43. **Two fields of `a[0]` and `a[1]` put the array's base in a register**
    (`lui v0; addiu v0; lhu 6(v0); lhu 2(v0)`). The original's separate `lui` for each means two
    separate globals, not an array. `func_80008F8C_main`, `func_8000B394_main`
    (0x800C3374/0x800C3378).
44. **Unrolling is a size budget; measure it with `tools/permuter.py unroll src/<f>.c <func>`.**
    uopt's `-unrolllimit` L (this build's default: 320-323; audio.c compiles alike only there)
    lets a loop whose body weighs B units be unrolled by 4 while 4B <= L, and by 2 while
    2B <= L. The `unroll` mode prints the function's size at each L where it changes; a step at
    L means B is about L/4 (by 4) or L/2 (by 2). So a loop the game unrolls by 2 where this C
    unrolls by 4 has a body below 81 units here and 81 or more in the game: the game's C is
    heavier. Casts and integer arithmetic weigh, without changing the code they compile to:
    gfxmgr.c func_800D5390 measured 78 with `(u32) (buf + size * count)`, 76 or less with `(u8 *)`
    pointer compares, and exactly 81, a match, with the u32 sum `(u32) buf + size * count`.
    Never set `-Wo,-unrolllimit` for the build: at 200, five matched files change.
    A one-store loop (plmain.c func_8018A5F8's clear) unrolls by 4 at every L from 16 to 1280;
    the history below is about those.
    **History (before the measurement):** `func_80008650_main`'s empty count to
    0x100000 in steps of 4, and `func_8000B494_main`'s 4-slot search, stay loops in the original.
    IDO here unrolls every form tried: for, while, do/while, a pointer, counting down, `!=`.
    Only `i < n` with `n` a local set to 4 stopped it, and that compares with `slt` where the
    original has `bne`. The same file's 16-track loops *are* unrolled in the original
    (`func_800090A8_main` by 4, `func_80008FCC_main` by 2). Scratch `dl_*.c`, `sl_*.c`.
    **Narrowed (scratch `ur.c`, `sl_d.c`):**
    - `-Wo,-loopunroll,0` compiles B494 to the original's code exactly. That setting also stops
      the 16-trip unrolling, which the file's other functions need.
    - The original uopt's options include `-loopunroll`, `-unrolllimit` and `-nomultibbunroll`
      (strings in `ido/7.1/usr/lib/uopt`). Every `-Wo` setting tried here stops all unrolling,
      except `-loopunroll,8` (16 trips by 8; 4 trips not at all) and `-unrolllimit,128`
      (both by 2).
    - By default, this build leaves a 3-trip one-block loop alone, unrolls a 4-trip loop with
      several blocks completely, and unrolls a 16-trip one by 4.
    - The original leaves B494's 4-trip loop (several blocks) a loop. Yet it unrolls
      `func_8000ADF0_main`'s one-block 4-slot clear completely (the stores come out [1], [2],
      [3], [0]).
    - Next step: survey the original's constant-trip loops across main to pin down its rule;
      then either a per-file `-Wo` option or a different uopt build.
    - A 4-trip loop with a call in it stays a loop in both builds (`func_8000B5D0_main`'s slot
      ramps), so a call is not the question.
    - IDO 5.3 (vendor/ido-5.3, through `IDO=` for one check) also leaves a 3-trip one-block loop
      rolled (polygon.c `func_800D6BE0`'s three angle stores, 2026-10-03): the older compiler
      is not the difference either. With `IDO_EXTRA` (build.py; one check only): that loop
      under -Wo,-nomultibbunroll or -Wo,-loopunroll,8 stays rolled; under -loopunroll,3 or ,2
      it unrolls, but [0] absolute and [1], [2] through a still-computed index, not the game's
      three absolute stores.
    - **Whether the original unrolls a top-level 3-trip loop is open.** Its tiny 3-trip loops
      that stay rolled (matched: mathutil.c `func_800D318C`'s `lu[i][j] = a[i][j]`, vanime.c
      `func_801738A0`'s two-store copy) are all INNER loops of a nest, and every matched
      top-level 3-trip loop has several blocks or a heavy body. Per-element `lui`s prove
      nothing for three elements: polygon.c `func_800D66B8`'s straight stores give them too.
45. **A table indexed by `x - K` folds `-K*4` into the `%lo`,** so splat names the address K
    entries before the table (`D_80037F50` for a table at 0x80038290, K = 0xD0). Write the real
    table, `tbl[x - K]()`, and give its name an address in `undefined_syms.txt`. It also decides
    the index register: `early[x]` indexes with the fresh load (v1), `tbl[x - K]` with the
    variable's copy (s0), as the original does. `func_8000BEEC_main`.
46. **`andi 0xFB` on a u8 is `&= 0xFB`;** `&= ~4` gives `andi 0xFFFB`. `func_8000BEEC_main`
    (and `0xFD` in `func_8000C600_main`).
47. **One value tested against three constants as `beq; beq; beql` is a `switch`** with no jump
    table (`case 0xF2: case 0xF3: ...; case 0xFF: ...`). The same tests written as
    `if (c == a || c == b) ... if (c == d)` give a `bnel`. `func_8000BEEC_main`.
48. **`x / 256.0f` is compiled as a multiply** (by the exact 1/256); `/ 256` (an int, converted)
    keeps the `div.s`. `func_8000C600_main`.
49. **`lui t6,0x800C; lhu t6,0x3398(t6)` with no relocation is an address written as a number**
    (`*(u16 *) 0x800C3398`), even where the same global is also used by name.
    `func_8000B5D0_main`.
50. **A fade's step global reached by a fresh `lui` each time, where IDO here keeps its address
    in a register, is not rule 38:** declaring it `static` changed nothing. Open.
    `func_8000B5D0_main` (D_800C336E), parked.
51. **The same shift done twice (`srl t7,a1,24` … `srl t8,a1,24`) is two expressions:**
    `if (x >> 24) y = v * (x / 0x1000000);`. Both compile to `srl`, and IDO does not share them;
    `(u8)(x >> 24)` adds an `andi`. `func_8000D28C_main`.
52. **A named local changes the code even when it lives in a register.** It reserves a stack
    slot (`b = *p++; value = b * 40;` gave the original's 0x28 frame where the inline read gave
    0x20). It also takes the lowest free a/v register, which moves the global addresses along
    (a1/a2 instead of v1/v0). `func_8000D5B8_main`, `func_8000D74C_main`.
53. **A subtraction landing in a v/a register before a `div`, with a t-register number skipped
    (or a dead `move`), is a local:** `diff = (u16)(t << 8) - cur; step = diff / ticks;`.
    `*p++`, `*++p` and `*p; p++` all compile alike and are not the lever.
    `func_8000CE8C_main`, `func_8000D2E8_main`, `func_8000D7A4_main`.
54. **An if's branch delay slot takes the else-block's first instruction in source order.**
    `unk99 = 2; v *= 4;` puts `li t4,2` there; `v *= 4; unk99 = 2;` puts the shift. The stores
    still end up at the blocks' ends. `func_8000D96C_main`.
55. **A pointer bumped in place (`addiu a0,a0,1`) before a call is `x = *p++; f(p);`,** not
    `f(p + 1)`. `func_8000D1E0_main`.
56. **Stores into a global array of structs (`arr[n].f = x`) cannot touch other globals,** so a
    count used as the index is not reloaded and the stores may be reordered (the original's
    `kind` store came last). With a pointer local (`e = &arr[n]; e->f = x`) IDO reloads the
    count after the stores. `func_8000ED60_main`.
57. **A global pointer re-read after a call (`lui v0; lw v0,%lo`) is written as the global each
    time;** a local copy (`t = G; ... t->f`) is spilled across the call instead. Conversely, a
    pointer the original keeps in a register past stores to other globals is a local.
    `func_8000E7F8_main`, `func_8000EE4C_main`, `func_8000E894_main` (global); `func_8000E62C_main`
    (local).
58. **A local copy of a field (`note = t->note`) costs a register; reading the field again does
    not** (it is shared until the next store). This shifted every register of
    `func_8000E288_main` and `func_8000DBA4_main` by one.
59. **A u8 local compared with a u8 field costs a register the code never uses** (a1 skipped);
    an s32 local does not. `func_8000E394_main`.
62. **A source pointer loaded again for every 3 bytes, all loads first (`lw t6,52(a0); lw
    t9,52(a0); …`), is struct copies:** `w->c0 = w->src[0];` with a 3-byte `{u8 r,g,b}`. Twelve
    byte assignments reload the pointer after each store instead. `func_800CC9A8_ovl_0006`.
63. **A state function called with two arguments but defined with one** (the callee homes no
    second argument): the pointer is unprototyped, `void (*state)();`. That also lets
    `w->state == func` compile against a one-parameter function. `func_800CCF58_ovl_0006`.
64. **A narrow parameter's reload from its home (`lhu t7,0x22(sp)`) is scheduled from where
    its store sits in the source.** Moving `w->unk52[0] = dawn;` to right after the first
    store put the reload where the original has it. `func_800CD904_ovl_0006`,
    `func_800CDAD4_ovl_0006`.
65. **One temporary register spent for nothing (t7 where the original has t6) can be a chained
    assignment:** `D_80088338.unk58[i] = w->unk52[i] = 0;` in place of two stores.
    `func_800CD8A4_ovl_0006`.
66. **A constant-index array element swaps an `addu`'s operands against a named field.**
    `w->unk38.r + q` gave `addu base,q`; the same byte as `w->top[0].r + q` gave `addu q,base`,
    and `q + w->top[0].r` gave `addu base,q` again. After retyping fields as arrays, re-check
    every function that used them. `func_800CBF20_ovl_0006`, `func_800CCA20_ovl_0006`.
67. **Locals kept in registers still move a stack array (rule 17).** The array `f32 v[4]` was
    4 bytes too high when declared first, and 12 too low when declared after `i, lo, hi`.
    `s32 i; f32 v[4]; f32 lo, hi;` matched: one register local above the array.
    `func_800C8C5C_ovl_0006`.
68. **A statement after a call can be scheduled before its `jal`.** The original's
    `move s6,zero` came right before the call's `mfc1 a1`, and after the argument's `cvt`.
    Written before the call (either order with the assignment next to it), it went first. Written
    after the call, it matched: IDO hoists a callee-saved register's constant set above the call.
    `func_800CA1E0_ovl_0006`.
69. **`sqrtf` is not an intrinsic in libultra 2.0I's gu.h** (only from 2.0K). Code with an inline
    `sqrt.s` needs `#pragma intrinsic (sqrtf)` itself. Goemon's main calls the library's `sqrtf`
    and overlay 6 inlines it, so the pragma is in camera.h, not common.h.
    `func_800CA4B8_ovl_0006`.
61. **`abs.s` inline is `fabsf` declared intrinsic** (`float fabsf(float); #pragma intrinsic
    (fabsf)`, as in OoT's and GoldenEye's math.h). Without the pragma IDO calls a function
    `fabsf`. Goemon's common.h has it. `func_800C8580_ovl_0006`.
60. **A byte loaded with lbu and sign-extended (`sll/sra 24`) before `<< 8` is
    `(s8) (v & 0xFF) << 8`** (the permuter's find). IDO drops the extension as redundant from
    `(s8) v << 8`, `* 256` and `(s16) (s8) v`, and an s8 local makes the load `lb`.
    `func_8000EDE4_main`.
70. **A branch's register operands the other way round (`beql t9,v0` for `beql v0,t9`)**, where
    either order of `!=` in the source gives the same: `(count ^ 0) != x` flips them (FAKE, from
    the permuter). `func_8000E394_main`, `func_8000E48C_main`.
71. **A u16 local in place of s32 can move which registers hold a loop's values** (the request
    id, `id = x & 0xFFFF`). `func_80009200_main` (the permuter's find).
72. **A base register the original keeps (`addiu v0,a0,0x20`, then fields off v0) is a pointer
    made through an integer:** `w = (Work *) ((u32) obj + sizeof(Obj))`. From `(Obj *) obj + 1`
    IDO folds the 0x20 into every offset. Across a call, make it before the call; made after,
    it is folded too. `func_800C848C_ovl_0006`, `func_800C866C_ovl_0006`,
    `func_800C8A00_ovl_0006`.
73. **Float constants in an overlay's C need the file's .rodata as its own splat piece**
    (`[0xROM, .rodata, file]`). splat then moves the constants into the asm functions as
    `.late_rodata`, and the C's literals land in order between them. The game's strings are
    EUC-JP while splat writes UTF-8: asm-processor takes `--input-enc utf-8 --output-enc euc-jp`,
    and .s files for GNU as are converted first (Goemon's tools/build.py). `tools/permuter.py
    check` shows the constant's load as a relocation difference (`%hi(0x8010C820)` against
    `.rodata`); only the full build tells.
74. **A string in .rodata that no code loads is a compiled-out debug print.** IDO keeps the
    literal of `if (0) osSyncPrintf("...")` in .rodata, in source order, and drops the code.
    Goemon's common.h has `#define PRINTF if (0) osSyncPrintf` (IDO 7.1's cpp has no variadic
    macros). Its position among the strings tells which function had it:
    "Warning: in aicamDeleteLookat !" sits between the users of "AI Cam" and "bgrad", so it is
    `func_800CACD0_ovl_0006`'s.
75. **Write a float constant with all the digits splat prints** (`.float 0.6981317401` is
    `0.6981317401f`). `0.6981317f` rounded to the float below and moved the last byte.
    `func_800C8AC8_ovl_0006`.
76. **Float arithmetic by small constants:** `x * 2.0f` compiles to `add.s x,x`; a `mul.s` by a
    loaded 2.0 is `x * 2` (an int, converted), as rule 48 has for division. An unsigned
    char converted to float (`(f32) w->pitch`) gets the unsigned fix-up (`bgez`, add 2^32), as the
    original has; `w->pitch / 256` without the cast divides as integers. gu.h's `M_PI` is a
    double (it would promote the arithmetic); the game's pi is a float, Goemon's `PI`.
    `func_800C9420_ovl_0006`.
77. **A call's result copied (`move v1,v0`) and the copy tested is a u16 function's result
    kept in an s32 local.** An s32 result tests v0 itself, and a u16 local copies it with
    `andi 0xFFFF`. `func_800C8F40_ovl_0006` (the buttons).
78. **`x = a - b` computed before the branch, then `x = 0` if `a < b`, is two variables:**
    `lim = f - dist; if (f < dist) lim = 0.0f;`. The if/else and the ternary on one variable
    put the subtraction in the else. `func_800C8F40_ovl_0006`.
79. **A call through t9 (`lui t9,%hi(f); addiu t9,..; jalr t9`) is `(*f)(x)`.** IDO does not
    turn an explicit dereference of a function into a direct `jal`, and `(*sqrtf)(x)` is not
    inlined as the intrinsic. The same file calls the same function both ways.
    `func_800C9DC4_ovl_0006` (`(*sqrtf)`), `func_800CFB7C_ovl_0006` (the Controller Pak probe).
80. **A struct field stored by absolute address (`lui at,%hi(S+0x8)`) while the same struct's
    base is in a register is a separate symbol in the original.** GameState's `fade` went
    through v1 and `fadeStart`/`fadeEnd` did not; externs at 0x800881C8 and 0x800881D2 fixed
    the length and branches. A local pointer to the struct made every access absolute.
    `func_800CB888_ovl_0006`, `func_800CAEF8_ovl_0006`.
81. **A struct returned by value:** the caller passes a hidden pointer in a0 (the real
    arguments start at a1), and the callee copies the struct to it and returns it in v0. So
    `lw v0,<a0's home>` right before the copy, with `swl/swr` stores to v0 for a 2-aligned
    struct, is `return set;`. With an `out` parameter IDO loads it into another register.
    `func_800CDDE0_ovl_0006` (a FileSet of four s16).
82. **A value computed twice in the source can be one CSE temp in the original, not a local.**
    `func_800CE310_ovl_0006`'s header size and `count * 0x14` were expressions (a macro), not
    named locals. Named, they took two slots and moved the saved registers. A pointer variable
    to the entries (`e = cache->entries`) then put the right values in s0 and s1.
83. **A loop over `base[i]` whose original starts its pointer at the array (`addiu v1,a0,0x18`)
    is a pointer walk:** `e = cache->entries; for (...) { e->flags &= keep; e++; }`. With
    `cache->entries[i]` IDO starts at `cache` and folds 0x18 into the load. `func_800CE718_ovl_0006`.
    - splat appends a C file's unreferenced .rodata (a compiled-out print's string) as a
      `GLOBAL_ASM` of its `D_` symbol at the end of the file; the C's `PRINTF` replaces it.
84. **Four copies of a test on array entries 0-3, each with its own `lui` (absolute address),
    and the first count as `n = 1`, are a 4-trip loop without calls that IDO unrolled.** Written
    out as four ifs, IDO builds a base register for the struct instead. A loop with a call in it
    stays a loop (as rule 44 has). `func_800CF3C0_ovl_0006`.
85. **A local `f32 r[3]` is not a local `Vec3f`.** Stores into the array keep the other values
    in registers and reload the arguments where the original does; the struct version spilled
    temporaries and grew the frame from 0x10 to 0x30. `func_800CF5B8_ovl_0006`.
86. **A struct copied with `lwl/lwr` is aligned below 4; with `lw/sw`, it holds a word.** The
    save block copied with `lw/sw` only after its first bytes became a `u32` (the slot's CRC).
    `func_800CFBC0_ovl_0006`, `func_800CFC64_ovl_0006`.
87. **`beqz v0, <the call>; ...; b <end>` after a probe is `if (ret != 0) return ret; return
    f(...);`:** the function returns a value. A void `if (ret) return; f();` gives
    `bnez v0, <end>`. `func_800CFB7C_ovl_0006`, `func_800CFDE0_ovl_0006`.
88. **`addu a0,<pointer>,<scaled index>` (the pointer first) is byte arithmetic:**
    `(T *) ((u8 *) p + i * sizeof(T))`. `p + i`, `i + p` and `&p[i]` all give the index first.
    Likewise `(u32) base + off` puts the base first where `base + off` and `off + base` do not.
    `func_800D0484_ovl_0006` (Goemon's `NODE_AT`), `func_800CFF3C_ovl_0006`.
89. **A call's result used straight from v0 in a later call's arguments, where its siblings were
    stored, is a call written inside the argument list.** `func_800D025C_ovl_0006`: two scale
    values went to locals and the third was `func_800D5520(...) * (1/256.0f)` in the arguments.
    Values stored back to back on the stack with no padding (s16 then u16) are one struct.
90. **IDO -O2 unrolls a 4-trip inner loop whose body is one statement, but not a 2-trip inner
    loop with a larger body.** In the matrix inverse (`func_800D1424_ovl_0006`), `for (k = 0;
    k < 4; k++)` loops came out as four straight copies. The game's guMtxF2L
    (`func_800D0C9C_ovl_0006`) has the `j` halves written out by hand: libultra's `for (j = 0;
    j < 2; j++)` kept a loop and a second counter. For `a + b` in float arithmetic, see also
    rules 30 and 33: the rotations (`func_800D1608` to `func_800D1BF4`) evaluate their right
    term first, so `s * m[1][0] + m[2][0] * c` loads `m[2][0]` first.
91. **A declared local that is never used still gets a stack slot.** Locals that live in
    registers do not. Slots go downward in declaration order: the first declared is highest.
    So a frame a few bytes too small, or an address-taken local 4 bytes off, is fixed by an
    unused local placed at the right point in the declarations.
    - `func_800D0F64_ovl_0006` matches only with one unused f32.
    - `func_800D2930_ovl_0006` has `tz; c; s`, and c lands at 0x20.
    - `func_800D127C_ovl_0006` needs `f32 pad[4]`.
    - `func_800D1D4C_ovl_0006`'s point is `f32 t[4]` (x, y, z, w) where 3 were used.
92. **`a + b`, where b is a plain load and a is an expression, comes out as `add.s b, a`.**
    - `x*m00 + y*m10 + z*m20 + m[3][0]` gives `add.s m30, sum` (`func_800D1D4C_ovl_0006`).
    - A lerp written `(p1 - p0) * t + p0` gives `add.s p0, prod` (`func_800D2B0C_ovl_0006`).
    Write the load last in the source to get it first in the add.
93. **A parameter reused for a result puts the result in its register.**
    `sx = 1.0f / sx; m[0][0] *= sx;` matches. `inv = 1.0f / sx` leaves sx in f12 and puts the
    quotient in f0 (`func_800D0F64_ovl_0006`).
94. **`!(x < k)` compiles to `c.lt` plus `bc1t`; `x >= k` compiles to a reversed `c.le`.**
    `func_800D23CC_ovl_0006` (atan2) has `c.lt.d |x|, 1e-6` and branches on true to the tiny
    case, which is placed after the normal case. Only `if (!(fabsf(x) < 1e-6))` reproduces it.
    Writing the tiny case first changes the block layout.
95. **The order of two initialisations can swap the double registers.**
    In `func_800D22C8_ovl_0006` (atan), `acc = 0.0; dx = x;` puts dx in f14 and acc in f2, as
    the original has. `dx = x; acc = 0.0;` gives the reverse.
    In the same function, `2 * n` stays a `mul.d` by a loaded 2.0 (rule 76), while `2.0 * n`
    becomes `add.d n, n`.
96. **A switch's case bodies come out in source order, and each `return 0;` keeps its own
    block (`b end` with `move v0,zero` in the delay slot).**
    - In `func_800D40FC_ovl_0006` (the default task handler) the blocks are in this order:
      default (which also takes 6 and 9), then 4/5/7/8, then 1/2, then 0/3/10.
    - The source matches only with `default: return 0;` written first, the other groups in
      that order, and a separate `return 0;` in each.
    - `break` in the empty groups merges them into the end, and the blocks disappear.
97. **Several early returns sharing one `return 0` block are one condition joined by `||`.**
    `func_800D41AC_ovl_0006` has `obj == NULL || !(flags & 1) || (flags & 2)`. Written as
    three ifs (one with a dead print), they give three blocks. A dead print's string then has
    to go in another function: the strings' order in .rodata is all that fixes where a dead
    print goes.
98. **Two saved registers holding the same pointer (`move s0,a0` and `move s1,a0`) are two
    variables in the source.** In `func_800D51B8_ovl_0006` (frmDestroy) the loop reads the
    argument through s0 and the loop test through s1. It matches only as
    `Frame *self = f; ... while (self->child != NULL) func_800D51B8(f->child);`.
    The reverse also happens: IDO sometimes makes a copy the source never had
    (`func_800D4C24_ovl_0006`, parked). A `move` between two argument registers (`move a1,a2`
    for a parent in `func_800D4F98_ovl_0006`) is the same pattern, still unsolved.
99. **IDO's unroll factor depends on the loop body's size before optimisation, not on the
    code it emits.** `func_800D5390_ovl_0006` has 25 kinds. The original is unrolled by 2, with
    kind 0 peeled off. Every C form tried here unrolls it by 4, with identical bodies. Park
    these; don't burn time.
100. **One splat `asm` block can be two source files.**
     - The sign: its functions' .rodata restarts. Strings, then floats, then zero padding to 16
       bytes, then strings again. Each .o has its strings first and its floats (late rodata)
       last.
     - The text boundary is a function starting on a 16-byte boundary right after the first
       file's last function.
     - Split both, text and rodata. At 0x590F90 (overlay 6), polygon.c ends and camlight.c
       starts at 0x800D7830. camlight.c's "camera"/"light"/"ambi" strings start at
       0x8010D020, after polygon.c's floats and three zero words.
101. **A call argument that the caller does not truncate means the callee's prototype in that
     file has a wider type.** camlight.c passes a kind's count straight through
     (`move a3,a0`) to frmNewKind. frmNewKind truncates its s16 arguments itself, so
     camlight.c's prototype has s32 (`func_800D81BC_ovl_0006`). With s16, the caller stores
     the value and reloads it with `lh`.
102. **An s16 loop counter keeps a 4-trip loop rolled.** IDO sign-extends the counter every
     trip (`sll/sra 16`), and the loop stays a loop. `func_800D8690_ovl_0006` and
     `func_800D878C_ovl_0006` match only with `s16 i`; with `s32 i` the loop is unrolled. In
     the first, the s16 declared before a `u8` local also puts that byte at the original's
     stack offset.
103. **A pointer into a global's work block that IDO did not fold (`addiu v0,v0,0x20`, then
     small offsets) is a local assigned from `(u32) obj + sizeof(Obj)` (rule 72).**
     `func_800D8F54`, `func_800D8F68` and `func_800D9144` match only with
     `DemoWork *w = ...; w->mode = ...`. Writing the field straight off the macro folds the 0x20
     into the store.
104. **`lui` + `addiu` for an address is a symbol; `lui` + `ori` is a constant.**
     - stage.c's scene changes store TLB addresses of handlers in files (0x0800390C and
       others).
     - Written as `(void *) 0x0800390C` they give `ori`. The original has `addiu`, so they are
       symbols. splat already defines them (`D_800390C = 0x800390C` in
       undefined_syms_auto.txt); declare them `extern u8 D_800390C[];`.
105. **A parked function still needs its prototype in the assembly build.** In the `#else`
     branch its C definition is gone. A caller below it then gets an implicit declaration:
     float arguments are promoted to double, and the caller grows.
     - `func_800DBEB8` grew by 0x18 bytes in the full build and moved overlay 6's data.
     - The first differing byte was in main, at an `addiu` of a data symbol.
     - `check` passed, because it compiles each function with the C definitions present.
     - Put a prototype above every `#ifdef NON_MATCHING` block for each function defined in
       it.
106. **A parameter the original never truncates is s32 in the original, whatever its field.**
     stage.c's stage numbers, angles and s8 flags are stored with `sh`/`sb` straight from
     `a1`. Declared s16/s8, IDO 7.1 adds `sll`/`sra` at entry. Declare them s32.
107. **`li a2,0` (`addiu a2,zero,0`) as an argument is `0.0f` in an integer register; `move a2,zero`
     is the integer 0.** A float that is not the first argument goes in an a-register, and the
     callee homes it with `sw`. gauge.c's clock `func_800DDB78` has an unused f32 third
     parameter. `func_800DDFC8` passes it `0.0f`; passed `0`, it gave `move`.
108. **A lui/lw of `sym+4k` for each element, with a counter folded to constants (`move v0,zero`
     … `li v0,1`), is a 4-trip loop IDO unrolled.** Written as four `if`s on the elements, IDO
     loads the array's base once into a register and indexes it (34 lines differ). As
     `for (i = 0; i < 4; i++)` it matches. gauge.c's `func_800DE5CC` (the n-th player in play)
     and `func_800DE750`.
109. **`for (i = 0, n = 0; …)` puts `move n,zero` after the loop's address setup; `n = 0;`
     before the loop puts it first.** gauge.c's `func_800DE66C`.
110. **`multu` by 10 for an array index means the constant 10 is already in a register.** That
     happens when 10 is used twice: two indexes (`func_800DEA20`: `[player ^ 1]` and `[player]`)
     or an index and a `div /= 10` (`func_800DDCE8`). With one use, IDO does `sll`/`addu`.
     - The loads go in the order IDO computes the operands. `a[player] + a[player ^ 1]` loads
       `[player ^ 1]` first.
111. **Three floats copied with integer `lw`/`sw`, interleaved, then the source pointer
     reloaded, are one struct assignment.** Three f32 assignments use `lwc1`/`swc1`. In cambg.c's
     handler, the camera's eye is copied into the work as a `Vec3f`
     (`w->eye = *(Vec3f *) view->eye`): 380 differing lines went to 149.
112. **A work pointer set once (`w = WORK(obj)`) keeps `addiu v0,a0,0x20` and small offsets.
     `WORK(obj)->field` folds 0x20 into every offset.**
     - If the original recomputes `addiu v0,a0,0x20` in both arms of an `if`, the source set `w`
       once before it. IDO copies the add into the branch's delay slot and into the fall-through
       arm. Examples: cambg.c's `func_800DF65C`, `func_800DF694` and `func_800DF71C`.
     - In a handler whose every case computes `obj + 0x20` itself, `w` is set before the
       `switch` (cambg.c's `func_800DECE0`: 149 differing lines went to 38).
113. **`D.time = w->to = expr;` and `w->to = expr; D.time = w->to;` allocate differently.** In
     cambg.c's `func_800DF424`, the chained form put the global's address in v1 and the sum in
     v0. The original's (address in v0, sum in t2) is the two-statement form.
114. **Inside a `switch` case, `if (a) {...; break;} if (b) {...}` and `if (a) {...} else if (b)
     {...}` compile differently.** The original reloads the switched value in the delay slot of
     the early `break`'s branch (`b end; lh v1,..`). The else-if chain is one instruction short.
     In pause.c's `func_800E0514`, 45 differing lines went to 14.
     - In the same function, wrapping the `switch` in `if (in play) {...}` instead of
       `if (!in play) continue;` allocated the constants -1 and 4 the other way round
       (14 lines to 6).
     - Loading `old = w->cursor` after `w->shown = 1` put its spill store last (6 lines to 0).
115. **A compiled-out `PRINTF` (`if (0) osSyncPrintf(...)`) still moves code.** In geography.c's
     handler (`func_800E08D0`), a PRINTF right after `obj->name = ...` kept a `lui/addiu` of
     `D_80172F08` from being scheduled up past the task check. Moved to the end of the case, it
     matches (14 differing lines to relocations only). Where a dead print goes is part of the
     match.
     - A marked FAKE block of dead prints at the very end of a leaf function
       (`func_800E0B58`) changed nothing there. It carries the strings of the functions still
       in assembly.
116. **A lone `u16` load tested, then kept (`lhu t6; beqz t6; andi s1,t6,0xFFFF`), is
     `for (t = list; *t != 0; t++) { ...*t... }`.** With a `u16 id = *t` variable, IDO allocated
     every register differently. The same function (`func_800E2100`) had its first parameter
     reused for the result: `geo = f((u32) geo)`, one register for both.
117. **A struct local's place follows the locals declared around it.** `func_800E1F90` needed
     `GeoWork *w; s32 pad; StageData d;` for `d` at 0x28.
     - `func_800E1C84` needed two locals declared after its six sin/cos values for its frame
       (0x50).
     - `x / 256.0f` became a multiply there too (rule 48). `(f32) s / 256` kept the `div.s`.
118. **Every declared local takes a stack slot, even one only ever in a register, in
     declaration order from the top.** vram.c's `func_800E2F0C` needed its two `Vec3f`s
     declared first (the top of the frame) and no `dx`/`dz` locals (8 bytes less).
119. **A value read from an address-taken local after a store through a pointer parameter is
     read again; copy it into a named local first to keep it in a register.** In
     `func_800E3158`, `nz = v.x;` before the two `*x +=`/`*z +=` stores made the match. With
     `v.x` written out, IDO reloads it after `*x` is stored.
     - A course check written as `w->rails[n].field` each time (no `Rail *r`) kept
       `w + n*16` in one register with offsets, as the original has (`func_800E3474`,
       `func_800E3540`).
120. **A handler whose dispatch starts with `beqz msg` has an explicit empty `case 0:`.**
     `func_800E2514`, "VRAM".
121. **A float register held for 0.0 across a run of stores (`mtc1 zero,f2` once) is one
     chained assignment.** mathutil's identity matrix, `func_800D15B8`, matched as
     `m[3][2] = m[3][1] = ... = m[0][1] = 0.0f;` (found by the permuter, then written
     plainly).
     - In ride.c's `func_800E5EE0`, putting `D.t = 2.0f` before the chained zeros gave 2.0
       the lower register, as the original has.
122. **`b[i].field` through a local `b = d->boxes` keeps two pointers, one for the box and
     one for its inner matrix (`addiu s2,s0,0x28`).** `d->boxes[i]` or `b++` did not
     (`func_800E5148`).
123. **A struct local declared after another local lands below it. To put 8 bytes above a
     0x70-byte struct, declare two words before it** (`Obj *obj; s32 pad; ScArg a;` in
     scenario.c's `func_800E78E8`). An unused `s32 pad[2]` array took no slot where a scalar
     does (`func_800E6F60`).
124. **When a parked function's `.s` carries the file's strings, prints in the matching C
     functions go under `#ifdef NON_MATCHING`,** or the strings are doubled. In scenario.c
     all the string blocks sit in the parked runners' assembly.
125. **A char parameter homed as a word and used untruncated (`sw a1,4(sp)`, then `sb a1`) is
     an old-style (K&R) definition:** `void f(win, state) Obj *win; u8 state; { ... }`. Its
     earlier declaration has to be non-prototype (`void f();`); an ANSI prototype makes IDO
     warn (642) and truncate. Proved on window.c's `func_800E94D0`, `func_800E94DC` and
     `func_800E94E8`.
126. **A spilled pointer's slot and the frame size are set separately by the pads.** In
     string.c's `func_800E9A44` an unused `s32 pad[2]` after `w` grew the frame by 8 without
     moving `w`'s spill (0x28). A scalar `s32 pad`, before or after `w`, moved the spill to
     0x2C. In `func_800E9800` a scalar pad before `w` moved the spill 0x20 → 0x24 and the
     frame 0x30 → 0x38, as the original. So an unused array does take a slot here (compare
     123).
     - `StrWork *w = STR(obj);` at the declaration keeps the `addiu v1,a0,0x20` (and its
       spill). The same assignment as a statement after an early `return` was folded into
       `a0` offsets (`func_800E9A44`, `func_800E9D1C`).
127. **Two locals of `(u32) obj + 0x20` side by side (`s2`, `s3 = s2`) come from one
     `w = STR(obj)` used for both the array and the count in a `for`** (string.c's
     `func_800E9D84`, `func_800EA07C`). IDO copies it for the loop test.
128. **`beql s2,v0` (the held constant first) against a `f() == TRUE` test means the callee
     returns `u8`.** With an `s32` return the branch is `beql v0,s2`, and `TRUE == f()` does
     not flip it. Six list walkers in menu.c (`func_800EA1E8`-`func_800EA52C`) matched when
     `func_800EAA98`/`func_800EAA6C` were typed `u8`. This may be what rule 70's FAKE
     `(x ^ 0)` stood for.
129. **`div.s` by 8.0 is `/ 8` (an int); `/ 8.0f` becomes `mul.s` by 0.125.** IDO turns a
     float literal that is a power of two into its reciprocal, but not a converted int.
     font.c's `func_800EB998`.
     - The same function converts a `u8` table entry with the unsigned fix-up (`bgez`, then
       `+ 4294967296.0f`) only when it is cast straight to float:
       `(f32) D_8010AA80[i] * f->w`.
130. **A parameter's register numbered before the constants' (`t0` for the stacked `text`,
     then `t1`-`t4` for 8, 12, 1, 2), with its store last, means its assignment comes before
     the constants' in the source.** The scheduler sinks the store. font.c's
     `func_800EB828`: `f->text = text;` after x and y, before the cell sizes.
131. **The character loops:**
     - With `u8 c`, the loop takes an extra register. `for (; *p != '\0'; p++)` with `*p`
       read twice matched (`func_800EBAD4`).
     - `andi a3,a2,0xFF` before `- 0x20` is `s32 c` with `(u8) c - ' '` (`func_800EB998`).
     - Its skip loop still wanted the `'#'` register first. Rule 70's FAKE `(c ^ 0)` fixed
       that.
132. **A handler that keeps obj in its home slot (`sw a0,0x20(sp)`, then `lw s0,0x20(sp);
     addiu s0,s0,0x20` or `lw v1,..` in each case) declares its work pointer with an
     initialiser:** `FaceWork *w = FACE(obj);`.
     - IDO sinks the addiu into each path and folds it where one field is read (`lw a0,0x30(t3)`).
     - Assigning `w = FACE(obj);` inside the cases instead keeps obj in a register, and the
       offsets fold.
     - Proved on face.c's `func_800EBB50` and menu.c's `func_800EAC6C` (with one `s32 pad` for
       the frame). camera.c's `func_800C8AC8` already had this form.
     - A `for` whose test is a comma expression duplicates the test at the bottom; the original
       menu handler's `while (TRUE)` + `break` has one test (rule 25, `func_800EA5A4`).
133. **`(D[k] & 0xFFFFFF) + f(...)` gives `addu t8,v0,t7`** (the call's result first).
     `f(...) + (D[k] & 0xFFFFFF)` gives it the other way round and moved the spill slot too
     (face.c's `func_800EBD2C`).
134. **A spill one word too high (`sw v0,0x1C(sp)` for `0x18`) under rule 132 wants `s32 pad;`
     declared before the work pointer.** scenario.c's two parents `func_800E6F60` and
     `func_800E7110`. The same pad after the pointer moved a spill up in string.c (rule 126).
     - An address a function fetches twice (0x08000040 in `func_800E6A00`) matched when the
       first use went through a named local assigned one statement early (the permuter's
       find). Without it IDO kept the address in a stack slot and grew the frame by 8.
135. **A clear loop `p = &A; p != &B; p++` with one store per step and no unrolling is an index
     loop over an array whose end is the next symbol:** `for (i = 0; i < 1; i++) A[i] = 0;`
     with `u8 A[1]` (swwork.c's `func_800EBF30`), or `< 100` for the 4-unrolled one
     (`func_800EBF54`). Written as a pointer loop, IDO unrolls it by 4 with a remainder loop.
     - swwork.c's `func_800EBF84` put its stack local `base` below the struct only when
       declared after the two `u16` register locals (`r, byte, i, base`). Register locals
       still take their slots in declaration order (compare rule 123).
136. **Texture coordinates were written in texels `<< 5`:** `(blink & 1) * 16 << 5`, not
     `(blink & 1) << 9`. The instructions come out the same, but the temporaries' numbers
     only match the first way. msgfrm.c's `func_800EC7CC`; also `(blink & 4) * 4 << 5,
     (blink & 3) * 16 << 5` in `func_800EC4F4`.
137. **Array access through a 1-based player number, three signs from sccheck.c:**
     - **`li a3,10; multu` twice, and `addiu v0,a1,-1` in the prologue:** the index `pl - 1`
       is a named local (`s32 i = pl - 1;`), used for the pointer (`p = &D.unk7A[i]`). The
       first test is written `D.unk7A[pl - 1].unk0`; IDO folds the -1 into the offset (0x70)
       and multiplies pl itself. Written with `pl - 1` both times, IDO folds both into one
       `sll` chain (`func_800EE248`, `func_800EE150`, `func_800EE3A0`, `func_800EE210`,
       `func_800EE2EC`, `func_800EE34C`).
     - **A getter whose `lui` takes v1 while the value goes to v0** (`lui v1,..; lhu v0,..(v1)`)
       returns a named local: `u16 v = D.unk76; return v;`. The same `s32 v = ...; *p = v;`
       moved the value into v0 for a store (`func_800ED56C`, `func_800ED300`, `func_800EDA7C`).
     - **A 7- or 6-bit count** with the first bits as constants, then `(i % 8)` shifts from a
       base of 4 or 3, is `for (i = 1; i < 8; i++) if (x & (u8) (1 << (u8) (i % 8))) n++;`.
       IDO peels the remainder and unrolls by 4. The byte is read in the test, not through a
       local (`func_800ED1A0`, `func_800ED440`).
     - Six calls with constants 30-35 were written out; a loop over calls stays a loop
       (`func_800ED37C`).
138. **Named locals decide the registers in small readers** (aniatk.c):
     - `u32 w = *h;` before using the word puts the pointer in a0 and the word in v1, where
       `*h` in the expression put the pointer in v1 (`func_800F1B28`, `func_800F1B6C`,
       `func_800F1BE4`).
     - `h++;` (or `h += 2;`) before an `if` whose branches both read `*h` is the
       `addiu v1,v1,4` in the branch's delay slot. `h[1]` in each branch folds the 4 into
       the loads (`func_800F1C4C`, `func_800F1CE4`).
     - One return variable assigned in both branches gives the result in a0 and a final
       `move v0,a0` (`func_800F1CE4`).
     - A float copy through a named local (`f32 t = p->time; f->time = t;`) uses $f0; the
       direct assignment uses $f4 (`func_800F1890`). The same function's `link` is `s32` (no
       `andi`, no home store), though the caller loads a byte.
     - A loop that ends `if (i + 1 < n) { i++; continue; } return first;` gives
       `beql at,zero` to the return; `if (i + 1 >= n) return first; i++;` gives
       `bnez` back (`func_800F18AC`).
139. **Store order and struct temps in the effect creators** (effect.c):
     - Three zero stores written separately (`v[0] = 0.0f; v[1] = 0.0f; v[2] = 0.0f;`) keep
       that order. The chain `v[2] = v[1] = v[0] = 0.0f` let the scheduler move `v[0]` after
       the others (`func_800F2C00`).
     - Of two float fields set in a row, the one assigned first takes the lower register:
       `pos[2] = z;` before `scale = 0.3f;` puts z in $f4 and the constant in $f6.
     - Two float constants stored one after the other keep their `lui`/`lwc1` pairs together;
       with other stores between them, IDO sinks the second pair to the end
       (`func_800F2D78`: `scale`, then `grow`, then the rest).
     - A call to a function that returns a struct, with the result unused, still takes a
       temp of the struct's size on the stack (`FileSet func_800CE130(s32)`: 8 bytes at
       sp+0x2C in `func_800F26C0`). The frame also needed `s32 pad[2]`.
     - `add.s f, x, x` is `x + x`, not `2.0f * x` (`func_800F4358`).
     - A switch's case bodies are laid out in source order, whatever the order of the tests:
       `case 2` before `case 1` (`func_800F26C0`).
140. **Chains, param reloads and a dead store** (itex.c):
     - `c.b = c.g = c.r = K` loads the constant once per field, r first (t4, t5, t0), and
       stores b, g, r. Separate statements load and store in source order
       (`func_800F4FF0`, `func_800F52B8`). The original also does "loaded r, g, b, stored
       b, g, r" with DIFFERENT constants (0xFF, 0xFF, 0x80). No order of separate statements
       and no partial chain gave that: four creators are parked on it (`func_800F4D3C`).
     - A float param that arrived in a0-a3 (so it lives in its home) and is stored to three
       fields in a row is reloaded for each: three `lwc1 x,0x7C(sp)` into $f6, $f8, $f10. The
       chain `s[0] = s[1] = s[2] = p` reloads from `s[2]` instead. When the stores come
       right after a statement whose value is in a register, the load is shared (`func_800F497C`).
     - `if (a->n == 0) { w->n = f(); } else { w->n = a->n; }` gives `bnel v0,zero` with the
       store in the delay slot, a `b` with the store in its slot, and a third, dead copy of
       the store. A ternary gives `beqzl` (`func_800F44FC`).
     - `-(a + b - 1) / b` is `addu`, `li 1`, `subu`, `div`. `(1 - (a + b)) / b` was
       re-associated into `negu`, `subu`, `addiu 1` (`func_800F44FC`).
     - Two 3-byte colour structs copied with `w->prim = init->prim;` give the original's
       `lbu`/`sb` register pattern (t7, t6, t7, t9, t8, t9). Six field copies did not.
141. **Handlers and creators in particle.c:**
     - **The work pointer is set once, at the top:** `FireWork *w = FX(obj);`. Setting it
       inside each case gave 361 differing lines; at the top the whole 0x4B4 handler matched
       (`func_800F5C54`). Where the original loads `obj` in the dispatch's delay slot and adds
       0x20 in the case, it is still this.
     - **Index arrays of structs directly:** `w->p[i].x`, not `p = &w->p[i]`. The pointer put
       the struct base in its own register (offsets 0-4, not 9-0xD) (`func_800F53B0`).
     - **Sums of two loads come out reversed:**
       - `x + vx` gives `lb vx; lb x; addu v1,vx,x`;
       - `(x >> 2) + ((y >> 2) << 6)` gives `addu y_part, x_part`.
       Write the operands in the other order from the asm.
     - **A float product stored twice is a named local:** `s = 0.02f * size; a.sx = s;
       a.sy = s;`. This puts it in $f0 and the 0.0f in $f2, and the local takes the frame's
       spare 8 bytes, so no pad (`func_800F71E0`, `func_800F72D4`).
     - **Chains store in different orders:**
       - a float 0.0f chain `x = y = z = 0.0f` stores z first;
       - `sy = sx = v` stored sy first;
       - a byte chain `b = g = r = 0xFF` stores b first (IDO_NOTES 140).
     - **Loading one field early:** `w->time = time;` before the colour stores put the time
       param's `lbu` ahead of them, as the original has it (`func_800F68A4`).
     - **A 0xC0 frame with locals from 0x8C** needed `s32 pad[3]` declared before the first
       float local (`func_800F62A0`, the rain handler).
     - **Clamps on u8 params** are written as they read: `if (x >= 64) x = 63; if (x <= 0)
       x = 0;`. IDO keeps a copy of x in v0 for the second test. `(f32)` of those u8 values
       gives the unsigned fix-up (rule 129) (`func_800F66B0`).
142. **Effect handlers** (effect.c):
     - **The work pointer at the top holds for every handler in these files.**
       - A `w = FX(obj)` set inside each case kept `obj` in s1 (puff, pompom).
       - It also folded `w` into offsets from `obj` (star).
       - The original reloads `obj` from its home, or keeps it in s1, and forms `w` again in
         each case. That is what the top-level variable gives (`func_800F27F8`,
         `func_800F3678`, `func_800F3B3C`).
     - **Colours in a macro:**
       - `u8 color[3]` fields in `gDPSetEnvColor` were loaded b, r, g;
       - `Color3` fields load r, g, b, as the original does (`func_800F3E08`).
     - **Float locals that are spilled across calls** (x, y, z around three random calls)
       took the original's slots only when declared before the pointers, with one scalar `s32
       pad;` after them (`func_800F3678`).
     - **The value a copy uses:** `w->timer = w->interval;`, written after the stores it
       follows, used the loaded `init->interval` (no reload). `w->timer = init->interval`
       reloaded it (`func_800F3678`).
     - **A chain whose source is a register** stores it everywhere without a reload:
       `f->scale[1] = f->scale[2] = f->scale[0];`. Separate statements reloaded `scale[0]`
       (`func_800F31C4`).
     - **Three calls through `(*f)()` in one expression** share one saved copy of the address.
       The original's slots for their results were not reproduced (`func_800F3B3C`, parked).
143. **zanzou.c:**
     - **`!x` and `x == 0` compile the same `sltiu`, but take different registers.** The
       original's `w->vtx[!w->buf]` / `w->buf = !w->buf` used t2; `== 0` used t7 and t5
       (`func_800F785C`).
     - **A triangle-strip loop's variable starts at 1.** `for (i = 1; i < n - 2; i += 2)` with
       `(i - 1, i + 1, i, i, i + 2, i + 1)` compares `2(i+1)` against `2n − 4`, as the original
       does. `i = 0; i < n - 3` compared `2(i+3)` (`func_800F8D08`).
     - **Comma inits set the delay slots:** `for (i = 0, k = 5; ...)` put `k = 5` in the
       `blez` delay slot and `i = 0` in the outer loop's latch. `k = 5;` before the loop did
       not (`func_800F785C`).
     - **Calls through `(*f)()` in a loop:** IDO keeps the address in a saved register
       (guMtxXFMF in s2), or in a stack slot when used across the whole function (`D_8018B144`
       at 0x60) (`func_800F85A0`).
     - **Locals with homes are not simply laid out in declaration order.** In
       `func_800F85A0`, scalars that are spilled (action, yaw, turn) sit between the
       address-taken ones with 4-byte gaps. Pads moved them, but not to the original's slots.
     - **An unused scalar `s32 pad;` moved one temp spill by 4** and matched
       (`func_800F8418`).
144. **pakfiler.c and the permuter's itex/effect/acoll finds:**
     - **Same-line statements fix colour-byte order.** `init.prim.r = 0xFF; init.prim.g = 0xFF;
       init.prim.b = 0x80;` on one line, and the env triple likewise, stores b, g, r with the
       constants loaded r, g, b, as the original does. On separate lines no order matched. All
       four itex makers (`func_800F4D3C`, `F4F00`, `F50DC`, `F51BC`) matched this way; found by
       decomp-permuter on F50DC. It is rule 12 again, for byte stores.
     - **The game's debug print compiles to its arguments.** `#define DPRINTF` (empty) makes
       `DPRINTF("%c", f(c));` into `("%c", f(c));`: the string goes to .rodata and `f` is still
       called. The list calls the character translator twice per character (`func_801746F4`).
       The repo's `PRINTF` (`if (0) osSyncPrintf`) emits the string but drops the call.
     - **Called with two arguments and with three in one file ⇒ no prototype.** pakfiler.c calls
       `func_800D8CF8` both ways. `u16 func_800D8CF8_ovl_0006();` compiles both.
     - **A void callee whose v0 the caller reads.** The list stores `func_800027B0`'s "result"
       (void in contpak.c; its v0 still holds the error it stored). Declare it `s32` in the
       caller's file.
     - **`s32` colour locals with `(u8)` casts** give `andi` then `sw` to a stack slot. `u8`
       locals give `sb` (`func_801750B0`).
     - **`(f32) timer / 30.0`** gives `cvt.s.w` then `cvt.d.s`. `timer / 30.0` gives `cvt.d.w`
       (`func_801750B0`).
     - **A character from a `char` field (`-signed`):** a `u8 c` local loads with `lbu` straight
       into its saved register. `s32 c = (u8) ...` gives `lbu` into a temp plus an `andi` copy
       (`func_801746F4`).
     - **Address-taken locals declared after the register locals sit lower.** Declaring
       `pattern` and `pages` after `i, c, j, p` moved them from 0x7B/0x74 to 0x6B/0x64 (the
       original's), with the frame unchanged. A pad declared before them grew the frame instead
       (`func_801746F4`).
     - **A terminator written after a fill loop with the loop's index**
       (`D_8017ECD0[i][j] = '\0'` after `for (j...)`) reuses the fill pointer (`func_80174E5C`).
     - **FAKE argument copies:** `px = x;` before `if (obj)` and `s = size;` inside it kept the
       splash maker's ring loads based on obj (`func_800F4358`, decomp-permuter).
     - **A FAKE empty `if (dmgs) {}`** between two early returns gave the attack walk its
       saved-register order (`func_800F01DC`, decomp-permuter).
     - **`(u8) w->alpha` of an `s32` field in memory loads its low byte** (`lbu 0x17` for the
       word at 0x14), and in `gDPSetPrimColor` it keeps the ORs in r, g, b, a order. A bare
       `w->alpha` gave `lw` + `andi`. `((u8 *) &w->alpha)[3]` and a union's byte gave the
       `lbu`, but the ORs came out in the order a, r, g, b (`func_800FA26C`).
     - **Named difference temps decide a loop's unrolling and the sum's shape.** `dx = ...;
       dy = ...; dz = ...; sqrtf(dx * dx + dy * dy + dz * dz)` unrolled the running-distance loop
       by 2 with `(dx² + dy²) + dz²`, as the original does. Inline differences left it rolled and
       summed `dx² + (dy² + dz²)`. The running sum is `ta[i - 1] + sqrtf(...)`
       (`func_800F9ECC`).
145. **plchara.c (overlay 9's first file):**
     - **A failing test that `break`s to the function's one final `return FALSE`** compiles
       as a branch-likely straight to the epilogue with `v0 = 0` in its delay slot
       (`beql t7, zero, END; or v0, zero, zero`). An inline `return FALSE` gives
       `b END; or v0, zero, zero` instead. Successes stay inline (`return TRUE`), and the
       `switch` has no default (`func_8017418C`).
     - **`(u16)` of an `s32` field in memory loads its low half** (`lhu 0x2E` for the word
       at 0x2C) when the value is passed as a `u16` argument (`func_80173E18`). It is the same
       narrowing as the `u8` case in 144.
     - **A call's (or `sqrt`'s) result inline in `a + f()` or `a * f()` is put first** by
       IDO, whichever way round it is written. The original's `add.d f6, f4, f0` (c + root)
       and `mul.s f10, f8, f0` ((speed * -6) * cos) came out only with the result in a named
       local: `r = sqrt(k); (c + r)`, `c = cos(a); speed * -6.0f * c` (`func_801738A0`,
       `func_801743D0`).
     - **The same goes for a division's result:** `v[0] = x * (max / d)` put the quotient
       first; `k = max / d; v[0] = x * k;` matched (`func_801764C0`, `func_80176510`).
     - **A `FALSE` kept in v1** (`func_801751B0`) is a ternary return; see 146.
146. **Overlay 9's pllimit.c, plchain.c and plonigiri.c:**
     - **A file's text boundary without nop padding.** Files start on 16 bytes, so a .rodata
       boundary (strings aligned to 16 after the last file's late .rodata) places the text
       boundary at the only 16-byte function start that fits. 0x5D7C40 became three files.
     - **Late .rodata that no instruction names** (0.6f and 0.85f after plchain's jump table)
       came out of splat as ordinary .rodata at the file's end, shifting every float after
       it. Typing them `type:f32` in symbol_addrs.txt put them back in the late .rodata.
     - **splat attaches a file's unreferenced strings to the function before them** (the
       "__plOnigiri" prints went into `func_80176FF8`'s .s). The C function that owns them
       prints them only under `NON_MATCHING` until that function is C. Printed twice, the
       strings shift the late .rodata.
     - **A task-init struct of 0x18 bytes, not 0x14,** gave `func_801751E0` its frame; one
       `s32 pad;` placed `func_801758E4`'s spill.
     - **A global vector read as `v.x, v.y, v.z`** (or `v[0..2]`) after its address was
       passed to calls comes out with the address CSE'd into a register (one instruction
       more). The original reads it through one `lui` with three `%lo`s. Open
       (`func_80177290`).
     - **Open: an empty branch on a parameter.** `func_80176540` is `beqz a1, next`. An empty
       if, an empty switch and `if (msg == 0) return;` all compile to nothing.
     - **`return c ? A : B;` keeps B in v1** and returns A in line
       (`li v1, B; ...; jr ra; li v0, A; ...; jr ra; move v0, v1`). `if (c) return A; return
       B;` folds B into v0. It settled the "FALSE kept in v1" question of 145
       (`func_80179764`, `func_801792C4`, `func_801751B0`).
     - **The game's bug in `func_80177A48`:** `if (w->puff == NULL)` tests the array (w + 4),
       compiled as `bne s6, -4`.
147. **demopl.c (the demo player, 57 of 58 on the first full pass):**
     - **A switch's case bodies are laid out in source order;** the compares are sorted. The
       demo actions compare 1 then 6 but lay out 6's body first, so the source has `case 6`
       before `case 1`.
     - **A reply that several cases share sits after the inner switch.** When a case falls
       through to a jump that lands after the other cases, the shared code is not a
       `default:` in the middle; it follows the switch, and the failing case `break`s to it.
     - **Comparisons of two loads come out reversed, like sums (rule 30).** `p->facing !=
       p->unk0F0` loaded 0xF0 first; `p->unk0F0 != p->facing` matched. The same for
       `(f32) p->phase == p->unk1E4 - 1.0f`.
     - **A field read after a call is re-read, not kept:** the original reads `p->unk098`
       twice after `func_801897F8`, so no local holds it. A store to a work field between two
       reads makes IDO read again, so the source assigns `w->dir = p->unk098` before
       `w->t = 0` (`func_80179ED8`).
     - **Check the constant's last bit:** the .rodata held 1.0200001 (0x3F828F5D), not 1.02f.
       The build found it at 0x617E87.
     - **Message tags are EUC-JP.** Constants such as 0xBEC3CCC7 are two kanji read as a word
       (消滅). `iconv -f EUC-JP` decodes them.
     - **Open: `b next; li a1, -1`.** The original sets `next = -1` in the delay slot of a
       `b` to the very next instruction. A plain if, a ternary and a one-case switch all set
       it in line (`func_80173F5C`).
148. **pldive.c (diving, 43 of 43):**
     - **`r = 1.0f;` / `r = a / b;` in an if/else, then `return r;`** gives the constant path
       its own `mtc1 at,$f0; jr ra` and the computed path `div.s $f12,..` then
       `mov.s $f0,$f12`. Two `return`s (or a local only for the division) put the division
       straight in f0 (`func_8017B6CC`).
     - **A table copied into a local** (`s32 v[3]`, `lw/sw` pairs through `addiu t6,..`) is a
       struct copy from an extern: `PlDiveTbl tbl = D_801A729C;`. No data migration is needed.
       Its slot follows declaration order: first declared sits highest (`func_8017BBF0` needed
       `s32 t; s32 next;` before it).
     - **Rule 132 again:** the screw task keeps obj in its home and the work pointer in s0 per
       case only with `PlScrew *w = SCREW(obj);` at the declaration. Assigned inside the cases,
       w was spilled; with the macro at every use, obj went into s0 (`func_8017BFCC`). The
       out-params x, y, z at 0x48/0x44/0x40 then needed one `s32 pad` declared before them.
     - **Last bits again:** .rodata held 0.010000001 (0x3C23D70B) for three thresholds beside a
       true 0.01f, and pi/24 was 0x3E060A91, one under 0.1308997f. The build found the first
       at 0x617F1F.
     - **Statement order under the delay slot:** `p->height -= ...; p->lean = 0.0f;` (not the
       other way) put the zero store after the height's load (`func_8017A69C`).
     - **A named `m = p->model` moved the model load off s0;** reading `p->model->pos[i]` three
       times matched (`func_8017A410`).
149. **diveef.c (the water effects, 8 of 10):**
     - **A chain's store order is not fixed; read it off the asm and try both orders.**
       Through a local frame pointer `f->scale[0] = f->scale[1] = f->scale[2] = K` stored 0x2C,
       0x30, 0x34 (`func_8017D19C`) and `[0] = [2] = [1] = 0.03f` stored 0x2C, 0x34, 0x30. But a
       computed value went right to left (`m->scale[0] = m->scale[2] = sin(..) * ..` stored 0x34
       first), and so did a constant on the parameter p in plbase.c (`p->unk098 = p->unk0C0 =
       0.0f` stores 0xC0 first, `func_8017D9E0`). Through a pointer re-read from memory
       (`w->frame->scale[..]`, pldive.c) the rightmost goes first and is read back.
     - **One local for "the model" in every case:** the bubble task keeps the player's model (case
       2) and its own frame (case 0) in s1. The same variable `m` in both cases matched; two
       names gave the frame s1 in case 2 and spilled the other (`func_8017C8EC`).
     - **`if (f()) { ...; break; } return -1;`** puts the failure's `b end; li v0,-1` after the
       case's code; `if (!f()) return -1;` puts it first.
     - **Rule 132 held for all three tasks** (`PlKira *w = FX(obj);` at the declaration). Two
       `s32 pad`s were needed where the frame was 8 or 16 bigger; locals take slots top down in
       declaration order, so a pad before a pointer moves that pointer's spill down.
     - **Store order before a loop:** `w->index = *arg;` first, then the two constants, made IDO
       materialise w before all three stores (`func_8017C450`).
     - **A task's parent in a named local** (`Obj *task = players[p->index];`) before an init
       struct's stores put the stores after the load and gave the frame its extra slot
       (`func_8017D4EC`).
     - **Render modes:** 0x00504240 is `G_RM_XLU_SURF, G_RM_XLU_SURF2` (no Z); 0x00504A50 is the
       ZB pair.
     - **A `u16` parameter only stored with `sh` is homed but not masked;** an `s16` one is
       sign-extended in place (`func_8017D81C`).
150. **plhoni.c (6 of 9):**
     - **A 0x40-byte struct copied into a local** is a loop moving 12 bytes a turn plus one
       word (`addiu t0,t7,0x3C` ... `bne t7,t0`): `HoniTbl tbl = D_801A7320;` with the table
       extern. The three combo actions still had 5 or 6 unused words above the table, matched
       with `s32 pad[5]` / `pad[6]` declared before it (`func_8017DC78`, `func_8017DD38`).
     - **Three floats copied as words (`lw/sw`) are a struct copy:** `Vec3f vel = *(Vec3f *)
       w->vel;`. Element by element gives `lwc1/swc1`.
     - **Unused slots can sit at the bottom too:** the honi task's frame matched from the top
       with pads above, then needed one more word declared last.
     - **`a + b` of a stack float and a loaded field:** `off[0] + p->model->pos[0]` gave the
       original's registers where `p->model->pos[0] + off[0]` did not.
     - **One slot is shared by two characters:** character 1 keeps two u16s at Player+0x284
       where the others keep a task pointer. A cast macro (`PL_HONI(p)`) reads them.
151. **ombupl.c (the piggyback, 87 of 88):**
     - **Jump-table switches lay out case bodies in source order too.** Moving `case 3` to the
       top of `func_8017F030` moved its code to the top.
     - **Dead prints whose text order contradicts the code:** both base actions' damage prints
       (case 3) come before their timeout prints (case 1) in .rodata, while case 3's code is
       last. Keep splat's `D_<addr>.s` of the strings as a pragma at the top of the file and
       name the prints in a comment. A `PRINTF` (an `if (0)`) can also move registers: one
       before an inner switch put the switch value in v0 instead of t4.
     - **Calls inline in an expression spill as temporaries:** `x * 0.1f * cosf(rot) - z * 0.1f
       * sinf(rot)` gave the original's slots (sinf's result at 0x2C); a named `s = sinf(rot)`
       took a slot of its own and grew the frame (`func_8017FAE4`).
     - **Nested ternaries for a call's argument** (`f(p, a() ? 3 : (b() ? 5 : 0))`) put the
       inner value in v0 and move it to a1, as the original does (`func_80180E98`).
     - **Rule 132 for Player.work:** `PlHop *w = (PlHop *) p->work;` at the declaration, plus
       two pads before the float out-param, matched the rider's bounce (`func_8017F540`).
       plonigiri.c's parked bounce (`func_80176558`) has the same shape.
     - **An unused copy of a counter check matched in one function and not another:** the
       `if (p->unk15C >= 4) p->unk15C = 3; p->unk15C--;` form that matched in the carrier's
       base also matched the rider's test once its early return became `if (!f()) {...}
       return TRUE;` (`func_80180B14`).
152. **plpick.c (the action pickers, 39 of 42):**
     - **Two calls, not one call with a ternary argument:** where the asm has two `jal`s to the
       same function with different constants, the source is `if (c) next = f(p, 3); else next
       = f(p, 0);` (`func_80181070`). Where it computes the value first and moves it to a1, it
       is one call with the ternary (IDO_NOTES 151).
     - **Trivial tests:** `if (x) return TRUE; return FALSE;` sets v0 = 0 first
       (`func_80181A78`); `return x ? TRUE : FALSE;` goes through v1 (`func_80181CD4`,
       `func_801820D0`). Read which register the zero lands in.
     - **The open `b`-to-next pattern (147) three more times:** `r = TRUE; if (!x) r = FALSE;
       return r;`-shaped tails where the original has `bc1f/bnez` + `nop` + `b next` with the
       assignment in the delay slot (`func_80181ADC`, `func_80181CF8`, ombupl.c's
       `func_80180DC4`). Nested ifs, a ternary and moving the initialiser after the call did
       not produce it.
     - **A PRINTF with arguments did not move registers here** (`func_801823B8`, "plTurn2"),
       unlike ombupl.c's.
     - **Loop constants in saved registers:** the original loads 4, 5, 8, 0x80000 into s3-s6
       in that order (the bound first); `for (i = 0; i < 4; i++)`, `i != 4` and a
       `while (TRUE)` with the test at the bottom all gave 5, 8, 0x80000, 4
       (`func_80182208`, parked).
153. **plact.c (the general actions, 62 of 63):**
     - **The count of differing lines hides relocation lines,** and with them a register on a
       `lui`/`addiu` of a symbol: `f(D_801A7B68, D_801A7B74)` against the original's
       `f(p, D_801A7B68, D_801A7B74)` counted 0 and failed the build (`func_801830D0`). Always
       build; when the build points into a "matching" function, read its relocation lines.
     - **An inner `switch` with `case -1`, `case 0`, `default`:** this C puts the default body
       right after the compares; the original jumps over the two cases to it (`b default`
       after `beqz`). An if-else chain was worse (`bne`); not solved (`func_80183E98`).
     - **Two-load comparisons reversed again** (147): `p->unk0F0 != p->facing` loads 0xFC
       first. Four functions matched only after flipping it.
154. **plmodel.c (the player's model, 6 of 9):**
     - **A print macro that is empty (`#define DPRINTF`) still evaluates its arguments:**
       `DPRINTF("...", a, b)` becomes the comma expression `("...", a, b)`, so a call in an
       argument still runs and the string still lands in .rodata. It is not the same as
       `PRINTF` (`if (0) osSyncPrintf`), which drops the calls.
     - **Per-character u16 tables read at `chara * 4` share one index only as a struct:**
       `PlHalf { u16 v; u16 next; } tbl[]` indexed by chara gave the shared `sll 2`; three
       separate `u16[]` arrays indexed by `chara * 2` did not.
155. **plmsg.c (the default message handler, 4 of 4):**
     - **A jump table's first index is the lowest `case`:** `sltiu at,a1,7` with no
       `addiu -1` means a `case 0:` exists (here an empty `break`), and a table starting at
       0x65 (`addiu -101`) means `case 0x65:`. Count the table's words against the cases.
     - **`li v0,1; sh v0,a; sh v0,b` is two stores of a constant**, `x = 1; y = 1;`. The
       chain `y = x = 1` reloads `x` (`lh a1,400(a0)`).
     - **The "`b` over a block, the assignment in the delay slot" layout of an if/else-if
       chain:** `bgez t0,call; li a1,48` (a non-likely branch with the last `else`'s value in
       its slot) is `} else if (f & 0x10000) { if (c) n = 0x5A; else n = 0x4D; } else { n =
       0x30; }`. Writing `n = 0x30;` first and the `if` inside gave the `li` before a `bne`
       instead (`func_80185798`).
156. **plmove.c (the player's movement, 23 of 41):**
     - **Locals are laid out from the bottom: the last declared is lowest.** A pad declared last
       moves every local up (the frame grows); a pad declared first only grows the frame. Most
       frame misses here were one pad: `s32 pad[2]` last (+8 for everything), `s32 pad0[2]`
       first (+8 above everything), `s32 pad` between two structs (a 4-byte hole). Scalar
       locals often get homes too: moving `f32 dx, dz` below four other scalars moved them
       16 down (`func_80188D4C`).
     - **m2c prints stores in the order they come out, not the source's.** IDO moves an
       independent store into a load's delay: `pos.y = a + b; kind = 0; flag = 0;` comes out
       as the two loads, `sh`, `sb`, `add.s`, `swc1`; written with `pos.y` last, the loads come
       after the stores. Same for `ray.dir.x = 0; ray.dir.y = ...; ray.dir.z = 0;` and for
       `ground; unk1A2 = g.unk6; unk1A8 = 0;` (the `sh zero` fills the `lbu`'s delay).
     - **A chained assignment across narrower fields reloads:** `g->unk4 = g->unk6 = g->unk8 =
       0` (s16, u8, u16) read the u16 back and truncated it; three statements matched.
     - **An angle kept in a local vs written twice:** `a = heading - pi/2; f(a); ... f(-a);`
       gave the frame 8 more bytes; writing `heading - 1.5707963f` both times lets IDO keep one
       spill slot, as the original has (`func_80187B30`).
     - **A struct copy of a u16-array global uses words** (`lw`/`sw`, or `lwl`/`lwr` through
       pointers); the original's halfword loop is an indexed loop, whose unrolled store order
       still differs (`func_80185B28`, parked).
     - **pi/2 is 1.5707963f (0x3FC90FDA) in all but one place** (`func_80187F90`: 1.5707964f,
       0x3FC90FDB); 0.2 is written 0.19999999f (0x3E4CCCCC) in the head and ceiling probes.
157. **plqueue.c and plutil.c (4 of 4, 52 of 57): the open "`b` to the next instruction with the
     assignment in its delay slot" is a ternary.**
     - `r = 0; if (c) r = f(); return r;` gives `jal; nop; move v1,v0` falling through; the
       original's `jal; nop; b next; move v1,v0` is `return c ? f() : 0;` (`func_80189218`).
       A ternary with a constant arm likewise keeps the 0 in a register from the top and
       returns it through `move v0,v1` (`func_801896F4`: `return (flags & 0x80) ? 1 : 0;`).
     - `if (c) return f(); return 0;` puts `move v0,zero` before the test and `b epilogue+4`
       with `lw ra` in the delay slot after the call (`func_801891D0`).
     - `if (a >= 0) { x = a; } else { return -1; } return a;` gives `move v0,a1; b end;
       sw` with the -1 return duplicated (`func_8018934C`).
     - A loop whose counter is bumped before the store: `for (i = 0; i < 3;) q[i++] = -1;`
       (`func_801890C0`); `ok = 1; if (q[2] != -1) ok = 0;` is `li v1,1` in a `beq`'s slot.
     - `x->old = x->cur; x->cur = next; ...` stores `old` last through a `t` register; a local
       `old` takes v0 instead (`func_801892C0`).
     - A record array read as `D.unk7A[i].field` everywhere addresses `base + i*10` with the
       field's full offset; through a `Unk88282 *s` pointer it adds 0x7A first. The damage
       function reads it directly each time (`func_80189D78`, parked on registers).
     - Two-load adds reversed again (147): `out->x = b->x + a->x` loads b first.
158. **plmain.c (the player's task, 29 of 35) and two permuter finds:**
     - **Four compares against `players[0..3]` with absolute `lui/lw %hi(D+0x68+4k)` each and a
       return per hit are a `for (i = 0; i < 4; i++)` loop that IDO unrolled fully**; four
       written-out `if`s CSE the base into a register instead (`func_8018B27C`, `func_8018B01C`).
     - **A pointer compared against a table's second entry through its own symbol** (`lui
       %hi(D_801A87A4)`, not the table's base plus 4): splat names 0x801A87A4 because the code
       does; comparing `tbl == D_801A87A4` matched, `&D_801A87A0[1]` CSEs the base
       (`func_8018B694`).
     - **A frame's u16 flags at 0 vs AnimeFrame's own flags at 0x38**: the blink writes
       `model->f.flags` (the Frame's), plMain_init sets 0x20 in `model->flags`.
     - **Statement order vs store order again:** `goalSpeed = s; goal |= 1;` gives the store
       of `goal` before `goalSpeed`'s `swc1` (it waits for the `lwc1`); `side = z; zone = n;`
       likewise lands the zone's `sw` between the float stores.
     - **Not solved, rule 44:** a 0xA6-word clear of the Player comes out unrolled by 4 as a
       `for`, a `do-while` and a pointer loop; the original's is not (`func_8018A5F8`, parked).
     - **Permuter finds (run_A):** `len = 1.0f / len;` reusing the length instead of a new
       `inv` (`func_801879D8`); `speed = fabsf(p->unk05C)` rather than `fabsf(v)` of its copy
       (`func_80187AB0`). Both are register choices that no hand variant reached.
159. **managerpl.c (11 of 17) and plutil's speed:**
     - **`if (x != NULL) { m = f(x); } else { return 1; }` then the rest**: the original's
       `beqz; addiu m; b over; sw m` with `b end; li v0,1` just after is an if/else whose
       else returns; `if (x) {...whole body...} return 1;` nests differently
       (`func_8018C1DC`).
     - **`if (c) return 0; return 1;` beat a ternary and `r = 1; if (c) r = 0;`**: both of
       those grew the frame by 8 as well as moving v0 (`func_8018BF9C`).
     - **Initializer order sets the float registers:** `f32 z = p->c; f32 x = p->a; f32 y =
       p->b;` gives x f12, y f14, z f2 as the original has; declaring z first but assigning in
       x, y, z order did not (`func_80189558`, `func_80189584`). The permuter found it as
       `(z * 1.0f) * z`; the plain form is enough.
     - **Strings of a parked function:** when the function that names a task ("pl親") stays
       assembly, splat puts every string of the file after that name into its .s; the other
       functions' prints must then be comments, or the strings come twice.
     - **Not solved:** the original builds a mask in a register and `nor`s it for `& ~mask`
       (`func_8018C53C`); a `u32 mask` local is folded by IDO and only grows the frame.
160. **plpush.c (9 of 10) and plsound.c (18 of 20):**
     - **`x * 2.0f` becomes `x + x`; `x * 2` (an int 2) stays a `mul.s` by 2.0.** The original
       writes `1.0f - p->facing * 0.3183099f * 2` (`func_8018CD78` and four more).
     - **A local stored to its home but used from the register** (`beqz t6; sw t6,0x18(sp)`,
       then `move a0,t6`) is an assignment inside the test: `if ((obj = p->unk178) != 0)`;
       assigning first and testing after reloads it (`func_8018C6E0`, `func_8018C78C`).
     - **Loads hoisted to the top of a long `||` chain are locals:** `flags = p->flags;
       action = p->action;` before the tests; reading `p->action` in each test reloads it per
       branch (`func_8018D47C`).
     - **`if (msg == 0 || msg != 1) f();`** is the `beqz; beql 1` shape of the actions'
       default; a switch with case 0 and default calling f twice does not merge (`func_8018C6B0`).
     - **Local order sets the frame here too:** `f32 t; s16 vol; f32 k; ...` put the spilled
       volume at 0x2A in a 0x30 frame (`func_8018D15C`, still parked on register order).
     - **Not solved:** `beqz v0; nop; b; li 0x29; b; li 0x27` (both arms behind a `b`)
       from if/else, ternary, nested ternary and goto (`func_8018CC40`); and a subtraction
       done first in the original whatever the source order (`func_8018D15C`).
161. **pltalk.c (4 of 5) and two permuter finds:**
     - **A null test on a field's address: `if (w)` tests the register; `if (w != NULL)` does
       not.** With `w = p->work` (p + 0x30), `w != NULL` and `(w = p->work) != NULL` both fold
       into `li at,-48; beq t6,at`; the original's `addiu v1,t6,0x30; beqzl v1` is
       `w = p->work; if (w) {` (`func_8018E6D0`).
     - **A work pointer taken before a call and used after it** is made again after the call
       (`addiu v0,s0,0x30`) and stored through at 0(v0)/4(v0); taken after the call, IDO folds
       it into 0x30(s0)/0x34(s0). `w = p->work; func_8018EA54(p); w[1] = 0; w[0] = 3;`
       (`func_8018E7AC`).
     - **The same pointer made in each branch** of the asm came from one local taken at the top
       (`w = MOYA(moya); f = w->frame;`, `func_8017CF7C`, found by the permuter): the rule-41
       reading of it (make it in each branch) was the wrong way round.
     - **A decremented field copied to v0 after its store** is the field read back:
       `if (n != -1 && n != 0) p->unk164 = n - 1; n = p->unk164;` (`func_8018B378`).
     - **Not solved:** `lw a0,0x68(t7)`, then `addiu v0,a0,0x20` in the slot of the index test
       and `lw t9,0x150(v0)`: `PL(o)`, `(Player *)(o + 1)`, a TaskBlock's `work` and a `Player *`
       local all fold to 0x170(a0) (`func_8018E994`, with the permuter). The permuter's answer,
       taken as FAKE: `q++; q--;` at the end of the loop body keeps `q` in its own register.
162. **plshot.c (32 of 40):**
     - **`check`'s count hides a float constant moved past a store.** Its `%hi`/`%lo` lines are
       left out, so `t[i].obj = NULL; t[i].dist = FLT_MAX;` read as 0 while the original loads
       the constant first; only the full build caught it (`func_801921A0`). The fix was the
       other order. The full build is the proof, every time.
     - **`w->kind = x; if (w->kind == 2)` keeps x in a t register; `if ((w->kind = x) == 2)`
       puts it in v1** (`func_801904B8`).
     - **m2c's order for a commutative `add.s` is the registers', not the source's.** It printed
       `table.y + model.y`; the source is `p->model->pos[1] + D_801A89A0[..][..].y`
       (`func_80192FF4`).
     - **A ternary loads its else value first:** `li v0,8; bnez; nop; b; li v0,7` is
       `x == 0 ? 7 : 8` (`func_8018EE64`; `x ? 8 : 7` loads 7 first).
     - **A call evaluated before its other operand's loads is a local:**
       `s = sinf(a); w->vel.x = (fabsf(h) * 0.3f + speed) * s;` (`func_80192FF4`); written
       inline, IDO loads `fabsf`'s operand first and spills it.
     - **`1.5707963f` is 0x3FC90FDA; `1.5707964f` is 0x3FC90FDB.** m2c prints the right one;
       `check` does not compare constants, the build does.
     - **Handlers whose work pointer is `FX(obj)` at the declaration** match with it computed
       in each case (`lw s0,<obj's home>; addiu s0,s0,0x20`, the load often in the branch's
       slot), in six handlers here. The bomb (`func_8018FA94`) differs in one slot.
     - **Not solved:** (1) a handler reading its `u16` message from the home with `lhu`
       (`func_8019221C`, as plmain.c's `func_8018A270`): a `u16` parameter gives `andi` and a
       spill, an old-style definition the same, an `s16` with a `(u16)` cast `lh; andi`.
       (2) Steering `k = 1/sqrtf(dx²+dy²+dz²)`: written inline, the differences come out z
       first; with `dx`/`dy`/`dz` locals the order is right but the registers swap (dx f2, k
       f12) and the frame changes (`func_8019267C`, `func_80192A80`); the same order problem,
       with no room for locals in a 0x28 frame, in `func_80192EEC` and `func_80191F68`.
163. **plattack.c (18 of 18):**
     - **A jump-table switch whose default is `li v1,5` before the table and `move v0,v1` at
       the end** is `form = N; break;` in every case and `default: form = 5;`, with one
       `return form;`: IDO copies the return into each case (`jr ra; li v0,N`). `return N;` in
       each case gives `li v0,5` (`func_80193B48`, `func_80193C10`).
     - **Every "no" path to one `move v0,zero`** at the end is `break;` in each case and one
       `return 0;` after the switch; `return 0;` in each case gives separate returns
       (`func_80193558`).
164. **pl3d.c (12 of 16):**
     - **`fabs` of a float against `1e-5f`** is `cvt.d.s; abs.d; c.lt.d` with the double
       0x3EE4F8B580000000, `(double) 1e-5f`: `fabs(x) < 1e-5f` with `double fabs(double)` an
       intrinsic (`func_80194E80`). An int against 5.0 is `cvt.d.w; abs.d` (`fabs(sx) < 5.0`).
     - **Stores of 0x70, 0xC0, 0xAC, 0x98 in that order from one zero** are
       `p->unk098 = p->unk0AC = p->unk0C0 = p->unk070 = 0.0f;` (the last named stored first).
       Two `mtc1 zero` for two stores are two statements (`func_80194110`).
     - **Rule 3 again, for two struct locals:** the floor query's 0x30-byte buffer is above
       the query, so it is declared first (`PlFloorBuf buf; PlFloor q;` puts buf at 0x48 and q
       at 0x1C, `func_80194964`), though the query is the one filled first.
     - **`return r->hit = f(...);`** is a store followed by nothing (v0 kept); returning
       `r->hit` after the store reloads it (`lw v0,0(s0)`, `func_801945A8`).
     - **Not solved:** a dot product `a * b + c * d` that the original evaluates `a` first; IDO
       loads `c` first here in any order of the terms or factors (`func_80194254`).
165. **enemy.c (22 of 32):**
     - **`addiu v1,v1,0x20` then an offset from it** (not one folded offset) is a work pointer
       declared and set at its declaration: `EneWork *w = ENEW(D_801B4500); return
       w->bases[i];` (rule 126 again, `func_80196314`, `func_80197224`, `func_80196160`).
     - **A parameter stored with `sh` and no `sll/sra`** is an `s32`, not an `s16`
       (`func_80197834`); an `s16` parameter's `lh` from the caller's area (0x7A of a 0x60
       frame) is a stack parameter declared `s16` (`func_80197138`).
     - **`jalr v1` for a callback** is the pointer in a local first: `c = a->c; c(...)`;
       `a->c(...)` loads it into t9 (`func_80197264`).
     - **Every path of a switch ending in `b end; move v0,zero`** with one `return 0` after
       it: `break` in the cases, not `return 0` (`func_80197264`).
     - **A load kept before six stores to another pointer** is a source order that loads it
       before them: `o->y = a->y; o->z = a->z;` then the zeros (`func_80197264`).
     - **An unused local, even an array, did not change the frame here** (`func_80197370`,
       pads of 4 and 1 words): the frame is set by what IDO spills, not only by what is
       declared (compare rules 118, 126).
166. **enecommon.c (53 of 60):**
     - **`EneObj *e = ENE(o);` at the top, used everywhere, is what gives IDO's
       rematerialised pointer.** You see `addiu v0,a0,0x20` repeated at join points, or the
       first uses folded (`0x30(a0)`) and the `addiu` after them. Twelve functions matched
       once written this way (`func_80197CE8`, `func_801982C4`, `func_80198594`,
       `func_80197FC4`). A plain `e = ENE(o);` later folds away.
     - **Stack arrays, not scalars, when the deltas are stored and reloaded** (`sw`/`lw` at
       0(sp), 4(sp)): `f32 v[3]` (`func_80198594`, `func_801986EC`), `f32 v[2]`
       (`func_80198654`, `func_801997D0`). A `Vec3f` struct was kept in registers.
     - **A sum of squares is written in its natural order**, `v[0]*v[0] + v[1]*v[1] +
       v[2]*v[2]`. IDO emits the outer add with its operands swapped, which is what m2c
       prints as `v2² + (v0² + v1²)`. Writing m2c's order does not match.
     - **u8 locals take one byte each** (rule 118). `u8 i; u8 n; Obj *list[4];` packs the two
       counters in one word above the array. `list[(u8) f(n)]` takes a slot for the cast
       temporary; `i = f(n); return list[i];` does not (`func_801983E8`).
     - **`0.0f <= target - *pos` was folded to `*pos <= target`**; `f32 d = target - *pos;
       if (d >= 0.0f)` keeps the `sub.s` (`func_80198C94`).
     - **A block-scoped local in a switch case**, or the direction as three `f32` locals read
       before the test, sets the slots (`func_80197FC4`: x, y, z each take a slot).
     - **A switch whose bodies come out in the order 2, 1, 0** (compares still 0, 1, 2) is a
       switch written in that order (`func_80199944`, `func_8019A048`).
     - **`case 0:` listed with `default`** gives a jump table from 0 (`sltiu 0x22`) instead of
       `addiu -1; sltiu 0x21` (`func_80197FC4`). The same holds for cases 1 and 27 in
       `func_80197EB4`.
     - **A dead string after a C function's own string** comes from that function's
       `PRINTF` (`if (0) osSyncPrintf`, common.h). kill_hahen's "PARTS SET\n" sits in its
       init case. Without it, every late .rodata address after it moves by 12.
     - **splat splits a jump table's targets into functions** (`func_801981D0`,
       `func_801981D8` are `li v0,2`/`li v0,3` of `func_8019819C`'s switch). With the switch
       in C, drop their pragmas; the build matched.
     - **`d = (c) ? d : -d;` into the same variable** moves d first and branches the other
       way. The original has the ternary in a register of its own; `func_80198A40` is parked
       on it.
     - Open: `from->zone` held in v0 for a `beqz` (this C uses s0 or `beqzl a0`), and a store
       through `o + 0x54` scheduled before the pointer's `addiu`. These are in
       `func_80199B9C`, `func_8019A508` and `func_8019A734`.
167. **eneproc.c (20 of 50):**
     - **The check's count filter (`%hi|%lo` removed) hid a moved `addiu fp,fp,%lo(...)` in
       `func_8019C0F8` and a wrong double constant in `func_8019E4CC`.** Both showed 0 and the
       build differed. Count with only `R_MIPS` filtered, and let the build decide.
       - The fix for the first was `for (i = 0, which = n - 1; ...)`. With `i = 0; which = n - 1;`
         as statements, the global's address was set after them.
     - **This author declares `EneObj *e = ENE(o);` at the top of nearly every function, and
       it takes a slot.** Several frames only matched with it (`func_8019BB94`,
       `func_8019C76C`, `func_8019C8C4`).
       - Locals whose frame is 8-16 bytes off take `s32 pad` words in declaration order.
       - Float locals that live only in registers still take slots in that order:
         `func_8019BFD0` matched as `x, y, z, min, n, zone`.
     - **Float locals passed to a call go through `lwc1` + `mfc1`**; fields passed directly
       go through `lw`. `func_8019B768` needed `x = e->x; z = e->z;` as locals.
     - **Which float gets which fs register follows the order the zeros are set.**
       `func_8019C0F8` needed `sz = 0.0f;` first, then `sx = sy = 0.0f;`.
     - **`min == -1.0f || d < min` as `if (...) min = d; else if (...) min = d;`** gave the
       original's `bc1f; b; mov.s`.
     - **A switch on a computed value with an `f32` temporary** (`t = ...; t += 256.0f;
       switch ((s32) t & 3)`) put the sum in one register, as the original has
       (`func_8019C8C4`).
     - **A pointer read twice from the same field (two `lw 0x2C(a0)`) is not CSE'd when one
       read goes through `e->` and the other through `ENE(o)->`.** Not yet matched that way
       (`func_8019D554`).
     - **Store order in the model makers is dl (or anim), then the flags.** The `lhu`/`ori` of
       the flags and the `e->y` load remain one instruction off (parked, all six).
     - **Degrees to radians is the double 0x3F91DF46AAA2FD9A** (0.0174532930038800785). It is
       not `0.017453293`, not `(f64) 0.017453293f`, and not `3.1415927f / 180.0`.
168. **person.c (11 of 25):**
     - **Reading a pointer field twice (`lw 0x2C(a0)` for the test, again for the use): test
       through the macro, use through the declared pointer.** In `func_8019F070` that is
       `PersonObj *p = PERSON(o); if (PERSON(o)->model != NULL) f(p->model);`. The other way
       round keeps one load.
     - **`PersonObj *p = PERSON(o); s32 pad;` at the top** gave the frame in four functions
       (`func_8019F1C0`, `func_8019F468`, `func_8019F070`, `func_8019F340`). The author seems
       to declare two pointers to the work.
     - **Every call through a register (`jalr`) in `func_801A0274` is `(*f)()`**. This file
       mixes the two styles function by function.
     - **A splat split whose build moves the text is a rodata shift.** Unnamed floats come out
       as `.rodata`; type them `f32` in symbol_addrs.txt (the plchain.c trap).
169. **shim.c (10 of 16):**
     - **asm-processor refuses a GLOBAL_ASM whose late rodata is over a third of its text**
       ("late rodata to text ratio is too high": a 20-instruction switch with a 32-entry jump
       table). Such a function must be C before the file builds.
       - `func_801A0F1C` and `func_801A0F88` matched at once.
       - A jump table that starts at a case the C never names needs that case listed with the
         default (`case 100: default: return 0;`).
     - **A `u8` parameter used as is, but homed (`sw a1,4(sp)`, no `andi`)** is declared `u8`
       (`func_801A0F6C`).
     - **This file calls through registers (`(*f)()`) in most functions.** Each call that is a
       `jalr` in the original is written `(*f)(...)`.
170. **demo.c (17 of 27):**
     - **Floats passed in `$f12`/`$f14`, then a2, a3 and the stack, are separate `f32`
       parameters, not a struct by value.** `func_801A10E0` takes eight floats. It fills a
       `Vec3f` local and passes that by value to `func_8018B4D4`.
     - **Switch bodies laid out 0, 3, 2 with a shared `li v0,1; jr ra` at the end** are
       `case 0: ... case 3: ... case 2: ...; break;` then `return 1;` after the switch
       (`func_801A2464`).
     - **A pointer field tested and then passed (`lw a0,...; beqzl a0`)** is a local:
       `by = box->hitPower; if (by != NULL) f(by, ...)` (`func_801A23FC`).
     - **Test through the macro, use through the declared pointer** matched three more
       (`func_801A2614`, `func_801A26F4`, and person.c's rule 168).
     - **Unaligned 16-byte copies (`lwl`/`lwr` pairs) are struct assignments** of a `u8[16]`
       record. Six records copy as two, then an unrolled four: IDO does the remainder of an
       unroll-by-4 first (`func_801A29D8`, parked).
171. **pipgauge.c (5 of 6) and dmenu.c (8 of 9):**
     - **Two words of zeros after a file's only string are `.rodata` alignment**, not two
       `PRINTF("")`. "gauge" plus 0x10 of region built without them (the ascii.c trap).
     - **A function address used by two `(*f)()` calls in a row is kept in a register and
       spilled (`jalr v0; sw v0,0x28(sp)`, then `lw t9,0x28(sp)`).** Every call is still
       written `(*f)(...)`; IDO makes the spill. The handler `func_801A3330` needed
       `s32 pad[2]` for its 0x40 frame.
     - **Open: IDO puts two such spills 8 bytes apart** (0x20/0x28, or 0x24/0x2C), where
       `func_801A3504` has them 4 apart (0x24/0x28) with its local at 0x38. Parked for the
       permuter (run_G).
     - **A demo's handler at 0x08000000 in a TLB-mapped file** is an extern function named
       as splat names it (`D_8001034`); `undefined_syms_auto.txt` gives its address.
172. **demotool.c (21 of 29 so far):**
     - **A dead `PRINTF` decides which variable gets a register.** `func_801A6358` kept its
       `step` parameter in its home slot only with its two "<tunagi_anime>" prints in the C;
       without them IDO gave `step` s1. Put a file's unreferenced strings back as PRINTFs
       before grinding registers.
     - **When splat folds unreferenced strings into a referenced label** (the four tunagi
       prints ride on OBJMOVE's blank line, `D_801B7A60`), writing that function in C drops
       them. A parked function's prints then go in its `#else` as `static const char`
       arrays under a FAKE comment; IDO keeps them in .rodata in source order.
     - **A string already in a parked function's rodata must not be printed again** from C:
       "強制フェード中です" rides on "democam"'s label, so a PRINTF of it in `func_801A6C20`
       made .rodata 0x18 too long.
     - **A display-list helper whose asm ends with `v0` still holding the old buffer head
       returns it**: `Gfx *dl = D_8010FCE0; ...; return dl;` (`func_801A40E0`, `func_801A41A4`).
     - **A 5th float parameter after three ints arrives in a3 as bits** (`sw a3,..; lwc1`), and a
       3rd float after two goes in a2 (`func_801A48A8`, `func_801A6C20`).
     - **A handler that reloads obj from its home and keeps obj+0x20 in s0 (or v1, spilled)
       declares its work pointer at the top**: `CamChk *w = CAMCHK(obj);`, then `w->` all
       through. With the macro at each use IDO keeps obj in s0 and every offset moves. Three
       handlers matched this way (`func_801A4BB8` with `s32 pad[2]`, `func_801A54A0`); it is
       camera.c's dracam shape (`func_800C8010`), and democam (`func_801A664C`) is its copy.
     - **democam's copy of CAM_MOVE compares with a double** (`> 0.00001`: `cvt.d.s; c.lt.d`
       against a .double in .rodata), where camera.c's uses `0.00001f`.
     - **`divu` on the frame count** is `(u32) D_800881C0_main.frameCount % 10` (`func_801A4E20`).
173. **Overlay 10 (vanime.c 3/5, bmenu.c 3/3, bappcam.c 2/3):**
     - **Overlay .bss symbols link with their suffix** (`D_8019ADBC_ovl_0010`); a bare
       `D_8019ADBC` is an undefined reference at link time, after every check has passed.
     - **Compute a float constant's decimal from its bits before writing it.** 0x3F32B8C3 is
       0.69813175, one ulp above 40 degrees (0.6981317 rounds to 0x3F32B8C2). IDO rounds
       literals correctly; the original's constant is simply not the exact angle.
     - **Two random calls in one call's arguments** (`f(cam, rnd() + x, rnd() + y, z)`) spill
       the first result as a temporary; a named local for it took a slot of its own and moved
       the frame (`func_8018AA9C`).
     - **Five `x &= ~bit` statements on three globals keep source order** for their loads;
       m2c's output interleaves them, so read the asm's load order (`func_8018A4B4`).
     - **A struct copied to the stack sits below locals declared before it**: two pointer
       locals before `VAnime h` put it 8 lower, as the original has (`func_801738A0`).
     - **A frame stepper's dead PRINTF of the frame** ("[vanime_act] L->vtimer[%f / %d]")
       carries the file's only string; without it .rodata moved every double after it.
     - **A symbol's address passed as `(u32) sym` many times is kept in a register** and
       spilled across the calls; passed as a pointer (`u8 *`) it is rebuilt with lui/addiu
       for each call, as the original has (`func_8019A224`, sixteen icon calls).
     - **A loop whose store comes before the pointer step (`sw v0,0x18(s0); bne; addiu s0`)**
       is `i = 0; do { ... } while (++i != 3);`. The `for` form put the step first and the
       store in the delay slot (`func_8019ABE4`).
     - **A handler's loop counter spilled to the stack (`sw v1,0x34(sp)`)** moved 4 bytes with
       one pad local fewer: the frame rounds up to 8 and the 4 spare bytes go below the
       temporaries (`func_8019A224`).
174. **bmove.c (54 of 66):**
     - **`w->f += x` with `BObj *w = BOBJ(obj)` at the top** loads through obj (folded offset)
       and stores through w (`addiu v0,v0,0x20; swc1 8(v0)`) (`func_8017C4FC`). The same split
       shows in conditions: the first operand through the macro (`lw t6,0x20(a0)`), the
       second through w (`func_8017C584`, `func_8017C698`, `func_8017D5F4`).
     - **Zeroing three fields in the order 2, 1, 0** is `v[0] = v[1] = v[2] = 0.0f`: a chain
       stores its rightmost target first (`func_8017B958`).
     - **u8 K&R parameters stay untruncated; s8 ones are sign-extended** (`sll/sra 24`) even
       old-style. `func_8017D690` homes a2/a3 and stores them with `sb`: `u8 a; u8 b;`.
     - **`cosf` evaluated before `sinf` with its result spilled** is a reused temporary:
       `t = cosf(a); *x = sinf(a) * d + s * t; t = sinf(a); ...` (`func_8017C174`).
     - **Open: a returned flag in v1 moved to v0 at the end** where the original keeps it in
       v0 from an early `move v0,zero` (the three turners `func_8017BF2C`, `func_8017C050`,
       `func_8017D4E8`, parked).
175. **beffect.c (32 of 34 so far):**
     - **`f * 2` with an int literal is `mul.s` by a loaded 2.0**; `f * 2.0f` folds to
       `add.s f,f`. Megaball's `arg->scale * 2` (`func_801935C0`).
     - **Stores out of order against the source: swap the statements.** flashP's spawner
       stores count before pos[2]: `arg.pos[1] = y; arg.count = count; arg.pos[2] = z;`
       (`func_80193CCC`).
     - **A create case with no `break` falls into the run case**: the first frame's step runs at
       once (Hitmark `func_801915C4`, flashF `func_801932D0`). m2c shows it as a duplicated
       tail; the asm has one copy.
     - **No `pad`, one, or `s32 pad[2]`** moves the handler's frame and its spill slots; read
       it from the `addiu sp` of each. Filter, Efilter, Hitring, Hitmark and flashP need none,
       Beffect and Pfilter one, Hitball, ATf, the M-effects, flashF and Megaball two.
     - **Open: msg held in s2** (`sll/sra` at entry, then compared from s2) with a 0x78 frame,
       where IDO reloads it from its home ("Belt" `func_80193814`, parked).
     - **The argument read through a typed local** (`EffPWork *a = arg;`) is reloaded from the
       parameter's home into v1 or a3, where `((EffPWork *) arg)->` uses t-registers and
       shifts every temporary after it (both "psmoke"s, `func_801966A8`, `func_80196E78`).
     - **`f / 2` with an int literal keeps `div.s` by 2.0**; `f / 2.0f` becomes `mul.s` by 0.5
       ("10RYOU" `func_80197528`). Likewise `f * 2` is `mul.s` and `f * 2.0f` is `add.s`
       ("Lball" `func_8019787C`).
     - **A function address used by two calls in one block is kept in a register or a stack
       slot** (`sw a0,0x30(sp)` ... `lw t9,0x30(sp); jalr t9`): write both calls as `(*f)(...)`
       and IDO does it itself (M6effect `func_8019428C`, the "reversal"s, "Star" `func_80195268`
       whose three `(*func_800CEF90)(255)` go through s0).
     - **A load hoisted above a store in the same block** means the source statement came
       first: Lball sets `w->unk3C = arg->unk3C` before `w->alpha = 255`, and the store sinks
       to the delay slot. A value loaded before the stores it should follow is a local read
       there (PunchE: `z = arg->pos[2]` before the frame's position).
     - **A string missing from a file's rodata shifts all its doubles** (strings come first):
       the first differing byte lands on the file's first `ldc1`. A spawner that calls through
       `(*func_800D3FF0)` with the battle's file usually has a failure PRINTF.
     - **Where a dead PRINTF sits decides registers**: "Bspace" `func_8019859C` matched only
       with the print before `D_8019ADCC &= ~0x80000`, not after; the wrong place also moved a
       branch's delay-slot fill elsewhere in the function.
     - **A `switch` with a jump table is fine once the file's .rodata is in C** ("fakeImp"
       `func_801980F8`, five cases, first try).
     - **Stores the scheduler sank into a delay slot came last in the source**: "megaRef"
       stores its three floats, then the two zeros; the third float's store lands in the
       branch's delay slot (`func_80198BA4`).
     - **Open: the constant in a3 with a dead `a1 = 0`** (`func_80198A7C`), and a loop that runs
       its counter in ra (`func_80198750`): parked.
176. **impact.c (13 of 15 so far):**
     - **One `s32 pad;` and `s32 pad[2]` give the same frame but different spill slots**: a
       spill one word too low (0x18 for 0x1C) wants the scalar ("shoulder" `func_801854F4`,
       "vc" `func_80187A74`, "Pgage" `func_80182878`).
     - **Digits split with a common subexpression, not a `rest` local**: `d100 = n * 0.01;
       d10 = (n - d100 * 100) * 0.1; d1 = (n - d100 * 100) - d10 * 10;`. A named `rest` costs
       a stack home and grows the frame ("money" `func_80182CFC`, "fuel" `func_801830D0`).
       Their homes then sat 4 bytes high until one scalar pad went above the digits and one
       below.
     - **`x += 0.0f` is a plain load and store** (the second "Bbg" `func_80187C50`).
     - **Open: "bosslife" `func_8018353C`** wants its four digits' homes 4 bytes lower in the
       same frame; every extra local grows the frame (parked).
     - **Stack homes follow declaration order, but only for locals that need one**: an unused
       `s32 pad` takes a slot; a local assigned and never read (`c = cosf(angle);`) takes none.
       "Jab" `func_8017ECE4` matched with `f32 z;` declared first (the extra word was a
       local, not a pad); "Valcan" `func_8017E020` with `pad0, angle, c, s, pad, Vec3f at, t,
       pad2, tx, tz`, worked out by dumping my build's slots (`objdump` of the permuter
       dir's `full.o`, one function's range) against the original's.
     - **`f = a - b` written as a local changes the add order of what uses it**: the punches'
       hit ring is `x = box->pos[0] + cosf(angle) * box->radius` (the original adds
       product + pos); a `z` local for the third argument let the loads of x and y sink
       after it.
     - **Two `return -1` blocks after a long then-block, in the order "outer test false, inner
       test false"**: the outer test has an `else { return -1; }` and the inner one falls to
       a `return -1` after the if/else ("Ovalcan" `func_801809F0`).
     - **u16 → f32 goes through the unsigned path** (`bgez` and `add.s` of 2^32): IDO promotes
       u16 to unsigned int here (the box radius in the punches).
177. **impact.c (done: "Storm", "MEGA"):**
     - **The reversed-sum rule (in the rules above) holds for float adds as well:**
       - `pos + rand()` gives `add.s rand, pos`.
       - `sqrtf(z*z + x*x)` gives x² first and `add.s x², z²`. m2c prints `x*x + z*z`, and
         writing that does not match. Write the operands the other way from m2c, as in the
         three-term sum of 175.
       - Example: "MEGA" `func_80185EDC`, both of its `sqrtf`s and its spark offsets.
     - **The fourth branch of an if-chain had different code for the same `0.0f, 0.0f, 0.0f`
       arguments** (`li a3,0` and two fresh `mtc1 zero`, against one `mtc1`/`mfc1` in the first
       three). My C reproduced it from identical source, so it is IDO's choice and needs no
       source change.
     - **Open: "MEGA"** (parked). The game keeps every short-lived pointer in v0 where IDO
       gives v1. Its stack temporaries sit one word higher (0x3C, 0x44/0x48 against 0x38,
       0x40/0x44), and its locals start 8 higher. One temporary more is allocated somewhere
       before stage 0's first `jalr`, and no spill of it survives.
       - Locals sit on top of the temporaries, so a pad grows only the frame's top. `pad[7]`
         gives the game's 0x88 frame with every local 8 too low.
       - `pad[5]` gives a 0x80 frame, with the locals in the same place as under `pad[7]`.
       - The missing 8 bytes belong to the temporaries.
178. **brother.c: `-Olimit`, and two traps in my own checking:**
     - **The game was compiled with a raised `-Olimit`.**
       - "Brother" (`func_8018B890`, 2,710 instructions) is over IDO's default limit. With the
         default, its scheduling differed all over: cross-block constant hoisting, delay slots,
         likely branches. That is 1,121 differing lines.
       - With `-Olimit 1000`, 1500 or 3000 it is 24, all stack.
       - All 102 C files rebuilt with `-Olimit 3000` still give both hashes, so build.py keeps
         it.
       - A function the size of MEGA (1,766) was not affected.
       - The sign: a giant whose calls and stores all match, but whose scheduling differs
         everywhere.
     - **My diff count drops every relocated line** (`grep -v '%hi|%lo|R_MIPS'`), so an opcode
       difference on one of those lines is invisible. One example: `lbu` for `lhu` when a
       prototype took a `u8` where the game passes a u16.
       - Compare the relocated lines' opcode and first register separately. Take
         `sed -E 's/^-([a-z0-9.]+) ([^,]+).*/\1 \2/'` of the `-` lines with `%hi`/`%lo` against
         the same of the `+` lines with `R_MIPS`.
       - Or build: the first differing byte names the instruction.
     - **A `-lui a0,0x801a` / `-lhu a0,-20760(a0)` pair with no `+` side** is splat failing to
       symbolise one access, not a literal address in the source. The bytes link the same.
       Writing `*(u16 *) 0x8019AEE8` there moves the whole register allocation (620 lines).
     - **`f = f OP c; g = c2;` statement order shows in the float registers.** The order of the
       stores does not tell it. In "Brother", `angle -= 7.0f; speed = -7.0f;` loads the angle
       first (f10), even though the speed store is scheduled first.
     - **The extra-temporary stack shift (rule 177's MEGA) also happens in "Brother"**, and
       here it is isolated.
       - Locals sit on top of the temporaries. A pad declared *last* (lowest) brings the
         locals into place.
       - The three spill slots still sit one word low: the game has one temporary more, below
         the others.
       - None of these move them: a dead-assigned local, `--x` in a condition, or a call in a
         dead `PRINTF`.
       - The water Brother has no CSE'd function-pointer spill and no shift.
179. **Block-scoped locals move the stack: "Brother" matched with `f32 tx, tz;` declared
     inside the block that uses them** (`if (!(D_8019ADC0 & 0x40)) { f32 tx; f32 tz; ... }`).
     - At function scope, `tx`/`tz` gave every spill slot one word too low, with the frame
       right only when a pad was put under them (rule 178).
     - Block-scoped, they sit right under the function's locals (0x58/0x5C), the three spill
       slots move up a word (0x3C, 0x44/0x48), and the frame stays 0x78 with one pad less.
       This is IDO's "one temporary more".
     - The permuter pointed there: its best (score 10) passed `&msg` for `&tx`, i.e. one
       function-scope local fewer.
     - It is not the whole story for MEGA. Block-scoped `at`/`t` (stage 1) land on the game's
       0x60/0x64, but `tx`/`tz` and the spill slots stay low; its temporaries need two words
       more.
     - In stage.c's `func_800DC774` a block-scoped `PlayerInit` comes 4 bytes closer (0x80
       for 0x7C).
     - So when a function's spill slots or locals are a word or two off and pads only move
       the top: try the locals used in one block, declared in that block.
180. **audio.c, stage.c, geography.c, plchain.c, tilemap.c:**
     - **`while (count)` and `while (count != 0)` differ** (rule 19, for a loop).
       - Case: audio.c's command queue (`func_8000959C`) clamps the count to 8 first.
       - With `!= 0`, IDO copied the clamped count into v0 for the first test. With the
         plain test it tests v1 itself, as the game does.
       - The permuter found it (output-0).
     - **A float divided by an int literal keeps `div.s`; by a float literal power of two it
       becomes a multiply.** So `/ 16`, `/ 32`, `/ 256`, `/ 512` in tilemap.c and
       geography.c are int literals.
     - **A float converted to u8 or u16 goes through the unsigned path**:
       `cfc1/ctc1 ... cvt.w.s`, and `bgez` + 2^32 for a u8 operand. tilemap.c's palette
       blend has `(u8) (... * f->blend + r)`.
     - **A switch whose default comes first in the source is laid out first**: test chain,
       default inline, then the cases in order. In geography's `func_800E0C44` the default is
       `if (w->view == NULL) return; break;` and the main code follows the switch.
     - **A self-increment that only a debug print reads**: the game keeps it, IDO drops it.
       An empty `if (missing != 0) {}` after the loop keeps it.
     - **Pad arrays and scalar pads land in the same places.** The locals are laid out from
       the frame top. A frame several words too big or small, with the locals in the right
       order, is usually the compiler temporaries' count, which pads do not fix.
181. **A third-party library in the game: try its upstream source first, with the game's
     flags, before writing any C.** (Goemon 185, zlib 1.1.3.)
     - Find the version from a string in the ROM (`inflate 1.1.3 Copyright ...`) and fetch
       that tag.
     - Then compile it and compare word for word with relocated fields masked
       (`tools/libz.py map`).
     - Seven of zlib's eight objects matched with the game's exact flags. The eighth differed
       in two words: `lbu` in the ROM, `lb` from me. Those are inflateInit2_'s two plain `char`
       loads (`version[0] != ZLIB_VERSION[0]`).
     - So **a word that differs only between `lb` and `lbu` on plain `char` data means `-signed`**.
       The game's own code was built with `-signed`, and zlib without it.
     - Check out such sources with LF line endings (`core.autocrlf=false`). cfe reads a
       CRLF line's `\` continuation as a syntax error (zlib.h:871).
182. **A work pointer set at its declaration is a real variable; one assigned later is folded
     away.** (gameproc.c `func_800C7B10`, enecommon.c `func_80199220`, Goemon 188-189.)
     - Case: game_proc. `w = (W *) ((u32) obj + sizeof(Obj));` assigned in `case 0:` gives
       `obj + 0x20` at every use: reload obj, store at 0x20+offset.
     - The game computes it once (`addiu t1,t0,32; sw t1,40(sp)`) and reloads it from that
       slot. `W *w = (W *) ((u32) obj + sizeof(Obj));` as a declaration does exactly that
       (276 differing lines to 112). camera.c's matched procs are written this way.
     - **IDO gives each declared local a slot in declaration order, from the frame top down**,
       even one that lives in a register. Unused words above the first stack-resident local
       mean locals declared before it.
     - Case: enecommon's wall ray. The game has two unused words above `from` and reloads `o`
       from its home.
       - It matched only with a top-level `ene = ENE(o)` assigned after the `from` stores, an
         `s32 pad`, and a block-scoped `EneObj *e = ene;` inside `if (push)` (rule 179).
       - Any one of the three alone left the frame 8 bytes short. The copy is FAKE-commented.
     - **Sweep idea:** parked procs whose work pointer is assigned inside a `case` may match
       once it is set at the declaration.
183. **Lessons from an early sweep across many files** (one file at a time;
     each lesson has the function that proved it).
     - **The repo's `PRINTF` (`if (0) osSyncPrintf`) is not free.**
       - Dead ifs between uses of a global can stop IDO keeping its address in s0. With an
         empty file-local `#define DPRINTF` (pakfiler.c's form) the strings stay in .rodata
         and the code matches (acoll.c `func_800EFA28`/`EFB24`).
       - Where a dead PRINTF sits moves code. As the first statement of an early-return block
         it puts the next switch's `li a2,1` in the `beqz` delay slot; as the last statement
         before `return -1` it does not (bappcam.c `func_8018AA9C`; rule 115).
       - A dead PRINTF using a loop counter can decide its saved register (demo.c
         `func_801A1A14`: i went s2→s1). Point its string at the file's GLOBAL_ASM .rodata
         label plus an offset, so no second copy lands in .rodata.
     - **Check every local prototype against the callee's definition.** acoll.c gave
       `func_800D50E0` an invented third argument, which moved f from a2 to a3. With the real
       one, `func_800EFE6C`/`EFEE4` matched at once.
     - **Floats.**
       - An int literal (`1 - x`, `y + 1`) shares one loaded float constant; `1.0f` loads it
         per use (plswing.c `func_801745F0`).
       - A commutative add's operand order follows CSE: the game's `add.s kin,tmp` came only
         with the sub-expression repeated (one CSE temp), not as a named local.
       - A named local for a quotient or subsum puts it in f0/f2 and can fix every register
         after it.
       - One float local reused in both arms of an if/else (where the game has the same FPR)
         rather than two locals (demopl.c `func_801793BC`).
       - `w->step = 0` rather than `0.0f` stopped one 0.0f register being shared (fade.c
         `func_800CE7D0`).
     - **Narrowing casts IDO folds away still cost temps:** `a + (s16) ((b - a) * t)` against
       `(s32)` moves the trunc's mfc1 two registers up (anim.c `func_800D5688`, aniatk.c
       `func_800F2298`).
     - **Rule 19 at scale:** `!n || i < n` and `n == 0 || i < n` allocate differently (all six
       tests of aniatk.c `func_800F23F8` and anim.c `func_800D5AA8`).
     - **Byte and pointer idioms** (aniatk.c `func_800F23F8`, `func_800F1D68`):
       - `(D[k] << 8) + D[k + 1]` loads in that order; the other order swaps the addu.
       - `hi = D[pos++]` puts the lbu in the branch's delay slot; `hi = D[pos]; pos++;` puts
         the andi there.
       - A read before the step uses the unmasked sum: `D[pos + n*2 - 2]` then
         `pos += n*2`.
       - `p = &p[k*4]; f(p);` register-allocates p. `p += k*4` swaps the addu, and
         `f(p + k*4)` reloads p.
     - **Registers follow first reference.**
       - Caller-saved registers go in the order locals are first referenced; declaration order
         does not matter (joypad.c `func_800D8850`, msgfrm.c `func_800EC0B0`).
       - One pointer local reused in two switch cases joins their allocation. A block-scoped
         local per case gives the game's registers back (diveef.c `func_8017D19C`).
       - Index globals directly (`G[idx][i]`) instead of through user pointers and IDO hoists
         and strength-reduces the arrays as the game does (joypad.c).
     - **Arrays vs fields:** an OR chain of constant-index array elements comes out rotated (last
       term first), even inside gbi macros. Named fields keep source order.
       - camlight.c's scissor, read through a FAKE `CamScissor` cast until object.h names the
         fields.
       - fade.c's `u8 color[3]` became `u8 r, g, b`.
     - **`continue` vs if/else in a loop** changes register-allocation priorities (anim.c
       `func_800D57F8`: the constant got fp and key0 was spilled, as in the game, only with
       if/else).
     - **Stack:** a local declared LAST pads the bottom of the frame; out-parameter scalars land
       in declaration order (demo.c `func_801A16A0` needed twelve separate floats, not
       `f32 p[4][3]`).
184. **The cockpit (cockpit.c `func_80174C60`, 6,672 instructions): from 7,455 differing lines
     to 374, at the game's exact size (0x683C).** Parked; the comment holds the rest.
     - **Case order is layout order.** The game's source had cases 2, 1, 0, 3. With 3 before 0
       the call sequence was the same calls in another order. An aligned call list (491 calls,
       each jal/jalr target in order) shows a block move at once.
     - **A reused loop counter adds a saved register.** The `% 60` result in its own local `j`,
       not the loop's `i`, took the function from s0-s4 to the game's s0-s3.
     - **`(*func)(...)` for every call the game makes by `jalr`:** 38 of the 491.
     - **`DPRINTF` (empty macro) instead of `PRINTF`** moved constants and spills to the game's
       registers (rule 183).
     - **Floats one ulp off:** 55°, 43° and 35° had to be written 0.95993114f, 0.75049162f
       and 0.61086524f (rule 173's case).
     - **A struct-return temporary held in a register covers at most 28 uses.** The 29
       `func_800CE09C` calls (FileSet returned by value) get s1 for 28. The first call computes
       the address itself, but the game covers all 29.
       - Proved by changing the call count: 27 and 28 calls are fully covered, 29 and 30 are
         not.
       - Labels, empty ifs, `(void)`, a K&R prototype, a block-scoped or function-scope
         `FileSet` (assigned or not, `fs[2]`) did not change it.
       - Still open; likely the same cause as the game's frame being 8 bytes bigger (0x100,
         against 0xF8).
     - **An `== 2` against a signed global** let IDO keep a switch's case-2 constant in t0 for
       the whole case, which shifts every temporary after it. `== 2U` (FAKE) gives the game's
       separate register.
     - The function needs `-Olimit` 1609 or more; any value above that gives the same code.

185. **A jump table's holes are the default only if no case claims them; read the table.**
     IDO fills a hole with the default label. So when the table sends a hole to a block that is
     not the one the out-of-range test branches to, the source had explicit cases there.
     - sccheck.c's two diary saves (`func_800EDE20`, `func_800EDFFC`): the holes 2, 6, 7, 8 and
       9 (PFS_ERR_NEW_PACK, BAD_DATA, DATA_FULL, DIR_FULL, EXIST) go with 0x102 to "File
       error?". Without them, IDO filled the 0x102 test's delay slot from its target; with
       them, the block has more than one way in, and the slot takes the next test's constant.
     - menu.c's `func_800EA438` and `func_800EAAD8`: a switch on 1-4 plus default compiled to
       a chain of `beq`. The game's five-entry table (index 0 = the default's block) is
       `case 0: default:`.
     - Functions parked as "jump table (rodata not migrated)" are worth re-checking once the
       file's `.rodata` piece exists (their `.s` then has `.late_rodata`). The repair
       (`func_800EDCD8`) matched as written.
     - **A table that splat cut into functions** (`.word func_...` entries, `jlabel func_` in the
       pieces) is one function. Write the whole switch in place of the first piece and drop all
       the pieces' pragmas; the build matches (string.c `func_800E9E5C`, seven pieces). Its case
       order was the data's order, with the default (and an explicit `case -5`) first.

186. **Smaller lessons from the same day:**
     - **A u32 sum is not pointer arithmetic.** gfxbuf.c's five reservers step the buffer as
       `(u32) D_8010FCE0 + n * sizeof(T)`. `(Gfx *) (m + n)` swaps the `addu`'s operands, and
       IDO also moved the global's address from a2 to a1.
     - **Stores in the array's order.** camera.c `func_800CDC78` stores `unk52[0..4]`; written
       0, 2, 3, 1, 4 the reload of a parameter lands one slot late; written 0, 1, 2, 3, 4 it
       matches.
     - **Four tests on one line** (decomp-permuter): stage.c `func_800DBC18`'s four
       `if (who == ... && ... > 0)` on separate lines schedule the `lui`s otherwise. Rule 12
       again; kept as FAKE (one macro's expansion would do it).

187. **More lessons from the same stretch:**
     - **The open `b`-to-the-next-instruction pattern of rules 147/157 is two returns in an
       explicit if/else.** `li v0,1` above the test, then `bnez; nop; b next; move v0,zero`
       is `if (!x) { return FALSE; } else { return TRUE; }` (plpick.c `func_80181ADC`). With
       a result variable (`if (c) { r = TRUE; } else { r = FALSE; } return r;`) the branches
       match but the value goes through v1, and the FALSE's `move v1,zero` lands before a
       float compare where the game has it after (plpick.c `func_80181CF8`, ombupl.c
       `func_80180DC4`: still open).
     - **A loop that stores a call's result before stepping the pointer is a do-while.**
       `sw v0,4(s1); bne; addiu s1` (step in the slot) is `i = 0; do { ... } while (++i != N);`
       (pause.c `func_800E06C0`, rule 173's second proof). With `slti` it is `++i < N`, and
       gauge.c `func_800DDFC8` also needed the reset on the loop's own line (`i = 0; do {`).
     - **One line includes the if.** mathutil.c `func_800D0870` (the sine table): each quarter
       `if (angle < 0x200) { angle++; *c = D[angle]; angle--; return D[angle]; }` on one line
       puts the table's `lui` after the index's `andi`; the body alone on one line fixes only
       the store.
     - **A field compared with several globals:** read it into a local at its declaration and
       write the constant first (`D_80172F44 == kind`): acoll.c `func_800F0AB8`.
     - **Reuse the variable for a new value.** pl3d.c `func_80194AB4`: `s = lim / s;` (a new
       live range of s) gets f14 as the game; a new local `k` for it swaps f14/f16.
     - **Dead PRINTFs move registers, and a parked check compiles them.** managerpl.c
       `func_8018B9D0` lost fp with its three prints compiled in; under `#ifdef NON_MATCHING`
       (their strings are in a parked neighbour's assembly) it matched. A parked function's
       `check` defines NON_MATCHING, so unpark and re-check before trusting a near miss.
     - **Assignment order picks registers:** tmanager.c `func_800D428C` reads next, prev, then
       the parent (the parent first swapped v1 and a2).
     - **OR chains and array steps:** polygon.c `func_800D713C` matched with
       `rot[2] | (rot[1] | rot[0])` and `node + node->child` (not a byte offset).
     - **Not levers (proved):** cambg.c `func_800DF308`'s last OR keeps the AND operand first
       whatever the source order; particle.c `func_800F68A4`'s one-off temp is moved by none of
       seven rewrites (both notes are in the files).

188. **More lessons, and the permuter:**
     - **Twins first.** `tools/permuter.py twins` found three parked functions whose
       instructions equal a matched one's; each took its twin's C at once. A
       twin by hand: managerpl.c `func_8018C07C` = gauge.c `func_800DE750`.
     - **A pre-set constant above a test, and a `b` to the very next instruction at the end of
       a nested block, is an OUTER if/else with the constant in the else** (plchara.c
       `func_80173F5C`: `if (chara == 0) { next = f(p); if (...) next = -1; } else { next =
       0xF; }`). IDO hoists the else's constant above the test; the then-block keeps its `b`.
     - **The else and the return on one line** (decomp-permuter): ombupl.c `func_80180DC4` and
       plpick.c `func_80181CF8` (`if (...) { r = TRUE; } else { r = FALSE; } return r;` on one
       line) put the FALSE's `move v1,zero` after the float compare, as the game.
     - **A failing nested test that falls through to one final `return 0`** compiles to
       `beql t9,zero,END; or v0,zero,zero` with the success inline as `b END; li v0,1` (plmain.c
       `func_8018B1BC`; extends rule 145). A ternary return keeps 0 in v1 instead.
     - **A small switch returning in v0, the default's 0 in a `b`'s slot:** a return per case and
       `default: return 0;` written FIRST (fileload.c `func_800CDE6C`; rule 96 again).
     - **A parameter copied at entry (`or a2,a1,zero`):** declare it as the base type and cast it
       into a derived-type local (gfxmgr.c `func_800D4988`: `Frame *frame; DLFrame *f = ...`).
     - **A retry loop whose counter store sits among the call's arguments:** the limit in the for
       test, `for (n = 0; n < 2 && call() == NULL; n++)` (zanzou.c `func_800F76E0`).
     - **Float call arguments in the game's order:** assign each to a local, in argument order,
       before the call (pl3d.c `func_80194B4C`); inline, IDO computes the second first.
     - **A narrow field compared directly** puts the held constant first (`beq v1,v0`); through a
       u32 local it comes out reversed (geography.c `func_800E1C84`; rules 128-129). Cast straight
       to float, the u16 still gets the unsigned fix-up.
     - **The descending clear is the index form** (file.c `func_8000479C`; rule 24 corrected).
     - **A shared `lui at` between stores to one symbol's fields needs that symbol DEFINED in the
       file** (as1 shares it only for a local definition, within an 8-byte-aligned block). An
       extern gives a `lui` per store and a longer function: audio.c `func_8000A928` and
       `func_8000ADF0` wait for audio.c's .bss to move into the C.
     - **Not levers (proved):** fileres.c `func_80007C78`'s end pointer in a3 (something takes a2
       unused); tmanager.c `func_800D4538`'s file in v0; plutil.c `func_8018967C`'s returned 0
       kept in a0 from the entry block. IDO -O2 replaces a local's NULL with the constant in
       every form tried. The notes in the files list the attempts.

189. **More lessons:**
     - **Read the frame from the game's own offsets.** Locals get slots from the frame top in
       declaration order, even those that live in registers. From where the game homes or spills
       a local (e.g. `sw zero,0x54(sp)` for an `allocated = 0`), count how many slots sit above
       it, reorder the declarations to match, and add `s32 pad[n]` below the last local when the
       game's frame is still bigger. This matched pic.c's three decoders (`func_80005A80`,
       `func_80005CC0`; `func_80005B98` down to one slot), pause.c `func_800DFC00` (one pad),
       and fixed the frames of geography.c `func_800E16A8` and menu.c `func_800EA5A4`.
     - **`s32` versus `u32` decides an add's operand order.** heap.c's two allocators matched
       with the address local as `s32` (the sum `addr + need` is then s32 + s32) and `need`
       computed before the list head is read.
     - **A wrong local prototype** (stage.c's `func_800CEF20` with an `s32` where fade.c's
       definition has an `f32`) and **a u8 field read as u16** were the whole difference in
       stage.c `func_800DB2D8`, parked as "case layout and registers". `tools/permuter.py
       protos` lists every prototype that disagrees with its definition; most are
       harmless return types, and a few are the game's own (matched callers pass ints where the
       definition says f32).
     - **`if ((x = f()) == NULL)` costs 8 bytes of frame** over `x = f(); if (x == NULL)`
       (window.c `func_800E95D4`).
     - **A rolled loop that keeps `slt` while IDO unrolls yours:** write the test `bound > i`
       (polygon.c `func_800D7034`); IDO only recognises the counter on the left.
     - **A register zeroed before a jal and saved around it:** the source assigns it after the
       call (`f(obj); ret = 0;`, tmanager.c `func_800D41AC`). Its `ret == -1 || ret == -1` is
       FAKE: the game compares -1 twice.
     - **A local uninitialised on one path stays in memory**; go through an intermediate
       register variable, then store once (sccheck.c `func_800EDB34`).
190. **More lessons, and the permuter:**
     - **A global u8 compared with a byte read through a pointer is loaded first**, on either
       side of the `==`. Read it as an array element, `((u8 *) &G)[0]`, and IDO keeps the
       source order (sndcmd.c `func_8000E710`): the game probably declared the pair as `u8[2]`.
       One `s32` there serves as both the mode and the loop index after it.
     - **Two range checks on one load, then `return 0`** (`sltiu 6; bnez; sltiu 11; bnel`) are
       one `&&` early return, `if (id >= 6 && id >= 11) return 0;`; a switch gives a jump table.
       `addu v0,base,mask` with the base first is `(u32) ((u8 *) base + mask)` (gfxbuf.c
       `func_800D07F0`).
     - **A commutative `mul.s` puts a computed operand first** (`r * -s` and `-s * r` both give
       `mul.s neg,r`); a named local, `ns = -s; r * ns`, gives `mul.s r,ns`. An integer `1` in
       `sqrtf(1 - c * c)` stops IDO sharing a nearby compare's `1.0f`, so it reloads it, as the
       game does (acoll.c `func_800F10A0`).
     - **Registers go per def-use web, not per variable** (file.c `func_800047EC`, measured): a
       reused variable's first value is stored to its slot only if it is live across a call.
       A dead `n;` or `if (n) {}` is gone before allocation and changes nothing.
     - **A dead in-loop `== 0.0f` test still takes a float register** for its constant, even
       with no compare emitted (mathutil.c `func_800D1424`): removing it moves the others.
     - **The frame is 0x38 + 4 x (locals + compiler temps), rounded up to 8**, the temps right
       below the locals, any padding word at the very bottom (pakfiler.c `func_8017570C`,
       measured). Strength-reduced `col * 64` in a loop makes compiler temps; a second loop
       variable `s += 64` removes them.
     - **A call inline in a long argument list costs compiler temps** (the arguments before it
       are saved across it): call it first, into a local. A u16 local declared between a
       pointer and a `u16[3]` sits in the array's alignment padding and costs no frame; a u16
       struct field instead adds `andi 0xFFFF` when IDO forwards it (model.c `func_800D025C`).
     - **A u8 temporary for a call's result** before an add: `top = rand(64 - len);
       y = top + len;` gave particle.c `func_800F68A4` its one missing temp register (the
       permuter's find; an `s32` local does nothing).
     - Unrolling is rule 44 (measured, `tools/permuter.py unroll`).
191. **More lessons:**
     - **Clamp-and-swing returns** (1 at a limit, 0 between): the `return 1` is shared at the end
       and the in-between cases share ONE `return 0` after their if/else-if/else chain. A
       `return 0` in each case schedules `move v0,zero` before the `b` instead of in its slot;
       `return 1` in each limit case gives `li v0,1` per branch. Five functions: bmove.c
       `func_8017D920`/`func_8017DA78`/`func_8017DC70`, shim.c `func_801A04E0`/`func_801A05E4`.
       The work pointer the game spills across a call is a named local (`BObj *w = BOBJ(obj)`),
       with `s32 pad;` placing its slot.
     - **`x / 2` stays a `div.s` by 2.0; `x / 2.0f` becomes a multiply by 0.5** (bmove.c
       `func_8017DA78`).
     - **A u16 field tested `!= 0` one register off** (address and level in v1/v0 where the game
       has a3/v1): test it as `(u32) x != 0` (sndtrack.c `func_8000C710`, the permuter's find
       applied to both ramps).
     - **A skipped temp register around an OR into a byte array:** cast the ORed value,
       `a[i] |= (u16) color` / `(u16) (color << 4)`; IDO drops the cast but spends the register
       (pic.c `func_80006D50`).
     - **Of two float locals live across a call, the one assigned second stayed in memory**
       (store and immediate reload at its home) and the first was spilled at the call; swapping
       the assignments swapped them (plsound.c `func_8018D2E0`). Its frame needed the locals in
       the game's slot order, two pad words and a `dy` the game computes and never uses (the `y`
       parameter is homed and never read).
     - **One line for an unrolled loop** moves its setup (pause.c `func_800DF920`, rule 12);
       **a statement reading a spilled local holds later pointer stores behind its reload**
       (plmodel.c `func_80184520`); **a global array indexed in a `for`** gives the game's
       `base + i * size` pointer with the field offset in the loads, where a pointer walk folds
       the offset into the pointer, and **zeroes in the `for`'s comma init** are scheduled after
       the pointer setup (stage.c `func_800DBCAC`).
192. **More lessons:**
     - **IDO -O2 removes self tail recursion.** A loop whose back edge is `b head; move s0,v0`
       (the parameter reassigned in the slot), off only in the prologue's `sw ra`, was a
       recursive call as the last statement: `if (child...) f(child); if (next...) f(next);`
       (acoll.c `func_800F0708`/`func_800F07AC`).
     - **A constant-index array element is loaded first in a commutative float op**, whatever
       the source order (`e->vel[1]`, `scale[0]`); a named field (`((Vec3f *) e->vel)->y`)
       keeps source order (eneproc.c `func_8019B4F4`, `func_8019AAA4`; impact.c
       `func_801809F0`'s distance).
     - **A field tested and then passed on**: assign the local inside the if, after the test,
       `if (o->f != 0) { x = o->f; f(.., x); }`; the test's load takes v0 and the copy folds
       into it. In the test, before it, or inline all give another register (tmanager.c
       `func_800D4538`; geography.c `func_800E16A8`, where the local must also be u32: a u16
       copy adds `andi 0xFFFF`).
     - **An empty block in a parameter's live range** (`default: break;`, or a dead `if (0) {}`)
       lowers its priority, so IDO keeps it in its stack home across calls (plchain.c
       `func_801753A8`/`func_80175830`).
     - **A work pointer set at its declaration** keeps the task in its home and is spilled across
       the call (enemy.c's four model makers); used on both sides of the only call, it is rebuilt
       from the task's home after it (bmove.c `func_8017D73C`). Set after the call, it is folded.
     - **A chain `a = b = c = K` stores a first**; write the chain in the game's store order.
     - **A global's address kept in an s-register across calls where the game re-loads it with
       `lui`**: read it as an element of a neighbouring global's array the function also uses
       (`((Frame **) &D_80172F0C)[10]` for D_80172F34, demotool.c `func_801A4E20`): the reverse of
       rule 43.
     - **The type of a loaded character decides `beq` operand order** (u8 `beq K,c`, s32
       `beq c,K`) and whether a pointer computed from it reuses its register (ascii.c, measured).
     - **A function parked only because the build compiled it differently** (the file's other
       parked C changed its schedule) matches once those are unparked: eneproc.c
       `func_8019D748`. Re-check a file's last parked functions after unparking the others.
193. **More lessons:**
     - **Calls the game makes through a register are pointer calls in the source.** `jalr` on an
       address built with `lui/addiu` (often hoisted into an s-register before a loop) is
       `(*func)(...)`; a direct call compiles to `jal` and moves every register after it.
       `tools/permuter.py calls` lists the parked functions where the counts differ. shim.c
       `func_801A06E8` and `func_801A0A44` matched with it plus their frames; `func_801A0D64`
       went from 105 to 53 lines.
     - **Where the frame is short and every local sits too low, the missing words are below
       the locals** (`s32 pad[n]` declared last); where the frame is right but a local sits too
       low, a word is missing above it (declare it first, or another local before it). shim.c
       `func_801A06E8` (pad[2] last), `func_801A0A44` (`in`, `all` first, pad[4] last),
       `func_801A0D64` (r and i before `reward`), ride.c `func_800E5BB8` (the MtxF first at the
       frame's top, pad[3] under p).
     - **`a[p] = --x;` schedules the array address before x's store**; the game's
       `x--; a[p] = x;` stores x first (menu.c `func_800EA5A4`). There a loop-invariant pointer
       local (`file = &T[no]`) read in place instead (`T[no]`) swapped two s-registers back.
     - **A chained store `m[3][0] = m[3][1] = m[3][2] = 0.0f`** stores in that order and keeps one
       `mtc1` for all three (ride.c `func_800E5BB8`).
     - **Also:** a u8 local holding a loaded byte sat one register off in a
       short leaf; as s32 (the `lbu` stays) it matched (sndcmd.c `func_8000D488`). A
       `T *k = &G[idx]` local made IDO copy the pointer into a second s-register and reload a
       field each pass; `G[idx].field` in place matched (gfxmgr.c `func_800D4C24`). A
       `lui/addiu` pair masked with 0xFFFFFF is a link-time symbol, `(u32) D_8000000 &
       0xFFFFFF`; a literal 0x08000000 folds away (stage.c `func_800DB6A0`).
     - **Strings move when a function is unparked.** A function whose prints' strings sit in
       another parked function's `.s` must keep its prints under `#ifdef NON_MATCHING`, or the
       build doubles them (stage.c `func_800DB6A0`, whose strings func_800DB530 carries). A
       function whose `.s` carried other functions' strings frees them: their guarded prints
       must lose the guard (menu.c `func_800EAA20` after `func_800EA5A4`). Both builds first
       differed at a relocation into .rodata.
194. **More lessons:**
     - **Float literals must be written as spimdisasm prints them.** `check` compares instructions,
       so a constant one bit off passes it; only the full build shows it (plchara.c
       `func_801739FC`: 1.1479592f and 1.147959f both missed 0x3F92F053; 1.147959113f is it).
     - **An int literal in a float expression** (`x < 0`, `y2a[lo] * 3`) stops IDO sharing a float
       constant another branch built and can flip a neighbouring `add.s` (plchara.c, zanzou.c
       `func_800F9DBC`).
     - **A parameter re-read from its home as a halfword** (`lhu s0,0x26(sp)`) and re-masked after
       the first call is an s32 parameter cut to a callee's u16 at each call (scenario.c
       `func_800E8958`).
     - **A pointer parameter stored twice at entry** (its home and a local slot) is a base-type
       parameter cast into a derived-type local, `Frame *frame` / `RideFrame *r = (RideFrame *)
       frame` (ride.c `func_800E6834`).
     - **A local the game computes at the top that IDO sinks to its use** stays put if one of its
       operand variables is reused for a later value (plutil.c `func_80189D78`). A loop bound held
       in a register is a local `s32 max = 100;` (tmanager.c `func_800D3EC0`).
     - **The game frees twice:** enemy.c `func_80196A74`'s failure path frees info and used, then
       the `fail:` label frees them again; written as the game has it (4 lines left).
     - **s16 locals take two-byte slots:** vanime.c `func_801738A0`'s four blend locals as s16 cost
       8 bytes of frame instead of 16 (8 bytes still over).

195. **More lessons:**
     - **One absolute load per element, each with its own `lui`, t-registers rotating, is a
       constant-trip loop IDO unrolled fully.** Hand-written tests of `D[0]`..`D[3]` let IDO CSE
       the base into one register instead (stage.c `func_800DB8C4`: "the absolute loads differ"
       was the whole note, and the loop matched it outright). Its reset loop, a walk of step 10
       ending at `%hi(G + 0x28)`, is an index loop over `G.arr[i]`, not a pointer walk.
     - **When only the base differs** (the game `lui/addiu 0x80088420` + 0xA2, the C
       `0x80088338` + 0x18A), the game named a struct member at its own address. A hand-written
       symbol in `undefined_syms.txt` reproduces it (demo.c `func_801A29D8`/`func_801A2B9C`).
       Found by comparing the `%hi` address sets of the game and the C in every parked
       function's check log.
     - **A table read through a local pointer is indexed afresh each pass** (`addu t6,a3,a0`);
       read as the global array, IDO strength-reduces it to a moving pointer (pic.c
       `func_80006E24`).
     - A local that only carries a value to a field costs a frame slot (camera.c
       `func_800CCD90`); integer zeros keep stores off a test's 0.0f register (plmodel.c
       `func_801847B4`); a two-dimensional table declaration spends the extra temporary the
       game spends after a lookup (plact.c `func_80183E98`).

196. **More lessons:**
     - **Frame slots follow declaration order, top down, register-only locals included** (the
       first declared gets the highest slot). Read the game's offsets and order the declarations
       to them: stage.c `func_800DC774` (count and i above the pads above the sky pair, init right
       under it), enemy.c `func_80197864` (z, y, x from the top). A trailing unused pad can take no
       slot at all (managerpl.c `func_8018B770`: the pad only counted placed before `fell`).
     - **A call's result kept in a named local** trades a compiler temporary for a slot
       (plshot.c `func_8018EE64`: `s = func_800D0870(..., &c);` gave the game's frame and slots).
     - **A value that takes a loop walker's s-register is assigned after the test that ends the
       search** (enemy.c `func_80197864`: `f = arr[i]` inside the kind test, so f and the walker
       never live together).
     - **A global the loop updates, copied into a local, has its store hoisted out of the loop**;
       read in place it stays in (pic.c `func_8000710C`).
     - **An int add of a (s16)/(s8)-cast trunc**: the trunc written first gives the game's
       `addu old, trunc` (vanime.c `func_80174448`).
     - `e = &d[i]` in an index loop gives a walker plus a counter by sizeof against `nd * sizeof`
       (enemy.c `func_80196E3C`); a for's comma init zeroes after the pointer setup (stage.c
       `func_800DC774`, as `func_800DBCAC`).
     - One shared `return v;` fills jr's delay slot (sndtrack.c `func_8000CB74`); a
       field read twice in a later loop is a CSE temporary whose slot lands above an earlier
       loop's spill (enemy.c `func_80196A74`); two FAKEs, recorded as such: a never-true unsigned
       test that spends a register (fileres.c `func_80007C78`), a dead-parameter copy that swaps
       two pointer registers (mathutil.c `func_800D28A0`).

197. **A file's own .bss, and more:**
     - **How to give a C file its .bss** (audio.c, the first): (1) prove it with `check` first,
       defining the variables in the file (a duplicate of the asm blob's labels does not matter to
       a single-file compile); (2) read the layout from the check's object, `objdump -t
       full.o | grep bss`: IDO orders .bss by each variable's FIRST declaration (externs count,
       headers included; sm64's prevent_bss_reordering.h), so declare the externs in address
       order, filling unused bytes with a declared variable of their own; (3) split the yaml's
       bss blob at 16-byte boundaries (the linker's SUBALIGN) with `{ type: .bss, vram: ...,
       name: <file> }` between; (4) size the variables in symbol_addrs.txt, or the parked
       functions' asm keeps naming their fields by labels nothing defines; (5) `build.py
       --split`, between runs only (it regenerates asm/).
     - A work pointer built in an if body and again on the skip path comes from an else
       holding only a compiled-out print (demo.c `func_801A24FC`: the function's own string is in
       the file's .rodata); a table read as s16 pairs `(c - ' ') << 1` keeps the -32 out of the
       base (ascii.c `func_800D8290`); an `(s32)` of a float always spends one t register before
       its mfc1, and t registers come back in the order they were freed (plmain.c
       `func_8018ABB0`); `(s32) G.base + G.size` with a u32 base gives `addu base,size` (heap.c
       `func_800054A4`); naming `sr - 1` in a tile loop drops nine compiler temps (pakfiler.c
       `func_8017570C`).
198. **Array elements versus struct fields, and more:**
     - **IDO sorts the operands of a commutative op when one is an array element; a struct field
       keeps the source order.** Found twice in one day: `radius * scale[0]` (a u16 times a float)
       loads `scale[0]` before the radius's unsigned conversion whichever way round it is written,
       and the game's order (convert, then load into the next float temp) is
       `radius * ((Vec3f *) scale)->x` (acoll.c `func_800F032C`, by hand); gDPFillRectangle's
       `scissor[0..3]` OR pairs came out reversed until read as `((Scissor *) scissor)->ulx`
       (gauge.c `func_800DE210`, 90 to 24 lines). Measured on a scratch file: five spellings of
       the array form (casts, a local for the radius, `*scale`, a pointer local) all load the
       element first; the Vec3f field alone does not.
     - **A pointer-chasing count loop is unrolled only with its count in a local:**
       `n = m->link; for (i = 0; i < n; i++) f = f->parent;` unrolls by 4 with a remainder loop;
       `i < m->link` inline stays rolled (`slt`), even though IDO hoists the load either way
       (acoll.c `func_800F032C`; scratch-tested). The reverse of pic.c `func_80006FA0`,
       where a rolled clear needed the bound as a local ON THE LEFT, `n > i`, and an inline
       `size / 4 > i` unrolled.
     - **Locals that are spilled around a call in a loop** (`sw v0,0x50(sp)` / reload after the
       jal), not given s-registers, came out once the unroll above was right: the same variable
       for both loops' counters, their slots set by declaration order with a `pad[2]` above.
     - More, each proven where named: one statement line per player's float stores
       sets the game's load pairs (demo.c `func_801A1FC0`); a store and immediate reload at one
       slot is a named float local kept in memory (ride.c `func_800E5D40`); a temp spilled 4 too
       low with the right frame means one local too many above it, so drop a named difference
       and write it inline (impact.c `func_801809F0`); a compiled-out PRINTF between
       `w = BOBJ(obj)` and w's first use stops IDO folding that use (bmove.c `func_8017B4A0`); a
       dead PRINTF in `if (x >= N) PRINTF else store` turns the counter into a byte offset
       (geography.c `func_800E21CC`); a chained `g8 = (s32) (gptr = NULL)` spends one ugen
       temp (scenario.c `func_800E7278`, still parked).
     - **Two fixes wait on overlay 9's .data** (one blob, `[0x609800, data]`): plshot.c
       `func_8018FA94` needs its handler table defined in the file (as1 fills a switch's `beq`
       slot from the fall-through only when the function's address is taken in the same file),
       and plonigiri.c `func_80177290` matches with its place as a function-local `static Vec3f`
       (absolute loads sharing one `lui at`).
199. **A file's own .data (plonigiri.c):**
     - **Range:** list the blob's labels with the files whose code (C or parked asm) names each;
       the file's run of labels, from its first to the next file's first, is its .data (here
       0x801A7220-0x801A7290, 16-aligned at both ends; trailing zeros up to the next file are
       the alignment, not a variable).
     - **Order:** IDO lays .data out by each variable's FIRST declaration, externs included (as
       .bss, rule 197), and a function's statics where that function is. An `extern` at the top
       of the file pins a variable there; to put a global after a function's static, declare it
       no earlier than the static (a block-scope `extern` inside the function, after the static)
       and define it after the function. Measured on a scratch file and on plonigiri.c.
     - **A function-local static is not a file-scope static to uopt:** the place's fields loaded
       through one `lui at` (as1's macro loads, D+0/D+8/D+4) only as a static inside the
       function; as a file-scope static, IDO kept its address in v1, as it does for a global
       (plonigiri.c `func_80177290`).
     - **Values:** a segmented address in the blob that splat wrote as a symbol (`.word
       D_3000D30`) is a plain number in C (`{ 0x03000D30, 4 }`); `-0.3f` is 0xBE99999A.
     - **Then** split the yaml, `[0x609C80, .data, plonigiri]` and `[0x609CF0, data]` after it,
       and `build.py --split`. Both hashes matched on the first build.
200. **One shared 0.0f, the other zeros int (acoll.c `func_800F11C4`, 218 to 0):**
     - The game held 0.0f in f0 across `dy >= 0.0f`, `dy <= h` and `dy > 0.0f`, but made fresh
       zeros (`mtc1 zero`) for a ternary's `: 0` and for `(0 - dz) * t`. All six as 0.0f: the
       third test re-made its zero and the temps shifted. The tests' zeros as `0.0f` and the
       others as int `0` gave the game's code (rule 195's "an int zero keeps a float zero
       un-shared", used the other way round).
     - **Copies cost callee-saved floats:** `pz = qz = ...` with qz/qx locals pushed r and h into
       f22/f24 (frame 0x70). The point as one `Vec3f p` in memory (its address goes to the
       rotate call), with `p.z` reused in the test, kept r/h in f16/f18 (frame 0x60) as the game.
     - `h / 2` (int) stays a `div.s` by 2.0, as rule 197's `x / 2`; `h / 2.0f` became `* 0.5f`.
     - A familiar trap hit again: a parked check's object is `full.o`.
     - **Also landed in the same stretch:** gfxmgr.c `func_800D4A58` matched with a dead
       `size * 0` in the head's address (`buf + size * 0`): it keeps the tail's size load in the
       loop's register; pic.c `func_80005B98` read a just-stored field back (`pic->dst`) instead
       of the global; a dead local holding a dead check's value still takes a register
       (gfxbuf.c `func_800D0630`, 16 to 6); an empty `if (x == K) {}` raises the hoisted
       constant K's s-register rank (plpick.c, vanime.c, not matched).
201. **Strings for a file whose prints move registers (managerpl.c `func_8018B770`):**
     - An empty-macro print, `#define DPRINTF` then `DPRINTF("x\n");`, is a discarded string
       expression: no code, but IDO still emits the literal in .rodata, at its place. (Measured
       with `objdump -s -j .rodata` on the check's `build.o`.)
     - The FAKE string block can live in an EMPTY function (`void f(void) {}`): `if (0)` prints
       there change no code anywhere, and the strings land in source order. Proof: the file's
       .rodata came out 0x140 bytes, the game's size, and both hashes matched on the first build.
     - Method that made it one try, not ten: dump the parked function's `.s` rodata (all twelve
       strings were in it), compare with the check object's `.rodata` dump, fix the difference.
202. **One variable for every loop (sccheck.c `func_800ED5F4`, the permuter's find):**
     the two searches reuse the talker's index `i` (`s32 i = pl - 1;`, then `for (i = 0; ...)`
     twice) instead of a separate `j`. With `j`, the 3 and the -1 coloured v0/v1 the other way
     round (70 lines); with `i` reused, MATCH. A 70-line register puzzle that ten hand forms had
     not moved; the permuter found it within its run (score 1580 to 130, then exact by hand).
     For a function whose note lists only colouring differences, run the permuter first.
203. **Two layout levers read off the game's asm (ride.c `func_800E5594`, 160 to 0):**
     - **A result written back into the source variable's register** (`cvt.s.d fv1` where fv1
       holds dx, then `mul.s fv1,fv1,-1`, then the store) is the variable modified in place:
       `dx += 0.1; if (q.x < 0.0f) dx *= -1.0f; push->x = dx;`. A separate `v` cost a frame
       slot and the moves (frame 0x60 for the game's 0x58; 160 to 54 lines).
     - **Where the switch's default sits in the source sets the block order.** The game's
       compare chain ends `b exit; move v0,zero` (the default's return right after the
       compares) and the shared tail (the call after the switch) follows the LAST case. With
       `default: return 0;` written last, IDO put the tail after case 1 and the others branch
       back to it (+2 instructions); written first, MATCH.
204. **Two spent temporaries from a dead (double) round trip (gfxbuf.c `func_800D0630`, Goemon
     (236), the permuter's find):** a dead local `s32 size = (double) (end - base);` read only
     by an empty `if (size > N) {}` keeps its colouring (v0) and spends two ugen temporaries
     (the int->double->int conversions, removed later), so the next values take t8/t9 as in the
     game. An `f32 size` lost v0 (16 lines). FAKE: it stands for a debug size check whose form is
     unknown. Also proven there: the dead check's empty if keeps the colouring with no PRINTF,
     so no third copy of the string.
205. **A block split between a pointer and its first use (managerpl.c `func_8018C380`, the
     permuter's find, 74 to 14 lines):** `Player *p = PL(players[i]); do {} while (0); if
     (players[i] != NULL) used[p->chara] = 1;` recoloured the whole function (the 0x50 bound, n,
     sum) toward the game. The same mechanism as bmove.c's compiled-out print between
     `w = BOBJ(obj)` and w's first use (rule 198's list). FAKE: it stands for a print or
     block the source had there. Not a general lever: after audio.c `func_8000A928`'s loop it
     made 10 lines 22.
206. **Read the game's slot map, then three proven levers (demo.c `func_801A274C`,
     206 lines to 0 in seven single-cause checks):**
     - List every `0xNN($sp)` the game's asm touches (`grep -o "0x[0-9A-F]*(\$sp)" | uniq -c`):
       the touched slots are the memory-resident locals; the untouched words between them are
       register-only locals or pads, in declaration order top-down. Lay the declarations out
       to that map (pads where nothing is touched).
     - A work pointer the game stores to its home right after making it (`addiu t0,o,0x20; sw
       t0,0x30(sp)`) and reloads at the end is set AT ITS DECLARATION (rule 132): assigned in a
       statement, IDO sank it to its last uses and remade it from o (no store, frame -8).
     - A field tested and then passed (`lw a0,0x5C(t0); beqzl a0`) is read once into a named
       local (shim.c's lever), the local taking a pad's slot so the frame holds.
207. **A chain's stores come out inner-first, and its temps are the "unused words" (polygon.c
     `func_800D73EC`, 134 to 28 lines):** the game stores one value to 0x6C,
     0x5C, 0x4C in that order (mf[2][3], mf[1][3], mf[0][3]): that is `mf[0][3] = mf[1][3] =
     mf[2][3] = 0.0f;`. The permuter found the two-store half (134 to 51); the full chain moved
     mf up 8 bytes, because each link costs a temp slot (rule 42), and those two slots were two
     of the five words under mf the C had filled with FAKE pads. Three pads, then 32 lines.
     **When a frame needs FAKE pads, look for a chain first: its temps may be the pads.** Also
     there: a spilled local's slot (sy at 0x90, not 0x98) moved by declaring it two floats
     lower, per the slot map (206).
208. **"if (a) r |= bit; else if (b) r |= bit;" is not "a || b" (managerpl.c `func_8018BBAC`,
     88 to 0):** the game's `bc1fl L; sub.s (in the likely slot); b next; ori rX,bit;
     sub.s (a dead copy); L: c.lt.s ...` is two bodies. `a || b` shares one body (bc1tl with the
     ori in its slot) and also swapped two s-registers. In the same loop, the list cleared
     (`ps[i] = NULL`) before the player is read put the pointer step in the bnez delay slot and
     the compiler temp at the game's 0x48.
209. **An empty test starts an address's live range (beffect.c `func_80194C0C`, 61 to
     0):** the game builds `&D_8019E800` in a2 before `bnez keep` and has `bnezl; lw t6,0(a2)`.
     A FAKE `if (D_8019E800) {}` right before `if (w->keep == 0)` emits nothing but starts the
     address's range there. Inside the keep block it starts one block late (16 lines); `== 1`
     shares the constant with the real test (55). No change from a bare `D_8019E800;`,
     `if (0) { D_8019E800 = 0; }`, a static, a struct cast, or the block on one line.
210. **An empty test after a call keeps a parameter live across it (vanime.c `func_80173E8C`,
     18 to 6):** `if (addr == 0) {}` (FAKE) between the call and the next statement
     holds addr's range over the call while IDO colours, which takes a low s-register and moves
     obj from s1 (the copy loops' 3) to s5 (shared with the keys' stride 10), as in the game.
     The same test on a parameter live anyway (file) changes nothing. Left there: the prologue
     copies obj first and sets a1 before a0 in the game (its sibling func_80174448, which has no
     such test, gets a1 first naturally).
211. **A signed difference gives a second load (file.c `func_80003E50`, 116 to 0):**
     the game loads the spilled rom twice in one block, once in a subtraction and once as an
     argument; `(s32) (X & 0x7FFFFFFF) - (s32) rom` passed to an s32 parameter is a different
     load from the u32 argument, so IDO keeps both. Also there: `file - 1` inline every time (a
     `k` local cost 58 lines); `i = 16;` above the zeroing loop keeps the delay loop's
     `li 16; beqz` entry test; lengths inline so their temp takes the game's 0x20 (frame 0x40).
212. **FAKE pads at the frame's top may be real locals declared too low (zanzou.c
     `func_800F85A0`, 320 to 0):** the C had two scalar pads above dist and turn and a
     frame two words too big, with the gap just below a memory array (d). The pads were the
     loop index and the first knot: declared in the pads' places (not at the bottom), the two
     extra words under d went and the frame came to the game's 0x170. **When the top has N
     unexplained words filled with pads AND the frame is N words too big, move N register-only
     locals up into the pads' places.** Also there: a value the game stores and reloads after
     other stores is a memory array (`f32 d[3]`), not a Vec3f IDO promotes to registers; a float
     call result added in a sum goes through a named local (`h = f(); (yaw + h) * K`); each
     step's change written before its flag (`w->step -= 0.1; w->unk5B = 1;`) gives the game's
     bnel with the next block's load in the slot.
213. **A multiply by 1.0 in the game is a variable (zanzou.c `func_800F7C6C`, 633 to 361
     lines):** IDO folds a literal `x * 1.0f` away, but the game multiplies the tip's three
     coordinates by fs4, the 1.0f it loaded for `nframes - 1.0f` and kept in a fifth saved FP
     register. `f32 scale = 1.0f;` and `... / 32.0f * scale` keep the multiplies, share the
     constant with the subtraction, and give the game's frame (0x60, five FP saves). Look for
     `mul.s` by a register that holds 0x3F800000.
214. **Store offsets one element off in an unrolled loop = the C's index is off (zanzou.c
     `func_800F982C`, 238 to 42 lines):** the game's remainder loop steps a0 and then stores at
     `0(a0)`, and the unrolled body at -12/-8/-4/0; the C's were -4 and -16/-12/-8/-4. The game
     writes `d[i + 1]`, the C had `d[i]`, so its later `d[2] - d[1]` read other slopes: a
     behaviour error in the parked C, not only a mismatch. **Check a parked function's array
     indices against its store offsets before tuning registers.** Also there: `* 2` and `2 *`
     (int) keep the game's `mul.s` by a 2.0f held in f2 where `2.0f` became `add.s x, x`; a
     `size` local the game does not have took a slot and moved its n * 4 temps down 4.
     **Matched** with two more: the allocations written as `if ((h = alloc()) ==
     NULL) goto end;` (twice) with `end:` before the frees, not `if ((h = ..) != NULL && (d =
     ..) != NULL) { ... }`. The permuter found a block split before the frees (131 to 10); the
     label is that block, and it also swapped y2 and h into the game's s1 and s3 (the same
     file's func_800F9120 is written with gotos). Then `h[0] * y2[0]` for the game's load order
     (IDO emits a commutative product's operands in reverse).
215. **When the code comes from a textbook, write the textbook (mathutil.c's solvers, Goemon
     (250): four functions, 196-537 lines each, to 0):** the LU decomposition and the inverse are
     Okumura's `lu` and `matinv` (C言語による最新アルゴリズム事典) in floats with n fixed. A
     paraphrase (named pivot/max/w/row locals, `p[i]` indexed inline, the weights declared first)
     missed by hundreds of lines; his text matched at the first check: his locals in his order
     (`i, j, k, ii, ik; t, u, det; weight[]` and in matinv `..., ii` then the arrays last), the
     row read once (`ii = ip[i]`), `u` reused for the largest and then the pivot, `j` left as the
     pivot's row, `t = (a[ii][k] /= u)`, `goto EXIT`. **The frame tells you: the game's slots
     were exactly his declaration list.** The same book's `gaussjordan` (func_800D1424) still
     has one float pair swapped.
216. **Untouched words at the frame's top can be a sibling's array, declared and unused
     (polygon.c `func_800D66B8`, 151 to 124):** the gani draw's frame is 0xA0 with 0x60-0x9F
     never touched; the pol draw beside it declares `f32 mf[4][4]` for its billboard. Declared
     first in gani (unused), it takes those 64 bytes (an unused array does take its space here).
     Also there: a count the game converts with plain `cvt.s.w` is an s32 (a u32 adds the
     `bgez`/2^32 fix-up), and `/ 128` (int) keeps the game's `div.s` by 128.0 where `/ 128.0f`
     became a multiply by 1/128 (the reverse of `* 2.0f` becoming an add, 214).
217. **A callee declared void that returns a value changes the caller's registers (gauge.c
     `func_800DE7FC`, 18 lines to a match, the permuter's find):** `func_8018A1BC` returns its
     init pointer (plmain.c's definition) but gauge.c declared it void; with the true return type
     the mask moved from v0 to the game's a0 (a returning call defines v0 at the call). The same
     fix measured on main.c func_80000F10 (four callees) and scenario.c func_800E7F8C changed
     nothing, so it is a lever only where v0 is in play around the call.
218. **A pointer the game loads into v0 right before a call, where IDO here uses v1: read it
     into a named local just before the call (impact.c `func_80185EDC`, 476 to 214):** the
     game's `lw v0,0(s0)` (w->frame) feeding a call's arguments came out `lw v1,0(s0)` with
     `call(w->frame->pos[0], ..)`; `f = w->frame; call(f->pos[0], ..)` gives v0, one spot per
     assignment. Not for stores: `w->frame->x = ..` and copies into another object stay inline
     (the game reloads w->frame after each store, which a local would drop, making the
     function shorter). The local took one of the FAKE pad words. Integer twin of the float
     lever "a new float local gets f0" (plonigiri.c func_80176558).
219. **A copy loop the game keeps rolled: start the index from a variable (file.c
     `func_800040D0`, 134 to a match):** `for (i = 0; i < (u32) (end - buf); i++) *out++ =
     buf[i];` unrolls by 4 here; `zero = 0; for (i = zero; ...)` stays a rolled byte loop with
     the game's `sltu` test and `end == buf` guard (FAKE: the variable costs a slot, placed by the
     slot map; the permuter then found the declaration order). `u32 i = 0;` with `for (; ...)`
     also stays rolled but with other registers (63). Not general: plmain.c func_8018A5F8's
     166-word clear unrolls anyway (its goto form stays rolled).
220. **A texture load whose dxt is a runtime `div` by a register holding 1 had a variable width
     (gauge.c `func_800DD314`, 21 instructions short to 3):** `gDPLoadTextureBlock_4b(.., 16,
     h, ..)` folds `CALC_DXT_4b(16)` (bpause.c's matched 16x16 load has no `div`). The game's
     `li t3,1; addiu t9,t3,0x7FF; li a1,1; div t9,a1` with the `break 7`/`break 6` checks is
     `TXL2WORDS_4b`'s `MAX(1, width/16)` over a local `width = 16`: cfe cannot fold the
     ternary, uopt folds its condition late and leaves each arm's 1 in a register. The same
     variable feeds `(x + width)` in the rectangle. Also there: **a few-case `if` chain on one
     value that the game compiles to `beq` with the bodies after is a `switch`** (rule 8), and
     the constant in each `beq`'s delay slot is the first assignment of that case.
221. **A float function that ends with no move into f0 had no `return` (mathutil.c
     `func_800D1424`, 25 lines to a match, found by the permuter):** with `return det;` IDO puts
     the pivot in f0 and det in f2 and ends with `mov.s f0,f2`; the game ends on `jr ra` with
     det in f0 and the pivot in f2. The original computes det and falls off its end. Some 25
     forms of the returning version (declaration order, literals, test placement, block
     splits) all stayed at 25. Check the epilogue for the return move before grinding the
     registers of a returning function. Also: a compiled-out print (`if (0) osSyncPrintf(..)`)
     still emits its string into `.rodata`, so a splat string block for it must go when the
     function is unparked (else the rodata after it moves).
222. **IDO 7.1 converts a u16 (or u8) to float as unsigned, with the 2^32 fix-up (`bgez`;
     `lui at,0x4F80; add.s`) (sys.c `func_80004880`, 693 to 635 with this alone):** the game's
     u16 fade colours have the fix-up and its scissor corners, loaded with the same `lhu`,
     do not, so the corners were signed locals (`s32 ulx`); as u16 locals they got four extra
     fix-ups. Count the `0x4F80` fix-ups in both before typing a converted variable. A `(u32)`
     cast on a u16 changes nothing.
223. **A macro's `_g`, spilled across a call inside the macro, goes to its own home, and that
     home sits after the slots of every local declared before it (sys.c `func_80004880`):**
     the game spilled gDPSetColorImage's `_g` (across the scheduler call in its argument) 7
     words below its frame's top, so only four function-scope locals precede the macro blocks;
     with all twenty fade variables at function scope this C spilled it near the bottom. The
     fade's variables declared inside `if (fade) { ... }` put it at the game's place. The same
     change with y set before n (the game reused one load of fadeStart; set after the macros'
     stores it is reloaded): 635 to 410.
224. **The game's block order gives the if/else and exit shape (scenario.c `func_800E7F8C`,
     522 to 32 on these alone):** IDO lays blocks out in source order and does not merge equal
     tails, so:
     - a test whose branch goes to another path's `b epilogue; li v0,1` shares that `return 1`:
       the source is `if (x != NULL) { if (f() == 3) {...; continue;} } else {...} return 1;`,
       not two `return 1;` statements;
     - a path that ends in its own `b loop_head` (the loop test duplicated) has its own
       `continue;` (here an if/else followed by one `continue;` gave a join instead);
     - a `move v0,zero` that falls into the epilogue, reached by a `b`, is a `return 0;` after
       the switch that a case reaches by `break`;
     - a pointer the game builds with `addiu vN,vN,8` before reading a field at a small offset
       is a local (`next = w->pc + 1`), where `w->pc[1].arg` folds to one offset;
     - a call on the object's work through `w` (s0) and not `obj + 0x20` is `w->flags`, not
       `SC(obj)->flags`.
     Compare the order of the `jal` targets first (one shell loop over the parked set found
     this function).
225. **A switch whose out-of-range path loads a value in a block of its own (after the last
     case, which then ends in a `b` over it) has an explicit `default: break;` (zanzou.c
     `func_800F7C6C`, 270 to 178 with lever 157's ternary just after):** uopt keeps the value
     each case already loaded (w->weapon in a0) for the code after the switch and inserts the
     load on the default path; with no default the out-of-range edge has no block to take it,
     and IDO reloads the value once after the switch instead. Count the unconditional `b`s
     in both: one fewer here is that last case's jump.
226. **`beq sN, vN` (the constant's register first) compares a loaded value as an expression;
     `beq vN, sN` compares a variable (scenario.c `func_800E7F8C`, 28 to 16; the same order in
     the matched pic.c `func_80005F50`):** with -23 and -21 held in s1/s2, `c = *++w->text; if
     (c == -23 || c == -21)` gives `beq v0,s1`; `if (*++w->text == -23 || *w->text == -21)`
     gives the game's `beq s1,v0` (the second load is CSE'd). Writing `-23 == c` changes
     nothing. A value read through a pointer (`s = func(); if (*s == -23 ...)`) behaves the
     same.
227. **A matched function whose dead prints' strings live in an assembly neighbour's string
     block (splat attaches unreferenced strings to the function before them):** the C's
     literals would land a second time and move every later .rodata address, and dropping the
     prints in the build changes the function's registers (a dead print's place matters). Keep
     the calls and swap only the literal: `#define SC_STR(s) D_8010D928_ovl_0006` in the build
     (the block's symbol; the call is dead, so nothing is referenced), `s` under NON_MATCHING
     (scenario.c `func_800E7F8C`, matched with both hashes).
228. **Saved registers given to the wrong pointers among many equal ones can turn on one block
     boundary (zanzou.c `func_800F9120`, 669 to 194):** the game kept ta/x2a (s6/s7) and xk (fp)
     and read yk/zk/y2a/z2a from memory; this C gave xk/yk/zk the registers. An empty
     `do {} while (0);` at the head of the return-1 path (the permuter wrapped seven statements
     in one; by hand only the head mattered) gives the game's choice. Empty splits after the
     copy call, after the second free and before `return 1`, and the end test as nested ifs,
     change nothing. A dead `PRINTF` (`if (0) osSyncPrintf`) at the head does the same as the
     FAKE, so a compiled-out print is a candidate for such a boundary when its string exists.
     When three or more equal-use pointers trade registers, try one boundary per path head.
229. **A grid walk's innermost `if (cell != EMPTY) { ... }` whose game code is 4 instructions
     longer and reloads the loop bounds from their homes is `if (cell == EMPTY) continue;`
     (floorhit.c `func_80195068`, 336 to 130, then a match):** the continue goes straight to the
     innermost step, the if-form joins first. The rest was the frame: the locals declared in the
     order of the game's slots (each read off its first store: the s16 corners above the loop
     indices above the grid's dimensions, the hit pointer below them), two FAKE pads for 24 bytes
     the original used for something unseen, and the zero chain written in the game's store
     order (`push.z = push.y = push.x = 0.0f` stores x first; rule 912). Its ray twin (vram.c
     `func_800E42B0`) takes the same loops this way, but then gives `at` the stack (305).
230. **Locals of two separate blocks: the block written first gets the higher slots. When the
     game has a later block's locals above an earlier one's, declare them in a block that
     encloses both** (impact.c `func_80185EDC`: stage 1's `Vec3f at; f32 t;` sat below stage
     0's `tx`/`tz`; declared at the head of message 0's case block, which holds both stages,
     they come out above, as the game). Then fix the leftover word with the function-scope pad
     and one pad in the lower block (the frame rounds to 8, so a 4-byte change at the top moves
     the frame by 8).
231. **A multiply whose result the game computes while copying a struct (the copy's lw/sw
     between multu and mflo) was written BEFORE the copy** (vram.c `func_800E42B0`: `best =
     &w->coll->tris[*l];` then `*at = hit;`). Written after it, IDO copies first and waits on
     mflo with a nop: the function is one instruction longer and every later branch is off.
232. **The game reloads a field it has just stored (`swc1 f, 0xE0(s0)` ... `lwc1 f6, 0xE0(s0)`
     for a copy): IDO forwards a stored value within a block, so the original had a block
     boundary between the store and the read.** One FAKE `do {} while (0);` after the store
     gives the reload (plmain.c `func_8018A5F8`, `unk0E8 = unk0E0` after `unk0E0 = 0.6f`, 383
     to 332; plchain.c `func_80175968`, `prev = f->pos` after `f->pos += vel`, 387 to 338). A
     second pointer to the same field (`link->frame->pos` for `f->pos`) is worse (446): it adds
     loads. A dead PRINTF would be the natural boundary; in both files every string is taken.
233. **A rolled counting loop whose game test is `sltiu at,i,N; bnez at` (no bound held in a
     register, no `bne i,rN`): write it as a goto loop with a block boundary before the
     back-edge goto** (plmain.c `func_8018A5F8`, the Player's 0xA6-word clear, a rule 44 case):
     `q = (s32 *) p; i = 0; clear: *q++ = 0; i++; if (i < N) { do {} while (0); goto clear; }`
     (FAKE). Without the boundary IDO replaces the test by `bne i, rN` with N in a register (the
     goto form) or unrolls it (for, while, do-while, `while (TRUE)` with a break). The pointer
     set before the counter gives the game's pointer v0, counter v1; the indexed `p[i] = 0`
     gives them the other way round.
234. **t-registers one behind the game's from some point on: the original spent one ugen
     temporary there that the optimiser then folded away. A FAKE double shift in the same
     direction, `(x << 4) << 4` for `x << 8`, folds to one instruction but keeps the temporary's
     number spent** (contpak.c `func_80002F40`, 100 to 50; the permuter found it on a byte of
     the word read, then moved by hand to where the shift is right). Move it statement by
     statement: too early and the registers between run one AHEAD. Measure a permuter find
     inside a macro's expansion in the real compile: base.c reprints a macro over lines, but
     a find there can still be real (READ_BE32 was this file's own macro; written out, the
     find carried).
235. **The natural spent temporary: a conversion straight to the narrow type the result is
     stored in** (zanzou.c `func_800F7C6C`, matched): `w->knots = (u8) nframes - 1;` for a u8
     field spends one temporary more than `(u32) nframes - 1` (the u8 mask folds into the
     store's own `& 0xFF`, its temporary's number kept), so every later t-register comes out as
     the game's (240 to 18; the last 18 were one statement's order). Before reaching for
     lever 234's FAKE, try the narrowest cast the destination allows.
236. **`abs.s f0,fX; mov.s fX,f0` (an intrinsic's result copied back into the register the
     operand was loaded into) is the parameter reassigned: `near = fabsf(near);`, and where the
     game does it among its first loads, as the first statement** (sndloc.c `func_800D9CD4`,
     51 to a match: a separate `n = fabsf(near)` gave `abs.s fX,f0` straight into n's
     register). Also: **IDO does not merge identical string literals** in one file (two prints
     of the same message compile to two copies), so the game's one copy means one print: give
     it to the function whose match needs it (here func_800D9CD4, not its sibling).
237. **A switch's case bodies come out in source order, the default's too (font.c: a default
     written last is laid out last, 580; plact.c `func_80183E98`'s note says a bare `default:`
     label goes right after the compares, which is a different case); code the game lays
     out right after the default's body, which other cases reach with a `b` (the target's first
     loads copied into their delay slots), is the default's own code, and those cases
     `goto` it** (font.c `func_800EB1DC`, 595 to 468, parked on registers): the compare chain,
     then `default: i = c - ' '; if (i < 0) continue; draw: <the glyph draw>; continue;`, then
     '#', '\n', '<', ... with each accent case ending `goto draw;`. The draw written after the
     switch (the cases `break` to it) puts it after the last case body; moving `default:` first
     alone only moves the default (593). Read the case-body order off the game's labels and
     write the cases in that order: it is not the compare chain's (sorted by value).
238. **An `andi 0xFF` at a USE of a value the game loaded with `lbu` means the variable is wider
     than u8 and the expression casts: an `s32 c` and `(u8) c - ' '`** (font.c `func_800EB1DC`,
     468 to 200). A u8 variable is masked where it is assigned, never where it is used, so a
     `u8 c` gives no andi; the s32 also moved the character from a0 to v1 and let IDO hoist two
     constants into a0/a1, as the game does.
239. **`if (x < 0) continue;` followed by code, and `if (x >= 0) { code }` then `continue;`, are
     different shapes: the first gives `bgez x, code; b <loop end>` (two instructions more),
     the second `bltzl x, <loop end>` with the loop end's first load in the slot** (font.c
     `func_800EB1DC`, 200 to 30, with a label inside the if's body that other cases `goto`).
     When the game skips forward with a branch-likely to the loop's end, write the code as the
     body of the opposite test.
240. **One pointer reused across a switch's cases is one variable with one register; where the
     game gives the same role a different register in each case, each case had its own
     variable** (particle.c `func_800F69FC`, 632 to a match, no FAKE): case 3's trail walk in
     its own `t` put it in s3 as the game does, and with s3 saved anyway IDO also hoisted
     case 2's loop-invariant `D_30006A0` into it (the game's `lui s3` before the loop); case 0's
     `prev` in its own variable took v0 where the shared `next` sat in t4.
     **And a user variable spilled to its home fixes the frame between homed locals:** the
     game spills `next` at 0x34 and w at 0x4C, so five words of locals were declared between
     them. Values the game computes before a store they could not pass (r, g, b loaded and
     multiplied before `t->dl = ...`) were those locals: declared there and assigned first,
     they replaced a FAKE `s32 pad[4]` (a pad declared after the last homed local takes no
     slot at all).
241. **A pointer parameter the game keeps in a saved register through a stretch where the C
     leaves it in its home (reloading it): that stretch reads it through a local copy, "g =
     f;" before it** (polygon.c `func_800D60BC`, 175 to a match: the segment loop and the list
     through g, the struct copy and the calls before it through f). The copy is a variable of
     its own, so IDO colours it apart from the parameter. Found by the permuter as
     "(new_var = f)->segs[i]" once its base compiled as the real C. Again in
     the gani draw `func_800D66B8` (124 to 31): the copy from the third angle's wrap on puts f
     in s0 and gfx in s1 as the game has them; in the anime draw `func_800D6BE0` the same copy
     is worse (63 from 53), so it is a lever to measure, not a rule to apply blind.
     `tools/permuter.py copies` tries it at every top-level statement for every parameter.
242. **The permuter's base must compile as the real file (`tools/permuter.py basecheck`).**
     pycparser's reprint puts each statement of a macro (and "if (0) osSyncPrintf") on its own
     line, IDO schedules by source line (rule 12), so a reprinted base is a different function;
     the import keeps such lines whole with decomp-permuter's sameline pragmas.
243. **Compiler temporaries sit below every declared scalar, so the count of named locals
     moves them: a game temp one word higher than this C's means one named local fewer**
     (floorhit.c `func_80196024`, 53 to 19: x, y, z, t declared put the CSE'd product at 0x14 of
     a 0x28 frame; with only z and t named and the x/y differences inline in their one use each,
     the frame is the game's 0x20). A register-only local still reserves its word above the
     temps. `tools/permuter.py inlines` tries each straight-line definition inlined.
244. **One value spilled across a call at the frame's top, recomputed on the other path and
     joined in a register for the return, is IDO's partial redundancy elimination of the
     RETURN expression, and it needs an else block to put the second computation in**
     (contpak.c `func_8000297C`, matched): "if (c) { ...; f(&G[i]); ... } else { do {} while
     (0); /* FAKE */ } return &G[i];". Without an else IDO recomputes after the join; a named
     local, a ternary or any joining variable takes the top slot itself and pushes the spill a
     word lower. (This corrects 14 for that case: the 0x18 spill was a temp below the local.)
245. **The prologue's parameter copies come in the reverse of the order as1 sets the call's
     argument registers, lower s-register first, unless the copy is a source statement: a
     parameter copied to a local before the first call ("v = vtx;") gets its s-register through
     that statement and so comes later** (vanime.c `func_80173E8C`, matched: the game's
     "move s5,a0 ... move s0,a1" and obj in s5).
246. **uopt colours register candidates by priority (references over blocks spanned, ties to the
     first seen); a FAKE empty test built from address arithmetic on a value, at the END of a
     block where it is live, raises its priority without code: "if (!&T[v + v]) {}" counts two
     uses** (sndcmd.c `func_8000D074`, matched: sustain then beats bank/in/snd for a1-a3).
     "&T[v]" adds one (a tie, lost); a plain "if (v) {}" is folded before colouring; at a
     block's head the test splits the block and lowers the priority instead.
247. **Loop constants hoisted to saved registers rank by uses inside the loop, ties in order of
     appearance in the body, so the loop bound (used at the bottom) comes last; to place one
     value between two others, give the higher and the target one extra use each in the SAME
     FAKE empty test, higher first, on a short-lived temp inside the loop** (plpick.c
     `func_80182208`, matched: "if (((u32) q == (u32) &G) | ((s32) q == 4)) {}" puts the base
     then 4 above 5, 8, 0x80000). On i it added a saved register; before the loop it moved the
     preheader's schedule.
248. **Float values used equally often seem to take float registers in order of first
     appearance in the source (f2, f12, f14, ... here); a load of an address-taken local used N
     times ties with a named product of 1 def + N-2 uses** (polygon.c `func_800D73EC`,
     matched: "sxsy = ...; cxsy = ...;" computed before the first row). Declaration order does
     not break the tie. A model that fits three measurements, not a proof.
249. **A value in an argument register (a1, a2...) for no visible reason, with v1 free, is a
     value the original passed as an extra argument to a call it lives across: IDO prefers the
     register a value is passed in, even when the callee ignores it** (file.c `func_80004638`,
     matched: phys in a1 through "u32 func_800045D4_main(); ... func_800045D4_main(file, phys)",
     the callee declared without a prototype at block scope so the call stays a jal; a cast to a
     two-parameter pointer gives jalr). Also: in a u32 + s32 sum the converted s32 operand
     comes first (declaring it u32 flips the addu); the order of a for's increment steps is the
     order of the latch's updates.
250. **Put the statement that uses all of a group of values BEFORE the stores of those values,
     with the expressions written out again in the stores (CSE'd), when the game does every
     load first: IDO computes each register local just before its first use whatever the
     statement order, and never lifts a stack parameter's load above a store through a pointer
     parameter** (floorhit.c `func_80196024`, matched: "t = (... products ...) / ...;" first).
251. **IDO carries a link-time address across an if/else join in a saved register (PRE, rebuilt
     at the end of the else) only when it appears twice in the if branch; a FAKE address-only
     test "if ((u32) SYM == 4) {}" in the middle of the branch supplies the second use with no
     code** (at the branch's end it is emitted). And one s32 variable used for both a switch's
     value and a counter in a case gives the counter the switch value's register (main.c
     `func_80000F10`, 30 to 18, mix distance 0).
252. **An "x & 1" on an array load in an OR chain is moved to the front and computed at the loop
     top; casts do not move it back. Copy the load into a local right before the store that is
     its last use: the copy shares the load's register, adds no instruction, keeps the AND last
     and gives the game's colouring** (tilemap.c `func_800EE568`, 64 to match: "pa =
     f->palA[i]; buf[i] = (skip << 11) | (g << 6) | (c << 1) | (pa & 1);"). Copied at the loop
     top while the load is still used later, it becomes a second register (a move, 369 lines).
253. **A float used once in one switch case ranks below one used in both; to push values into
     the higher f-registers, put a block boundary after a sqrt: a FAKE dead "s =
     sqrtf(unused); if (s == 0.0f) { do {} while (0); }" emits no code (the test is dropped
     after colouring and ugen merges the blocks) but costs one temp word at the frame's
     bottom, which a 1-word pad among the locals moves back** (sndloc.c `func_800D982C`, 56 to
     match). A dead sqrt of an expression the code also computes is CSE'd with it instead.
     A float local declared as an array (vx[1]) swaps the operands of the adds it appears in.
254. **A variable assigned a link-time address never holds it the way a PRE temp does: IDO
     propagates the address into later expressions, builds it in its own CSE temp (v0) and
     gives the variable a copy ("move s0,v0"), whether the assignment comes before, after or
     inside the expression. A dead store of the address is not a use for PRE (removed first);
     an empty test is. An assigned parameter is homed at entry even when its incoming value is
     never read** (main.c `func_80000F10`, nine forms measured, still 18).
255. **Each chained assignment ("a = b = c = K") adds one compiler-temp slot to the frame: when a
     frame has one temp too many, count the chains first** (gameproc.c `func_800C7B10`: removing
     every chain took the frame from 0x58 to the game's 0x50). Three separate stores of a u8
     constant and a named local "c = K" both give uopt's register.
256. **To lower a compiler CSE temp's colouring rank with no code: a FAKE block boundary after
     its last use ("if (gfx == NULL) { do {} while (0); }"), then a FAKE address-only use of
     it ("if (!&T[size / 12]) {}"): the temp now spans two uopt blocks and drops below the
     others** (msgfrm.c `func_800EC0B0`, 68 to 56: the quotient v0 to a3, four pointers to the
     game's registers). Putting user variables in the same FAKE use moves them the other way.
     A register freed by moving a variable's first appearance is taken by a repeated constant.
257. **The value of an assignment to an s8 global field is reloaded ("sb zero; lb t5"), never
     forwarded; dead-store elimination runs before forwarding ("D = D = NULL" keeps both
     stores), and a literal store after a chain to the same place is dropped. So a chain's
     forwarded-from store always comes first** (scenario.c `func_800E7278`). An empty test of a
     call result just stored, and a load through a local before a call, spend no ugen temp.
258. **In a for loop whose body is only a switch, cases ending in "break" or "continue" compile
     alike; the reloads of hoisted constants that IDO puts in an if's join block do not come
     from a (u8)-cast temp in that block** (font.c `func_800EB1DC`).
259. **When the game loads some of a statement's operands before an earlier store and the rest
     after it, try the sum's operands in the other order** (particle.c `func_800F53B0`, 224 to
     a match): after "p->bright -= 4;", "y = p->y + (p->vy >> 2);" loads vy above the bright
     store and y below it, as the game; "y = (p->vy >> 2) + p->y;" stored first and reused the
     store's register for vy. Moving the decrement after x or y instead: 277.
260. **A constant the game leaves in an argument register at a call is not always an argument:
     read the callee's DEFINITION before trusting a decompiled extra argument** (plmain.c
     `func_8018A270`, 90 to a match): `func_8018C180(Player *p)` takes one parameter; the a1 =
     10 at the call was the multiplier of the 10-byte entries around it. Passing it as a second
     argument cost a `move a1,a2` and moved the constant. Also there: a byte read once and used
     in tests and a store is an s32 local (a u8 local kept a copy in another register), and a
     FAKE found for an earlier form of the C can be what blocks the next one: re-test each FAKE
     after the C under it changes.
261. **A chained store of a constant spends one ugen temporary and stores its INNER target
     first; when a chain gives the game's registers but the wrong store order, build the chain
     the other way round** (camera.c `func_800CAEF8`, 134 to 130): "[0] = [2] = 0; [1] = 0;"
     stored [2], [0], [1]; "[1] = [0] = 0; [2] = 0;" keeps the spent temporary and stores [0],
     [1], [2] as the game (the last one in the break's delay slot). The inner target of an s8
     field cannot be chained from (its value is reloaded, 257).
262. **Four source facts from one match** (enecommon.c `func_8019A048`, 273 to a match, found
     by `screen`'s op counts): (1) one loaded value compared twice in the game is an "else if"
     on the same expression, not a test through a pointer copied from it; (2) the game's
     "or s0,v1" at the head of a branch is a local assigned from the field there ("f =
     e->model;" per branch, after testing e->model->kind), not a local set before the test;
     (3) "x += d" compiles as x + d: write "x = d + x" for the game's d + x; (4) a global read
     as an element of its neighbouring array (D_80172F08[2] for D_80172F10) loads in the other
     order against a field. Unparking: a parked function's assembly may carry LATER functions'
     strings (rodata short by their size, "file N changed"): look for prints held under
     NON_MATCHING for it.
263. **A loop whose test the game evaluates twice in different ways (the entry test from a
     register, the bottom test from a value hoisted out of the loop and spilled across its
     call) has the conversion or expression IN its condition: loop rotation copies it.** A
     variable set once before the loop gives one value for both (sndloc.c `func_800DA52C`:
     "while ((s32) now >= k)" for "end = now; while (end >= k)", 131 to 95, one trunc.w.s more
     as the game). Also there: locals with homes are laid top-down in declaration order and
     register-only locals still reserve their words, so the game's stack map gives the order;
     but the block of locals sits on top of the compiler temps, so one temp word too many moves
     every local up (plchain.c `func_80175968` the same).
264. **A value uopt carries to a join (partial redundancy elimination) must be written as the
     SAME expression on both sides, conversions included** (main.c `func_80000F10`,
     18 lines to a match): branch 1 computes the heap size from file 7's RAM address and the
     join passes that address to the loader. With the size on pointers ("D_80342080 -
     D_801738A0 - 0x47000") and the call on the pointer, the size took s0 and the address s1;
     with the size on u32 ("(u32) D_80342080 - (u32) D_801738A0 - 0x47000") but the call still
     on the pointer, the PRE was lost (48). Both written on "(u32) D_80342080", the call as
     "func_80003A8C_main(7, (void *) (u32) D_80342080)": the PRE'd value is the conversion, it
     takes s0 before size as the game, and the old FAKE second use of the address goes: MATCH.
     Also from these diagnostics (`tools/permuter.py drops`): where this C has
     frame words the game lacks, the statements whose removal alone gives the game's frame
     name the cause (sndloc.c `func_800DA52C`: "last = (s32) prev;"; plchain.c
     `func_80175968`: link's copies of the strength-reduced pointer).
265. **Three facts, measured together:**
     - **A late-folded copy moves a hoisted statement** (pakfiler.c `func_8017570C`, 16 to 6,
       mix 0): `cc -S` is ugen's code before as1 (as1 deletes a load right after its store), so
       a kept "sw vX,N; lw vY,N" means the original's ugen order had another write between
       them. "r = row * 1;" (also "/ 1", "^ 0", "<< 0") in the outer body and "r * 64" in the
       inner loop moves the multiply after the other hoisted code; a copy uopt folds early
       ("r = row", "+ 0", "| 0", "& -1") is propagated and row takes s0 throughout.
     - **Every value one register above the game's (v1/a0 for v0/v1): look for a callee's
       result the original kept live** (file.c `func_800047EC`, 16 to 13 at mix 1, not kept):
       the prototype as its definition (s32), "r = f(...)" and a FAKE empty test of r after its
       last rival give the game's registers. A local with two definitions is not
       copy-propagated; a dead first definition is still removed.
     - **A test of a global right after a constant store to it spends one ugen t-register**, as
       a chain does (scenario.c `func_800E7278`, measured on a scratch copy that #includes the
       permuter's base.c, ~75 forms), but its folded branch leaves an extra block. Dead tests of
       reloaded fields, casts on call arguments or table indexes, index locals: no spend.
266. **More facts** (plchain.c `func_80175968` matched from 180; sndloc.c
     `func_800DA52C` 93 to 10):
     - **A dead print after a loop that names the loop's pointer local keeps that local live
       out of the loop**, and IDO then keeps it as its own copy of the strength-reduced pointer
       ("or s5,s1" at the head, reads after the step through s5) instead of folding it into the
       induction register. Strings the parked .s carries that the C never prints are where
       such prints go (plchain's "%.2f" and "base:..."). Inside the body: no change.
     - **One pointer variable reused in two switch cases gets one s-register in both**; a
       variable of its own in one case lets IDO colour them apart.
     - **Locals all 0xC high with one-word FAKE pads among them: the pads are register-only
       locals declared too low** (rule 212 again); a word missing at the bottom with one extra
       at the top is a named local that belongs higher (plchain's f32 dot for the print).
     - **"s->f = call(); x = s->f;" gives the game's "sw v0; bnez v0; or s0,v0"**; "x = call();
       s->f = x;" schedules the move before the store.
     - **One float reused for several roles in turn removes a named local's word without adding a
       temp** (DA52C: |frame - step|, frame - step, then |last - prev| in one variable: the game's
       frame and every slot). A spill one word higher in the game means one named local fewer.
     - **An address-taken local the game loads at a join into ugen's next t-temp, where this C
       gets it in uopt's v0**: a FAKE volatile on the local gives the game's load (DA52C 69 to 10).
267. **Facts from three more matches** (sndloc.c `func_800DA52C`, gameproc.c
     `func_800C7B10`, pakfiler.c `func_80174210`):
     - **A loop counter's step on the wrong side of a call-save reload: write it in the for
       loop's third clause.** After a call, ugen reloads a caller-saved hoisted value right
       after the jal; "k++;" as a statement after the call (or on its line, or "call(k++)", or a
       do-while "++k" test) comes after that reload, "for (; c; k++)" puts the add before it
       (DA52C's last two lines).
     - **A constant stored three times becomes a uopt register candidate** (three literal
       stores, any three-link chain, a local, (u8)/-1 spellings, volatile stores, splits between
       them). Stored once and COPIED onward ("a = K; b = a; c = b;"), it is forwarded as K but
       loaded once by ugen into its own t-register, as the game has it (C7B10's case 7).
     - **How a read-modify-write is written moves the compiler temps**: "old = x; x = old & m;
       y = old;" put the spill and a struct-return buffer 8 bytes lower than "y = x; x &= m;"
       (174210). A decompiled third argument that only the "== 1" tests' constant explains is
       the register the game left set: call as every other caller does (rule 260 again).
       Block-scoped locals take their own slots just below the function-scope ones; temps sit
       directly under the locals, rounded to 8 (a slack word at 0x18 when locals are 4 mod 8).
268. **Facts from three more matches** (pakfiler.c `func_8017570C`, file.c
     `func_800047EC`, bmove.c `func_8017CDF8`):
     - **A local holding a constant ("s32 tw = 64; ... row * tw") is folded late**, like the
       FAKE "x * 1", and moves a loop-hoisted expression after the other hoisted code with no
       FAKE; a 22nd local (any) took one compiler temp's word (17570C).
     - **A store the game makes and never reloads can be a CSE temp spilled across a call whose
       only later use is an empty test**, which ugen drops with its operand: write the test on
       the original expression ("(s32) file == -1"), not on a named copy (47EC).
     - **With a FAKE boundary right after three stores to a local float array, IDO emitted them
       in reverse and forwarded the one stored last**: write the stores in reverse (and swap
       the commutative add back) to get the game's order (17CDF8). The order of a chain's links
       ("a = b = c = 0") decides three register flags' a-registers; their declarations set only
       their slots. "if (all) return TRUE; return FALSE;" lets IDO put v0 = 0 at a block head.
     - **as1 forwards a word reload right after a store to the same address** and deletes it:
       the temporary stays spent with no instruction (lb/lbu after sb are kept). uopt does not
       forward a stored constant into a later test of a global (scenario.c E7278's note).
269. **More facts** (polygon.c `func_800D6BE0` and zanzou.c `func_800F9120` matched;
     msgfrm.c `func_800EC0B0` 56 to 28, font.c `func_800EB1DC` 30 to 26):
     - **A register-only local takes a frame word in declaration order, and the compiler temps
       fill downward from just below the last declared local**: when the game's temp sits one
       word higher and the frame's top word is never touched, declare the register-only local
       FIRST (in place of a top "s32 pad"): every temp moves up one (6BE0's copy g).
     - **A value used only before a loop can be the loop index reused** ("i = n * sizeof(T);
       alloc(i); ... for (i = side; ...)"): one named word fewer, and the game's one s-register
       for both (F9120: frame 0xB8 to 0xB0 and an s1/s2 swap fixed together).
     - **Any "if" (even a FAKE empty one) between the last command and "return gfx" puts the
       return's v0 copy in another uopt block**, and the pointer temps stop treating v0 as
       taken (EC0B0's last difference).
     - **"i = (u8) c - K" is computed in place only when it is the only definition of i
       reaching i's uses**; a second definition merging at a label after the test gives the
       intermediate a uopt register of its own (EB1DC).
     - **A commutative compare against a hoisted constant: a plain variable operand is emitted
       first ("beq v1,s2"), an expression after the constant ("beq s2,v1")**; a FAKE "x * 1",
       folded later, flips the order at no cost (EB1DC's '#' test).
     - **Screen frames and registers cheaply**: one scratch .c holding N variant functions
       (the body in an include, the variants by #define), compiled once with "tools/permuter.py
       compile", each variant's frame and slots read off objdump; IDO's "cc -S" (ugen before
       as1) shows uopt's register choices directly.
270. **More facts** (plmain.c `func_8018A5F8` and polygon.c `func_800D6990` matched;
     gauge.c `func_800DD314` 302 to 124):
     - **Which of two call-crossing values gets s0 is a priority (references over blocks
       spanned), and FAKE "do {} while (0);" blocks placed where ONLY the value to demote is
       live flip it at no code cost** (6990: seven blocks after f's last use, so gfx loses s0;
       six were not enough). Blocks where both are live cannot flip it. "if (G) {}", labels,
       nested do-whiles, gotos and an early return do not count as blocks; "register" on a
       parameter is ignored.
     - **A zero chain through u16/s16 fields spends one ugen t-register and stores its OUTER
       target first; an s32 chain spends none** (18A5F8: "p->unk048 = p->held = 0" for the
       game's 0x48 then 0x4A).
     - **A FAKE empty "if (i) {}" right after a statement pins that statement's store**: without
       it as1 sinks the store below later float stores while its load stays up (18A5F8's index).
     - **A float chain stored "= 0" (int) after an if/else join gets its own "mtc1 zero,f0"**
       instead of sharing the earlier 0.0f.
     - **"b = (u8 *) &p->field; b[j] = 0" starts the loop pointer at the field; "((u8 *)
       &p->field)[j]" walks from p with the offset in each store.**
     - **Where a value is computed decides whether its multiply is in place** (DD314: x computed
       right after the inner switch, or between the sync and the load, gave the game's
       "lw v0,0x50(sp); move at,v0" multiply and the game's registers for panel, h, y; each
       earlier position did not). A copy of a parameter declared as a ninth word moved a spill
       (gfx's home 0 for the game's 4): removing the copy fixed the slots.
271. **More facts** (camera.c `func_800CAEF8` matched from 130):
     - **"cur -= step" and "cur = cur - step" colour the float registers differently**: the
       compound form gives cur the lower register and step the higher (130 to 11).
     - **A zero halfword stored to a field and read back in the same block ("w->f = 0; ...
       G = w->f;") costs a ugen temporary that as1's forwarding hides**: the lhu goes, its
       t-register stays spent. Only when the stores between go through the same base (a global
       store between may alias and keeps the reload), and only for an unsigned read: an lh
       read-back is never forwarded. So a t-register the game skips with nothing to show for it
       can be a field stored and read back into a global, the field unsigned.
     - **Spilled temporaries share a stack home only when their live ranges do not overlap**, and
       a register-only temporary can take a home too (dmenu.c `func_801A3504`, matched): a global
       expression repeated in a second case ("w->cursor->f.parent") makes its temporary span
       both cases and pushed a spill home one word down. One occurrence read through a named
       local ("cursor = w->cursor; ... cursor->f.parent") breaks the expression's identity and
       gave the game's slots. Find such a gap by cutting statements out of a scratch copy until
       the slots move.
     - **uopt hoists a constant store to a register variable to the head of its block, and a dead
       print starts a block** (scenario.c `func_800E7B34`, matched): with the print before a call,
       "li s3,1" lands before the argument load; moved after the call, as1 puts the li in the
       jal's delay slot as the game does. To move a constant store into or out of a jal's slot,
       move the dead print across the call.
     - **A pointer register reloaded after stores through another pointer, and stepped in place
       ("addiu s1,t3,8; sw s1"), is IDO's PRE of the field expression (w->pc), not a variable**:
       every access written "w->pc->x" removed a copy and two FAKEs.
     - **A function's FIRST float registers can depend on its LAST statements** (vram.c
       `func_800E3630`, 160 to 38): cut after s, the first 150 lines were the game's; one FAKE
       block boundary late in the function re-coloured all of it (272 to 62 at mix 0). Sweep
       splits even when the first difference is at the top.
     - **An odd count of declared locals puts an alignment word inside the compiler-temp area**,
       pushing every temp above it up one word; a trailing pad making the count even moves them
       back (62 to 38).
     - **Count the temp area's words to tell a temp from a variable:** the game's temp area held
       exactly this C's never-used words plus the alignment word, so the values the game keeps
       there (bz, bx) are compiler temps, not declared locals. And the nearest point read as
       p[best] everywhere (no q pointer, no corner locals) gives IDO's own pointer for it, built in
       a1 and decremented in place, as the game.
272. **More facts** (vram.c `func_800E3A24` 194 to 10, floorhit.c `func_80195488`
     measured):
     - **The order two values first appear in decides which gets the register and which the
       home**: "bx = ...; bz = ..." gave bx the register and bz the home; "bz = ...; bx = ..." in
       every case handed f18 to bz as in the game and fixed the float rotation from the first
       point on (232 to 18); the distance's z term first then gave 10.
     - **With PRE (no named locals for p[best+1]'s coordinates) IDO loads them AFTER a
       "best--"**; a game that loads p[best].x/.z before the decrement assigned them explicitly
       before it.
     - **Adding declared words never moves the compiler-temp area** (21 declared words: frame
       0x98, the same 10-word temp area).
     - **A dead "if (0) osSyncPrintf" is not always a block boundary**: ten of them in floorhit.c
       `func_80195488` (one with float arguments) left its code exactly the same, where in
       scenario.c `func_800E7B34` one moved a constant store into a jal's slot (271).
     - **f32[3] arrays are laid out apart from struct locals** (floorhit.c: two Vec3f as arrays
       moved the frame 0x148 to 0x138, 1323 lines).
     - **Unused frame words after named counts can be register-only locals holding
       differences** (geography.c `func_800E0C44`, 376 to 10): "dx = x - x0; dy = y - y0; dz = z
       - z0;" set right before the eight marks restored the game's registers, and copy
       propagation then put the count's compiler-temp copy (0xC8) into the z+1 marks and the scan
       by itself: no hand-written (y1 - y0 + 1), no s16 copy of y. The box sizes went into the
       counts' own variables (nx, ny, nz reused), as the game keeps bz and nz together in s7.
     - **Several statements that needed FAKE tests and pointer copies were one constant 4-trip
       loop**, which IDO unrolls fully (the four side planes' offsets).
     - **IDO swapped a commutative float multiply's operands relative to the source**: writing
       "eye[2] * planes[k][2]" gave the game's order.
     - **A scratch harness compiling a whole variant is not line-faithful to the real file**: it
       got a store order right that the real file's compile did not. Confirm with check.
     - **uopt propagates a copy "v = <CSE expression>" only inside one basic block** (sndtrack.c
       `func_8000B5D0`, 250 to 106): the game's "or a0,t0,zero" needs v defined in an earlier
       block than its use, or a FAKE boundary between. Block-scoped locals take a slot just above
       the compiler temps (pushing them down a word). An empty "do {} while (0)" or "if (G) {}" in
       a loop can stop IDO hoisting a local array's address out of it.
     - **A global incremented through a register where the game uses a lui each time: write
       "G = NEIGHBOUR[n] + 1"**, the same address under the neighbouring array's name (FAKE).
273. **More facts** (floorhit.c `func_80195488`, measured on five copies at once, 916 to 910):
     - **IDO sorts a commutative operator's operands when one is an ARRAY ELEMENT; a STRUCT FIELD
       keeps the source order**, and the sort also sets the order of the loads. The game's
       "addu t8,t7,v1" (v first) came from "TV(t)->x + TE1(t)->x" with s16[3] arrays read through a
       struct view (CollVec), not "t->v[0] + t->e1[0]". Read as fields, the whole edges section came
       out as the game's (z, y, x loads; "wz * e"; "(x + y) + z").
     - **A load expression keeps its registers across the whole function**: e1 and e2 sit in the
       same registers in two distant sections in both builds, so a register swap between two such
       loads is decided by their whole-function live ranges, not where it shows.
     - **A variable with two definitions is not copy-propagated** ("a.x = c.x" stays a store when c
       is redefined later, "c.x += t->e2[0]"): a "running corner" makes IDO home a and keep c in
       registers, the game's corner code exactly.
     - **The game's unexplained temp words were spills of one expression's differences** (dC's x,
       y, z at 0x50/0x4C/0x48), caused by a value kept in registers across the section, and a
       one-expression "n*k + p - v" foot adds a temp word that "foot first, then q -= v" does not.
274. **More facts** (four matches: beffect.c `func_80198750`,
     geography.c `func_800E0C44`, scenario.c `func_800E7278`, font.c `func_800EAF38`):
     - **Work placed BEFORE a block's commands moves where uopt starts that block's values, even
       work uopt later sinks to its use** (font.c EAF38, 184 to a match, found independently
       twice): its rectangle's ix, iy and the steps ds/dt computed in the declarations
       moved h's load one command earlier, so the LoadSync pointer took v0 as in the game.
     - **"T *p = ARR[f()];" at the declaration is not "s32 i = f(); ... ARR[i]"** (beffect.c,
       206 to a match): the pointer from the call at the declaration changes which values uopt
       keeps in saved and t-registers; moving "p = ARR[i]" before the loop did nothing.
     - **Re-test old FAKEs after every structural fix**: a greedy-added "if (i) {}" that had
       helped (216 to 206) was the last 46 lines once the pointer form was in.
     - **A pointer stored and read back as an s32 ("w->m = f(); if (*(s32 *) &w->m == 0)") is
       not forwarded by uopt**: ugen reloads the word, as1 forwards the reload into the branch
       and deletes it, and its t-register stays spent (scenario.c E7278's hidden temporary).
     - **A literal "x + 16 - 1" folds to x + 15 before CSE**; the game's shared x + 16 came from a
       variable substituted later ("x + width - 1", width = 16), which is not folded again
       (gauge.c DD314, measured).
     - **ugen hands out the float temps f4/f6/f8/f10 from a first-in-first-out free queue**
       (read off cc -S, mathutil.c D249C): a temp takes the queue's head, a register goes to the
       tail when its value dies, so a change anywhere moves every temp, the prologue's included;
       uopt sinks a register product to just before its first use; "nx = k; nx = tz * nx + ..."
       gives "mul.s r, tz, k" where "tz * k" gives "mul.s r, k, tz".
     - **IDO's own "cc -S" (ugen's output before as1) names uopt's global registers directly**:
       vendor/ido-static-recomp/build/7.1/out/cc.exe -S with build_rom.py's CFLAGS (no -o),
       devkitPro's msys2 on PATH; it writes <name>.s into the current directory (run it from a
       scratch directory, never the repository root).
275. **IDO's frame layout, top-down** (vram.c `func_800E3A24`, measured over several passes): the declared
     locals, then uopt's temps in the order they are created (8-aligned), then ugen's spill words
     at the bottom. A volatile local declared LAST is a probe: its slot shows how many temp words
     the code creates, and cutting the function shows which code creates them (in E3A24 only the
     code after the join). A value the game keeps in a temp word cannot be a declared variable:
     no declaration order, block scope, struct, array, chain or reused name reaches it (367 pads
     layouts). In "bx = q = E" the merged value lives in q's slot; in "q = bx = E", in bx's.
276. **More facts** (msgfrm.c `func_800EC0B0` and geography.c `func_800E0C44` matched):
     - **A function's return copy ("move v0,a0") hoists only when it shares a uopt block with the
       commands that would otherwise take v0**: msgfrm.c EC0B0 matched with gSPTextureRectangle
       written out (on one line, as the macro expands) and two FAKE empty tests between its first
       command and RDPHALF_1/2 ("if (ix + iy == 0) {}", "if (size / 12 == 0) {}").
     - **When a spill lands one block early, inline a single-use local copy that is live at the
       same time** (geography.c E0C44: guAlignF reading w->view straight instead of "view =
       w->view" moved dimY/dimZ's spill into the scan's preheader, as the game; the unused
       declaration stays as a FAKE frame word). The spill's place depends on how many other
       locals are live alongside it.
     - **"if (x) goto L;" is one conditional branch; "if (x) continue;" becomes "bgez; b"**, and a
       label at a for body's end is its own block, where uopt puts edge reloads of hoisted
       constants (font.c EB1DC, measured).
277. **Facts from matching B5D0 and E74D4** (sndtrack.c `func_8000B5D0_main` matched, scenario.c
     `func_800E74D4_ovl_0006` 225 → 24):
     - **A global's load that the .s prints as literals ("lui t6,0x800c" with no %hi, then "lhu
       t6,0x3398(t6)") may still be a symbol in the source.** spimdisasm pairs %hi/%lo only when
       nothing between them rewrites the base; a bnezl's likely slot in between broke the pair.
       Written out as "*(u16 *) 0x800C3398", IDO loads into a second ugen temp ("lhu t7,(t6)")
       and every later t-register shifts by one; the symbol gives the game's single register.
       check then shows those 2 instructions as 4 differing lines; the build links them to the
       same bytes. Look at the instruction between the lui and the load before trusting a literal.
     - **"(u32) field != 0" on an if's test makes the game's copy of the field ("or a0,t0" at the
       head of the body)** with no named local; so does a late-folding no-op ("field * 1", "/ 1",
       "<< 0", "^ 0"; not casts or "& 0xFFFF") on every read in the body. A plain "lv = field"
       copy is propagated away inside one block, or lands in the beqz delay slot if made before.
     - **A long function can spill a pointer that a shorter twin keeps in a register: its
       priority is too low for the size.** One FAKE address-only test with several uses ("if
       (!&D_80172F08[(s32) w + ... + (s32) w]) {}", eight uses, mid-block) raises it with no
       code; at a block's end the same test leaves a branch.
     - **ugen's float temp queue (274) starts fresh in each function**: a float function compiled
       just above mathutil.c D249C left its 60 lines and every temp unchanged. Whatever moves
       D249C's temps is inside D249C.
278. **Facts from matching EB1DC and E3630** (font.c `func_800EB1DC_ovl_0006` matched, vram.c
     `func_800E3630_ovl_0006` one FAKE fewer):
     - **uopt substitutes a variable's defining expression into a test in the same block**
       ("i = E; if (i >= 0)" becomes "E >= 0"). When other definitions of i reach its later uses
       (a label inside the if, reached by goto), E then has two uses, becomes a CSE with its own
       register, and ugen writes "move i,E". For the game's in-place computation, kill an operand
       of E between the definition and the test with a dead store ("c = i;", FAKE). cc -S names
       such a stray register as a uopt CSE at once: read it before trying forms.
     - **The same kill moved where uopt reloads hoisted loop constants after a call**; with it a
       "continue;" at the end of the if body put the reloads in the body's tail (font.c EB1DC).
     - **Block-scoped locals take the words right under the function-scope locals, never below a
       compiler temp**, so a home below a temp word is a uopt temp (vram.c E3630's bz/bx).
     - **A variable's store in a block is emitted before the loads of that block's test, even
       written inside the test with a comma**; a game that stores a value after the test's
       other loads computed it as a temp inside the compare. uopt creates PRE temps in the
       reverse order of the compare's terms.
     - **as1 lifts a load above an "addiu base" and adjusts its offset** ("addu a1,-12; l.s
       12(a1)" becomes "lwc1 0(a1); addiu a1,a1,-0xC"): a FAKE boundary there only stood in for
       plain source (E3630's third case).
     - **A constant-false "if (K == n) {}" is not neutral**: it moved the float register rotation.
       Use #if to place a FAKE in scratch.
279. **ugen generates each function TWICE; the real pass starts with the float temp queue the
     first pass left at the function's END** (floorhit.c `func_80195488_ovl_0009` matched from
     910, measured in a second pass). The first pass starts from f4 f6 f8 f10. Within a pass
     the queue is FIFO (274): a temp takes the head; a value that dies goes to the tail in the
     asm operand order; a compare releases its operands in the order they were computed; an
     in-place op reuses its dying source. A simulator over cc -S
     reproduces every float temp of the function. So
     the registers at the top are set by every release below, and any commutative operand
     order IDO sorted (an array element, 273) permutes every temp, the prologue's included. The
     fix: find where the game-to-C temp mapping FIRST changes and read that expression through
     a named-field view (#define TN(t) ((Vec3f *) (t)->n)). This also explains 277's probe: a
     function above cannot move the queue, its own end does (mathutil.c D249C).
     - **A struct member with two definitions ("b.x = a.x; b.x += e") is not forwarded** and is
       reloaded from its home (the game's dB read b from 0x118).
     - **Reusing a variable that is live later (v as the foot's step) set the game's register
       priorities**; the unused k stays declared as a frame word.
     - **gauge.c DD314: a value's live range across an inner switch's blocks reorders uopt's
       colouring**: "x = ..." moved to the case's top (before the inner switch; uopt still sinks
       the multiply) gave the game's whole register map, no FAKE.
280. **A pointer read used in place several times becomes a uopt CSE placed with the sunk
     register products, in expression-tree order** (mathutil.c `func_800D249C_ovl_0006` matched):
     "nx = .. * *x + .. * *y + .. * *z" with no tx/ty/tz locals puts
     the *x/*y reads in uopt temps just below the named locals (the first placed takes the
     higher word) and loads cosX for cxsy before them, as the game. A read written as its own
     statement ("tx = *x") stays where it is written, above the products uopt sinks to their
     first use. When a game loads a product's operand before the point reads, try the reads in
     place. Also: an array element (cosX[0]) is not sunk but makes IDO sort commutative operands;
     a read through a struct view ("((Vec3f *) &cosX)->x") is not propagated (a2's FAKE match);
     "(&v)[0]", a volatile read or a union view grow the frame; dead stores are removed first.
281. **uopt gives a value a saved register only when that value's own benefit pays for the
     save; other values then share the paid registers for free** (scenario.c
     `func_800E74D4_ovl_0006`, 24 to 10). Loops on ONE index share a
     single strength-reduced counter range and a single walker range, so they colour alike; a
     loop whose registers are swapped against its siblings has its OWN index. If that index
     collapses the allocation (everything to t-registers spilled around the calls), the payer
     lost its benefit: dead "if (0)" PRINTF blocks count against it by their NUMBER, not where
     they sit (E74D4: 7 of 9 prints as the empty DPRINTF let phase 2's i pay again; 6 or fewer
     collapse). Check the print macro first when a natural index collapses a function.
282. **Facts from the sys 4880 match** (sys.c `func_80004880_main`, from 303):
     - **A double constant used by several add.d gets a uopt float register (one li.d) when the
       adds sit where a caller-saved float register is free**; written right after statements
       that keep every caller-saved float register live (the divides), ugen rebuilds it per add
       (li.d each time) and the FIFO temps come out as the game's.
     - **uopt substitutes a plain copy "y = v" into a later test ("v < 240" at a loop's entry)**,
       stretching v's live range over everything between (278); a FAKE dead store "v = 0;"
       right after the copy puts the test back on y and frees v's register.
     - **A global load CSE'd across blocks makes a uopt temp that takes a temp-area word**;
       reading it into an existing named variable (dead until later) gives the same code with
       no temp word.
     - **ugen emits a compare of two (u32) float conversions in reverse source order**:
       "(u32) r != (u32) toR" gives "bne conv(toR), cse(r)". A named u32 variable on one side
       does not flip.
283. **uopt's own colouring trace: "cc -S ... -Wo,-zdbug:6" writes uoptlist** (the recompiled
     uopt asserts in ecvt only after the colouring trace) (cockpit.c `func_80174C60_ovl_0010`
     matched). It names every live range, its priority, splits ("2322
     split out 2539") and refusals ("not colored (-ve save)"). Read it before guessing at a
     register puzzle.
     - **"IDO keeps an address in a register for at most 28 uses" is a colouring-order race**: a
       long CSE range coloured before any range has paid for a callee-saved register is split,
       and its cheap piece is left uncoloured. FAKE empty do-whiles inside its span lower its
       priority (save / units) so it is coloured after the first s1 user and takes s1 whole.
     - **A nested call whose result is narrow (u16, s16, u8) gets a ugen temp word of its own
       class**, never reused by int or float temps: a u16 prototype for an s32 callee (no andi:
       IDO callers trust the callee's extension) gave the game's extra frame word. Front-end temp
       words also come from 3-target narrow chains through a pointer and a narrow assignment's
       value being used (visible at -O1, and by the spills shifting at -O2).
     - **Canon since 2026-10-12**:
       - `match_function.py trace <c_file> <func>` runs both traces and lists each live range:
         register, adjsave, blocks and expression. A float constant's value is printed after
         "const------>" in pri.txt.
       - `ugen-edit <func> <A> <Z> <block>` proves an instruction-level target in ugen's listing.
       - The ecvt-patched uopt is vendor/uopt-ecvt/: myecvt.o linked in front of a copy of
         libc_impl_71.o (libc_w.o) whose ecvt/fcvt stubs are weakened with objcopy.
       - Read the trace before any try. On 2026-10-12, guessing blind on the last 36 wasted
         several tries before this was written down.
       - **Reading it:**
         - Register numbers in ranges.txt are uopt's own: r1-r13 are $2-$14 (v0..t6), r14 is s0,
           24-31 the floats. t7, t8, t9 belong to ugen.
         - uopt gives a range the lowest free register, unless the value is a call argument. A
           "live bb" line's third figure is then that argument register (`31:103` = a0), and
           the range takes it even with a lower one free.
         - Constrained ranges are coloured first, by priority; then the rest by range number.
           A float range counts as constrained at 12 interfering ranges (func_080105DC).
         - A constant is one range per value over the whole function, in disconnected pieces.
           A piece with one use in one block has adjsave (1 - 1) / 1 = 0 and is never coloured
           (oniFightOniProc's 15.0).
         - The same constant can sit in different "webs" by use: the u8 stores' 1 and the
           compares' 1 are separate ranges (dkeeperProc).
         - **How uopt picks a register, read from its source** (
           line numbers are vendor/ido-static-recomp/build/7.1/uopt.c's):
           - Priority: `f_compute_save` (151016). save = Σ(uses + defs) × freq, minus freq
             for each needed reload and each needed save. count = live bbs + live-through bbs;
             from 3 up, count = (count − 2)/4 + 2. adjsave = save / count.
           - Order: `f_globalcolor` (153914). Constrained ranges go first, highest adjsave
             first. The comparison is strict, so a tie keeps table order.
           - Forbidden set: `f_updateforbidden` (149316). It starts as the regsused of the
             range's live bbs, minus each bb's preferred register. It grows with the colours of
             earlier-coloured neighbours and of precoloured values (R2 the call result).
           - The pick: the caller-saved loop (154634), then callee-saved (154754). The cheapest
             non-forbidden register wins, the lowest number on a tie. An unused callee-saved
             register costs f24 more. If adjsave × count ≤ the best cost, the range is split
             (`f_split`, 154906). `needsplit` (151623) splits only a range with no register
             left.
           - Costs: `f_cupcosts` (153010).
             - Each call the range crosses adds freq for each register the callee clobbers.
             - A live bb with a preferred register p adds freq/10 to every other register.
             - v0/v1/f0/f2 cost 1e20 in such a bb.
             - p comes only from `f_passedinreg` (call arguments, 141824).
           - Homes: `gettemp` (175947) hands out words downward from the end of cfe's frame,
             in emission order. A node used 2 or more times gets its home before `inreg()`
             checks, so a home can stay unwritten (IDO_NOTES 354).
           - **ugen's temp registers** (ugen.c 80781-80962,
             81664-81840): its list is t6-t9 and t0-t5, minus every register uopt uses
             anywhere in the function. It is first-in, first-out: f_get_one_free_reg takes
             the head, and f_free_reg returns a register to the tail (`la $25` for a call takes
             t9, and the call frees it).
             - So one uopt-held value anywhere (oniFightSpiritProc's glow in t2) shifts every
               temp in the function.
             - "Every later t-register one off" means: find which register uopt holds in the
               game, or how many temps are taken and freed before.
           - **The "unused words" under the locals are uopt's spill slots** (Goemon
             (807); uopt.c `f_spilltemps` 148313, slot pick 148853-148899, temp-saving
             136273-136289). This explains notes 283, 354, 362, 376.
             - Before colouring, uopt walks its expression table in order of first appearance.
               Each expression that code motion saved to a temp takes the first free slot of
               its size, or a new one below, aligned to its size.
             - So the work pointer's expression (o + 0x20, entered at the first line) holds a
               slot even when it gets s0. That slot is never written.
             - Not cfe: at -O1 the word is absent (probes ihaword.c, ihaO1.c).
             - Order: a dead local holding a call's address, declared before w, moved that
               address to slot 0 (ihaslot2.c).
             - Correction to 378 (E8): a constant chain (`[0] = [1] = [2] = 0.2f`) can still
               spend the float word, with identical code.
             - Amended (V1, the same day): table order is not only source order. Expressions
               made by copy propagation are numbered later: effLensflareProc's position loads
               are bits 239-245, after q[0..2]. Two values that don't interfere may also share a
               slot (effLensflareProc's game puts mathAtan2's address in slot 0 beside the
               work pointer's empty slot).
             - **Lever, proven (ppressProc):** a field read as an element of an
               `f32[3]` (`vel[1]`) and the same offset read as a plain field (`vel.y`) can load
               an add's operands in opposite order. ugen-trace showed ugen did not swap them;
               the order is set before ugen. When two loads come in the other order, check the
               struct's field types.
             - **Lever, proven (karasuTamaProc):** any expression used twice or
               more gets a home word from uopt's gettemp (41621) before uopt checks whether it
               is in a register. A missing pair of never-written words can be expressions the
               game used twice: writing `(px - x) * (px - x)` inside the sqrtf, in place of a
               dx local, gave the game's two words. The same lever took Ktengu from 683 to 577.
             - Telling ordering from sharing needs the per-block temp sets f_spilltemps
               reads (each block's bit vector at +0x15c). No -zdbug level prints them.
           - **uopt's block limit** (staffCinescoDraw): f_getop (75192) counts each
             load of a variable not stored earlier in its block. A variable stored with a
             simple value is substituted without a count. The block ends at a statement
             boundary at the limit. Plain variables as arguments move block ends.
           - Lever, proven (func_08000538_tlb_0080): an empty `if (x) {} else {}` adds a block
             to a range's span. That lowers its adjsave and moves it later in the order.
         - check's "real" missed absolute-address differences until (799): an endParentProc
           form read real 0 with its stores in the wrong order. Build before believing a 0.
     - The recompiled uopt (vendor/ido-static-recomp/build/7.1/uopt.c) runs makelivranges,
       regdataflow, localcolor, spilltemps, globalcolor (single-block ranges first, weights from
       bb_frequencies; vram.c E3A24's round 3).
284. **A never-true unsigned test that breaks, "if ((u32) x + 1 < 0) break;", spends exactly one
     ugen t-register with no instruction left** (scenario.c `func_800E74D4_ovl_0006` matched;
     fileres.c func_80007C78 has the same FAKE): ugen computes the
     operand into a temp, as1 deletes the never-taken branch, then the dead addu. Place it where
     its block boundary splits nothing that must stay together (before a store block whose last
     store fills the case's "b" slot; between the stores it split them). An always-taken branch
     instead leaves a dead "b" block in as1. Prove such a target first by hand-editing ugen's
     listing and assembling it with as0 + as1 (r2/a1's asmx.sh). In scratch, #include the whole
     function at file level: a body #included inside a function puts every statement on one
     .loc line, and as1 schedules by line.
     - **Unparking a function whose .s carries a shared string block**: the later functions'
       prints must then emit their own literals in the block's order (scenario.c: SC_STR, the
       guarded errors, and two DPRINTFs for strings no C printed). Read the .s's .rodata first.
285. **uopt ends a basic block at a statement boundary once the block has read about 20
     variable values** (varreflimit 0x14; varrefs/curvarreflimit at 0x1001c430/0x1001c43c,
     checked in getop, reset per block in oneproc) (vram.c `func_800E3630_ovl_0006` matched).
     Constant stores do not count; loads of a variable stored earlier
     in the same block barely count. Priority is adjsave = sum over blocks of (uses + defs) x
     frequency / f(N blocks) (compute_save), so a split through a straight stretch halves every
     live range across it, and ONE extra load can flip a whole colouring. The fix that keeps the
     code: read the stretch's values through a variable stored in that block ("i = best;"
     then p[i]); copy propagation turns it back before PRE, so the PRE values stay the same.
     - **uopt's priorities in full**: link the vendor's uopt.o with a working ecvt/fcvt
       (the libc copy's stubs weakened by objcopy, myecvt.c) and run it on cc -K's .B/.T
       files: -zdbug:5 prints every live range's
       priority and blocks, -zdbug:6 the colouring order (283).
286. **A call through the function's address: `(*f)(...)`, `((T) f)(...)` and `(&f)(...)` all
     give `lui t9,%hi(f); addiu t9,t9,%lo(f); jalr t9`; plain `f(...)` gives `jal`** (IDO 7.1
     -non_shared, measured with a probe). -KPIC and -call_shared go through $gp instead.
     The game's TLB code (archive files 16-327, linked at 0x08000000) calls main and the
     overlays only this way: a jal cannot leave its 256 MB region. Called twice, the address is
     a CSE value: held in v1 and spilled across the first call (tlb_0182's eneMakeEx at
     sp+0x30). include/tlb.h declares each such function, then `#define f (*f)` (a macro is not
     expanded inside itself), so the C reads f(...) and compiles to the far call.
287. **Every string literal goes to .rodata, in source order, those in data initializers too
     (`{ 1, "gate" }`, `char *names[] = { "anchor" }`); then the late rodata (floats, doubles,
     jump tables); .data holds no strings. Both sections are 16-aligned** (IDO 7.1,
     measured with a probe). So a file's .rodata starts at its first string literal,
     referenced or not (a compiled-out PRINTF's format is still there: tlb_0037's
     "????? kaku = %f\n"), else at its first late item. **A string AFTER late rodata is the
     next object's .rodata: the file was linked from several objects** (tlb_0021's "goecreate\n"
     after its doubles; 60 of the 311 TLB files). nisitenma_archive.py rodata_start.
     - Strings are told from data by being valid EUC-JP (the game's encoding), at least 3
       characters, NUL-padded to a word: a display list's E7000000, 40.0f (42200000, "B "),
       11.0f ("A0") and a double's bytes (3F91DF46...) each passed a looser test once.
288. **A loop body's first test written as `if (!x) continue;` instead of the first term of one
     `&&` chain gives the same instructions but a different saved-register colouring.** In
     tlb_0048 gimEdopoleProc, message 0's loop `if (w->atk[i] != NULL && hit(..) == 1 && p != NULL)`
     put the constant 1 (shared with the switch's case-1 compare) in s2 and the loop end 16 in s5,
     the reverse of the game. `if (w->atk[i] == NULL) continue;` then `if (hit(..) == 1 && p !=
     NULL)` matched (20 lines to MATCH). Eleven other loop forms changed nothing or added code:
     nested ifs, a local, while, do-while, `1 == call`, a second index, `!= 1 continue`, gotos.
     Found by compiling all 12 forms at once in
     scratch. Its parked twin tlb_0045 bpoleProc has the same loop.
289. **A task proc's work pointer: set it at its declaration (`W *w = (W *) (obj + 1);`), not in
     each case.** When the game rebuilds w from obj's stack home in each case (obj never in an
     s-register, w spilled at 0x20), assigning w inside each case made IDO fold w into obj's
     offsets and give obj a register: 184 lines. At the declaration it was 130, and with
     `s32 pad;` and grouped cases it came to 12. tlb_0033_4 utaRetuProc and utaSongProc.
290. **Switch layout from the jump table:**
     - Table entries that all go to one shared block laid out after other cases are one group of
       explicit `case` labels written after those cases in the source. Written as pairs between
       the other cases, IDO laid out three copies of the block.
     - Entries at the end of the table that go to the switch's end need explicit
       `case 10: case 11: case 12: break;`. They set the table's length (13 entries; 13 and up
       goes to the end through sltiu).

     tlb_0033_4 utaRetuProc, 12 lines to MATCH, the last step being one-line
     `if (p != NULL) { frmDestroy(p); }` statements (rule 12, the delay slot).

291. **IDA shows a double literal's high word in an odd FPR as a float.** For `lui + mtc1` into
   f17, f19 or f11, IDA prints `li.s $f17, 1.875`. That word is 0x3FF00000, the high half of
   the double 1.0; 1.75 is 0x3FE00000, the double 0.5. Read the odd register as the high half
   of a double, and write the double in C. Found on tlb_0176's saburoMain0/1.
292. **An unfolded `addiu vN,a0,0x20` before a branch is a work local used across it.** Written
     as `W *w = (W *) FX(o);` at the declaration and read through `w` in both branches (or after
     an `if (obj == NULL)` test), IDO keeps the work's address in a register and the fields at
     their work offsets (0x23(v0), not 0x43(a0)).
     - mapSetHold/mapSetCamHold and the cursor's helpers in tlb_0016_0, and atoya's flip in
       tlb_0037, all match this way.
     - Each of these failed: `FX(o)` written in each branch; a local assigned per branch; a local
       assigned again in one branch.
     - It does not hold in straight-line code: tlb_0102's `w->count++` still folds (rule 161's
       open case).
     - The branch sense is a separate lever: `if (w->on) {..=0} else {..=1}` gave atoya's
       beqzl, where `== 0` first gave bnezl.
     - **Mixed: folded offsets first, then a register.** Some functions use the a0-folded
       offsets for their first statements and `addiu vN,a0,0x20` for the rest. They match with
       `W *w = (W *) FX(o);` at the declaration. The first statements are written with the cast
       expression (`MACRO(o)->field`) and the later ones through `w`. This held for:
       - nobuko's gauge (tlb_0037 func_08005870);
       - sh_hani's play-once (tlb_0085 func_0800057C);
       - enemy 0xE61's gauge (tlb_0039 func_08002060);
       - sippo's course placement (tlb_0037 func_08003F30).
294. **Stack locals go top-down in declaration order, one per line.** The first declared gets
     the highest offset. Words the game leaves unused are `s32 pad[n]`: above the locals if
     declared first, below a struct if declared after it.
     - Examples:
       - obisu_r's zone test (tlb_0142 func_08002E38): zone, x, y, z at 0x24..0x30 under 12
         free bytes is `s32 pad[3]; f32 z; f32 y; f32 x; s32 zone;`;
       - a starter's argument struct with free words under it is `Arg arg; s32 pad[n];`.
     - **Not solved:** three floats at the bottom of a 0x28 frame (0x18..0x24, the top word free),
       in the camera heading twins (tlb_0037 func_080016B0). IDO puts them against the top, and
       a pad first grows the frame to 0x30.
293. **`(u8) (1 << (u8) (n % 8))` for a bit of a flag byte.** Without the outer `(u8)`, a
     t-register is one lower than the game's (t0 where it skips to t1). This is tlb_0126's form;
     it matched tlb_0134's four bit tests and sets on gGameWork.unkBA/unkC4.

295. **In a task's create case, a field copied from the argument goes before the constant
     stores around it.** The game loads `arg->type` (lw from the arg's home, then the field)
     before storing a constant such as `alpha = 255` or `time = 0`, and stores the type last.
     Writing `w->type = arg->type;` first in the source gives exactly that. Written after the
     constants, the load moves below them.
     - Proved by tlb_0326_1's slap hand (`func_080119E8`, 8 lines to a match) and kabukiDB
       (`func_0801393C`, 22 lines to a match).
     - The same group of handlers also wants `s32 pad[2]` after the work pointer when the
       game's frame is 8 bytes bigger. That covers Kstage, KballE, Klift, Ohuda and wakemi;
       the first fan needed `s32 kind; s32 pad;`.
296. **A no-op 64-bit mask on a call argument can move one temporary register.** In tlb_0018's
     title intro (`func_08004564`), `joyGetHeldTime(i & 0xFFFFFFFFFFFFFFFFu, ...)` was the
     permuter's score-0 find: it moved a temporary of a later branch from t6 to t8. `u32 i`, a
     `(u32)` cast and a 32-bit mask do not do this. It is kept as a FAKE.
297. **The game can keep a test around a dead print, and this C cannot reproduce it yet.** In
     tlb_0326_1's "true kabuki ball" (`func_08015BE4`), the game loads the hit kind, compares
     it with 1 and loads the attack box on both paths.
     - `if (kind != 1) PRINTF(...)`, an empty `DPRINTF` and the same code written in both
       branches all fail: IDO folds the test away, or does not tail-merge the branches.
     - Open; the function is parked.
298. **The game forms a symbol's address again at every use; IDO keeps it unless the uses have
     different names.** Where one address is used several times around calls, the game has
     lui/addiu at each use. Under one name, IDO keeps the address in an s-register or spills it
     to the stack between two calls.
     - The fix is a second name for the same address in undefined_syms.txt, given to some of
       the uses. IDO never shares a register across two names. This is the old lever of main's
       thread 5 (D_800500C4_store/_reset).
     - Proved by tlb_0323_1's two vertex-animation tasks. In "bVDemo" (`func_0801373C`) and
       "bVani" (`func_08012C7C`), the vanimeStep calls name the animation D_301C240_step and
       vanimeMakeVtx keeps D_301C240. Both matched (136 and 54 lines before).
     - Failed first: the address as a number (lui/ori in place of lui/addiu), an if-chain in
       place of the switch, and a different cast at the second use.
     - First written up as open before the notes were searched for "name".
     - Also unparked: tlb_0325_1 func_08002ED0, whose flag's set got D_080179C0_set. The two
       flip actions improved only (166 to 164); they have other differences.
299. **Rule 295 holds for a plain parameter too.** In tlb_0031_1's and tlb_0032_1's five ONARA
     starters (`func_08006D3C`, `func_0800782C` and its three twins), `o->n = n;` (the fifth
     argument, read from the stack) written before `unk30 = 10; unk24 = 0;` gives the game's
     early load and late store. The four had been parked on exactly that.
300. **Which float parameters passed in a-registers the game homes follows the order they are
     read.** In tlb_0324_5's IKA_BOMB starter (`func_08012420`), the game homes x and y and
     takes z with mtc1. Written x, y, z, this C homed y and z. Reading z first
     (`arg.pos[2] = z;` before x and y) matched.
     - In the same function, an angle the game keeps as a compiler temporary (at 0x1C, below
       the locals) is the expression written twice, not a named local (rule 243).
301. **A distance's sum order decides which float parameter is homed (rule 300 again).** In the
     boss heading helpers, the game keeps x and y in f12/f14 and homes z:
     - Functions: tlb_0324_4's `func_08010234` and `func_08010350`, and tlb_0325_4's
       `func_0800DD74` and `func_0800DEA4`.
     - Written x² + y² + z², this C homed y (64 lines).
     - The permuter's z² + (x² + y²) was 12 lines off with x and y swapped.
     - **z² + (y² + x²)** matched all four.
302. **Aggregates are laid out in reverse order of declaration; scalars come first.** In
     tlb_0019_1's dValcan starter, `ValcanArg arg; s32 pad[2];` put the pad BELOW arg. Also:
     - A scalar `s32 pad;` declared before the angle moved only the scalars.
     - In tlb_0038's bomb launch (`func_08004088`), the scalar locals come out with the
       first-declared highest. The game's order was matched by `pad; aim; place; height;
       side; zone`, reversing the order they are used in.
303. **A check whose leftover lines all look like relocations can still be a miss.** In nuri's
     swing (tlb_0080 `func_080006C0`), the game loads its last constant (lui/lwc1) before the
     timer and state stores. The check listed the `-`/`+` pairs next to each other, so the
     line read as relocation-only, but the full build differed at that lui.
     - Only "MATCH" or "MATCH except own section addresses" counts.
     - The fix: the angle's store written first in the source. IDO still stores it last but
       loads its constant first.
304. **An unrolled 4-row copy loop gives four hoisted row addresses.** The journal's costume
     sharing (tlb_0018_0 `func_08000744`) copies a 4x4 byte table back as four
     lwl/lwr pairs, each from its own callee-saved register.
     - The C is `for (j = 0; j < 4; j++) rows[j] = owned[j];` over a 4-byte row struct.
       IDO unrolls the loop (rule 15) and hoists `&owned[j]`.
     - One 16-byte struct copy gives a single base instead.
     - Its compare loop is `k = 0..2` reading `c[k + 1]` (the game's `lb 1(a0)`), not
       `k = 1..3`.
305. **Open: an enemy proc whose model sits in a homed argument's register.** In "dai"
     (tlb_0037 `func_08003358`) and "hanabiHiroba_main" (tlb_0131 `func_0800331C`), msg and
     arg live in their home slots. The game reads the model into a1 or a2 (their free
     registers); this C uses v0 and a0.
     - `w = ENE(o)` assigned in case 2 (rule 132), a chained scale (rule 42) and `pad[2]` set
       everything else.
     - Pads on either side of w did not move its spill (0x24 in the game, 0x20 here).
     - A permuter run is on the fireworks square.
     - **Solved for the fireworks square:** w at the declaration, used in cases 2 and 1, with
       case 0 reading through ENE(o). Dai, written so, still has its spill 4 low (16 lines).
306. **A one-byte pad moves spill slots by 4 without growing the frame.** hokusai (tlb_0018_1
     `func_08001CC4`): `s8 pad;` after the work pointer put both spills at the game's 0x20
     and 0x2C. s32 pads before or after w grew the frame or moved the wrong way. Found by the
     permuter (`char pad`). It did not help dai.
307. **A switch case's two stores on one line keep the game's order** (tlb_0037 Nobuko's
     script `func_080058D0`). The game loads the state's constant first and stores the timer
     first, with the state store in the branch's delay slot.
     - Two lines in either order gave reversed registers or a moved delay slot.
     - `w->state = 13; w->timer = 20;` on one line matched every case.
308. **Number forms (this stretch):**
     - `(u16) v * 0.1f` adds the unsigned-conversion fixup (bgez/add 2^32); `(v & 0xFFFF) *
       0.1f` does not (bashi, tlb_0178).
     - `x / 512` (an int) keeps div.s; `/ 512.0f` becomes a multiply by its reciprocal.
     - A float from a u16 field that the game converts with the fixup is
       `(f32) (u32) w->heading`.
     - Also found in the same stretch:
       - `w->id = f() & 0xFF` for a byte stored from a call;
       - `0.2` and `0.2f` for two loads of one literal;
       - separate loop indices per loop.
309. **A work-pointer local decides whether a proc keeps msg or obj in a register.** "atoya"
     (tlb_0037 `func_08005A38`) has no locals of its own.
     - Without a local, this C kept msg sign-extended in a3 and homed obj (42 lines).
     - With `EneObj *w = ENE(o);` used in case 1, obj went to a3 and msg to its home, as the
       game does: MATCH.
     - The thunder maker (tlb_0033_3 `func_0800CA74`) is the opposite case: the game keeps
       msg in a3 (sh/lh around each call) and obj in its home. w at the declaration homes
       both (118); w made in case 0 gives msg in a3 but obj in s0 (110). Open.

310. **`li tN,6; beqz tN; sw tN,var` then `sw zero,var` is `if (var = 6) var = 0;`**, the
     game's `=` for `==`. The lightning setters (tlb_0019 `func_0800560C`/`5810`) have it next
     to a correct `if (++var == 6) var = 0;`. IDO keeps the test of a constant it could fold.
     Write the bug as the game has it, with a comment. A dead PRINTF placed where the game
     did not have one can still change register allocation: shadow (`func_08005A0C`) went
     from MATCH to 123 lines with four more; the Impact lines belong to the scenario switch
     `func_08005C84` (MATCH there).

311. **IDO_NOTES 161's open case, partly: an empty `if (w != NULL) {}` after the work
     pointer is set keeps it in its own register** (`addiu v0,a0,0x20; lw 0x44(v0)`) where IDO
     would fold it into the base's offsets (`0x64(a0)`). `(void) w;` does not, nor does a
     `(u8 *)` form of FX.
     - tlb_0102 `func_08000AF4`: MATCH, as FAKE.
     - namiMake (tlb_0071): 16 lines to 9, with one more spill.
     - Worse on a Player local in a loop (tlb_0102 `10FC`/`11F8`: the saved registers move),
       and no change in the lightning setters.
     - The game's own shape is still unknown: something that uses w as a value and leaves
       no code.

312. **A TLB file's .data can be C: the yaml subsegment `data` becomes `.data`, then split.**
     The C defines every word in address order. File-scope definitions come first in the
     source, function statics follow at their functions, and later file-scope data after
     them.
     - tlb_0030_1 (0x80 bytes) unparked its four demo-player procs; tlb_0098 unparked
       wagaStepTest. Each had C that matched only with function-local statics.
     - Two traps:
       - A string the game prints first (Goemon's "goecreate" before his name) must be a
         DPRINTF at the function's top. The rodata order is the source order.
       - Uninitialised statics go in .bss, which needs a `.bss` subsegment with a vram
         (main's audio.c does it). tlb_0029_2's bss has unexplained gaps, still open.
     - Big data blocks (tlb_0034: 0xA70 bytes) want a generator, not hand typing.

313. **A callee's return type moves its caller's registers.** doucyu (tlb_0032_1
     `func_080062F0`) kept sndPlay's CSE'd address in v1 where the game has v0. Its EF
     DOUCYU starter `return tskCreate(...)` written as `void ... { tskCreate(...); }`
     compiles to the same starter, and the caller then matches. The permuter's find,
     natural C.
314. **Globals that only one function reads, laid out in function order and each reached by a
     fresh `lui`, are that function's statics, and they say which file owns the data.**
     tlb_0021: 0x08010944 is read only by func_080014A8, 0948-0954 only by func_08002B5C, and
     0958 only by OPMAIN. The yaml had given the block to tlb_0021_1 and tlb_0021_3; it is
     tlb_0021_0's .data, the overlay's first. As externs the functions reached them through a
     base register (step 0: 100 lines; OPMAIN: 337). As function-local statics, the script
     matched and OPMAIN went to 181.
     - A static read, changed and used again is kept in a register: `eyeZ += speed;` ...
       `camSetEye(..., eyeZ)` gave the game's single store, where a local `z` copied back
       (`eyeZ = z`) moved the float registers.
     - A function still in assembly gets its static as a file-scope definition at its place
       (`f32 D_08010944 = 1.0f;` just before its GLOBAL_ASM), so .data keeps its order.
315. **A TLB function-local static in .bss lands through a `.bss` subsegment** (
     (530)).
     - The yaml gives the segment `bss_size`, then `{ type: .bss, vram: <its address>, name:
       tlb/<file> }`, with an asm `{ type: bss, ... }` blob before it if the file's statics do
       not start the segment's .bss.
     - Then split. The segment's .bss symbols become D_0800xxxx_tlb_NNNN, so C that used the
       segment-less D_800xxxx names is renamed.
     - IDO packs a file's statics in declaration order, with no gaps. Measure the offsets from
       the check's object before building: objdump -dr, the LO16 .bss addends per function.
     - Space the game leaves between them (tlb_0029_2: 4 bytes in genoGOE, 12 after the second
       character) is unread statics, declared where the layout puts them.
316. **A switch on a function-local static keeps it in a register, stores it back at the head of
     each case that calls, and reloads it after.** The demo directors' Start skip (
     (546)).
     - In the game, `sw a2, skip` comes first in cases 3, 40 and 50, with `lw; addiu 1; sw`
       after their calls.
     - That is uopt promoting a variable whose address is never taken. An extern, or a global
       defined in the file, is reached through a base register instead (337 lines on OPMAIN).
     - The case bodies come out in source order. The game's `case 0` (empty, or the time-out
       test) follows `case 50`, so the C writes it there.
     - Its .data word is the function's: the data after it (a table) is defined after the
       function. Its initializer is kept as a macro where data-c wrote it, so the line is not
       retyped.
     - Open: with a fourth calling case (46), the game holds the skip in
       a2 or v1 where natural C takes v0. Then case 50's `lui v0, gfxGetPendingSwaps` cannot
       fill its beq's slot. A file-scope static is worse.
317. **A store to a global makes IDO reload a field read through a pointer; a store to a
     function-local static does not.** No pointer can reach the static, so it cannot alias
     `w->timer`.
     - In tlb_0019_0's GDEMO script, `if (w->timer == 225) period = 60;` is followed by
       `w->timer % period` with no reload in the game. As an extern, IDO reloaded the timer.
     - The game's word at 0x08006C44 is that function's static, which also told which file
       owns the segment's .data.
     - **A missing reload after a global store is evidence that the global is a static.**
318. **Two 1.0 factors IDO folds as literals stay as locals.** tlb_0029_0's quake multiplies
     by 1.0f and by the double 1.0 in the game. Written as literals, IDO dropped both
     multiplies. As `f32 amp = 1.0f; f64 scale = 1.0;`, both stay; only float registers
     differed (parked).
319. **A function's frame slots follow its locals' declaration order, the first at the top.**
     - tlb_0022_0's camera script: its spilled sine term must sit at the frame's top, so it is
       declared first, before `w`.
     - A pad after `w` moved the compiler's call temporaries down 4. The same pad did the same
       for tlb_0019_0's script.
320. **An integer `0` in a double compare is its own constant.** Two `0.0` literals share one
     register; `0.0 - x` with `... > 0` holds two 0.0s in separate saved FPRs (fs1 and fs0).
     - In tlb_0102's squash `(0.0 - s) / 31.0 > 0 ? ... : -(...)` the game has three saved FPRs
       (31.0, 0.0 and 0.0).
     - Two companions from the same functions:
       - **A read-modify-write through `PL(x)`'s u32 cast splits the store's address** (`addiu
         v1,v1,32; sw 336(v1)`). `(Player *) (players[i] + 1)` folds it (`sw 368(v1)`).
       - **A field tested and then used lives in v0/a0 only when taken into a local**
         (`(hitter = w->atk->hitPower) != 0`).
     - These unparked func_08000484 and matched func_0800063C.
321. **A `do { ... } while (0)` around a work pointer's setup keeps the pointer unfolded.**
     - In tlb_0033_3's cCLDMAN the game forms obj + 0x20 in v0 and stores the kind through it.
       Plain C folds it into 0x20(s0).
     - Measured, all fold: the store in the switch's head, a pointer set at the top, and the
       empty test of rule 161.
     - The permuter's do-while(0) around `name = ...; w = FX(obj);` matched. That is the shape a
       setup macro expands to, so suspect a macro wherever a fresh pointer stays unfolded.
322. **A local kept in a register still takes its turn in the stack slots' declaration order.**
     - tlb_0037's ihand: `s32 hitter` (always in a0) declared before x, y and z moved all three
       down 4, to the game's slots. Declared after them it changed nothing.
     - In the same way, a scalar pad declared before w moved a spill slot up 4 in tlb_0152's
       broke. A pad[2] after w grew the frame instead.
323. **An integer literal where the rest use 0.0f-style floats is its own constant.**
     - tlb_0018_1's slot summary has `x + 96 + 16.0f` once, among several `x + 96.0f`. With
       96.0f, IDO shares one constant and temporary and the frame is 8 smaller than the game's.
     - The same family as rule 320 (an integer 0 in a double compare).
324. **Statements on one line schedule differently (rule 12): one-line `if (p != NULL) {
     frmDestroy(p); }` is this game's style.**
     - The tlb_0325 Ddmybomb's destroys matched only on one line each. Over three lines, IDO
       moved w's addiu out of the first test's delay slot.
     - tlb_0039's matched code has the same one-line form.
325. **A global that is loaded at a function's top with its own lui, and stored back with a fresh
     `lui at` after everything else (here after a division's break checks), is a function-local
     static.** uopt keeps the static in a register for the whole function and stores it at the
     exit (rule 316).
     - Proved by the game's five random number generators: tlb_0325_3 func_0800DAF0/DB98,
       tlb_0324_3 func_0800FFB0/08010058, and tlb_0033_3 func_0800C490.
       Each is `static u32 seed = 3749696; seed = seed * 1103515245 + 5963; return ...;`.
     - Before this, a second name for the store (rule 298) gave the store's lui but not its
       place: 18 lines each, parked.
     - The static needs its file to own the word (.data in C), and it must be the right file.
       (513)'s data-c split followed the yaml, and the yaml had given tlb_0324_3 four shared
       globals that tlb_0324_0/1/2/5 read. Map which file reads each word before moving data
       (a grep of the label in every asm and C file of the segment).
     - Because the seed is the function's static, every data definition before it in address
       order must come before the function in the source. tlb_0033_3's data block moved up
       above the generator.
326. **A float argument's zero comes in three kinds: `0` (an integer), `0.0f` and `0.0`. IDO
     keeps each as its own constant, and where one is shared decides the code at another call.**
     - In tlb_0325_3's opening camera (`func_0800BB74`), the game passes two zeros
       to the plasma (f12, f14) as `mtc1 zero,$f12` plus `mov.s $f14,$f12` in the jalr's delay
       slot. That is `(0, 0, ...)`: two `0.0f`s gave two mtc1.
     - With the plasma's integer 0s, a later `camMoveAt(cam, 0.0f, 0.0f, z)` came out as
       `li a1,0; li a2,0`. The game has `mtc1 zero,$f0` plus mfc1s there, which is
       `camMoveAt(cam, 0.0, 0.0, z)`.
     - Found by brute force over the 3 x 3 forms; only that pair matched. The permuter's best in
       2,000 iterations was the original 2-line difference.
     - The same `mov.s $fa1,$fa0` shape is in tlb_0325_3's func_0800D414 (an effectKiranCreate
       with two zeros).
327. **Read-modify-writes of different globals in a row: their t-registers are numbered in
     source order, even where IDO schedules the stores in another order.**
     - In tlb_0325_3's win sequence (`func_0800C074`), the game's stores went
       flags2, flags2, gBossFlags, flags2. Written in that order, the C gave the right
       instructions with the temporaries renamed (t0/t8/t9 for t7/t9/t8).
     - Numbering the game's temporaries from its first load (t5 flags2, t6 its `| 0x400`, t7
       the gBossFlags load, t8 its `| 0x800`, t9 the `| 0x1000`) gives the source order:
       `flags2 |= 0x400; gBossFlags |= 0x800; flags2 |= 0x1000; flags2 |= 0x1000000;`. The
       same reading fixed its frame-1040 block (gBossFlags first).
     - Also there: `s32 pad[11]`, not the [12] that gives the frame size, put the two spilled
       call addresses at sp+0x44/0x48. Try the pad sizes around the frame's.
328. **One float register stored to a work field and then to a chain of frame fields is the chain
     reading the field: `f->scale[0] = f->scale[1] = f->scale[2] = w->scale;`.**
     - In tlb_0324_5's TMarble and IKA_BOMB, the game builds 0.5 (0.3) once, stores
       it to w->scale, computes w->dist, then stores the same register to the frame's three
       scales.
     - Written `= 0.5f` the chain built its own constant: 500 lines. Reading `w->scale`: 62.
       With w->dist's statement moved before w->scale's: MATCH (the scheduling followed).
     - The same two functions needed the work pointer set at its declaration. The game rebuilds
       it from obj's home in each case (`lw s1,home; addiu s1,s1,0x20`), and assigned per case
       it was a different register.
     - A float local declared before that pointer takes the higher stack slot (rule 322).
     - Lever order that worked: declaration and pads first (the stack line), then zeros
       (rule 326), then chains, then statement order.
329. **A task proc whose message dispatch tests 3 (`li at,3; beq t6,at,L; nop; b default`) has an
     explicit `case 3: break;`, even when case 3 does nothing.**
     - Without it, the dispatch is shorter (`bne` to the default), and every later instruction
       moves.
     - Proved by tlb_0152's l_stone (310 lines to 40 with it) and tlb_0085's sh_hani (Goemon
       (631), (632)). With the work pointer set at its declaration, both then kept obj in its
       stack home, as the game does. Before that, sh_hani had obj in s1.
330. **Open: a u16 message the game reads from its home (`sw a1,home; lhu t6,home+2`), not masked
     in a1.**
     - The two boss tasks tlb_0324_0 Btai and tlb_0325_0 Bfri switch on
       msg with `lhu` from the home and pass it on with `lh`.
     - An ANSI `u16 msg` gives `andi a1,a1,0xffff` and a spill. `s16` with a `(u16)` switch
       gives `lh` and `andi`. `s32` gives `andi`. A K&R definition stores the home but still
       masks the register.
     - Taking msg's address (`u16 *p = &msg;`) gives the game's `lhu` from the home. So in the
       source something held msg in memory. It isn't in the code, so it may be compiled out
       (a debug macro?). Not kept: a FAKE that adds instructions.
     - An s16 msg (IKA_BOMB, every enemy proc) is homed and read with `lh` with no help.
331. **A register local declared first takes the top slot turn (rule 322), and that can be the
     game's empty word above its locals.**
     - tlb_0111's second fire (func_08001A4C): `s32 v = eneSetup(...)` declared
       above `w` put plAskPos's four locals and sa/ty at the game's 0x58-0x6C.
     - In the same layout, a `pad` or a named local declared there grows the frame by 8 instead.
     - The same lever on its twin func_080013B0 moved the spills down with the locals (parked).
332. **When the game has one word more between its locals and its spills, pad both sides: one
     `pad` above the locals and one below them.**
     - tlb_0102 Hikari (func_08000B08) matched this way.
     - A pad above alone moves the locals to the game's slots, and the spills move up with
       them, because the spill area sits a fixed distance below the lowest local.
     - In tlb_0111's fire, the same two pads grow the frame (0x70 to 0x78), so it is not general.
333. **IDO pads every object's .data to 16 bytes.** Zero words after a file's last data, up to a
     16-byte boundary, belong to that object.
     - tlb_0020_0: its .data is chademoTick (5), a function-local static (1),
       and two zero words. tlb_0020_1's tables start at 0x08005340.
     - An 8-byte .data for it shifted everything after it by 8.
     - Making the static a global in the next file's data lost the static's register caching
       (130 lines, rule 325).
334. **Orphan strings land with the function that prints them.** A main proc whose .s holds a
     step's print strings can leave its .s once that step is C, with each print written as
     `DPRINTF(...)` where it belongs.
     - tlb_0102's transfer and its squash step: "transfer START", "FADE START",
       "GO" are in the squash step and "transfer IDLE [%d]" is in the counter. Both landed in one
       build.
     - nuri (tlb_0080) and haita (tlb_0169) match their checks and wait: nuri on its parked
       swing step, and haita on which of two steps prints "vx %f".
335. **A file's .bss or .data in C: globals first in address order, then statics in function
     order, every unread word declared where it sits** (tlb_0024_1,
     tlb_0026_1, tlb_0019_1, tlb_0025_1, tlb_0028_1). This extends rules 197, 199 and 315.
     - Map every word of the blob to its readers (grep the D_ labels over the asm and C) before
       choosing the start. A segment's whole .bss was one file's each time.
     - IDO 8-aligns an array, in .bss and .data alike. A 4-aligned run of unread words is
       scalars (tlb_0026_1: `D_08005FA4[8]` landed at +8 and broke the build).
     - A static the asm still reads becomes a global of the same splat name, declared where
       the layout puts it: at the top, or after the function whose static precedes it.
     - `static s32 x = 0;` goes to .data, not .bss.
336. **A local that IDO folds to one register in one case can be forced out by a literal of
     another type.** genoGOE (tlb_0026_1, (670)) reused the shadow's 0.3f in case 6; the game
     loads it again. Writing that call's literal as the double `0.3` stops the reuse, as
     chaYAE's demoAnimeStep did.
337. **Open: the last function's statics come first in two files.**
     - yuki (tlb_0024_1, 0x080066B4) and WATER (tlb_0019_1, 0x08007FB0-B7) are their files'
       last functions, but their statics sit ahead of the first function's.
     - My all-C compile puts them last.
     - A file-scope static changes yuki's code (587 lines), and `= 0` sends it to .data.
     - Both stay parked, their statics defined as top-of-file globals for their asm.
     - A third case, 2026-10-10: tlb_0022_1's jiji (its last function). Its alpha, step and
       speed (0x08006A90-98) come before the file's globals and every other static. It matches
       its check.
     - Rejected, measured on yuki: the static declared at the head of the switch body (a nested
       block), and the static named `step` like D6GOE's. Both keep the code, but the static
       stays last (+0x2C).
     - Rejected, measured on jiji 2026-10-11: his three as file-scope statics at the top (912
       of 1263 lines differ; it matches as function statics) and as `static volatile` (925).
       Only a function static gives his code.
     - Measured 2026-10-12, nine probe compiles (`match_function.py probe`,
       read with `objdump -t/-dr`). .bss stayed in source order
       under every form:
       - a static that is never written;
       - statics reusing earlier functions' names;
       - `__inline` on a function defined first (ignored: its code stays first);
       - a file static redefined below the functions;
       - names first declared at file scope as an `extern` or a prototype's parameter.
     - IDO emits no COMMON: an extern first, a tentative definition twice and a plain global
       all go to .bss.
     - Static arrays and structs are 8-aligned (a word of padding before them).
     - Codegen, three bodies the same: a function static gets `lui at` per use against `.bss`.
       A global (defined above or below) and a file static (above, or redefined below) all
       keep `lui/addiu v1` in a register.
     - jiji with his three words as the top-of-file globals: real 318, 10 instructions longer.
     - tlb_0022_1 is one object: its .rodata strings run goe, EBI, chaS, chaYAE, jiji, KEMURI
       in source order, and jiji is not its last function (jijiDemoJijiCreate, kemuri and the
       chimney smoke follow him).
     - **Solved 2026-10-12: .bss is NOT declaration order. It is ugen's
       256-bucket hash of cfe's symbol numbers.** Probes compiled
       with the build's cc and flags plus `-S` (the .s shows the `.lcomm` order) or `-keep`
       (the ucode `.B` shows each symbol's number: `0x27` = function static, `0x2a` = global,
       `0x53` = file static/string, `0x24` = extern, `0x21` = procedure).
       - **The rule:** cfe gives every symbol a number from one counter; ugen emits the
         `.lcomm`s by bucket `number % 256`, buckets 0..255, ascending within a bucket. Under
         ~250 symbols that looks like source order (343). In a big file, a late function's
         statics pass 256, wrap into the low buckets and come out FIRST.
       - Run E (`bss337e.c`): 1000 file statics `va00..vj99` (numbers 3..1002), then
         function statics. Out: `vc53` (256), `vf09` (512), `vh65` (768), `vc54`, ... `va00`
         (3) is the 10th line, after buckets 0-2. The function statics follow their buckets.
       - **What takes a number:** each global, file static and function static (at its
         declaration); each extern (at its first use); each string literal; each procedure, and
         its exit label at the end of its body. These take none: typedefs, struct tags, enums,
         prototypes, unused externs, locals, params, blocks, if/else, loops, switch, ?:, &&,
         float literals (runs F and G, `bss337f.c`, `bss337g.c`, one `-DCASE=n` each).
       - **Codegen:** only a function static gets the `lui at` per access. In the ucode a
         function static is declared by a `0x27` inside its procedure; a file static or global
         is declared at the end of the file (`0x53`/`0x2a`), and uopt then makes `la` addresses
         and holds them (run B `bss337b.c`, file statics; run C `bss337c.c`, globals: both
         `lui/addiu` into a0-a2/v1). Run D (`bss337d.c`): a function static named like a file
         static is a new object (both `.lcomm`ed). -O3 (run A at -O3): no change in order.
       - **WATER, proven (run H, `bss337h.c`):** `tlb_0019_0.c` and `tlb_0019_1.c` included
         into one translation unit (all C, `-DNON_MATCHING`; two `#define` renames for
         symbols the two files type differently). WATER's eight statics get numbers 274-281
         (buckets 18-25) and are the object's first `.lcomm`s; the D_08007FB0 placeholders
         (127-134) and GenoGoe's `step` (159) follow. In the object WATER's stores are
         `lui at; sb zero,0..7(at)` against `.bss`, in the game's order (FB1..FB5, FB7, FB0,
         FB6), and its .s is identical, label numbers aside, to tlb_0019_1.c compiled alone
         (the form `check` calls MATCH). Without the placeholders (dead once WATER lands) the
         layout is the game's: the eight at +0, step at +8.
       - **So the form is the plain function static, in the game's source file:** tlb_0019_0
         and tlb_0019_1 were ONE C file (349's D_0800766C straddling the split says the same).
         tlb_0019_1.c alone has ~174 numbers before WATER, too few to wrap. Landing WATER needs
         the two merged into one object (yaml + one .c), a decision outside this note's scope;
         no in-file form does it.
       - Run I (`bss337i.c`, tlb_0022_0 + _1 as one unit): jiji's statics get 212-214, short
         of 256 by 44, so they stay last. tlb_0022_0's .data is defined as a few blobs; if the
         game's file had ~44 more symbols (its data as separate arrays, more strings or
         externs), jiji wraps. Not proven; count the game's symbols before claiming it.
       - **Run H's merge (above), tried in the real build and reverted: the game's
         tlb_0019 was NOT one translation unit.** Built as one object (tlb_0019_1.c included
         at the end of tlb_0019_0.c), the .bss and every function's code came out as the
         game's, but the .rodata did not. IDO put all of both halves' strings first and the
         floats and jump tables after. The game has _0's strings, then _0's floats and jump
         table, then _1's strings. So the two halves were separate objects, and the hash
         wrap needs another source of symbol numbers inside tlb_0019_1's own file. Two
         candidates:
         - more symbols than the C has now: strings, externs used, separate data arrays;
         - WATER's statics declared later in a longer file.
         Run H checked code and .bss only: always compare .rodata (and .data) too. Also, a
         .c pulled in by #include skips the build's UTF-8 to EUC-JP string conversion.
       - **Measured further in 374:** about 146 extra numbers fit all four, as a shared header's
         would. WATER and yuki landed with a FAKE label pad.
338. **An integer divisor keeps `div.s` (rule 129 again):** Kurobei's `/ 64` (tlb_0019_1) and
     tlb_0025_1's chaSAS `/ 2`. An integer `2` stored to a float field hoists the constant's
     load where `2.0f` does not.
339. **Two dead reads can fix a whole function's v0/v1 colouring.** suzaku (tlb_0026_1, (671)):
     the permuter's `if (w->model && w->model) {}` in one case gave a full match from 70 lines.
     It goes in as a FAKE; a guarded DPRINTF of w->model compiles to nothing.
340. **Smaller levers from these files:**
     - Store order inside create matters: `w->model = p->model;` before `w->chara = 0;`
       (tlb_0019_1 genoGOE).
     - A step whose frame count m2c shows in one temp across two tests is two ifs: the game
       reloads.
     - An explicit empty `case 7: break;` gives a jump table its entry.
     - One `s32 pad;` where the twins take `pad[2]` (tlb_0028_1 chaSAS: 188 lines to 0).
     - A helper in the same TLB file can still be called far (`(*func)(...)`), when the game
       loads it with lui/addiu and jalr (Kurobei's dust).
341. **Levers from (675)-(683):**
     - **Every declared local takes a stack slot**, even one that lives in a register or is
       block-local.
       - So a block-local `Player *pl` reproduces the game's stack holes (tlb_0042 main).
       - A stray slot can mean the variable is declared at function level.
       - Pads above the first local fix a frame that is short at the top (ihead, `pad0[2]`).
     - **`PL(x)->field` folds the +0x20; the game doesn't.** Use the twins' `pl = PL(x);
       pl->field = ...` (ihead, from tlb_0040).
     - **An int constant where the game has mul.s**: `x * 2` keeps the multiply where `* 2.0f`
       becomes an add (kabukiB). An int float argument is loaded later than a float literal
       (tlb_0021_2 jiji).
     - **A float constant shared by two cases is promoted to $f20.** Writing one use as a
       double stops it (tlb_0042 main's 64.0, dochuki's 69.0).
     - **m2c misreads**: an increment in a branch delay slot, and post-decrement tests.
       - `for (i = 0; i < 4; i++)` looked like `++i` before the call.
       - `if (w->timer-- <= 0)` compares the old value; read the asm's branch register.
     - **Jump tables**:
       - an explicit empty case for a table longer than the cases (ihead's case 17);
       - `case 0: break;` when entry 0 is the default (dochuki, behimos).
     - **`for (i = 0, n = 0; ...)`** when i is set before n (ihead, tlb_0040's form).
     - **A file without DPRINTF calls it as a function.** Check that the macro exists before
       reading a mix distance (kabukiB: 23 to 4).
     - **Trap**: `effect` is a far-call macro in tlb.h, so a struct field named `effect` breaks
       match_function's parse with a misleading "Syntax error in base.c".
342. **Levers from (684)** (BIT, tlb_0325_7 `func_08014FD4`, 2367 differing lines to the game's
     length with its first 1060 instructions matching; and two subagents):
     - **Measure a big switch by its jump table first.** The table's targets give each case's
       length in both objects, which names the wrong cases at once. BIT: modes 1 (+2) and 4
       (-3) of seven.
     - **Float `/ 8 * 3` written with ints keeps div.s.** With `/ 8.0f` IDO makes a multiply by
       0.125 (BIT mode 1: four div.s). The same holds for float `* 2` and `* 1` with int
       constants (subagent, tlb_0324_1).
     - **A `cvt.d.s` then `cvt.s.d` pair with nothing between is `* 1.0`.** IDO folds the
       multiply and keeps both conversions (BIT mode 4's sx, beside sy's real `* 1.1`).
     - **A call the game makes before a store comes before it in the source.** IDO does not move
       a store to a global past a call. BIT's trail loop computes its third point (two calls)
       before storing the first two.
     - **The frame's holes, from the top:** x at the frame's top (no pad above), 5 words, sx and
       sy, 5 words, then the register locals z, a, n and i, which each take a slot. The
       barrier centre (cx, cy, cz) are block locals of mode 1, so they sit below everything.
     - **A store order that the scheduler keeps can fix the registers and not the stores.**
       BIT mode 1 writes colour then alpha, which gives the game's t2/t5/t8 but not the order
       of its stores. Neither order matches; it is parked for the permuter.
     - **A label reached with a fresh lui at every access is a function static.** As an extern,
       IDO shares its address in one register (subagent, tlb_0325_1: 546 to 518 lines).
     - **`if (x <= 0) break;` gives `bgtzl` with an explicit reload on the exit path**
       (subagent, tlb_0325_1: 518 to 271).
     - **Spilled `fadeRandRange` results are call-argument temps, not locals.** Write
       `hitPos + rand` (subagent: 202 to 116).
     - **Zeros written as the int 0 share one f0** where `0.0f` loads each time (obisu, tlb_0142
       step 6).
     - **A field's width comes from its reader.** sabi's `mode` is a u8 because its step reads it
       with lbu; as s8 the registers of 1 and 2 swap (tlb_0017_5, which then matched).
343. **IDO's .bss is in declaration order: file-scope variables and function statics
     interleave by where they are declared** (2026-10-11, test compiles with the build's cc, -O2).
     - `int gA; static int fsTop; int gB; f1 { static a, b, unused; } static int fsMid;
       f2 { static s2; } f3 { static s3a; } int gC;` puts them at 0, 4, 8, 0xC-0x14, 0x18,
       0x1C, 0x20 and 0x24. A global declared after the functions lands after their statics.
     - An unused function static still takes its word.
     - A function static `= 0` goes to .data. A reference to a function earlier in the file
       (`h(f3)` in f1) does not move f3's statics.
     - **Codegen:** only a function-scope static gives the game's lui at every access.
       A file-scope static, a block-scope `extern` and a file-scope extern all keep the address
       in a register (tlb_0023_1 genoGOE: function static MATCH; file-scope static 176 lines;
       block extern 172).
     - **So for 337:** a word that comes before the first function's statics is a file-scope
       variable declared above that function. In tlb_0023_1, the old man's alpha and step,
       Kurobei's and Omitchan's alphas (0x08009640-78) are file-scope, ahead of 4GOE's,
       genoGOE's and chaSAS's statics (0x0800967C-8C). That is the game's layout, and genoGOE
       and chaSAS match with function statics. The yuki/WATER/jiji cases need the same reading:
       a variable that must be a function static yet sits first is still open.
     - Also measured: a `static` function defined first keeps its place first in .text and
       its statics first. IDO does not reorder functions.
344. **311's open case (w folded into offsets), two more measurements on namiMake (tlb_0071):**
     - a dead `PRINTF("…", w)` (common.h's `if (0) osSyncPrintf`) after w is set gives 9 lines,
       the same as the empty `if (w != NULL) {}`;
     - the empty-macro form `("…", w);` changes nothing (16).
     The same family is parked in tlb_0102, tlb_0109, tlb_0137, tlb_0138 and tlb_0026_1's road
     test.
345. **Levers from Bdev_proc (tlb_0327_0, a subagent, (686)):**
     - `x * DEG * 1` with an int 1 keeps the game's `mul.d` by 1.0; `* 1.0` folds away
       (actions 0x2C/0x2D: 230 to the game's 224).
     - A sub-switch needs 5 or more cases for a jump table; 4 make a compare chain (explicit
       empty cases fill it).
     - `switch (*(u16 *) &msg)` (FAKE) gives the game's prologue (`lhu` from the home slot,
       `lh` for tskDefaultProc). Btai's and Bfri's procs park on the same prologue.
     - Open: IDO keeps D_3000804's address in s3 where the game makes it at all 33 uses. IDO
       always merges repeated uses of one symbol's address, so the game's case is a register
       priority, not a CSE limit.
     - Landing with .rodata: a GLOBAL_ASM rodata pragma kept for a parked function's string
       must go when that function lands, or the string is emitted twice (tlb_0021_3's
       "goecreate": the build failed at its rodata until it went).
346. **Levers from Bbisu_proc (tlb_0323_0, 16,121 instructions, a subagent, MATCH, (687)):**
     - A countdown m2c prints as decrement-then-test is `if (w->timer-- == 0)` (the game's
       `sltiu` on the old value): 16118 to 16121 instructions. Bkas showed the same.
     - Empty cases where the game's jump table points at the break fixed every table size.
     - One `s32 pad` below the last local moved the compiler temps up 4: 4696 to 4510 lines.
     - **A float field given a double literal** (`rot[2] = 3.1415927406984143`, the f32 pi as
       a double) keeps f0 free where `3.1415927f` loads into it and shifts the float temps for
       the rest of the function: 4510 to 796.
     - Read-modify-write order (note 327) twice in action 0xF: 796 to 770.
     - `w->count--; w->vel[1] -= 4.0; if (w->count == 0)`: 770 to 744.
     - A multi-statement macro is one source line (note 324): the death knock-backs need the
       angle and both velocities in one macro, the stores after it on their own lines.
     - Three separate zero stores where a chain stored in the wrong order: 744 to 740.
     - One FAKE line (an if and the next statement on one line) in action 0: 740 to MATCH.
347. **The "work pointer's addiu not in the beqz slot" family is note 324: one source line.**
     Where the game has `addiu sN,sN,0x20` (w) in the first test's delay slot and IDO here puts
     the far call's address addiu there, write the first null-tested block and the next
     statement on ONE line: `if (w->a != NULL) { frmDestroy(&w->a->f); } if (w->b != NULL) {`.
     Found by the permuter on kensukeMoneyProc (tlb_0137), then it matched tegata (tlb_0138),
     teleclub (tlb_0141) and bigBomb (tlb_0043, msg 1 and msg 3), and took gareki (tlb_0048) from
     24 to 20. tlb_0081's haniKo had found the same form in 2026-10 ("one line each"). Earlier
     splits sweeps had cut statements across lines but never joined two onto one.
348. **The compiler temps sit right under the last-declared local** (the (691) sweep, a
     subagent). So a pad array's size moves them one word at a time:
     - temps 4 low against the game: drop one pad word (the 8-byte frame rounding usually
       keeps the frame). tlb_0327_1 (three functions), tlb_0326_1, tlb_0040's hahen2 and
       tlb_0038's 292C went straight to MATCH;
     - temps 4 high: add one pad word below the last local (fushaProc, tlb_0155 02C8);
     - no slack (the frame grows by 8 instead): the trick does not apply (cFIRE, trapFireball,
       monoeyeTear, karasuTama).
     Note 346's "pad below the last local" is one case of this. Also from the sweep: a park note
     can hide a wrong literal (tlb_0023_1 466C's sink was 1.75 in the C, 0.5 in the game: 660
     to 274 lines).
349. **Data boundaries to recheck before landing statics.** tlb_0019_1's flash counters, and
     tlb_0324_1's and tlb_0325_1's statics, match as function statics, but their words sit in
     a neighbour file's .data. In tlb_0019, D_0800766C[6] starts in tlb_0019_0's .data (two
     words nothing there reads) and runs into tlb_0019_1's, so the split point there is
     likely wrong.
     - Found in (692): in tlb_0324 and tlb_0325 the first .data block after the boss's own
       was named tlb_0324_4's / tlb_0325_6's. Its words are all read by _1 (then _2's from a
       16-aligned point), which is the link order. Rule: a data block belongs to the object
       whose code reads its words; check that before naming a data subsegment.
     - Placement: the definitions go in address order, with each landing function's static
       inside it; the words after it are defined after that function's body.
350. **Levers from (693)-(694):**
     - **Where the game loads a float constant early, its store comes first in the source.**
       tlb_0017_2's twin pol makers matched with `rot[1] = pi` written before the three
       `scale = 0.3f` stores (the permuter's find; every one-line and chain form was 14 lines).
     - **A matched function whose .s holds a later parked function's print strings lands with
       those prints emitted at its end, in .rodata order** (tlb_0080, tlb_0169). Guard them
       with `#ifndef NON_MATCHING` when the owner's C prints them too.
     - **Read the string order before placing a print.** tlb_0080's "奥へ"/"手前へ" came after
       the case-0 print in the game's .rodata, but create (case 2) falls into case 0. So they
       were never create's.
     - Second sweep (note 348): the direction is "game's temps higher → remove a word"; two
       more matched that way. Every declared local, even one in a register, seems to keep a
       stack slot, so the order of declarations alone moves slots (tlb_0038 29C8: 86 to 66).

351. **Levers from (695)-(696):**
     - **A multi-statement macro is one source line, so IDO may reorder its stores (note 346).**
       tlb_0326_0's knock-away on death, written as a macro, stored `action` before `unk58`;
       the game stores them in source order. Written out at its six sites, the stores follow
       the source. Its two velocity lines stay on one line: written apart, the angle's reload
       for `cosf` moved before the `sinf` multiply.
     - **When one macro matches at one use and not at another, the game's two bodies differ.**
       tlb_0326_0's hand attacks (actions 3 and 4) share every step but the raise: action 3
       sets flag 4 after the turn, action 4 before. Pass the step as a macro argument
       (`RAISE_FLAG_LATE` / `RAISE_FLAG_FIRST`) rather than copying the macro.
     - **A float local that holds a field can block the matching operand order.** tlb_0080
       0CDC: reading `w->model->frame` directly in the test (not through `frame`) fixed one
       `17.0f + w->speed` add (the permuter's find). The second add still takes 17.0 reused
       from the `<= 17.0f` test in the other operand order.
     - Splitting a one-line macro does NOT fix a constant address kept in a saved register
       (tlb_0326_0's gFrmEffectLayer, tlb_0327_0's D_3000804): no change either time.
     - **A second use of an address cast differently is built again.** `(s32) D_30009DC` where
       the first use is `(u32)`: IDO treats them as two expressions and does not keep the first
       one's address (tlb_0040, tlb_0085, tlb_0099). Not enough when the first use is inside a
       macro (Bdev's D_3000804).
     - **A mask that as1 deletes still spends a temporary.** `eneSetup(o, arg) & 0xFF` stored by
       sb: the andi is gone from the output but ugen used a register for it, so every later
       t-register moves up one (tlb_0142, tlb_0169's tama1).

352. **A function's C changes how IDO compiles the function after it.** tlb_0018_6's
     func_08008470 compiles to 181 instructions (the game's) with func_080082B8's C before it,
     and 180 with 82B8's GLOBAL_ASM or with nothing there (measured with the build's compile,
     2026-10-11). So a function can match only once the one before it is C, and a check that
     compiles parked neighbours as C can print MATCH where the build does not.
     `match_function.py import` now leaves the other parked functions' C out, as the build does.
     **The mechanism, measured (82B8's body cut down in a scratch copy, build flags):** what
     moves 8470 to 181 is 82B8 TAKING 8470'S ADDRESS (`w->handler = func_08008470`). An empty
     body, a store to gSaveData, a call to 8470, or taking another function's address: 180. A
     dead `if (0) { w->handler = func_08008470; }`: 180. So uopt compiles a function whose
     address live code earlier in the file takes differently; nothing in C can stand in for
     the parked function that takes it (no code-free FAKE), so such a function lands with or
     after the function that takes its address. geton's step (tlb_0017_0 func_08000458, whose
     address the parked func_08000000 stores as w->step) is the same case. Not yadoya: its
     address taken first in a scratch copy changes nothing.
     **Possible build fix, not done:** asm-processor's stand-in for a GLOBAL_ASM function takes
     no addresses; one that took the same function addresses as the real assembly would give
     the functions after it the game's context.

353. **Levers found 2026-10-11 (each measured; the functions named matched with it):**
     - **The edopole hit-loop form** stops IDO keeping the constant 1 in a saved register:
       `if (box == NULL) continue; if (acollAtkWasHit(box) == 1 && (Obj *) box->hitPower !=
       NULL) tskSendMsg(...)` (tlb_0045 bpoleProc, tlb_0053 gear1, tlb_0059 girotin).
     - **`(Obj *) x->hitPower != NULL`** loads the hitter straight into a0 (beqz), where `!= 0`
       loads v0 and copies it (tlb_0037 T_fire, tlb_0099 koban).
     - **A pad word is often a real local.** Read a repeated field or subexpression into it:
       `since = w->unk49` fixed a v0/v1 colouring (tlb_0111 gasya); the move's difference in a
       local `d` fixed float colouring (tlb_0169 E58). The game also names call results and
       arguments where the C had compiler temps, which moves the locals a word down
       (tlb_0038's hand, koban, sirobzStepDrop).
     - **A dead test of a parameter's address** (`if (&o) {}`, FAKE) keeps it in its home, so
       IDO keeps only the index product across calls (tlb_0131 14C8). Not karaE1 (w then built
       at entry).
     - **A FAKE `^ 0` on a work pointer** keeps its addiu in its own register (tlb_0102 10FC
       and 11F8 down to 6 lines; not kept on tlb_0324_2 0C0C8, where it made the pointer once).
     - **A pad before w** lowers the compiler temps without growing the frame when they sit one
       word high (tlb_0326_1 Taki).
     - **A second spelling of the same read** (`(*w->model).frame` for `w->model->frame`,
       FAKE) stops IDO reusing a constant across the two (tlb_0080 0CDC, the permuter's find).
     - Block locals are allocated below the function's locals, in declaration order.

354. **SOLVED 2026-10-12 (getonProc): an unused word in the spill area is a pointer set early
     and rebuilt.** Set the work pointer (`w = FX(obj)`) before the calls, at the top of the
     case: IDO gives w a spill slot, then rematerializes it from obj (addiu) at each use, so the
     slot is reserved and never written. Measured with `match_function.py probe`,
     one change at a time:
     - The order top-down is function-level and block locals, then struct-return temps, then
       spills. A pad local, at function level or in a block, sits above the temp and moves
       everything down 4.
     - Only an early pointer gave the game's frame 120, spill 76, hole 80, temp 84, with the
       same 235 instructions. getonProc then matched with no pad.
     Apply it to the family below: the hole is a pointer the game computed before its calls.
     The history, as it stood:
     **The game's spill temps sit above words it never uses.** IDO places the spill temps
     up from the bottom of the frame (above the outgoing arguments and saved registers), not
     under the last local: in tlb_0042's dochuki (func_08002D30), shrinking the lowest block
     local by two words and adding two at the top moves every local down 8 and leaves the temps
     at 0x3C/0x40. The game's temps are at 0x44/0x48, with 0x3C-0x43 never read or written.
     karasuTamaProc (tlb_0135, two words), kpaFireballProc (tlb_0062, one), gimFboardProc
     (tlb_0048, one), tlb_0111's func_080013B0 (one) and monoeyeTearProc (tlb_0177, one) have
     the same shape. tlb_0039's main (func_08000000) shows the order: both it and the game
     reserve three temp words for state 6's loop region, but the unused one is the game's lowest
     (0x50, temps at 0x54/0x58) and this C's highest (0x58, temps at 0x50/0x54). Temps are given
     out from the bottom in order, so the game makes its unused temp before the loop's two and
     this C makes it after. tlb_0033_3's cFIRE (func_0800B8F4) is the smallest case: a local
     declared first takes the block's lowest word (0x50) and pushes the spill to the game's 0x54,
     but the frame grows by 8 (0x70 to 0x78), so this C also reserves 8 bytes the game does not.
     IDO's `cc -S` (run from the file's folder, with devkitPro's msys runtime on PATH) shows the
     frame: saved registers right above the outgoing arguments, then this block. Variants of
     cFIRE (one call or block left out at a time): the locals' words sit in a fixed three-word
     gap between the outgoing arguments and the saved registers whatever is removed; the block
     above the saved registers is the spill area, the spill in its first word and the rest
     reserved. So the game's first spill word went to a value that in the end was not spilled.
     Something in the
     original made temps that were then not used; not found yet. A probe (a far-call loop under
     register pressure, build flags): an int-to-float conversion, a float multiply and a double
     multiply grow the frame only by the saved registers they add (s4, $f20) and alignment; none
     of them leaves an unused temp word.
     getonProc (tlb_0017_0), 2026-10-11, the cleanest case: floadLoadSet returns a FileSet
     through a hidden pointer (a0 = sp+0x54, one temp for its five calls), and tskCreate's
     address is spilled (sw v0,0x4C; lw t9,0x4C before each jalr). The game: spill 0x4C, hole
     0x50, struct temp 0x54. This C with no pad: spill 0x50, temp 0x54 (the temp is already the
     game's); with a pad: spill 0x4C, temp 0x50. So the hole sits between the one real spill and
     the struct temp, in the spill area. Not the return type's alignment (a 4-aligned
     { s32, s32 } through a cast: no change). Every repeated callee is reached the same way in
     both (frmCreate and floadLoadSet in registers, tskCreate spilled). Next to test: what
     IDO spills and then rematerializes (a constant address live across a call), which would
     leave its slot reserved and never written.
355. **A const initializer that holds an address goes to .data, not .rodata.** tlb_0250's 26
     `{ 0x40B0E92D, D_3000000 }` pairs are .rodata in the game; written `const u32 x[] =
     { 0x40B0E92D, (u32) D_3000000 };` IDO emits them into .data (match_function.py rodata-c:
     ".data, .rodata and their relocations differ"). Const strings and plain const words do go
     to .rodata (122 blocks, (708)). How the game got address words into .rodata is not found
     yet; they stay assembly. Also .data: the same words as `void *const x[] = { (void *)
     0x40B0E92D, D_3000000 }`. The blocks are UKIYO TOWN's place tables (ukiyoTownPlaces...),
     the same records other stage files keep in .data, so the open question is how they came
     to sit after the file's .rodata start (0x08000560), not how to write them.
356. **A diff that drops a lot does not prove a statement order; the jal order does.** The swaps
     sweep found swaps that cut a parked function's diff by hundreds of lines (clr5DemoGoeProc
     1,659 to 845, tlb_0042's func_08004374 850 to 164) by moving a call past a store or another
     call. Read against the game's asm, all 12 checked were the wrong way round: the order shifts
     the register numbering through the whole function (float registers one behind), and that
     made up the drop. IDO keeps calls and memory stores in source order, so the asm shows it.
     Only swaps of stores to different places, or of locals, are free; match_function.py
     swaps-apply refuses the rest. (710)
358. **A call's result passed straight as an argument costs a spill-area temporary.** Where the
     game reads it into a local first (`data = eneFileAddr(...); rideCreate(..., data)`), the
     inline form adds a temp and moves the spills one word (gimFboardProc, tlb_0111's
     func_080013B0 and func_0800205C: all three matched with the local). Try this before a pad
     when spill offsets are off by a word.
     - 2026-10-12, two more. Both were 1-3 lines off, a temp a word from the
       game's:
       - tlb_0039's main: state 16's camSetAt height, in a block local;
       - func_08005728_tlb_0039: fadeSegToRam's result before eneNewAnime2.
       One such call anywhere in a function moves every temp after it.
     - **Its reverse** (karasuTamaProc, open): when this C's spill area is two or three words
       SMALLER than the game's, look for results the game passed inline.
     - **An early return frees a t-register** (lanternSwing, tlb_0146, (748)). The body was
       wrapped in `if (w->hurt == 0) { ... }`; written as `if (w->hurt != 0) return;` and the
       body after it, ugen frees the reloaded obj's t6 first, as the game does.
359. **IDO picks the operand order of `+` and `*` itself; only the shape of a sum moves it.** It
     calls the functions in a sum left to right and emits `a + b` with b's register first. Write
     a rotated coordinate as position plus the turn, the product whose call comes first on the
     left: `pos[0] + (z * sinf(r) + cosf(r) * x)` (gasyaBodyPointFollow, tlb_0111's 205C,
     tlb_0099's 21EC). Swapping terms inside the same shape changes nothing.
360. **An empty `else {}` arm keeps the game's `bnez` + `nop` and a `b` over the arm** where IDO
     would otherwise use a branch-likely and come out two instructions short (the four debug-menu
     directors, (714)). A compiled-out debug print leaves the same code, so it is a likely
     original.
361. **A function static read directly (no local copy) is held in a register through the block
     and stored back by IDO before each call** (2026-10-11, tlb_0325_1's
     func_08003700: the long attack's counter, `if (count < 260)` ... `10000 - count`, matched
     from 275 lines). A local copy `n = count; ... count = n;` gives a `move` and a separate
     store the game does not have.
362. **The stack's spill-temp area grows up from just above the saved registers; locals grow
     down from the top of the frame** (2026-10-11, tlb_0042: saved registers at
     0x28-0x34, temps from 0x38). A gap between the two is unused words. So a `pad` local
     moves the locals but not the temps, and a missing temp word shows up as temps one word low
     while the locals are right.
363. **A float literal can be pooled twice with nothing loading the second copy.** In
     endMac2Proc (tlb_0034, (717)), 0.67f is stored three times before a NULL check and three
     times after it. IDO pooled 0.67, pi, 0.67, 0.67, pi; the game has 0.67, pi, 0.67, pi.
     - Written `0.67` (a double) on one side, the same instructions pool one copy per side.
     - Probed with several forms of the check; all of them kept the extra copy (`== NULL`,
       `!`, an assignment in the test, `goto`); f/double mixed in either order dropped it.
     - Read the game's .rodata when a function is MATCH but parked for its literals.
364. **The operand order of `a + b` follows how each operand was last reached** (tlb_0324_1's
     func_08005C98, (718)). The game's `add.s f10, f2(x), f8(vel)` comes from `z` as a local
     with x read through `w->frame` in `sqrtf(pos[0] * pos[0] + z * z)`, then
     `w->frame->pos[0] += w->vel[0]`.
     - x and z both locals: `x * x + z * z` gives vel + x; `z * z + x * x` gives x + vel but
       squares z first.
     - No locals at all: the order is right, but the spill temps sit a word high.
     - vel[0] in its own local: the order is right, but vel takes f14 and every register after
       it moves.
     - So when only an add's operands are swapped, change which values are locals, one at a
       time, before trying the order in the expression.
365. **IDO tells real constants apart by their spelling, not their value.** Two literals that
     are the same double (`0.0174532930038800785` and `0.01745329300388008`, both
     3F91DF46AAA2FD9A, measured with a probe) are two constants to uopt. They are not shared,
     and a constant it keeps in a register (Bdev_proc's DEG in f2, reloaded after each call)
     does not serve the other spelling.
     - Bdev_proc, action 6 step 6, written as DEG: its region's clusters go (170 lines to 16),
       but the allocation elsewhere shifts.
     - Read a .rodata value back to its spelling in the matched code nearby before choosing
       between equal literals. (717)'s double 0.67 is the same family.
     - Splitting works as well as unifying. Bkas_proc ((729), 70 KB) went from 2166 real lines
       to 38 with 0.1f written three ways. The game keeps some uses of 0.1 in a saved float
       register and loads others into temporaries, each from its own .rodata entry:
       - action 14's steps 2 and 9: `0.10f`;
       - action 15's step-1 pompoms and step 3's second pompom: `.1f`;
       - everything else: `0.1f`.
       A spelling shared by every use was promoted in the wrong places or not at all.
     - How to read it from the game: a constant loaded with `lwc1 $fsN` and passed or reused
       from that register is a promoted spelling. A separate `.rodata` entry for the same value
       loaded into a temporary is another spelling.
     - **When every float temp is one register off from some point on, count each f-register's
       uses over the whole function.** A register this C almost never uses (f16: 3 uses here, 62
       in the game) is held by a constant IDO kept for the whole function. In tlb_0042's behimos
       (func_08004374) that was case 55's last 0.1. Spelled `0.10f`, it took the
       function from 298 to 3.
366. **An address the game loads into an argument register early, above the test that guards
     its call, was read into a local first** (st3clearJijiProc, tlb_0028_1, (727)). The game
     sets a1 = D_3000410 above the first `== 0x593` test; this C built it at the call. With
     `anim = D_3000410;` before the tests and `demoAnimeSet(w->model, (u32) anim, ...)`, the
     960-instruction function matches.
     - The local takes a stack word: it goes where a `pad` was.
     - The permuter (with --stack-diffs) found it from 545 to 361; its other change, an empty
       `if (w->model && w->model)`, was not needed.
367. **`!(x % 2)` keeps IDO's whole signed remainder; `x % 2 == 0` folds to an `andi`**
     (omiDemoJijiProc, tlb_0023_1, (728)). The game's `andi; bgez; beqz; nop; addiu -2; bnez`
     for a signed x is `if (!(x % 2))`. `% 2 == 0` was 8 instructions short and `% 2 != 1` two
     long.
     - Also from that unpark: .data is laid out in *definition* order, and an `extern` above
       does not move a later definition (measured with a probe). A function static
       that sits inside a .data block defined at the file's end needs the block's earlier
       definitions moved above the function (`match_function.py move-lines`).
368. **Array elements and named fields are evaluated in different orders** (tlb_0178's
     fireball, (733)). `gDPSetPrimColor(..., w->prim[0], w->prim[1], w->prim[2], 0)` on a
     `u8 prim[3]` loads blue first. The same bytes as fields (`w->primR, w->primG, w->primB`)
     load red, green, blue as the game does: MATCH from real 24.
     - When a GBI colour word (or any OR/sum of several struct bytes) comes out in the wrong
       order, try the bytes as named fields before anything else.
369. **Open family: one value in v0 here, v1 in the game, the code otherwise the game's.**
     Six parked functions differ only this way (2026-10-12):
     - sabiStep: case 7's clamped level;
     - utaDemoScript: the scene's switch value;
     - chademoSasProc: demoTexAnimK2's address;
     - demo22GoeProc: case 7's timer;
     - Etsk_beto_main: the model pointer in states 6 and 7;
     - omiDemoShortProc and omiDemoProc: the skip count, a promoted function static. In the
       game, v0 then takes gfxGetPendingSwaps' address, hoisted into case 50's branch slot.
     Probes give v1 for sabiStep's block alone and v0 for
     beto's pointer alone, whatever the callee before returns. So the choice comes from the rest
     of the function: IDO colors before it schedules, so suspect which values it colors first
     (use counts). Untried: changing how often the value is read (a test written twice or
     once), and bisecting by deleting other cases until this C gives v1.
     - **SOLVED for three (2026-10-12): a call's dropped result.**
       Whether a call statement leaves a value in v0 decides whether v0 is free for the values
       live around it. Both directions matched:
       - utaDemoScript: `joyGetHeldTime(1, 8);` as a bare call (a u16 result) held v0, and the
         scene took v1. Written `if (joyGetHeldTime(1, 8)) {\n}`, a compiled-out debug test,
         the scene gets v0. A call through a void type also matched; `(void)` did not.
       - chademoSasProc and tegaFProc: a callee the header declares void returns a value in the
         game's C (an undeclared function is int; soundIncamCam's code returns SoundIncam2's
         value). Called through an int-returning cast for that function only (FAKE: `#undef` /
         `#define f (*(s32 (*)(...)) & f)`, restored after), the kept value takes v1.
       - haniRRoll went from 22 to 1 the same way (eneAnimLoop's result in an empty if).
       - First move on any v0/v1 swap: for each call near the swapped value, try its result
         used (an empty if), and try its callee as returning int.
       - Same day, six more:
         - Bdev_proc's ghosts: ONE call cast (create's bossSetFrameFlags2(w->ghost[1])). All six
           calls cast gave 187, because other values move instead. Try single calls first.
         - demo22GoeProc and both omiDemo procs: scRunEnd cast at one call.
         - Etsk_beto_main: soundIncamCam.
         - dochukiDemoGoeProc: demoTexAnimK0.
       - Tool: `match_function.py results <file> <func>` tries each call statement both ways and
         keeps the best. Proved on demo22GoeProc's original line (4 to 0, 83 s).
       Still open in the family: sabiStep (a probe puts the cause in case 4's players loop).
357. **IDO puts a `lui $at` before every store to a global; one `lui $at` serving several stores
     is not this toolchain's output.** endParentProc (tlb_0034) clears its two u16[4] arrays
     with one `lui $at` per array and four `sh $zero, %lo(sym+k)($at)` each. Probed with
     match_function.py probe (2026-10-12): loop, unrolled stores,
     buttons first, two loops, one 8-element array, a struct of both, each element its own
     variable, the arrays extern or defined. Each was tried under IDO 7.1 at -O0, -O1, -O2 and
     -O3 (called directly), and under IDO 5.3. Every one gave a `lui $at` per store (or a base
     register, or a loop). A search of every .s: 0 of the 6,225 matched functions have the
     pattern; the one parked function that has it is endParentProc (func_08000000_tlb_0034.s
     is its stale pre-rename copy). So this clear came from something else than this file's
     C compiled the usual way (2026-10-12, measured with a probe: also file `static` u16s,
     relocated against .data, and -g; the same). Unknown yet: a different assembler pass, a hand edit, or a form
     no probe has covered. My first -O3/-g3 probe through IDO_EXTRA was void: asm-processor
     refuses a second -O and the probe printed the previous run's object; probe now deletes
     its object and calls IDO directly for a level.
370. **Strings, prints and .rodata order (2026-10-12, main's string pragmas).**
     - IDO emits a file's string literals in SOURCE order, even from a switch's default or its
       later cases (measured with two probes). A string out of place in the game's
       .rodata means its print sat elsewhere in the source: a later function, or an earlier
       case.
     - `PRINTF` (`if (0) osSyncPrintf`) and an empty `DPRINTF` both keep their string literal.
       A print written as an offset into a pragma (`gStrs + 0x24`) emits nothing.
     - A print is a statement boundary. At a function's head, or between two statements IDO
       schedules together, it can change code (BmoveX, plDamage, shimEneDeathAct). Inside a
       branch, an else or a return block it usually doesn't. Inside an unrolled loop it can cost
       instructions per copy (eneDeathAct: +8).
371. **Assigning a call's result inside an if's test costs a spill word** that "x = f(); if (x ==
     NULL)" does not (obj22Falls22VProc/RProc). When the game's spill area is one
     word smaller, look for this; when it is larger, look for the reverse.
372. **An in-place divide** ("sx = c[3]; sx /= 10.0f;") gives the game's `div.s fN,fN,fM` and a
     different float colouring from "sx = c[3] / 10.0f" (obisuRunActStep).
373. **Check field offsets against the game's loads.** Three functions matched once a wrong
     field was fixed: func_0800C0C8_tlb_0324 (the AnimeFrame's flags are f.flags at 0, not
     0x38), func_08018AD0_tlb_0324 (the sway is rot[0]), gasyaMainProc (the ambient light is an
     AmbFrame). Several also had wrong float constants, which `check` now catches.
374. **337 measured, 2026-10-12: `match_function.py bsswrap`.**
     - `bsswrap <file.c> <N>` compiles a file all C behind N extra symbol numbers and prints
       its .bss layout. `bsswrap all <N>` lists the TLB files whose layout changes.
     - At N = 145 only the four family files change (WATER tlb_0019_1, jiji tlb_0022_1, the
       old man tlb_0023_1, yuki tlb_0024_1). Every other TLB file keeps its layout. Windows:
       WATER 86-161, jiji 142-152, yuki 135-149. One count of 142-149 fits all of them. So the
       game's TLB files likely shared a header that took ~146 numbers this C does not have.
     - What takes a number and emits nothing, measured (probes bss337k/l/m/n.c):
       - **a goto label: yes.** But a label only fits in a function.
       - sizeof of an extern, sizeof of a string: no number.
       - Unused static data reaches .data. Unused static functions reach .text as a
         `jr ra; nop` stub, even with a body. `#pragma weak` needs a defined name.
     - **SYMBOL_PAD** (include/symbol_pad.h, FAKE) is 146 labels in the file's first function.
       WATER landed with it. Yuki landed with it plus Goemon's statics in a block after the
       pad (a second FAKE), because his statics, numbered before the pad, took the lowest
       buckets.
     - jiji and the old man have .bss globals at the top of their files. Those keep numbers
       2-15, and the wrapped statics sort into the same buckets: jiji's speed landed after
       D_08006A9C. They need numbers ahead of the globals, at file scope.
       - jiji landed: the file's first string, "goecreate\n", already first
         in its .rodata, is a file-scope `static const char goeCreateStr[]` ahead of the
         globals, printed through its name. Its numbers are the ones the globals needed.
       - The old man landed. tlb_0023_1's eleven top "globals" were statics of
         the three Short procs that use them: the old man's 6, Kurobei's 6, Omitchan's 3, each
         with its unread words. SYMBOL_PAD and a block in omiDemoGoeProc, as in yuki's file.
         All four of the family are C.
       - **.data is NOT hashed** (bsswrap's .data view, tlb_0324_1 at 0 and 145 extra
         numbers): globals go where they are defined, and a function's initialised statics go
         at the function, in source order either way (probe dataord.c: declaration order).
         - **Corrected the same day:** func_080032D0_tlb_0324 landed. "Fits no
           source order" was wrong: it assumed both words were the function's statics. Only F0C
           is. swerve (EF8) is a plain global: with swerve global and F0C static, check matched
           at once; the earlier 169 was F0C's alone. The marble words after F0C are globals
           defined after the function. Test each word alone before calling a layout impossible.
375. **One stack word per type for a value used straight out of a side effect** (probes
     frontwords.c, ihtemps.c ihB-ihG). Each of these makes IDO spend one unused word:
     a call's result as another call's argument, an assignment in a test, a chain, a tested
     post-decrement, a ?: argument, and every switch. All the uses of one type share the word.
     Every proc has a switch, so the int word is always there and only a float word shows up as
     a difference. oniFightOniProc went 38 to 19 with `ang = eneCourseAngle(..);
     eneTurnToward(o, ang, ..)` in place of the nested call.
     - **The type is the call's return type, and s16 counts as its own type**.
       clr5DemoOnaraProc and its twin were one word off: floadGetSetFile (s16) passed straight
       to fadeAnimStart spent an s16 word between w's spill and the ternary's temp. The
       permuter found an int return type. Through a cast pointer,
       `((u32 (*)(s32, s32)) floadGetSetFile)(n, 0)` (FAKE), both match.
     - tlb.h's prototype as u32 for every TLB file broke another file's code (first difference
       0x678807), so the game's TLB code is not uniform here. The FAKE stays per call.
376. **Stack order, measured together.** Locals, then block locals,
     then spill temps stack down from the frame top. A chain's reserved word sits above a
     spill; a used block local's word sits below it.
     - A spill one word off means one word moved across the temp, not one word too many.
     - Levers:
       - merge two locals used in different cases (kpa/trapFireballProc's n);
       - read a call's result into a local before the call that takes it (gasyaBombProc);
       - a block local for a lower word (utaCFireProc's scale; st2clearRoadTestProc's w per
         case).
378. **Levers, 2026-10-12:**
     - **An address constant built at every site** (kabuki's D_30007BC): spell it as a pointer
       at every use, call parameters included (FAKE casts where the prototype says u32). With
       every site pointer-typed, IDO builds it in a1 at each one, as the game does. Mixed
       spellings get it held in a register. A second form of the same thing: `(u32) D_x`
       inside a loop with calls is hoisted to a saved register as loop-invariant, while the
       uncast pointer is formed at the call (both opGate functions).
     - **One local reused across messages earns a saved register** (ntama): IDO counts its
       uses together, so eneSetup's result gets s0 as in the game. Split into separate
       variables, the result is spilled.
     - **A two-link chain spends its word in the low spill area** (hanaoya). Writing the chain
       through a new local declared last moves the word to the bottom of the locals.
     - **Also:**
       - An empty case in a jump-table switch costs no code (its word is the default's), but
         it changes register allocation: it kept o out of a0 at the case heads
         (deroKarasuEve1/2). In a compare-chain switch it adds code (shopYadoya: +6).
       - A pointer local set before the calls (`Player *p = PL(pl)`) is rebuilt after them as
         the game's `addiu v0,s0,0x20`. Written `PL(pl)->f` at each use, IDO folds the 0x20
         into each load (insectProc; note 354 applied to PL()).
       - Argument loads come ahead of the stack-argument stores only when they read the table
         in the call's own block (hanashot).
       - A test through a local (`dir = w->dir; if (dir == 4)`) removed a one-register shift
         that ran from that test to the function's end (wagaKasaProc, 136 to 18).
     - **Also:**
       - A same-file callee the game wrote as `void`: a dropped result keeps v0 busy around
         the call (369). Made void, the callee still matched and the caller went 64 to 6
         (utaCThunderCreate).
       - "Pad" words in drawing loops can be named constants: titleDrawImage4b's tile size as
         two int locals tw, th (64) matched it and removed its twin's two FAKE pads.
       - One more local word can move a spill down without growing the frame, when the frame
         has alignment slack.
       - v1 or s0 in the game where this C has a temp means the value crosses a block boundary
         there (a PRINTF between definition and use is a natural one).
     - **Also:**
       - The source line a value is defined on drives as1's order: utaGoeProc's `move a0,v0`
         followed pl's defining line. pl defined on the store-and-call line put the move after
         the store, as in the game.
       - A pointer the game keeps in one register across cases is one variable set at its
         declaration. Per-case assignments let uopt fold it into obj offsets in cases with no
         call (endPlCtlProc, effLensflareProc: two FAKEs removed).
       - A caller-saved register plus spills, where the game has a callee-saved one, can be one
         missing use across a call: an empty `if (p) {}` between the call and the next store
         emits nothing and keeps p live. After the store it leaves a beqz. `if (0)` and PRINTF
         uses don't count for liveness.
       - Trap: the Edit tool strips trailing spaces from new text.
     - **Also:**
       - Register choice follows each live range's priority. A counter shared by loops in
         different cases is one long range that loses its register. Give a loop its own counter
         (BIT_PA: 96 to 50). Moving a pointer's setup above a store loop finished it.
       - One local reused for two messages' call results frees a stack word (monoeyeTearProc).
         `i < 3; i++` with `i * 6` costs a temp word that `i < 18; i += 6` does not.
     - **Also:**
       - The float type word (375) is spent by a chain whose source is a field
         (`a = b = m->f`), a float call result passed directly, or one inside another call's
         argument. A chain of a constant spends nothing. Reading the chain through an existing
         local keeps its code with no word (monoeyeMain0: 8 bytes off the frame).
       - Spill area, top down: block locals, type words, spills. Values in "temp" slots just
         under the locals can be block locals of the if.
       - `a + b` order (probe addord.c):
         - call result + expression: the result first;
         - variable + expression: the expression first;
         - variable + variable: source order;
         - `(f32) var + expr`: the variable first.
       - An empty `if (w->field == NULL) {}` between two calls can move a pointer's register and
         emits nothing (shopRoomBGProc).
     - **Float queue offset**: when every float register from some point on is one off, look for
       one constant spelled differently far from the first difference (`0` vs `0.0f`,
       `25.0f` vs `25.00f`).
377. **`if (x <= 0) break;` vs `if (x > 0) { ... }` in a case**
     (func_080005D0_tlb_0325). With the early break, the exit reaches the code
     after the switch with the switch value still in its register. The wrapped form exits
     through a reload block.
379. **Levers measured 2026-10-16 (each proved first with `force`, ugen-trace
     or ugen-edit):**
     - **v0 blocked by a block-scoped copy** (dochuki): `T *g = w->arr[i];` inside the test's
       body. Its def and use sit in one block (high adjsave), so it is coloured first in v0, and
       as1 deletes its move. A range tied on cost then takes the next register (lowest number
       wins).
     - **Priority by empty tests:**
       - An empty `if (x) {} else {}` adds a use and lowers x's adjsave, so it is coloured later
         (gasyaBodyProc).
       - An empty never-true `if ((u32) &g + 1 >= 0) {}` adds uses and lifts a range
         (genki: &gGfxDlHead above three gfx `_g` locals).
       - Inside a later branch it gives variables a use after a point (gimIbodyStepSound: ex/ez
         kept out of f0).
     - **Spill-slot order** (gimIheadProc, ihandpointProc): f_spilltemps numbers saved
       expressions in creation order. A dead store of a call address before `w = FX(o)`
       (`dx = (s32) eneNewPol;`, into an existing local so the homes stay) makes that address
       slot 0.
     - **A count tested through a local bypasses ugen's FIFO** (func_08000520):
       `v = --w->n; if (v < 0)`. ugen reloads into the local's uopt register, not a list temp.
       as1 deletes the reload, and no temp is spent.
     - **as1's last tie-break is the source line** (Ktengu): among ready nodes with equal height
       and start time, the lower `.loc` line goes first (as1.c func_433668). Joining a statement
       onto the previous line (347's same-line lever) reorders a hoisted instruction.
     - **as1 pass 1 runs only after pass 0 finds stalls** (shopYadoya). A stall-free block keeps
       pass 0's height order. Only a block boundary changes it (a label in ugen's listing; C
       labels do not survive uopt).
     - **Float constants:** a constant one ulp off the game's word is written as the rodata's
       own value (0.05235988274f; gasyaBodyProc). `check` catches it after the code matches.
380. **A compiled-out debug print's TEST survives register allocation** (2026-10-17, Goemon
     (880)/(881); fileres.c).
     - `if (x < 0x8000) DPRINTF("%04d", x); else DPRINTF("%04x", x);`, written as fileLoadOverlay
       writes the game's own prints, emits no code: the empty arms are dropped. But uopt has
       already given x a use after the call, which puts it in its home across the call
       (`sw a0` at entry, `lhu` for the argument), and the 0x8000 constant has taken a register.
     - So the traces that several FAKEs imitated are exactly what a removed print leaves: an
       empty test, a dead read, an extra live range, `volatile` on a parameter (resDrop), a dead
       multu (resEndRegister).
     - Evidence rule: use it only where the file's own .rodata holds the print's strings. The
       strings prove the print was in the original, and a string never present is invented C.
     - This overrides the readings of 27 and 330: a parameter reloaded from its home with no
       call between needs no `volatile` when the original had a print there.
     - It follows that many kept "empty test" marks (fake_kept.txt) may be compiled-out prints.
       Where the file has the strings, the print is the more faithful form.

## splat
- **Compiled-out debug strings of an asm function are put in the PREVIOUS function's .s.**
  - splat ties a string to the function that loads it. A DPRINTF format nothing loads trails
    the last loaded string, so it goes to the earlier function's `.rodata`.
  - Moving only the earlier function to C then drops those strings. In tlb_0019_0, GDEMO in C
    put its floats at .rodata+0xB0 instead of +0x14C.
  - The function that prints them goes to C in the same step, its DPRINTFs where its steps
    change.
- **A leaf function whose switch cases each end in `jr ra` is cut into one "function" per case**
  (tlb_0018 `func_08001794`, 11 cases; overlay 9's `func_801981D0`/`D8` in 166). With its
  .rodata migrated, the jump table travels with the head piece and asm-processor refuses it
  ("late rodata to text ratio is too high"). Its true size in symbol_addrs.txt
  (`// type:func size:0xF0 segment:tlb_0018`) keeps it one function.
- **A jump table left in an asm .rodata file cannot reach `.L` case labels in a C file's
  GLOBAL_ASM** (`.L` is local to its object even with `jlabel`): migrate the .rodata into the
  functions (`.rodata` with the C file's name) or keep the whole file asm.
- **spimdisasm, asked for an address outside an overlay's own segment, takes the first symbol
  ANY other overlay has there** (ElementBase.getSymbol), unless the segment names
  `prioritized_segments`: the TLB files reference the shared slot 0x801738A0, which overlays
  8, 9 and 10 all occupy; each TLB segment names the overlay it runs with.
- **More nops after a file's last function than IDO's 16-byte alignment makes need a `pad`
  subsegment** (`- [0xROM, pad]`, splat writes `. += size;`). menu.c's text ends at
  0x5A6128 with 0x18 bytes of nops before 0x5A6140; the object pads to 0x5A6130. Without
  the pad the next piece lands 0x10 early and the first difference shows up in main, where
  code points into the overlay.
- **Nops between two functions inside one c subsegment are an object boundary.** IDO puts no
  padding between the functions of one object. When nops follow a function's `jr ra` slot up
  to a 16-aligned function, a new object starts there.
  - Example: tlb_0018's `func_08003434` is 5 instructions and 2 nops, and the next function is
    at 0x08003450. Its correct C moved that next function up 8 bytes.
  - `nisitenma_archive.py pad-cuts` finds these from IDA's functions (this and the pad case
    above), and `recut` cuts them. Ten were missing in the TLB files, five of
    them in file 322 (one object per staff-roll builder).
- **A function nothing calls, right after another, is folded into it.** `func_80003C94`'s asm
  ended with a second `jr ra; nop`: an empty function at `0x80003D24`. A `symbol_addrs.txt`
  entry at that address (even with `size:`, even under a non-`func_` name) did NOT make splat
  0.50.0 split it. The fix is in C: write the empty function after the other one, and the
  full build matches. **If a function's asm ends in an extra `jr ra; nop` that no source shape
  reproduces, it is an empty function.**
