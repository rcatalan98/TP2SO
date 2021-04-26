#include "../include/scheduler.h"
#define MAX_NAME 1024
#define STACK_SIZE 1024
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
static uint64_t pidCounter = 1;
static uint64_t initializeProcess(pcb_t *process, char *name);
static void addProcess(struct processNode *nodeToAdd, struct processList *list);
static struct processNode *removeProcess(struct processList *list);
static int isEmpty(struct processList *list);
static void exit();
static void freeProcess(struct processNode *nodeToRemove);
uint16_t changeState(uint64_t pid, states newState);

static void copyArguments(char ** d, char ** from, int amount){
    for (int i = 0; i < amount; i++){
        d[i] = mallocFF(sizeof(char) * (strlen(from[i]) + 1));
        memcpy(d[i], from[i], strlen(from[i]));
    }
}

static processNode *currentProcess;
static processNode *dummyProcess;

static int dummyP(int argc, char ** argv)
{
    while(1)
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

void initializeScheduler()
{
    //Inicializamos la lista
    currentList.first = NULL;
    currentList.last = NULL;
    currentList.nReady = 0;
    currentList.size = 0;
    
    currentProcess = NULL;
    //Agregamos como primer proceso el dummy
    char *argv[] = {"dummyProcess"};
    createProcess((void *) &dummyP, 1, argv);
}

static uint64_t switchContext(uint64_t rsp)
{
    if(currentProcess != NULL)
    {
        currentProcess->pcb.rsp = rsp;
    }
    return currentProcess->pcb.rsp;
}

static processNode *findNextReady(struct processList * list)
{
    processNode *toReturn = removeProcess(list);
    while(toReturn->pcb.state != READY)
    {
        if(toReturn->pcb.state == KILLED)
        {
            //hacemos el free del proceso.
            freeProcess(toReturn);
        }else if(toReturn->pcb.state == BLOCKED)
            addProcess(toReturn, list);
        toReturn = removeProcess(list);
    }
    return toReturn;
}

uint64_t scheduler(uint64_t rsp)
{
    //Tengo que fijarme si hay algun proceso corriendo. Si no es asi debo elegir uno de la lista con estado READY.
    //Si hay algun proceso debo hacer el switch context y chequear el tema del timeslot
    if(currentProcess != NULL)
    {
        currentProcess->pcb.rsp = rsp;
        if(currentProcess->pcb.pid != dummyProcess->pcb.pid){
            addProcess(currentProcess, &currentList);
        }
    }
    if (currentList.nReady > 0)
    {
        currentProcess = findNextReady(&currentList);
    }
    else
    {
        currentProcess = dummyProcess;
    }
    
    return currentProcess->pcb.rsp;
}

//Funcion auxiliar para la creacion del PCB.
static uint64_t initializeProcess(pcb_t *process, char *name)
{
    process->pid = getNewPid();
    memcpy(process->name, name, strlen(name));
    process->rbp = (uint64_t)mallocFF(STACK_SIZE);
    // if(process->rbp == NULL){
    //     printf("Malloc returned NULL");
    //     return
    // }
    process->rbp = process->rbp + STACK_SIZE - 1;
    process->rsp = process->rbp - 1;
    process->state = READY;
    uint64_t toReturn = process->pid;
    return toReturn;
}



//Funcion auxiliar para la creacion del stack frame del proceso
static void initializeStackFrame(int argc, char **argv, void *rbp, void (*fn)(int, char **), uint64_t pid)
{
    stackFrame *stack = (stackFrame *)rbp - 1;
    // Se incializan los registros con numeros seguidos, se hace mas facil para debuggear.
    stack->r15 = 0x001;
    stack->r14 = 0x002;
    stack->r13 = 0x003;
    stack->r12 = 0x004;
    stack->r11 = 0x005;
    stack->r10 = 0x006;
    stack->r9 = 0x007;
    stack->r8 = 0x008;
    //Se completa el stack con la informacion necesaria a la hora de ejecutar el proceso
    stack->rsi = (uint64_t)argv;
    stack->rdi = argc - 1;
    stack->rbp = (uint64_t)rbp;
    stack->rdx = (uint64_t)fn;
    stack->rcx = pid;
    stack->rip = (uint64_t)loaderStart;
    stack->cs = 0x8;
    stack->rflags = 0x202;
    stack->rsp = stack->rbp;
    stack->ss = 0;
}

// Funciones de la lista circular para los procesos
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
    if(nodeToAdd->pcb.state == READY)
        list->nReady++;
    // En caso del que proceso agregado este en ready falta sumar para decir que se agrego.
    list->size++;
}
// Se crea un nuevo proceso creando el pcb y el stackframe que le corresponde. Se retorna su PID > 0 en caso de que salga todo bien. En caso de error se retorna 0
uint64_t createProcess(void (*fn)(int, char **), int argc, char **argv)
{
    processNode *newProcess = mallocFF(sizeof(processNode));
    if (newProcess == NULL)
        return 0;
    initializeProcess(&newProcess->pcb, argv[0]);
    char **args = mallocFF(sizeof(char *) * argc);
    copyArguments(args, argv, argc);
    if (args == NULL)
    {
        return 0;
    }
    initializeStackFrame(argc, args, (void *) newProcess->pcb.rbp, fn, newProcess->pcb.pid);
    addProcess(newProcess, &currentList);
    return newProcess->pcb.pid;
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
        list->nReady --;
    //TODO Falta disminuir la cantidad de readys que hay.
    list->size--;
    return ret;
}

static int isEmpty(struct processList *list)
{
    return list->size == 0;
}


static void loaderStart(int argc, char *argv[], void *function(int, char **))
{
    function(argc, argv);
    exit();
}

static void exit()
{
    kill(currentProcess->pcb.pid);
}

// Retorna 0 en caso de exito, -1 si existe algun tipo de error. Como en linux.
static uint16_t kill(uint64_t pid)
{
    uint16_t done = changeState(pid, KILLED);
    if (pid == currentProcess->pcb.pid)
        forceTimer();
    return done;
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

    while (aux != NULL){
        if(aux->pcb.pid == pid)
            return aux;
        aux = aux->next;
    }
    return NULL;
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

    if(aux == NULL || aux->pcb.state == KILLED)
        return -1; //No se puede hacer el cambio porque o no encontro el pid o ya esta muerto

    if(aux->pcb.state == newState)
        return 1;

    if(aux->pcb.state != READY && newState == READY)
        currentList.nReady++;
    else if(aux->pcb.state == READY && newState != READY)
        currentList.nReady--;
    
    aux->pcb.state = newState;
    return 0;
}
