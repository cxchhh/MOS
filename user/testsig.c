#include <lib.h>

void sigint_handler(int sig) {
	debugf("capture SIGINT!!!\n");
	exit();
}

int main() {
	debugf("Enter sigtst.\n");
	struct sigaction sa;
	sa.sa_handler = sigint_handler;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
    int child = fork();
    if(child == 0){
        while(1);
    }
    else{
        debugf("Sending SIGINT to child\n");
	    kill(child, SIGINT);
	    debugf("fa returned\n");
    }
	
	return 0;
}