#ifdef __x86_64__
    #include <unistd.h>
    #include <signal.h>
    #include <stdlib.h>
    #include <stdio.h>
    #define PRINT printf
#else 
    #include <lib.h>
    #define PRINT debugf
#endif
#define VICTIM 2
int gbl = 0;

void handler(int sig) {
    PRINT("handler: sig is %d\n", sig);
    PRINT("handler: gbl is %d\n", gbl++);
    kill(0, VICTIM);
    PRINT("handler: reaching end\n");
}

int main() {
    struct sigaction sa2;
    sa2.sa_handler = handler;
    sigemptyset(&sa2.sa_mask);
    sigaction(VICTIM, &sa2, 0);
    kill(0, VICTIM);
    PRINT("returned to current flow\n");
    return 0;
}