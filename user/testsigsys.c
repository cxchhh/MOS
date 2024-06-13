#include <lib.h>

void sigsegv_handler(int sig) {
    debugf("capture SIGSEGV signal.\n");
    //exit();
}

void sigsys_handler(int sig) {
    int *ptr = 0;
    debugf("accessing...\n");
    int va = *ptr;
    debugf("capture SIGSYS signal.\n");
    exit();
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigsys_handler;
    sigemptyset(&sa.sa_mask);
    //sigaction(SIGSYS, &sa, NULL);

    sa.sa_handler = sigsegv_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGSEGV, &sa, NULL);

    asm("\t li $a0,0x2233\r\n"
        "\t syscall \r\n");

    debugf("If you see this on your screen, it means that the signal is not handled correctly.\n");
    return 0;
}
