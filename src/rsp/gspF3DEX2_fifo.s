/* gspF3DEX2_fifo, Nintendo's F3DEX2 graphics microcode, FIFO build, version 2.04 (its data says
 * "RSP Gfx ucode F3DEX       fifo 2.04  Yoshitaka Yasumoto 1998 Nintendo."). It is the SDK's own
 * binary object (PR/gspF3DEX2.fifo.o), not the game's code, so it is kept as the decomps before
 * this one keep their graphics microcode: named binaries cut from the ROM by splat (sm64
 * lib/PR/f3dex2/F3DEX2.bin and F3DEX2_data.bin, fzerox src/rsp/f3dex2.s).
 * Its two halves are far apart in main, as the original link put them:
 *   .text   0x80024540, 0x1390 bytes: the IMEM image, right after rspboot (src/rspboot.s).
 *   .rodata 0x8003E630, 0x420 bytes: the DMEM image, the last of main, after zlib.
 * dma.c gives both to each of its three graphics tasks (ucode, ucode_data). The names are the
 * SDK ones (lib/ultralib/include/PR/ucode.h). The .bin files are written by splat (assets/ is
 * not in git): the yaml lines are textbin and rodatabin entries named rsp/gspF3DEX2_fifo. */

.include "macro.inc"

.section .text, "ax"

glabel gspF3DEX2_fifoTextStart
.incbin "assets/rsp/gspF3DEX2_fifo.textbin.bin"
endlabel gspF3DEX2_fifoTextStart

.globl gspF3DEX2_fifoTextEnd
gspF3DEX2_fifoTextEnd:

.section .rodata, "a"

dlabel gspF3DEX2_fifoDataStart
.incbin "assets/rsp/gspF3DEX2_fifo.rodatabin.bin"
enddlabel gspF3DEX2_fifoDataStart

.globl gspF3DEX2_fifoDataEnd
gspF3DEX2_fifoDataEnd:
