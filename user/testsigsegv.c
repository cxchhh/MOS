#include <lib.h>
#include <signal.h>

void sigsegv_handler(int sig) {
    printf("capture SIGSEGV signal.\n");
    exit();
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigsegv_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGSEGV, &sa, NULL);
    int* ptr = (int*)0x0;
    printf("Accessing invalid memory address...\n");
    int val = *ptr; 
    printf("If you see this on your screen, it means that the signal is not handled correctly.\n");
    return 0;
}