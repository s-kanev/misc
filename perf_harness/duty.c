#define HIGH_RATE (1ULL << 9)
#define LOW_RATE 3200
//#define LOW_RATE (1ULL << 8)

void duty(unsigned long iter)
{
    unsigned long long i,j;
    for(j=0; j<iter; j++) {
    // 16* insts
        for (i=0; i < HIGH_RATE; i++) {
            asm volatile ("fmull	-24(%%ebp)\n\
                            mov     -16(%%ebp), %%eax\n\
                            fadd    %%st(0), %%st(1)\n\
                            faddl	-16(%%ebp)\n\
                            fadd    %%st(0), %%st(2)\n\
                            fmull	-24(%%ebp)\n\
                            mov     -16(%%ebp), %%ebx\n\
                            fadd    %%st(0), %%st(1)\n\
                            faddl	-16(%%ebp)\n\
                            fadd    %%st(0), %%st(2)":::"eax","ebx");
            asm volatile ("fmull	-24(%%ebp)\n\
                            mov     -16(%%ebp), %%eax\n\
                            fadd    %%st(0), %%st(1)\n\
                            faddl	-16(%%ebp)\n\
                            fadd    %%st(0), %%st(2)\n\
                            fmull	-24(%%ebp)\n\
                            mov     -16(%%ebp), %%ebx\n\
                            fadd    %%st(0), %%st(1)\n\
                            faddl	-16(%%ebp)\n\
                            fadd    %%st(0), %%st(2)":::"eax","ebx");
            asm volatile ("fmull	-24(%%ebp)\n\
                            mov     -16(%%ebp), %%eax\n\
                            fadd    %%st(0), %%st(1)\n\
                            faddl	-16(%%ebp)\n\
                            fadd    %%st(0), %%st(2)\n\
                            fmull	-24(%%ebp)\n\
                            mov     -16(%%ebp), %%ebx\n\
                            fadd    %%st(0), %%st(1)\n\
                            faddl	-16(%%ebp)\n\
                            fadd    %%st(0), %%st(2)":::"eax","ebx");
            asm volatile ("fmull	-24(%%ebp)\n\
                            mov     -16(%%ebp), %%eax\n\
                            fadd    %%st(0), %%st(1)\n\
                            faddl	-16(%%ebp)\n\
                            fadd    %%st(0), %%st(2)\n\
                            fmull	-24(%%ebp)\n\
                            mov     -16(%%ebp), %%ebx\n\
                            fadd    %%st(0), %%st(1)\n\
                            faddl	-16(%%ebp)\n\
                            fadd    %%st(0), %%st(2)":::"eax","ebx");
        }
        usleep(LOW_RATE);
        // 16*64 nops
/*        for (i=0; i < LOW_RATE; i++) {
            asm volatile ("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;":::);
            asm volatile ("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;":::);
            asm volatile ("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;":::);
            asm volatile ("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;":::);
            asm volatile ("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;":::);
            asm volatile ("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;":::);
            asm volatile ("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;":::);
            asm volatile ("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;":::);
        }*/
    }
}
