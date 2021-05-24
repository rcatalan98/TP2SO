#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "stdio.h"
#include "lib.h"
#include "defs.h"
#include "memoryManager.h"
#include "interrupts.h"
#include "naiveConsole.h"
typedef enum
{
    BACKGROUND,
    FOREGROUND
} context;
extern void forceTimer();
void initializeScheduler();
uint64_t getPid();
void wait(uint64_t pid);
void loaderStart(int argc, char *argv[], void *function(int, char **));
uint64_t block(uint64_t pid);
uint64_t unblock(uint64_t pid);
uint64_t kill(uint64_t pid);
uint64_t scheduler(uint64_t rsp);
uint64_t createProcess(void (*fn)(int, char **), int argc, char **argv, context cxt, int fd[2]);
void ps();
uint64_t nice(uint64_t pid, uint64_t newPriority);
void yield();
uint64_t getFdIn();  // fdIn del current
uint64_t getFdOut(); // fdOut del current
uint64_t killFg();
#endif