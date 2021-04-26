#include "../include/circularList.h"

static void listInit(list_t *list)
{
    list->prev = list;
    list->next = list;
}

static void listPush(list_t *list, list_t *entry)
{
    list_t *prev = list->prev;
    entry->prev = prev;
    entry->next = list;
    prev->next = entry;
    list->prev = entry;
}

static void listRemove(list_t *entry)
{
    list_t *prev = entry->prev;
    list_t *next = entry->next;
    prev->next = next;
    next->prev = prev;
}

static list_t *listPop(list_t *list)
{
    list_t *back = list->prev;
    if (back == list)
        return NULL;
    listRemove(back);
    return back;
}
