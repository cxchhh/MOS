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
    [10] = handle_ill,
#endif
};

/* Overview:
 *   The fallback handler when an unknown exception code is encountered.
 *   'genex.S' wraps this function in 'handle_reserved'.
 */
void do_reserved(struct Trapframe *tf) {
	print_tf(tf);
	panic("Unknown ExcCode %2d", (tf->cp0_cause >> 2) & 0x1f);
}

void do_ill(struct Trapframe *tf){
    u_int sig = SIGILL;
    if(curenv->env_sigset.sig & (1 << (sig - 1))){
		curenv->env_sigpending.sig |= (1 << (sig - 1));
		return 0;
	}

	curenv->env_sigrecv |= (1 << (sig - 1));
	curenv->env_sigset.sig |= curenv->env_sigaction[sig - 1].sa_mask.sig;
    return;
}


void do_signal(struct Trapframe *tf){
    
    u_int sig;
    for(sig = SIG_MIN; sig <= SIG_MAX; sig++){
        if(curenv->env_sigrecv & (1 << (sig - 1))){
            break;
        }
    }
    
    if(sig > SIG_MAX){
        return;
    }
    if(curenv->env_sig_flag && !(sig == SIGSEGV && curenv->env_sig_flag != SIGSEGV)){
        return;
    }
    curenv->env_sig_flag = sig;
    //printk("%x recv %d %x\n", curenv->env_id, sig, curenv->env_user_sig_entry);
    // struct Trapframe tmp_tf = *tf;
	// if (tf->regs[29] < USTACKTOP || tf->regs[29] >= UXSTACKTOP) {
	// 	tf->regs[29] = UXSTACKTOP; // 将栈指针指向用户异常处理栈
	// }
	// tf->regs[29] -= sizeof(struct Trapframe); // 将当前的 Trapframe 压入异常处理栈
	// *(struct Trapframe *)tf->regs[29] = tmp_tf;
	
	if (curenv->env_user_sig_entry) {
		tf->regs[4] = curenv->env_id;
		tf->regs[5] = sig;
		tf->regs[6] = curenv->env_sigaction[sig - 1].sa_handler;
		tf->regs[7] = tf->regs[29];
		tf->regs[29] -= sizeof(tf->regs[4]);
		tf->regs[29] -= sizeof(tf->regs[5]);
		tf->regs[29] -= sizeof(tf->regs[6]);
		tf->regs[29] -= sizeof(tf->regs[7]);
        
		tf->cp0_epc = curenv->env_user_sig_entry;
	}
    else if(sig == SIGINT || sig == SIGILL || sig == SIGSEGV){
        env_destroy(curenv);
		return;
    }

    return;
}