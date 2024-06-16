#include <bitops.h>
#include <env.h>
#include <pmap.h>
#include <syscall.h>

/* Lab 2 Key Code "tlb_invalidate" */
/* Overview:
 *   Invalidate the TLB entry with specified 'asid' and virtual address 'va'.
 *
 * Hint:
 *   Construct a new Entry HI and call 'tlb_out' to flush TLB.
 *   'tlb_out' is defined in mm/tlb_asm.S
 */
void tlb_invalidate(u_int asid, u_long va) {
	tlb_out((va & ~GENMASK(PGSHIFT, 0)) | (asid & (NASID - 1)));
}
/* End of Key Code "tlb_invalidate" */

static void passive_alloc(u_int va, Pde *pgdir, u_int asid) {
	struct Page *p = NULL;

	if (va < UTEMP) {
		//panic("curenv: %x, address too low: %x",curenv->env_id,va);
		sys_kill(0, SIGSEGV);
	}

	if (va >= USTACKTOP && va < USTACKTOP + PAGE_SIZE) {
		panic("invalid memory, maybe stack overflow");
	}

	if (va >= UENVS && va < UPAGES) {
		panic("envs zone");
	}

	if (va >= UPAGES && va < UVPT) {
		panic("pages zone");
	}

	if (va >= ULIM) {
		panic("kernel address");
	}

	panic_on(page_alloc(&p));
	panic_on(page_insert(pgdir, asid, p, PTE_ADDR(va), (va >= UVPT && va < ULIM) ? 0 : PTE_D));
}

/* Overview:
 *  Refill TLB.
 */
void _do_tlb_refill(u_long *pentrylo, u_int va, u_int asid) {
	tlb_invalidate(asid, va);
	Pte *ppte;
	/* Hints:
	 *  Invoke 'page_lookup' repeatedly in a loop to find the page table entry '*ppte'
	 * associated with the virtual address 'va' in the current address space 'cur_pgdir'.
	 *
	 *  **While** 'page_lookup' returns 'NULL', indicating that the '*ppte' could not be found,
	 *  allocate a new page using 'passive_alloc' until 'page_lookup' succeeds.
	 */

	/* Exercise 2.9: Your code here. */
	while(page_lookup(cur_pgdir, va, &ppte) == NULL){
		passive_alloc(va, cur_pgdir, asid);
	}
	ppte = (Pte *)((u_long)ppte & ~0x7);
	pentrylo[0] = ppte[0] >> 6;
	pentrylo[1] = ppte[1] >> 6;
}

#if !defined(LAB) || LAB >= 4
/* Overview:
 *   This is the TLB Mod exception handler in kernel.
 *   Our kernel allows user programs to handle TLB Mod exception in user mode, so we copy its
 *   context 'tf' into UXSTACK and modify the EPC to the registered user exception entry.
 *
 * Hints:
 *   'env_user_tlb_mod_entry' is the user space entry registered using
 *   'sys_set_user_tlb_mod_entry'.
 *
 *   The user entry should handle this TLB Mod exception and restore the context.
 */
void do_tlb_mod(struct Trapframe *tf) {
	struct Trapframe tmp_tf = *tf;

	if (tf->regs[29] < USTACKTOP || tf->regs[29] >= UXSTACKTOP) {
		tf->regs[29] = UXSTACKTOP;
	}
	tf->regs[29] -= sizeof(struct Trapframe);
	*(struct Trapframe *)tf->regs[29] = tmp_tf;
	Pte *pte;
	page_lookup(cur_pgdir, tf->cp0_badvaddr, &pte);
	if (curenv->env_user_tlb_mod_entry) {
		tf->regs[4] = tf->regs[29];
		tf->regs[29] -= sizeof(tf->regs[4]);
		// Hint: Set 'cp0_epc' in the context 'tf' to 'curenv->env_user_tlb_mod_entry'.
		/* Exercise 4.11: Your code here. */
		tf->cp0_epc = curenv->env_user_tlb_mod_entry;
	} else {
		panic("TLB Mod but no user handler registered");
	}
}

#endif

void do_signal(struct Trapframe *tf){
	u_int signums = curenv->env_sig_pending.sig & ~(curenv->env_sigset.sig);
	if(signums == 0){
		return;
	}
    u_int sig;
    if(curenv->env_sig_pending.sig & (1 << (SIGKILL - 1))){
        //sig = SIGKILL;
        curenv->env_sig_pending.sig &= ~(1 << (SIGKILL - 1));
		printk("[%08x] destroying %08x\n", curenv->env_id, curenv->env_id);
		env_destroy(curenv);
    }

        
	for(sig = SIG_MIN; sig <= SIG_MAX; sig++){
		if(signums & (1 << (sig - 1))){
			break;
		}
	}
    
    if(sig > SIG_MAX){
        return;
    }
    
	//printk("%x recv %d, pending %x, handler %x\n", curenv->env_id, sig,curenv->env_sig_pending, curenv->env_sigaction[sig - 1].sa_handler);
	struct Env* env;
	struct sigaction sa = curenv->env_sigaction[sig - 1];
	if (sa.sa_handler == NULL) { // 默认处理
		//printk("[%08x] my handler to signr: %02d is DFL\n",curenv->env_id, signr);
		curenv->env_sig_pending.sig &= ~(1 << (sig - 1)); //恢复pending
		switch(sig) {
			case SIGINT: case SIGILL: case SIGSEGV:	
				if (envid2env(0, &env, 1) < 0) {
					return;
				}
				printk("[%08x] destroying %08x\n", curenv->env_id, curenv->env_id);
				env_destroy(env);
				return;                                   //终止进程 
			default:
				tf->cp0_epc += 4;
				return;                                  //忽略信号
		}
	}
    curenv->env_sig_pending.sig &= ~(1 << (sig - 1));
	u_int old_mask = curenv->env_sigset.sig;
	curenv->env_sigset.sig = old_mask | curenv->env_sigaction[sig - 1].sa_mask.sig | (1 << (sig - 1));
	curenv->env_sig_top++;
	curenv->env_sig_stack[curenv->env_sig_top] = sig;
	curenv->env_sig_mask_stack[curenv->env_sig_top] = curenv->env_sigset.sig;

    
	if (curenv->env_user_sig_entry) {
		struct Trapframe tmp_tf = *tf;
		if (tf->regs[29] < USTACKTOP || tf->regs[29] >= UXSTACKTOP) {
			tf->regs[29] = UXSTACKTOP;
		}
		tf->regs[29] -= sizeof(struct Trapframe);
		tf->regs[4] = tf->regs[29];
		tf->regs[5] = sig;
		tf->regs[6] = curenv->env_sigaction[sig - 1].sa_handler;
        tf->regs[7] = old_mask;
		*(struct Trapframe *)tf->regs[29] = tmp_tf; 

		tf->regs[29] -= sizeof(tf->regs[7]);
		tf->regs[29] -= sizeof(tf->regs[6]);
		tf->regs[29] -= sizeof(tf->regs[5]);
        tf->regs[29] -= sizeof(tf->regs[4]);

		tf->cp0_epc = curenv->env_user_sig_entry;
	}
    else{
        panic("no handler entry for %x\n", curenv->env_id);
    }
    return;
}

