// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../include/scheduler.h"
#include "../include/stdio.h" // Para debugging.
#define STACK_SIZE (4 * 1024)
#define PROCESS_SIZE (8 * 1024 - sizeof(proccessNode)) // 8 KB
#define FIRST_PID 1
#define MAX_REGISTER_SIZE 16
#define INIT_PRIORITY 1
#define MAX_PRIORITY 12
#define QUANTUM 1
typedef enum
{
    READY,
    BLOCKED,
    KILLED
} states;
typedef struct stackFrame
{
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rsi; // argv
    uint64_t rdi; // argc
    uint64_t rbp;
    uint64_t rdx; // function
    uint64_t rcx; // pid
    uint64_t rbx;
    uint64_t rax;
    uint64_t rip;    // loader
    uint64_t cs;     // 0x8
    uint64_t rflags; // 0x202
    uint64_t rsp;    // rbp
    uint64_t ss;     // 0
} stackFrame;
typedef struct pcb_t
{
    char name[MAX_NAME];
    uint64_t pid;
    uint64_t ppid; //parent pid
    uint64_t rsp;
    uint64_t rbp;
    uint64_t priority;
    uint64_t tickets;
    states state;
    context context; // 1 -> FOREGROUND, 0 -> BACKGROUND
    uint64_t fdIn;
    uint64_t fdOut;
    uint64_t children[10];
    uint64_t waiting[10];
    int waitingIndex;
    int indexChild;
} pcb_t;
typedef struct processNode
{
    pcb_t pcb;
    struct processNode *next;
} processNode;
typedef struct processList
{
    processNode *first;
    processNode *last;
    uint32_t size;
    uint32_t nReady;
} processList;

static struct processList currentList;
static processNode *currentProcess;
static processNode *dummyProcess;
static uint64_t pidCounter = 1;

static uint64_t initializeProcess(processNode *node, char *name, context cxt, int fd[2]);
static void addProcess(struct processNode *nodeToAdd, struct processList *list);
static struct processNode *removeProcess(struct processList *list);
static int isEmpty(struct processList *list);
static void freeProcess(struct processNode *nodeToRemove);
struct processNode *getProcess(uint64_t pid);
static void exitProcess();
uint16_t changeState(uint64_t pid, states newState);
static uint64_t getNewPid();
uint64_t searchForForegroundKill();

static void copyArguments(char **d, char **from, int amount)
{
    for (int i = 0; i < amount; i++)
    {
        d[i] = mallocFF(sizeof(char) * (strlen(from[i]) + 1));
        if (d[i] == NULL)
            return;
        memcpy(d[i], from[i], strlen(from[i]));
    }
}

static int dummyP(int argc, char **argv)
{
    while (1)
        _hlt();
    return 0;
}

/*
 * PID = 0 -> error.
 * PID > 1 -> proceso funcionando correctamente.
 */
static uint64_t getNewPid()
{
    return pidCounter++;
}

uint64_t getPid()
{
    return currentProcess->pcb.pid;
}

void initializeScheduler()
{
    //Inicializamos la lista
    currentList.first = NULL;
    currentList.last = NULL;
    currentList.nReady = 0;
    currentList.size = 0;
    currentProcess = NULL;
    // Agregamos como primer proceso el dummy.
    char *argv[] = {"dummyProcess"};
    createProcess((void *)&dummyP, 1, argv, 1, NULL);
    dummyProcess = removeProcess(&currentList);
}

static processNode *findNextReady(struct processList *list)
{
    processNode *toReturn = removeProcess(list);
    while (toReturn->pcb.state != READY)
    {
        if (toReturn->pcb.state == KILLED)
        {
            // Si el proceso era fg -> desbloqueo al padre.
            if (toReturn->pcb.context == FOREGROUND)
            {
                struct processNode *parent = getProcess(toReturn->pcb.ppid);
                if (parent != NULL)
                    unblock(parent->pcb.pid);
            }
            // Hacemos el free del proceso.
            freeProcess(toReturn);
        }
        else
            addProcess(toReturn, list);
        toReturn = removeProcess(list);
    }
    return toReturn;
}

