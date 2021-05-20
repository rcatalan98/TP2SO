#ifndef list_H
#define list_H
#include <stdio.h>
#include "lib.h"
#include "defs.h"
typedef struct list_t
{
    uint64_t occupied;
    uint64_t bucketLevel;
    struct list_t *prev, *next;
} list_t;

void listInit(list_t *list);
void listPush(list_t *list, list_t *entry);
void listRemove(list_t *entry);
list_t *listPop(list_t *list);
int isEmpty(list_t *list);
#endif
