/* The game's entry point (ROM 0x1000, run at 0x80000400 by the boot code): clears main's BSS,
 * 8 bytes at a time, sets the stack and jumps to main. Hand-written assembly in the original
 * (its "addi" is an instruction IDO never emits, and C cannot set sp), so it is kept as source
 * here, as sm64 keeps its asm/entry.s. */

.include "macro.inc"

.set noat
.set noreorder
.set gp=64

.section .text, "ax"

glabel entrypoint
    lui     $t0, %hi(main_BSS_START)
    lui     $t1, (0x88600 >> 16)            /* main's BSS size: 0x88600 bytes */
    addiu   $t0, $t0, %lo(main_BSS_START)
    ori     $t1, $t1, (0x88600 & 0xFFFF)
.Lclear:
    addi    $t1, $t1, -8
    sw      $zero, 0($t0)
    sw      $zero, 4($t0)
    bnez    $t1, .Lclear
     addi   $t0, $t0, 8
    lui     $t2, %hi(main)
    lui     $sp, %hi(gIdleThread)       /* the boot stack's top */
    addiu   $t2, $t2, %lo(main)
    jr      $t2
     addiu  $sp, $sp, %lo(gIdleThread)
endlabel entrypoint

    /* zeros to the end of the entry segment (0x60 bytes) */
    .space 0x28
