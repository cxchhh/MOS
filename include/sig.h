#ifndef SIG_H
#define SIG_H

#include <types.h>

typedef struct sigset_t {
    u_int sig;
} sigset_t;

struct sigaction {
    void     (*sa_handler)(int);
    sigset_t   sa_mask;
};

// __how
#define SIG_BLOCK   0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2

//signals
#define SIG_MIN 1
#define SIGINT  2
#define SIGILL  4
#define SIGKILL 9
#define SIGSEGV 11
#define SIGCHLD 17
#define SIGSYS  31
#define SIG_MAX 32

#endif