uint64_t scheduler(uint64_t rsp)
{
    // Tengo que fijarme si hay algun proceso corriendo. Si no es asi debo elegir uno de la lista con estado READY.
    // Si hay algun proceso debo hacer el switch context y chequear el tema del timeslot.
    if (currentProcess == NULL)
    {
        if (isEmpty(&currentList))
        {
            currentProcess = dummyProcess;
            currentProcess->pcb.tickets = currentProcess->pcb.priority * QUANTUM;
        }
        else
        {
            currentProcess = removeProcess(&currentList);
            currentProcess->pcb.tickets = currentProcess->pcb.priority * QUANTUM;
            addProcess(currentProcess, &currentList);
        }
    }
    else
    {
        currentProcess->pcb.rsp = rsp;
        if (currentProcess->pcb.tickets <= 0 && currentList.nReady > 0)
        {
            currentProcess = findNextReady(&currentList);
            currentProcess->pcb.tickets = currentProcess->pcb.priority * QUANTUM;
            addProcess(currentProcess, &currentList);
        }
        else if (currentList.nReady <= 0)
        {
            //print("Cambiando al dummy\n");
            currentProcess = dummyProcess;
            currentProcess->pcb.tickets = currentProcess->pcb.priority * QUANTUM;
        }
    }
    //addProcess(currentProcess, &currentList);
    currentProcess->pcb.tickets--;
    return currentProcess->pcb.rsp;
}

// Funcion auxiliar para la creacion del PCB.
static uint64_t initializeProcess(processNode *node, char *name, context cxt, int fd[2])
{
    pcb_t *pcb = &(node->pcb);
    pcb->pid = getNewPid();
    pcb->ppid = currentProcess == NULL ? 0 : currentProcess->pcb.pid;
    if (pcb->ppid != 0)
        currentProcess->pcb.children[currentProcess->pcb.indexChild++] = pcb->pid; //se guarda el pid del hijo nuevo.
    pcb->indexChild = 0;
    memcpy(pcb->name, name, strlen(name));
    pcb->rbp = (uint64_t)node + STACK_SIZE + sizeof(processNode) - sizeof(char *);
    pcb->rsp = (uint64_t)(pcb->rbp - sizeof(stackFrame));
    pcb->state = READY;
    pcb->priority = INIT_PRIORITY;
    pcb->tickets = INIT_PRIORITY * QUANTUM;
    pcb->context = cxt;

    if (fd == NULL)
    {
        pcb->fdIn = 0;
        pcb->fdOut = 0;
    }
    else
    {
        if (fd[0] == 0 && cxt == BACKGROUND)
        {
            return 0;
        }
        pcb->fdIn = fd[0];
        pcb->fdOut = fd[1];
    }

    return pcb->pid;
}

// Funcion auxiliar para la creacion del stack frame del proceso
static void initializeStackFrame(int argc, char **argv, processNode *node, void (*fn)(int, char **), uint64_t pid)
{
    stackFrame *stack = (stackFrame *)(node->pcb.rsp);
    // Se incializan los registros con numeros seguidos, se hace mas facil para debuggear.
    stack->r15 = 0x001;
    stack->r14 = 0x002;
    stack->r13 = 0x003;
    stack->r12 = 0x004;
    stack->r11 = 0x005;
    stack->r10 = 0x006;
    stack->r9 = 0x007;
    stack->r8 = 0x008;
    // Se completa el stack con la informacion necesaria a la hora de ejecutar el proceso.
    stack->rsi = (uint64_t)argv;
    stack->rdi = argc;
    stack->rbp = 0;
    stack->rdx = (uint64_t)fn;
    stack->rcx = pid;
    stack->rip = (uint64_t)loaderStart;
    stack->cs = 0x8;
    stack->rflags = 0x202;
    stack->rsp = (uint64_t)(node->pcb.rsp);
    stack->ss = 0x0;
}

