#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

pid_t rt_pid;

const int MOD = 10;

// CPU Intensive Task
void cpu_intensive_task(void) {
    int x;
    while (1) {
        x = ((x + 1) % MOD);
    }
}

int main() {
    cpu_set_t cpuset;

    printf("Creating realtime process...\n");

    // bind to CPU core 0
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);

    // create a real time child process
    rt_pid = fork();
    if (rt_pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (rt_pid == 0) {
        if (sched_setaffinity(0, sizeof(cpu_set_t), &cpuset) != 0) {
            perror("sched_setaffinity failed");
            exit(1);
        }

        // set real-time scheduling policy
        struct sched_param param;
        param.sched_priority = 99; 
        if (sched_setscheduler(0, SCHED_RR, &param) != 0) {
            perror("sched_setscheduler failed");
            exit(1);
        }

        printf("Realtime process (PID: %d) started with SCHED_RR, priority 99\n", getpid());

        // run the task
        cpu_intensive_task();
        exit(0); 
    }

    wait(NULL); 

    return 0;
}