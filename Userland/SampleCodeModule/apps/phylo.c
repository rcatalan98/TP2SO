#include "../include/phylo.h"

#define MAX_PHYL 15
#define MIN_PHYL 2
#define INIT_PHYL 5
#define SEM_PHYL "semPhylos"
#define LEN 10
#define QUANTUM 3

typedef struct
{
    uint64_t pid;
    int state;
    uint64_t semIndex;
    char semName[LEN];
} phylo_t;

enum state
{
    THINKING,
    EATING,
    WAITING
};

static phylo_t phylos[MAX_PHYL];
static int seated;
uint64_t sem;

static int addPhylo(int pIndex);
static int removePhylo(int pIndex);
void endTable();
void phyloProcess(int argc, char **argv);
static void printState();
void putChopstick();
void takeChopstick(int pIndex);
void update(int pIndex);
int left(int pIndex);
int right(int pIndex);

/**
 * Ejecuta el dilema de los filosofos.
 */
void phylo(int argc, char **argv)
{
    printWithColor("WELCOME TO PHYLO!\n", YELLOW);
    if ((sem = _semOpen(SEM_PHYL, 1)) == -1)
    {
        printWithColor("Error opening main semaphore in phylo.\n", RED);
        return;
    }
    seated = 0;
    for (int i = 0; i < INIT_PHYL; i++)
    {
        if (addPhylo(i) == -1)
        {
            printWithColor("Error adding initial philosophers.\n", RED);
        }
    }
    print("Press 'a' to add or 'r' to remove a philosopher. Press 'q' to exit.\n");
    char c;
    while ((c = getChar()) != 0)
    {
        switch (c)
        {
        case 'a':
        case 'A':
            if (addPhylo(seated) == -1)
            {
                printWithColor("Error adding philosopher.\n", RED);
            }
            break;
        case 'r':
        case 'R':
            if (removePhylo(seated - 1) == -1)
            {
                printWithColor("Error removing philosopher.\n", RED);
            }
            break;
        case 'q':
        case 'Q':
            endTable();
            if (_semClose(SEM_PHYL) == -1)
                printWithColor("Error closing main semaphore in Phylo.\n", RED);
            // _unblock(2);
            return;
        }
    }
}

/**
 * Se suma un filosofo a la mesa, prepara sus cosas para comer.
 * Retorna -1 en caso de error.
 */
static int addPhylo(int pIndex)
{
    if (pIndex > MAX_PHYL || pIndex < 0)
    {
        return -1;
    }
    _semWait(sem);
    seated++;
    char semName[LEN] = "phyl";
    numToStr(pIndex, phylos[pIndex].semName, LEN);
    strcat(phylos[pIndex].semName, semName);
    if ((phylos[pIndex].semIndex = _semOpen(phylos[pIndex].semName, 1)) == -1)
    {
        printWithColor("Error opening sem in addPhylo.\n", RED);
        return -1;
    }
    char currSeated[LEN];
    uintToBase(seated, currSeated, 10);
    char *argv[] = {"phi", currSeated};
    phylos[pIndex].state = THINKING;
    if ((phylos[pIndex].pid = _createProcess(&phyloProcess, 2, argv, BACKGROUND, NULL)) == 0) //TODO Cambiar al implementar pipes
    {
        printWithColor("Error creating philosopher process", RED);
        return -1;
    }
    _semPost(sem);
    return 0;
}

/**
 * Proceso que hace que los filosofos lleguen a la mesa,
 * busquen sus chopsticks y se sienten, si no pueden,
 * deben esperar a su turno.
 */
void phyloProcess(int argc, char **argv)
{
    int index = atoi2(argv[1]);
    while (1)
    {
        sleep(QUANTUM);
        // Busca chopsticks ya que tiene hambre.
        takeChopstick(index);
        sleep(QUANTUM);
        // Termino de comer, deja sus chopsticks.
        putChopstick();
    }
}

/**
 * El filosofo desea comer, toma unos palillos y espera su turno. 
 */
void takeChopstick(int pIndex)
{
    _semWait(sem);
    phylos[pIndex].state = WAITING;
    update(pIndex);
    _semPost(sem);
    _semWait(phylos[pIndex].semIndex);
}

/**
 *  El filosofo piensa y se ve junto a sus vecinos, quienes se preguntan si pueden comer.
 */
void putChopstick(int pIndex)
{
    _semWait(sem);
    phylos[pIndex].state = THINKING;
    update(left(pIndex));
    update(right(pIndex));
    _semPost(sem);
}

// Devuelve el vecino izquierdo del indice.
int left(int pIndex)
{
    return (pIndex == 0) ? seated - 1 : pIndex - 1;
}

// Devuelve el vecino derecha del indice.
int right(int pIndex)
{
    return (pIndex == seated - 1) ? 0 : pIndex + 1;
}

// Actualiza la mesa con los filosofos que desean y se encuentran comiendo.
void update(int pIndex)
{
    if (phylos[pIndex].state == WAITING && phylos[left(pIndex)].state != EATING && phylos[right(pIndex)].state != EATING)
    {
        phylos[pIndex].state = EATING;
        printState();
        _semPost(phylos[pIndex].semIndex);
    }
}

/**
 * El filosofo se retira de la mesa, no quiere comer más entonces
 * su lugar es liberado y se reordena el resto.
 */
static int removePhylo(int pIndex)
{
    if (pIndex > MAX_PHYL || pIndex < MIN_PHYL)
        return -1;
    _semWait(sem);
    seated--;
    int eats = (phylos[pIndex].state == EATING);
    if (_semClose(phylos[pIndex].semName) == -1)
        printWithColor("Error in closing phylo's sem.\n", RED);
    if (_kill(phylos[pIndex].pid) == -1)
        printWithColor("Error in killing phylo process.\n", RED);
    if (eats && pIndex)
    {
        update(pIndex - 1);
        update(0);
    }
    _semPost(sem);
    return 0;
}

/**
 * Se cierra la mesa, todo filosofo es echado.
 */
void endTable()
{
    while (seated > 0)
    {
        if (_semClose(phylos[seated - 1].semName) == -1)
        {
            printWithColor("Error in closing phylo's sem while ending.\n", RED);
            return;
        }
        if (_kill(phylos[seated - 1].pid) == -1)
        {
            printWithColor("Error in killing phylo process while ending.\n", RED);
            return;
        }
        seated--;
    }
}

static void printState()
{
    for (int i = 0; i < seated; i++)
    {
        (phylos[i].state == EATING) ? print(" E ") : print(" . ");
    }
    putChar('\n');
}
