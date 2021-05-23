#ifndef PIPE_H
#define PIPE_H

#define BUFFER_SIZE 1024
#define MAX_LEN 20
#define MAX_PIPES 10

#include "lib.h"
#include "defs.h"
#include "semaphore.h"

uint64_t initPipes();
uint64_t pipeOpen(char *name);
uint64_t pipeClose(uint64_t pipeIndex);
uint64_t writePipe(uint64_t pipeIndex, char *string);
char readPipe(uint64_t pipeIndex);
#endif
