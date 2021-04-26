#ifndef circularList_H
#define circularList_H
#include <stdio.h>
#include "lib.h"
#include "defs.h"
typedef struct list_t
{
    struct list_t *prev, *next;
} list_t;

static void listInit(list_t *list);
static void listPush(list_t *list, list_t *entry);
static void listRemove(list_t *entry);
static list_t *listPop(list_t *list);
#endif
