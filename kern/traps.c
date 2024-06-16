#include <env.h>
#include <pmap.h>
#include <printk.h>
#include <trap.h>
#include<printk.h>

extern void handle_int(void);
extern void handle_tlb(void);
extern void handle_sys(void);
extern void handle_mod(void);
extern void handle_reserved(void);
extern void handle_ill(void);

void (*exception_handlers[32])(void) = {
    [0 ... 31] = handle_reserved,
    [0] = handle_int,
    [2 ... 3] = handle_tlb,
#if !defined(LAB) || LAB >= 4
    [1] = handle_mod,
    [8] = handle_sys,
#endif
    //[10] = handle_ill,
};

/* Overview:
 *   The fallback handler when an unknown exception code is encountered.
 *   'genex.S' wraps this function in 'handle_reserved'.
 */
void do_reserved(struct Trapframe *tf) {
	//print_tf(tf);
	//panic("Unknown ExcCode %2d", (tf->cp0_cause >> 2) & 0x1f);
    sys_kill(0, SIGILL);
}

void do_ill(struct Trapframe *tf){
    sys_kill(0, SIGILL);
    return;
}
