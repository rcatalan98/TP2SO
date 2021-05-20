#ifndef BUDDY_MM
#include "../include/memoryManager.h"
#define BLOCK_SIZE sizeof(Node)
#define BLOCK_AMOUNT (baseSize / BLOCK_SIZE)
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

static int baseSize;
static Node *base;
static Node *freep = NULL; // Inicio de una lista libre.

/* Inicializa el sector de la memoria del heap indicando lugar y tamaño. */
void initializeMem(void *baseAllocation, uint32_t size)
{
    base = (Node *)baseAllocation;
    baseSize = size;
    freep = base;
    freep->s.size = BLOCK_AMOUNT;
    freep->s.ptr = NULL;
}

/* Asignador de memoria con el algoritmo FIRST-FIT. */
void *mallocFF(uint32_t size)
{
    // Si size == 0 entonces retorno NULL
    if (size == 0){
        return NULL;
    }
    Node *p;
    Node *prevp = freep;
    int nunits = (size + sizeof(Node) - 1) / sizeof(Node) + 1;
    for (p = freep; p != NULL; prevp = p, p = p->s.ptr)
    {
        if (p->s.size >= nunits)
        {
            if (p->s.size == nunits)
            {
                if (p == freep) // Me quedo con la base de los vacios.
                    freep = p->s.ptr;
                else // el siguiente del anterior pasa a ser el siguiente.
                    prevp->s.ptr = p->s.ptr;
            }
            else
            {
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }
            return (void *)(p + 1);
        }
    }
    return NULL;
}

/* Liberador de memoria con algoritmo FIRST-FIT. */
void freeFF(void *ap)
{
    Node *curr, *prev, *aux;
    aux = (Node *)(ap);
    aux--;

    if (aux == NULL || aux < (Node *)base || aux >= (Node *)base + baseSize)
    {
        return;
    }

    curr = freep;
    while (curr != NULL && curr < aux)
    { // Buscamos el anterior libre.
        prev = curr;
        curr = curr->s.ptr;
    }
    if (curr != NULL && (aux + aux->s.size == curr))
    { // Vemos de unir con el siguiente ptr.
        aux->s.size += curr->s.size;
        aux->s.ptr = curr->s.ptr;
    }
    else // Si no me puedo unir hago simplemente un cambio de puntero.
        aux->s.ptr = curr;

    if (curr == freep)
    { // Con esto veo si estoy antes que el primero libre.
        freep = aux;
    }
    else
    {
        if (prev + prev->s.size == aux)
        { // Uno los bloques si tengo uno libre atrás.
            prev->s.size += aux->s.size;
            prev->s.ptr = aux->s.ptr;
        }
        else
        {
            prev->s.ptr = aux; // Si no hago que el de atrás me apunte.
        }
    }
}

void mem()
{
    Node *curr;
    char *message1 = "Block: ";
    char *message2 = "Bytes: ";
    curr = freep;
    for (int i = 1; curr != NULL; curr = curr->s.ptr, i++)
    {
        print(message1);
        printInt(i);
        print("\t");
        print(message2);
        printInt(curr->s.size);
        print("\n");
    }
}
#endif