    .section .bss.stage

/* Outer Wall is the largest stage file at 18008 bytes */
/* 18008 - 4 (header) - 4 (size) - 74 (error vars) = 17926 */
    .globl stagePXM
stagePXM:    ds.b 8
    .globl stageBlocks
stageBlocks: ds.b 15766 /* 15848 - 8 - 74, old size 17926 */
