#ifndef memoryManager_H
#define memoryManager_H
#include "stdio.h"
#include "lib.h"
#include "defs.h"
void *mallocFF(uint32_t size);
void freeFF(void *ap);
void initializeMem(void *baseAllocation, uint32_t baseSize);
#endif