// Funciones de la lista circular para los procesos.
static void addProcess(struct processNode *nodeToAdd, struct processList *list)
{
    if (nodeToAdd == NULL || list == NULL)
    {
        return;
    }
    if (list->first == NULL)
        list->first = nodeToAdd;
    else
        list->last->next = nodeToAdd;
    list->last = nodeToAdd;
    nodeToAdd->next = NULL;
    if (nodeToAdd->pcb.state == READY)
        list->nReady++;
    // En caso del que proceso agregado este en ready falta sumar para decir que se agrego.
    list->size++;
}

static struct processNode *removeProcess(struct processList *list)
{
    if (list == NULL || list->size == 0)
        return NULL;
    struct processNode *ret = list->first;
    // Si el proceso que sacamos es igual al ulitmo entonces last tiene que apuntar a null.
    if (list->last == ret)
        list->last = NULL;

    list->first = list->first->next;
    if (ret->pcb.state == READY)
        list->nReady--; // Se disminuye la cantidad de readys que hay.
    list->size--;
    return ret;
}

// Se crea un nuevo proceso creando el pcb y el stackframe que le corresponde. Se retorna su PID > 0 en caso de que salga todo bien. En caso de error se retorna 0
uint64_t createProcess(void (*fn)(int, char **), int argc, char **argv, context cxt, int fd[2])
{
    //print("Creando el proceso...\n");
    if (currentProcess != NULL && currentProcess->pcb.context == BACKGROUND && cxt == FOREGROUND)
    {
        return 0;
    }
    //print("Alocando mem para el proceso...\n");
    processNode *newProcess = mallocFF(sizeof(processNode) + STACK_SIZE);
    //print("Initializando el proceso...\n");
    if (newProcess == NULL || (initializeProcess(newProcess, argv[0], cxt, fd) == 0))
    {
        return 0;
    }
    //print("Asignando mem para args...\n");
    char **args = mallocFF(sizeof(char *) * argc);
    if (args == NULL)
    {
        return 0;
    }
    //print("Copiando args...\n");
    copyArguments(args, argv, argc);
    //print("Init stackFrame...\n");
    initializeStackFrame(argc, args, newProcess, fn, newProcess->pcb.pid);
    //print("Aniadieno el proceso...\n");
    addProcess(newProcess, &currentList);
    if (newProcess->pcb.context == FOREGROUND && newProcess->pcb.ppid > 0)
    { // Chequear esta condicion.
        //print("Bloqueando al padre...\n");
        block(newProcess->pcb.ppid);
    }
    //print("Proceso creado.\n");
    return newProcess->pcb.pid;
}

static int isEmpty(struct processList *list)
{
    return list->size == 0;
}

void yield()
{
    currentProcess->pcb.tickets = 0;
    forceTimer();
}

void loaderStart(int argc, char *argv[], void *function(int, char **))
{
    function(argc, argv);
    exitProcess();
}

static void exitProcess()
{
    kill(currentProcess->pcb.pid);
}

uint64_t killChildren()
{
    for (int i = 0; i < currentProcess->pcb.indexChild; i++)
    {
        int pidChild = currentProcess->pcb.children[i];
        if (pidChild > 0)
        {
            kill(pidChild);
        }
    }
}
// Retorna 0 en caso de exito, -1 si existe algun tipo de error tal como en linux.
uint64_t kill(uint64_t pid)
{
    uint16_t done = changeState(pid, KILLED);

    if (pid == currentProcess->pcb.pid)
    {
        if (currentProcess->pcb.indexChild != 0)
        {
            killChildren();
        }
        forceTimer();
    }

    return done;
}

uint64_t block(uint64_t pid)
{
    if (pid < FIRST_PID)
        return -1;
    int toReturn = changeState(pid, BLOCKED);
    if (pid == currentProcess->pcb.pid)
    {
        forceTimer();
    }
    return toReturn;
}

uint64_t unblock(uint64_t pid)
{
    if (pid < FIRST_PID)
        return -1;
    return changeState(pid, READY);
}

static void freeProcess(struct processNode *nodeToRemove)
{
    freeFF((void *)nodeToRemove);
}

struct processNode *getProcess(uint64_t pid)
{
    if (currentProcess->pcb.pid == pid)
        return currentProcess;
    struct processNode *aux = currentList.first;

