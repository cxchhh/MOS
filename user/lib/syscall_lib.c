#include <env.h>
#include <lib.h>
#include <mmu.h>
#include <syscall.h>
#include <trap.h>

void syscall_putchar(int ch) {
	msyscall(SYS_putchar, ch);
}

int syscall_print_cons(const void *str, u_int num) {
	return msyscall(SYS_print_cons, str, num);
}

u_int syscall_getenvid(void) {
	return msyscall(SYS_getenvid);
}

void syscall_yield(void) {
	msyscall(SYS_yield);
}

int syscall_env_destroy(u_int envid) {
	return msyscall(SYS_env_destroy, envid);
}

int syscall_set_tlb_mod_entry(u_int envid, void (*func)(struct Trapframe *)) {
	return msyscall(SYS_set_tlb_mod_entry, envid, func);
}

int syscall_mem_alloc(u_int envid, void *va, u_int perm) {
	return msyscall(SYS_mem_alloc, envid, va, perm);
}

int syscall_mem_map(u_int srcid, void *srcva, u_int dstid, void *dstva, u_int perm) {
	return msyscall(SYS_mem_map, srcid, srcva, dstid, dstva, perm);
}

int syscall_mem_unmap(u_int envid, void *va) {
	return msyscall(SYS_mem_unmap, envid, va);
}

int syscall_set_env_status(u_int envid, u_int status) {
	return msyscall(SYS_set_env_status, envid, status);
}

int syscall_set_trapframe(u_int envid, struct Trapframe *tf) {
	return msyscall(SYS_set_trapframe, envid, tf);
}

void syscall_panic(const char *msg) {
	int r = msyscall(SYS_panic, msg);
	user_panic("SYS_panic returned %d", r);
}

int syscall_ipc_try_send(u_int envid, u_int value, const void *srcva, u_int perm) {
	return msyscall(SYS_ipc_try_send, envid, value, srcva, perm);
}

int syscall_ipc_recv(void *dstva) {
	return msyscall(SYS_ipc_recv, dstva);
}

int syscall_cgetc() {
	return msyscall(SYS_cgetc);
}

int syscall_write_dev(void *va, u_int dev, u_int size) {
	/* Exercise 5.2: Your code here. (1/2) */
	return msyscall(SYS_write_dev, va, dev, size);
}

int syscall_read_dev(void *va, u_int dev, u_int size) {
	/* Exercise 5.2: Your code here. (2/2) */
	return msyscall(SYS_read_dev, va, dev, size);
}

int syscall_sigprocmask(int __how, const sigset_t * __set, sigset_t * __oset) {
	return msyscall(SYS_sigprocmask, __how, __set, __oset);
}

int syscall_sigaction(int signum, const struct sigaction *newact, struct sigaction *oldact){
	return msyscall(SYS_sigaction, signum, newact, oldact);
}

int syscall_kill(u_int envid, int sig){
	return msyscall(SYS_kill, envid, sig);
}

int syscall_finish_sig(u_int envid, u_int signum, struct Trapframe *tf){
	return msyscall(SYS_finish_sig, envid, signum, tf);
}

int syscall_set_sig_entry(u_int envid, void (*func)(u_int, u_int, void (*handler)(int), struct Trapframe *)) {
	return msyscall(SYS_set_sig_entry, envid, func);
}