#include <lib.h>

void sigint_handler(int sig)
{
    debugf("capture SIGINT signal.\n");
    debugf("SIGINT signal handler returns.\n");
}

void sigsegv_handler(int sig)
{
    debugf("capture SIGSEGV signal.\n", env->env_id);
    //debugf("fa yield\n");
    syscall_yield();
    debugf("SIGSEGV signal handler returns.\n");
}

void sigchld_handler(int sig)
{
    debugf("capture SIGCHLD signal.\n");
    debugf("SIGCHLD signal handler returns.\n");
}

void sub()
{
    ipc_recv(NULL, 0, NULL);
    int ppid = env->env_parent_id;
    kill(ppid, SIGSEGV);
    //debugf("chld yield\n");
    syscall_yield();
    //debugf("back to sub\n");
    kill(ppid, SIGINT);
    kill(ppid, SIGCHLD);
}

int main()
{
    int child;
    if ((child = fork()) == 0) {
        sub();
        return 0;
    }
    struct sigaction sa_int;
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);

    struct sigaction sa_segv;
    sa_segv.sa_handler = sigsegv_handler;
    sigemptyset(&sa_segv.sa_mask);

    struct sigaction sa_chld;
    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);

    sigaddset(&sa_segv.sa_mask, SIGINT);

    sigaction(SIGINT, &sa_int, NULL);
    sigaction(SIGSEGV, &sa_segv, NULL);
    sigaction(SIGCHLD, &sa_chld, NULL);

    syscall_yield();

    ipc_send(child, 0, NULL, 0);

    syscall_yield();
    return 0;
}