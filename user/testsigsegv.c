#include <lib.h>
int cnt = 0;

void sigsegv_handler(int sig) {
    debugf("in sigsegv handler. cnt: %d\n", cnt);
    if(cnt <= 2){
        kill(0, SIGSEGV);
        debugf("capture SIGSEGV signal. %d\n", cnt);
        cnt++;
    }
    else{
        debugf("capture SIGSEGV signal. %d\n", cnt);
        exit();
    }
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigsegv_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGSEGV, &sa, NULL);
    int* ptr = (int*)0x0;
    debugf("Accessing invalid memory address...\n");
    int val = *ptr; 
    debugf("If you see this on your screen, it means that the signal is not handled correctly.\n");
    return 0;
}