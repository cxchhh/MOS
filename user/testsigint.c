#include <lib.h>
#include <signal.h>
void sigint_handler(int sig) {
    printf("capture SIGINT.\n");
    exit();
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    printf("sending SIGINT to myself\n");
    kill(0,SIGINT);
    printf("If you see this on your screen, it means that the signal is not handled correctly.\n");
    while(1);
    return 0;
}