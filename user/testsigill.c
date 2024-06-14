#include <lib.h>
#include <signal.h>

void sigill_handler(int sig) {
    debugf("capture SIGILL signal.\n");
    syscall_yield();
    debugf("SIGILL signal handler returns.\n");
    exit();
}

void sigchld_handler(int sig)
{
    debugf("capture SIGCHLD signal.\n");
    debugf("SIGCHLD signal handler returns.\n");
}

void sigint_handler(int sig)
{
    debugf("capture SIGINT signal.\n");
    debugf("SIGINT signal handler returns.\n");
}

int main() {
   
    struct sigaction sa;
    sa.sa_handler = sigill_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGILL, &sa, NULL);

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGCHLD, &sa, NULL);

    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    u_int pid = syscall_getenvid();
    if(fork() == 0){
        debugf("send SIGINT.\n");
        kill(pid, SIGINT);
        debugf("send SIGCHLD.\n");
        exit();
    }
    asm("\tmove $t0,$sp\r\n"
        "\tjr $t0\r\n");
    debugf("Hello, world!\n");
    return 0;
}
