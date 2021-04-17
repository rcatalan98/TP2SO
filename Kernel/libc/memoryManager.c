#include "../include/memoryManager.h"

#define BLOCK_SIZE sizeof(node)

static Node *base;
static Node *freep = NULL; // Inicio de una lista libre

/* Asignador de memoria con el algoritmo FIRST-FIT */
void *mallocFF(uint32_t size)
{
    // Si size == 0 entonces retorno NULL
    if (size == 0)
        return NULL;
    Node *p;
    Node *prevp = freep;
    uint32_t nunits = (size + sizeof(Node) - 1) / sizeof(Node) + 1;
    if (prevp == NULL)
    {
        // Se inicializa la lista nueva.
        prevp = base;
        freep = prevp;
        base->s.ptr = freep;
        base->s.size = 0;
    }
    for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr)
    {
        if (p->s.size >= size)
        {
            if (p->s.size == size)
                prevp->s.ptr = p->s.ptr;
            else
            {
                p->s.size -= size;
                p += p->s.size;
                p->s.size = size;
            }
            freep = prevp;
            return (void *)(p + 1);
        }
        if (p == freep)
        {
            return NULL;
        }
    }
}
void freeFF(void *ap)
{
    Node *bp, *p;
    bp = (Node *)ap - 1; /* point to block header */
    for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
        if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
            break; /* freed block at start or end of arena */
    if (bp + bp->s.size == p->s.ptr)
    { 
        /* join to upper nbr */
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    }
    else
        bp->s.ptr = p->s.ptr;
    if (p + p->s.size == bp)
    { /* join to lower nbr */
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    }
    else
        p->s.ptr = bp;
    freep = p;
}