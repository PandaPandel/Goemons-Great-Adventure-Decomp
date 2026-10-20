/* aspMain, Nintendo's audio microcode (the one alAudioFrame command lists are written for). It is
 * the SDK's own binary object (PR/aspMain.o), not the game's code, so it is kept as named
 * binaries cut from the ROM by splat, as fzerox keeps it (src/rsp/aspmain.s). sm64 builds the
 * same microcode from RSP source (rsp/audio.s, with armips): this DMEM image equals the data
 * that file writes for JP/US/EU word for word (336 halfwords) except the 16-entry command table
 * at 0x10, which holds IMEM addresses of the command handlers in this text. This text was not
 * compared with that source.
 * Its two halves are far apart in main, as the original link put them:
 *   .text   0x800258D0, 0xE20 bytes: the IMEM image, after F3DEX2 (src/rsp/gspF3DEX2_fifo.s).
 *   .rodata 0x8003EA50, 0x2C0 bytes: the DMEM image, after F3DEX2 data; main ends with it.
 * audio.c gives both to each audio task (ucode, ucode_data). The names are the SDK ones
 * (lib/ultralib/include/PR/ucode.h). The .bin files are written by splat (assets/ is not in git):
 * the yaml lines are textbin and rodatabin entries named rsp/aspMain. */

.include "macro.inc"

.section .text, "ax"

glabel aspMainTextStart
.incbin "assets/rsp/aspMain.textbin.bin"
endlabel aspMainTextStart

.globl aspMainTextEnd
aspMainTextEnd:

.section .rodata, "a"

dlabel aspMainDataStart
.incbin "assets/rsp/aspMain.rodatabin.bin"
enddlabel aspMainDataStart

.globl aspMainDataEnd
aspMainDataEnd:
