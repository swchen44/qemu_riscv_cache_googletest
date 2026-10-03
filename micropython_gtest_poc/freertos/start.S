.section .text.init
.global _start
_start:
    .option push
    .option norelax
    la gp, __global_pointer$
    .option pop
    la sp, __stack_top
    la t0, __bss_start
    la t1, __bss_end
1:  bgeu t0, t1, 2f
    sw zero, 0(t0)
    addi t0, t0, 4
    j 1b
2:
#ifdef USE_FREERTOS
    la t0, freertos_risc_v_trap_handler
    csrw mtvec, t0
#endif
    call __libc_init_array
    call demo_main
    call _exit
3:  wfi
    j 3b
