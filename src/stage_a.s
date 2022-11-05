    .section .bss.stage

/* Outer Wall is the largest stage file at 18008 bytes */
/* 18008 - 4 (header) - 4 (size) - 76 (error vars) = 17924 */
    .globl stagePXM
stagePXM:    ds.b 8
    .globl stageBlocks
stageBlocks: ds.b 17924
