// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * Source: https://github.com/evanw/buddy-malloc/blob/master/buddy-malloc.c
 */
#include "../include/list.h"

void listInit(list_t *list)
{
    list->prev = list;
    list->next = list;
    list->occupied = 1;
}

void listPush(list_t *list, list_t *entry)
{
    list_t *prev = list->prev;
    entry->prev = prev;
    entry->next = list;
    prev->next = entry;
    list->prev = entry;
}

void listRemove(list_t *entry)
{
    list_t *prev = entry->prev;
    list_t *next = entry->next;
    prev->next = next;
    next->prev = prev;
}

list_t *listPop(list_t *list)
{
    list_t *back = list->prev;
    if (back == list)
        return NULL;
    listRemove(back);
    return back;
}

int isEmpty(list_t *list)
{
    return list->prev==list;
}
