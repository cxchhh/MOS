#include <lib.h>
#include <signal.h>
void sigint_handler(int sig) {
    
    debugf("capture SIGINT.\n");
    int *ptr = 0;
    ptr = *ptr;
    debugf("backfrom segv.\n");
    //exit();
}
void sigill_handler(int sig) {
    debugf("in SIGILL.\n");
    kill(0, SIGINT);
    debugf("capture SIGILL.\n");
    exit();
}
void sigsegv_handler(int sig) {
    debugf("capture SIGSEGV.\n");
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    sa.sa_handler = sigsegv_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGSEGV, &sa, NULL);
    
    struct sigaction sa2;
    sa2.sa_handler = sigill_handler;
    sigemptyset(&sa2.sa_mask);
    sigaction(SIGILL, &sa2, NULL);

    debugf("sending SIGINT to myself\n");
    kill(0,SIGILL);
    debugf("If you see this on your screen, it means that the signal is not handled correctly.\n");
    while(1);
    return 0;
}