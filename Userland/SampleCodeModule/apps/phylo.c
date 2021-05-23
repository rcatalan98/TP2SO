#include "../include/phylo.h"

#define MAX_PHYL 30
#define MIN_PHYL 2
#define INIT_PHYL 5
#define SEM_PHYL "semPhylos"

typedef struct
{
    uint64_t pid;
    int state;
} phylo_t;

enum state
{
    THINKING = 0,
    EATING
};

static phylo_t phylos[MAX_PHYL];
static uint64_t chopstick[MAX_PHYL];
static int seated;
uint64_t sem;

static int addPhylo(int i);
static int removePhylo(int i);
static void printState();

void phylo(int argc, char **argv)
{
    if ((sem = _semOpen(SEM_PHYL, 1)) == -1)
    {
        print("Error at semOpen en phylo.\n");
        return;
    }
    seated = 0;
    for (int i = 0; i < INIT_PHYL; i++)
    {
        addPhylo(i);
    }
    char c;
    while ((c = getChar()) != 0)
    {
        switch (c)
        {
        case 'a':
        case 'A':
            if(addPhylo(seated) == -1)
            {
                print("Error at addPhylo en phylo\n");
                return ;
            }
            break;
        case 'r':
        case 'R':
            if(removePhylo(seated) == -1)
            {
                print("Error at removePhylo en phylo\n");
                return ;
            }
            break;
        }
    }
    printState();
    // endPhylo();
}

static int addPhylo(int i)
{
    if(i > MAX_PHYL || i < MIN_PHYL)
        return -1;
    semWait(sem);
    
    return 0;
}

static int removePhylo(int i)
{
    if(i > MAX_PHYL || i < MIN_PHYL)
        return -1;
    
    return 0;
}

static void printState()
{
    for (int i = 0; i < seated; i++)
    {
        if (phylos[i].state == EATING)
            print(" E ");
        else
            print(" . ");
    }

    putChar('\n');
}