    while (aux != NULL)
    {
        if (aux->pcb.pid == pid)
            return aux;
        aux = aux->next;
    }
    return NULL;
}

void printProcess(struct processNode *toPrint)
{
    printInt(toPrint->pcb.pid);
    print("  \t\t\t");
    printInt(toPrint->pcb.priority);
    print(" \t\t\t");
    printHex(toPrint->pcb.rsp);
    print("\t");
    printHex(toPrint->pcb.rbp);
    print("    \t");
    printInt(toPrint->pcb.context);
    print("\t\t\t\t");
    printInt(toPrint->pcb.state);
    print("   \t");
    print(toPrint->pcb.name);
    print("\n");
}

// Falta agregar foreground.
void ps()
{
    struct processNode *aux = currentList.first;
    print("list size ");
    printInt(currentList.size);
    print("\n");
    if (aux == NULL)
    {
        print("There are no processes to show\n");
        return;
    }
    print("PID\t\t\tPRIORITY\t\tRSP\t\tRBP\t\tFOREGROUND\t\tSTATE\tNAME\n");
    while (aux != NULL)
    {
        printProcess(aux);
        aux = aux->next;
    }
    return;
}

uint16_t changeState(uint64_t pid, states newState)
{
    if (pid == currentProcess->pcb.pid)
    {
        if (currentProcess->pcb.state == newState)
            return 1;
        if (currentProcess->pcb.state != READY && newState == READY)
            currentList.nReady++;
        else if (currentProcess->pcb.state == READY && newState != READY)
        {
            if (newState == KILLED && currentProcess->pcb.context == FOREGROUND && currentProcess->pcb.ppid > 0)
                unblock(currentProcess->pcb.ppid);
            currentList.nReady--;
        }
        currentProcess->pcb.state = newState;
        return 0;
    }
    struct processNode *aux = getProcess(pid);

    if (aux == NULL || aux->pcb.state == KILLED)
        return -1; // No se puede hacer el cambio porque o no encontro el pid o ya esta muerto.

    if (aux->pcb.state == newState)
        return 1;

    if (aux->pcb.state != READY && newState == READY)
        currentList.nReady++;
    else if (aux->pcb.state == READY && newState != READY)
    {
        if (newState == KILLED && aux->pcb.context == FOREGROUND && aux->pcb.ppid > 0)
            unblock(aux->pcb.ppid);
        currentList.nReady--;
    }
    aux->pcb.state = newState;
    return 0;
}

uint64_t nice(uint64_t pid, uint64_t newPriority)
{
    if (newPriority <= 0 || newPriority > 12)
    {
        print("Wrong priority. Priority is between 1 and 12.\n");
        return -1;
    }
    if (currentProcess->pcb.pid == pid)
        currentProcess->pcb.priority = newPriority;
    else
    {
        processNode *node = getProcess(pid);
        if (node == NULL)
        {
            return -1;
        }
        node->pcb.priority = newPriority;
    }
    return 0;
}

uint64_t getFdIn()
{
    return currentProcess->pcb.fdIn;
}

uint64_t getFdOut()
{
    return currentProcess->pcb.fdOut;
}
//Busca el primer proceso foreground en ready y lo mata.
uint64_t searchForForegroundKill()
{
    processNode *aux = currentList.first;
    while (aux != NULL)
    {
        if (aux->pcb.context == FOREGROUND && aux->pcb.ppid > 0)
        {
            unblock(aux->pcb.ppid);
            return kill(aux->pcb.pid);
        }

        aux = aux->next;
    }
    return -1; //No encontro proceso con las caract.
}
uint64_t killFg()
{
    if (currentProcess != NULL && currentProcess != dummyProcess && currentProcess->pcb.context == FOREGROUND && currentProcess->pcb.state == READY)
    {
        return kill(currentProcess->pcb.pid);
    }
    return searchForForegroundKill();
}

void waitPid(int pid)
{
    // wait
    currentProcess->pcb.waiting[currentProcess->pcb.waitingIndex++] = pid;
}

void waitChildren()
{
    for (int i = 0; i < currentProcess->pcb.indexChild; i++)
    {
        waitPid(currentProcess->pcb.children[i]);
    }
}