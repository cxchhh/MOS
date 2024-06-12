#include <lib.h>
#include <signal.h>

void sigill_handler(int sig) {
    debugf("capture SIGILL signal.\n");
    exit();
}

int main() {
   
    struct sigaction sa;
    sa.sa_handler = sigill_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGILL, &sa, NULL);
    asm("\tmove $t0,$sp\r\n"
        "\tjr $t0\r\n");
    debugf("Hello, world!\n");
    return 0;
}
