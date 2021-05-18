#include "../include/semaphore.h"
typedef struct
{
    sem_t sem;
    uint64_t available;
} space;
static space semSpaces[MAX_SEM];

// Funciones internas
static uint64_t createSem(char *name, uint64_t initValue);
static uint64_t findAvailableSpace();
static uint64_t lockSem; // Para bloquear al momento de un open o close de cualquier sem.
static uint64_t findSem(char *name);
static uint64_t enqeueProcess(uint64_t pid, sem_t *sem);
static uint64_t dequeueProcess(sem_t *sem);
//Se inicializa el vector para que todos los lugares esten disponibles
void initSems()
{
    for (int i = 0; i < MAX_SEM; i++)
    {
        semSpaces[i].available = TRUE;
    }
}

static uint64_t findAvailableSpace()
{
    for (int i = 0; i < MAX_SEM; i++)
    {
        if (semSpaces[i].available == TRUE)
        {
            return i;
        }
    }
    return -1; //No hay mas espacio en el vector para crear otro semaforo
}

// Retorna la posicion dentro de la estructura donde esta guardado
static uint64_t createSem(char *name, uint64_t initValue)
{
    // Debo buscar el primer lugar disponible en el vector
    uint64_t pos;
    if ((pos = findAvailableSpace()) != -1)
    {
        // Inicializamos la estructura
        memcpy(semSpaces[pos].sem.name, name, strlen(name));
        semSpaces[pos].sem.value = initValue;
        semSpaces[pos].sem.lock = 0; // Inicialmente no esta lockeado.
        semSpaces[pos].sem.firstProcess = NULL;
        semSpaces[pos].sem.lastProcess = NULL;
    }
    return pos;
}
//Retorna un puntero al semaforo al igual que en POSIX. En caso de error retorna NULL
sem_t *semOpen(char *name, uint64_t initValue)
{
    while (_xchg(&lockSem, 1) != 0) // esperando a que el lock este disponible
        ;
    // Primero me fijo si ya existe el sem por nombre
    // Si no existe debo crear el sem por primera vez
    int semIndex = findSem(name);
    if (semIndex == -1) //Si no existe el sem, hay que crearlo
    {
        semIndex = createSem(name, initValue);
        if (semIndex == -1)
        {
            _xchg(&lockSem, 0);
            return NULL; // No habia mas lugar para crear sem.
        }
    }
    semSpaces[semIndex].sem.size++;
    _xchg(&lockSem, 0);
    return &semSpaces[semIndex].sem;
}
//Retorna -1 en caso de error
uint64_t semClose(sem_t *sem)
{
    while (_xchg(&lockSem, 1) != 0)
        ;
    int semIndex = findSem(sem->name);
    if (semIndex == -1)
    {
        return -1; // No se encontro el semaforo pedido.
    }
    semSpaces[semIndex].sem.size--;
    _xchg(&lockSem, 0);
    return 1;
}
//Retorna 0 en caso de exito y -1 si fracasa. Blockea el sem.
uint64_t semWait(sem_t *sem)
{
    while (_xchg(&sem->lock, 1) != 0)
        ;
    if (sem->value > 0)
    {
        sem->value--;
        _xchg(&sem->lock, 0);
        return 0;
    }
    //Si el valor es 0 entonces debo poner al proceso a dormir (encolarlo)
    uint64_t pid = getPid();
    if (enqeueProcess(pid, sem) == -1)
    {
        _xchg(&sem->lock, 0);
        return -1;
    }
    block(pid);
    _xchg(&sem->lock, 0);
    return 0;
}

uint64_t semPost(sem_t *sem)
{
    while (_xchg(&sem->lock, 1) != 0)
        ;
    sem->value++;
    int pid;
    if ((pid = dequeueProcess(sem)) == -1)
    {
        _xchg(&sem->lock, 0);
        return -1;
    }
    unblock(pid);
    _xchg(&sem->lock, 0);
    return 0;
}

//Retorna -1 en caso de no encontrar el sem
static uint64_t findSem(char *name)
{
    for (int i = 0; i < MAX_SEM; i++)
    {
        if (strcmp(name, semSpaces[i].sem.name) == 0)
        {
            return i;
        }
    }
    return -1;
}

//Agrega un proceso a la lista, en caso de fallar retorna -1
uint64_t enqeueProcess(uint64_t pid, sem_t *sem)
{
    process_t *process = mallocFF(sizeof(process_t));
    if (process == NULL)
    {
        return -1;
    }
    process->pid = pid;
    if (sem->size == 0)
    {
        sem->firstProcess = process;
        sem->lastProcess = process;
    }
    else
    {
        sem->lastProcess->next = process;
        process->next = NULL;
        sem->lastProcess = process;
    }
    sem->size++;
    return 0;
}

uint64_t dequeueProcess(sem_t *sem)
{
    if (sem == NULL || sem->firstProcess == NULL)
        return -1;
    process_t *current = sem->firstProcess;
    int pid = current->pid;
    sem->firstProcess = current->next;
    if (sem->firstProcess == NULL)
    {
        sem->lastProcess = NULL;
    }
    freeFF(current);
    return pid;
}