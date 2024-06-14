#include <env.h>
#include <lib.h>
#include <mmu.h>

/* Overview:
 *   Map the faulting page to a private writable copy.
 *
 * Pre-Condition:
 * 	'va' is the address which led to the TLB Mod exception.
 *
 * Post-Condition:
 *  - Launch a 'user_panic' if 'va' is not a copy-on-write page.
 *  - Otherwise, this handler should map a private writable copy of
 *    the faulting page at the same address.
 */
static void __attribute__((noreturn)) cow_entry(struct Trapframe *tf) {
	u_int va = tf->cp0_badvaddr;
	u_int perm;

	/* Step 1: Find the 'perm' in which the faulting address 'va' is mapped. */
	/* Hint: Use 'vpt' and 'VPN' to find the page table entry. If the 'perm' doesn't have
	 * 'PTE_COW', launch a 'user_panic'. */
	/* Exercise 4.13: Your code here. (1/6) */
	perm = PTE_FLAGS(vpt[VPN(va)]);
	if(!(perm & PTE_COW)){
		user_panic("No PTE_COW for page %x",va);
	}
	/* Step 2: Remove 'PTE_COW' from the 'perm', and add 'PTE_D' to it. */
	/* Exercise 4.13: Your code here. (2/6) */
	perm &= ~PTE_COW;
	perm |= PTE_D;
	/* Step 3: Allocate a new page at 'UCOW'. */
	/* Exercise 4.13: Your code here. (3/6) */
	try(syscall_mem_alloc(0, UCOW, perm));
	/* Step 4: Copy the content of the faulting page at 'va' to 'UCOW'. */
	/* Hint: 'va' may not be aligned to a page! */
	/* Exercise 4.13: Your code here. (4/6) */
	memcpy(UCOW, ROUNDDOWN(va, PAGE_SIZE), PAGE_SIZE);
	// Step 5: Map the page at 'UCOW' to 'va' with the new 'perm'.
	/* Exercise 4.13: Your code here. (5/6) */
	try(syscall_mem_map(0, UCOW, 0, ROUNDDOWN(va, PAGE_SIZE), perm));
	// Step 6: Unmap the page at 'UCOW'.
	/* Exercise 4.13: Your code here. (6/6) */
	try(syscall_mem_unmap(0, UCOW));
	// Step 7: Return to the faulting routine.
	int r = syscall_set_trapframe(0, tf);
	user_panic("syscall_set_trapframe returned %d", r);
}

/* Overview:
 *   Grant our child 'envid' access to the virtual page 'vpn' (with address 'vpn' * 'PAGE_SIZE') in
 * our (current env's) address space. 'PTE_COW' should be used to isolate the modifications on
 * unshared memory from a parent and its children.
 *
 * Post-Condition:
 *   If the virtual page 'vpn' has 'PTE_D' and doesn't has 'PTE_LIBRARY', both our original virtual
 *   page and 'envid''s newly-mapped virtual page should be marked 'PTE_COW' and without 'PTE_D',
 *   while the other permission bits are kept.
 *
 *   If not, the newly-mapped virtual page in 'envid' should have the exact same permission as our
 *   original virtual page.
 *
 * Hint:
 *   - 'PTE_LIBRARY' indicates that the page should be shared among a parent and its children.
 *   - A page with 'PTE_LIBRARY' may have 'PTE_D' at the same time, you should handle it correctly.
 *   - You can pass '0' as an 'envid' in arguments of 'syscall_*' to indicate current env because
 *     kernel 'envid2env' converts '0' to 'curenv').
 *   - You should use 'syscall_mem_map', the user space wrapper around 'msyscall' to invoke
 *     'sys_mem_map' in kernel.
 */
