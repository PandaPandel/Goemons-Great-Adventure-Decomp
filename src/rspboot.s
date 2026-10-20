/* rspboot, libultra's RSP boot microcode (0xD0 bytes, the task's ucode_boot: audio.c and dma.c
 * hand it to every RSP task). The RSP runs it from IMEM 0x04001000: it loads the task's ucode
 * data into DMEM and its ucode into IMEM 0x1080 by DMA, then jumps there. spimdisasm read its
 * first 0x50 bytes as two hand-written CPU functions (rspbootTextStart/78: "addi", the SP
 * registers as cop0 moves) and the rest as data. It is RSP assembly in the original; kept as
 * source here, as sm64 keeps rsp/rspboot.s (this is that file without its JP/US "ori $1, $1, 1").
 * All of it is scalar code, so GNU as builds it. Its jumps name IMEM addresses (it runs at
 * 0x04001000), written out: GNU as cannot take "0x04001000 + (label - start)" in a j/jal. */

.include "macro.inc"

.set noat
.set noreorder
.set gp=64

/* the OSTask in DMEM, and its fields */
.set OSTask_addr, 0x0FC0
.set OSTask_flags, 0x04
.set OSTask_ucode, 0x10
.set OSTask_ucode_data, 0x18
.set OSTask_ucode_data_size, 0x1C
.set OS_TASK_DP_WAIT, 0x0002
.set DPC_STATUS_DMA_BUSY, 0x0100

/* the RSP's coprocessor-0 registers, by number: $0 SP_MEM_ADDR, $1 SP_DRAM_ADDR, $2 SP_RD_LEN,
 * $4 SP_STATUS, $5 SP_DMA_FULL, $6 SP_DMA_BUSY, $7 SP_SEMAPHORE, $11 DPC_STATUS */

.section .text, "ax"

glabel rspbootTextStart
    j       0x04001064              /* boot_check_flags */
     addi   $1, $zero, OSTask_addr
boot_load_ucode:
    lw      $2, OSTask_ucode($1)
    addi    $3, $zero, 0x0F7F       /* length 0xF80 */
    addi    $7, $zero, 0x1080       /* to IMEM 0x1080 */
    mtc0    $7, $0                  /* SP_MEM_ADDR */
    mtc0    $2, $1                  /* SP_DRAM_ADDR */
    mtc0    $3, $2                  /* SP_RD_LEN */
boot_ucode_dma_busy:
    mfc0    $4, $6                  /* SP_DMA_BUSY */
    bne     $4, $zero, boot_ucode_dma_busy
     nop
    jal     0x0400103C              /* check_yielded */
     nop
    jr      $7                      /* into the loaded ucode */
     mtc0   $zero, $7               /* SP_SEMAPHORE */

check_yielded:
    mfc0    $8, $4                  /* SP_STATUS */
    andi    $8, $8, 0x0080          /* the yield signal */
    bne     $8, $zero, boot_yielded
     nop
    jr      $31
boot_yielded:
     mtc0   $zero, $7               /* SP_SEMAPHORE */
    ori     $8, $zero, 0x5200       /* clear yield, set yielded and task done */
    mtc0    $8, $4                  /* SP_STATUS */
    break                           /* halt the RSP */
    nop

boot_check_flags:
    lw      $2, OSTask_flags($1)
    andi    $2, $2, OS_TASK_DP_WAIT
    beq     $2, $zero, boot_load_data
     nop
    jal     0x0400103C              /* check_yielded */
     nop
    mfc0    $2, $11                 /* DPC_STATUS */
    andi    $2, $2, DPC_STATUS_DMA_BUSY
    bgtz    $2, check_yielded
     nop
boot_load_data:
    lw      $2, OSTask_ucode_data($1)
    lw      $3, OSTask_ucode_data_size($1)
    addi    $3, $3, -1
boot_dma_not_full:
    mfc0    $30, $5                 /* SP_DMA_FULL */
    bne     $30, $zero, boot_dma_not_full
     nop
    mtc0    $zero, $0               /* SP_MEM_ADDR: the ucode data at DMEM 0 */
    mtc0    $2, $1                  /* SP_DRAM_ADDR */
    mtc0    $3, $2                  /* SP_RD_LEN */
boot_data_dma_busy:
    mfc0    $4, $6                  /* SP_DMA_BUSY */
    bne     $4, $zero, boot_data_dma_busy
     nop
    jal     0x0400103C              /* check_yielded */
     nop
    j       0x04001008              /* boot_load_ucode */
     nop
    nop
endlabel rspbootTextStart

.globl rspbootTextEnd
rspbootTextEnd:
