#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <stdio.h>
#include "lib.h"
#include "defs.h"
#include "memoryManager.h"
#include "interrupts.h"
extern void forceTimer();
void initializeScheduler();
void wait(uint64_t pid);
static uint16_t kill(uint64_t pid);
uint16_t block(uint64_t pid);
static void loaderStart(int argc, char *argv[], void *function(int, char **));
uint64_t scheduler(uint64_t rsp);
uint64_t createProcess(void (*fn)(int, char **), int argc, char **argv);
#endif