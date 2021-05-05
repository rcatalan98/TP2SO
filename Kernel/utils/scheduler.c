#include "../include/scheduler.h"
#include "../include/stdio.h" // Para debugging.
#define MAX_NAME 20
#define STACK_SIZE 4 * 1024
#define PROCESS_SIZE (8 * 1024 - sizeof(proccessNode)) // 8 KB
#define FIRST_PID 1
#define MAX_REGISTER_SIZE 16

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
    uint64_t rsp;
    uint64_t rbp;
    uint64_t priority; //por ahora no se usa
    states state;
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

static uint64_t initializeProcess(processNode *process, char *name);
static void addProcess(struct processNode *nodeToAdd, struct processList *list);
static struct processNode *removeProcess(struct processList *list);
static int isEmpty(struct processList *list);
static void freeProcess(struct processNode *nodeToRemove);
struct processNode *getProcess(uint64_t pid);
static void exitProcess();
uint16_t changeState(uint64_t pid, states newState);
static uint64_t getNewPid();
uint64_t getPid();

static void copyArguments(char **d, char **from, int amount)
{
    for (int i = 0; i < amount; i++)
    {
        d[i] = mallocFF(sizeof(char) * (strlen(from[i]) + 1));
        if(d[i]== NULL)
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
    int pid = createProcess((void *)&dummyP, 1, argv);
    dummyProcess = removeProcess(&currentList);
}

static processNode *findNextReady(struct processList *list)
{
    processNode *toReturn = removeProcess(list);
    while (toReturn->pcb.state != READY)
    {
        if (toReturn->pcb.state == KILLED)
        {
            // Hacemos el free del proceso.
            freeProcess(toReturn);
        }
        else if (toReturn->pcb.state == BLOCKED)
            addProcess(toReturn, list);
        toReturn = removeProcess(list);
    }
    return toReturn;
}

uint64_t scheduler(uint64_t rsp)
{
    // Tengo que fijarme si hay algun proceso corriendo. Si no es asi debo elegir uno de la lista con estado READY.
    // Si hay algun proceso debo hacer el switch context y chequear el tema del timeslot
    // return rsp;
    // if (currentProcess != NULL)
    // {
    //     print("El current no es null\n");
    //     currentProcess->pcb.rsp = rsp;
    //     if (currentProcess->pcb.pid != dummyProcess->pcb.pid)
    //     {
    //         addProcess(currentProcess, currentList);
    //     }
    // }
    // if (currentList.nReady > 0)
    // {
    //     currentProcess = findNextReady(currentList);
    //     if (currentProcess == NULL)
    //     {
    //         print("el findNextReady es null\n");
    //     }
    //     print("Hay elementos ready\n");
    //     currentProcess = removeProcess(currentList);
    // }
    // else
    // {
    //     print("Entro al else\n");
    //     currentProcess = dummyProcess;
    // }
    // print(currentProcess->pcb.name);
    if (currentProcess == NULL)
    {
        if (!isEmpty(&currentList))
            currentProcess = removeProcess(&currentList);
        else
            return rsp;
    }
    else
    {
        currentProcess->pcb.rsp = rsp;
        //addProcess(currentProcess, &currentList);
        if (currentList.nReady > 0)
        {
            currentProcess = findNextReady(&currentList);
        }
    }
    addProcess(currentProcess, &currentList);
    return currentProcess->pcb.rsp;
}

// Funcion auxiliar para la creacion del PCB.
static uint64_t initializeProcess(processNode *node, char *name)
{
    pcb_t *pcb = &(node->pcb);
    pcb->pid = getNewPid();
    memcpy(pcb->name, name, strlen(name));
    // if(process->rbp == NULL){
    //     printf("Malloc returned NULL");
    //     return
    // }
    pcb->rbp = (uint64_t)node + STACK_SIZE + sizeof(processNode) - sizeof(char *);
    pcb->rsp = (uint64_t)(pcb->rbp - sizeof(stackFrame));
    pcb->state = READY;
    pcb->priority = 0; //despues hay que implementar
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
uint64_t createProcess(void (*fn)(int, char **), int argc, char **argv)
{
    //print("Entra a createProcess\n");
    processNode *newProcess = mallocFF(sizeof(processNode) + STACK_SIZE);
    if (newProcess == NULL)
    {
        return 0;
    }
    initializeProcess(newProcess, argv[0]);
    //print("Sale de initializeProcess\n");
    //print((char *)newProcess->pcb.pid);
    print("argc: ");
    printInt(argc);
    char **args = mallocFF(sizeof(char *) * argc);
    if (args == NULL)
    {
        return 0;
    }
    copyArguments(args, argv, argc);
    initializeStackFrame(argc, args, newProcess, fn, newProcess->pcb.pid);
    //print("Sale de initializeStackFrame\n");
    addProcess(newProcess, &currentList);
    //print(newProcess->pcb.name);
    // print("Sale de addProcess\n");
    return newProcess->pcb.pid;
}

static int isEmpty(struct processList *list)
{
    return list->size == 0;
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

// Retorna 0 en caso de exito, -1 si existe algun tipo de error. Como en linux.
uint64_t kill(uint64_t pid)
{
    uint16_t done = changeState(pid, KILLED);
    if (pid == currentProcess->pcb.pid)
        forceTimer();
    return done;
}

uint64_t block(uint64_t pid)
{
    if(pid<FIRST_PID)
        return -1;
    int toReturn = changeState(pid, BLOCKED);
    if (pid == currentProcess->pcb.pid)
    {
        forceTimer();
    }
    return toReturn;
}
uint64_t unblock(uint64_t pid){
    if(pid<FIRST_PID)
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
    char aux[MAX_REGISTER_SIZE + 1];
    print(toPrint->pcb.name);
    print("\t");
    printInt(toPrint->pcb.pid);
    print("\t\t\t");
    printInt(toPrint->pcb.priority);
    print("\t\t");
    //printInt(uintToBase(toPrint->pcb.rsp, aux, 16));
    printHex(toPrint->pcb.rsp);
    print("\t");
    //printInt(uintToBase(toPrint->pcb.rbp, aux, 16));
    printHex(toPrint->pcb.rbp);
    print("\t");
    print("falta");
    print("\t\t\t");
    printInt(toPrint->pcb.state);
    print("\n");
}

//Falta agregar foreground y prioridad
uint64_t ps()
{
    struct processNode *aux = currentList.first;
    print("list size ");
    printInt(currentList.size);
    print("\n");
    if(aux == NULL)
    {
        print("There are no processes to show\n");
        return 0;
    }
    print("NAME    PID      PRIORITY     RSP        RBP        FOREGROUND    STATE\n");

    while (aux != NULL)
    {
        printProcess(aux);
        aux = aux->next;
    } 
    return 1;  
}

uint16_t changeState(uint64_t pid, states newState)
{
    if (pid == currentProcess->pcb.pid)
    {
        if (currentProcess->pcb.state == newState)
            return 1;
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
        currentList.nReady--;
    aux->pcb.state = newState;
    return 0;
}
