#ifndef memoryManager_H
#define memoryManager_H
#include "../include/lib.h"
#include <stdio.h>
#include "../include/defs.h"

typedef uint64_t Align;

union node
{
    struct
    {
        union node *ptr;
        uint32_t size;
    } s;
    Align x;
} node;
typedef union node Node;
void *mallocFF(uint32_t size);
void freeFF(void *ap);
#endif