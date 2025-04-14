#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <errno.h>
#include <string.h>

#define NUM_PROCS 10
#define HIGH_PRIO_COUNT 5
#define LOW_PRIO_COUNT 5

const int MOD = 10;

// CPU Intensive Task
void cpu_intensive_task(int id) {
    int x = 0;
    while (1) {
        x = ((x + 1) % MOD);
    }
}

int main() {
    pid_t pids[NUM_PROCS];
    cpu_set_t cpuset;
    int i;

    printf("Creating %d CPU-intensive processes...\n", NUM_PROCS);

    // bind to cpu core 0
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);

    for (i = 0; i < NUM_PROCS; i++) {
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork failed");
            exit(1);
        } else if (pids[i] == 0) {
            if (sched_setaffinity(0, sizeof(cpu_set_t), &cpuset) != 0) {
                perror("sched_setaffinity failed");
                exit(1);
            }

            // set priority
            int nice_value;
            if (i < HIGH_PRIO_COUNT) {
                nice_value = -3; // high
                printf("Process %d (PID: %d) set to high priority (nice: %d)\n", i, getpid(), nice_value);
            } else {
                nice_value = 1;  // low 
                printf("Process %d (PID: %d) set to low priority (nice: %d)\n", i, getpid(), nice_value);
            }

            if (setpriority(PRIO_PROCESS, 0, nice_value) != 0) {
                perror("setpriority failed");
                exit(1);
            }

            // run the task
            cpu_intensive_task(i);
            exit(0); 
        }
    }

    // wait for the child process
    // hold the parent process

    for (i = 0; i < NUM_PROCS; i++) {
        int status;
        pid_t pid = waitpid(pids[i], &status, 0);
        if (pid < 0) {
            perror("waitpid failed");
            exit(1);
        }
        if (WIFEXITED(status)) {
            printf("Process %d (PID: %d) exited with status %d\n", i, pid, WEXITSTATUS(status));
        } else {
            printf("Process %d (PID: %d) terminated abnormally\n", i, pid);
        }
    }

    return 0;
}