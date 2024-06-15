#include <lib.h>
#include <signal.h>

static int a = 0;

void sigill_handler(int sig) {
    debugf("capture SIGILL signal. %d\n", a);
    syscall_yield();
    debugf("SIGILL signal handler returns. %d\n", a);
    if(a>=1) exit();
}

void sigchld_handler(int sig)
{
    if(a==1){
        user_panic("yes!\n");
    }
    //debugf("capture SIGCHLD signal. %d\n", a);
    //debugf("SIGCHLD signal handler returns. %d\n", a);
}

void sigint_handler(int sig)
{
    a++;
    //printf("capture SIGINT signal. %d\n", a);
    //printf("SIGINT signal handler returns. %d\n", a);
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