static void duppage(u_int envid, u_int vpn) {
	int r;
	u_int addr;
	u_int perm;
	/* Step 1: Get the permission of the page. */
	/* Hint: Use 'vpt' to find the page table entry. */
	/* Exercise 4.10: Your code here. (1/2) */
	Pte *pte = &vpt[vpn];
	perm = PTE_FLAGS(*pte);
	/* Step 2: If the page is writable, and not shared with children, and not marked as COW yet,
	 * then map it as copy-on-write, both in the parent (0) and the child (envid). */
	/* Hint: The page should be first mapped to the child before remapped in the parent. (Why?)
	 */
	/* Exercise 4.10: Your code here. (2/2) */
	if((perm & PTE_D) && !(perm & PTE_COW) && !(perm & PTE_LIBRARY)){
		perm &= ~PTE_D;
		perm |= PTE_COW;
		syscall_mem_map(0, (vpn * PAGE_SIZE), envid, (vpn * PAGE_SIZE), perm);
		syscall_mem_map(0, (vpn * PAGE_SIZE), 0, (vpn * PAGE_SIZE), perm);
	}
	else{
		syscall_mem_map(0, (vpn * PAGE_SIZE), envid, (vpn * PAGE_SIZE), perm);
	}
}

static void __attribute__((noreturn)) sig_entry(struct Trapframe *tf, u_int signum, void (*handler)(int), u_int mask) {
	//debugf("%x %d in sig_entry! %x\n", env->env_id, signum, handler);
	int r;
	sigset_t sa_mask;
	sigset_t old_mask;
	sa_mask.sig = mask;
	if(handler != NULL){
		sa_mask.sig |= (1 << (signum - 1));
		try(syscall_sigprocmask(SIG_BLOCK, &sa_mask, &old_mask));
		try(syscall_set_sig_flag(0, 2));
		handler(signum);
		try(syscall_sigprocmask(SIG_SETMASK, &old_mask, NULL));
	}
	else if(signum == SIGKILL || signum == SIGINT || signum == SIGILL || signum == SIGSEGV){
        exit();
		return;
    }
	else if (signum == SIGSYS) {
		try(syscall_set_sig_flag(0, 2));
		tf->cp0_epc += 4;
	}
	r = syscall_set_trapframe(0, tf);
	user_panic("syscall_set_trapframe returned %d", r);
}

int env_set_sig_entry(){
	return syscall_set_sig_entry(0, sig_entry);
}

/* Overview:
 *   User-level 'fork'. Create a child and then copy our address space.
 *   Set up ours and its TLB Mod user exception entry to 'cow_entry'.
 *
 * Post-Conditon:
 *   Child's 'env' is properly set.
 *
 * Hint:
 *   Use global symbols 'env', 'vpt' and 'vpd'.
 *   Use 'syscall_set_tlb_mod_entry', 'syscall_getenvid', 'syscall_exofork',  and 'duppage'.
 */
int fork(void) {
	u_int child;
	u_int i;

	/* Step 1: Set our TLB Mod user exception entry to 'cow_entry' if not done yet. */
	if (env->env_user_tlb_mod_entry != (u_int)cow_entry) {
		try(syscall_set_tlb_mod_entry(0, cow_entry));
	}

	if (env->env_user_sig_entry != (u_int)sig_entry) {
		try(syscall_set_sig_entry(0, sig_entry));
	}

	/* Step 2: Create a child env that's not ready to be scheduled. */
	// Hint: 'env' should always point to the current env itself, so we should fix it to the
	// correct value.
	child = syscall_exofork();
	if (child == 0) {
		env = envs + ENVX(syscall_getenvid());
		return 0;
	}
	/* Step 3: Map all mapped pages below 'USTACKTOP' into the child's address space. */
	// Hint: You should use 'duppage'.
	/* Exercise 4.15: Your code here. (1/2) */
	for(u_long va = 0; va < USTACKTOP; va += PAGE_SIZE){
		if ((vpd[PDX(va)] & PTE_V) && (vpt[VPN(va)] & PTE_V)) {
        	duppage(child, VPN(va));
    	}
	}
	/* Step 4: Set up the child's tlb mod handler and set child's 'env_status' to
	 * 'ENV_RUNNABLE'. */
	/* Hint:
	 *   You may use 'syscall_set_tlb_mod_entry' and 'syscall_set_env_status'
	 *   Child's TLB Mod user exception entry should handle COW, so set it to 'cow_entry'
	 */
	/* Exercise 4.15: Your code here. (2/2) */
	try(syscall_set_tlb_mod_entry(child, cow_entry));
	try(syscall_set_sig_entry(child, sig_entry));
	try(syscall_set_env_status(child, ENV_RUNNABLE));
	return child;
}
