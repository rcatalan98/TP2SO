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

// Se inicializa el vector para que todos los lugares esten disponibles
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
            semSpaces[i].available = FALSE;
            return i;
        }
    }
    return -1; // No hay mas espacio en el vector para crear otro semaforo
}

// Retorna la posicion dentro de la estructura donde esta guardado
static uint64_t createSem(char *name, uint64_t initValue)
{
    uint64_t pos;
    if ((pos = findAvailableSpace()) != -1)
    {
        // Inicializamos la estructura
        memcpy(semSpaces[pos].sem.name, name, strlen(name));
        semSpaces[pos].sem.value = initValue;
        semSpaces[pos].sem.lock = 0; // Inicialmente no esta lockeado.
        semSpaces[pos].sem.firstProcess = NULL;
        semSpaces[pos].sem.lastProcess = semSpaces[pos].sem.firstProcess;
        semSpaces[pos].sem.size = 0;
        semSpaces[pos].sem.sizeList = 0;
    }
    return pos;
}

// Retorna un puntero al semaforo al igual que en POSIX. En caso de error retorna NULL
uint64_t semOpen(char *name, uint64_t initValue)
{
    while (_xchg(&lockSem, 1) != 0) // esperando a que el lock este disponible
        ;
    // Primero me fijo si ya existe el sem por nombre
    // Si no existe debo crear el sem por primera vez
    int semIndex = findSem(name);
    if (semIndex == -1) // Si no existe el sem, hay que crearlo
    {
        semIndex = createSem(name, initValue);
        if (semIndex == -1)
        {
            _xchg(&lockSem, 0);
            return -1; // No habia mas lugar para crear sem.
        }
    }
    semSpaces[semIndex].sem.size++;
    _xchg(&lockSem, 0);
    return semIndex; // Retornamos el indice del sem.
}

// Retorna -1 en caso de error
uint64_t semClose(char *name)
{
    while (_xchg(&lockSem, 1) != 0)
        ;
    int semIndex = findSem(name);
    if (semIndex == -1)
    {
        print("Wrong semIndex\n");
        return -1; // No se encontro el semaforo pedido.
    }
    semSpaces[semIndex].sem.size--;
    _xchg(&lockSem, 0);
    return 1;
}

// Retorna 0 en caso de exito y -1 si fracasa. Blockea el sem.
uint64_t semWait(uint64_t semIndex)
{
    if (semIndex < 0 || semIndex >= MAX_SEM)
        return -1;
    sem_t *sem = &semSpaces[semIndex].sem;
    
    while (_xchg(&sem->lock, 1) != 0);
    // print("name sem: ");
    // print(sem->name);
    // print("\n");
    // print("Value: ");
    // printInt(sem->value);
    // print("\n");
    
    if (sem->value > 0)
    {
        // print("Antes Value: ");
        // printInt(sem->value);
        // print("\n");
        sem->value--;
        // print("Despues Value: ");
        // printInt(sem->value);
        // print("\n");
        _xchg(&sem->lock, 0);
    }
    else
    {
        // Si el valor es 0 entonces debo poner al proceso a dormir (encolarlo)
        uint64_t pid = getPid();
        if (enqeueProcess(pid, sem) == -1)
        {
            _xchg(&sem->lock, 0);
            print("Error enqeue");
            return -1;
        }
        
        _xchg(&sem->lock, 0);
        //print("Antes del block\n");
        if (block(pid) == -1)
        {
            print("Error en el block");
            return -1;
        }
       //print("Despues del block\n");
        sem->value--;
    }
    return 0;
}

uint64_t semPost(uint64_t semIndex)
{
    if (semIndex < 0 || semIndex >= MAX_SEM){
        print("Error index menor a 0 o mayor a max_sem\n");
        return -1;
    }
        
    sem_t *sem = &semSpaces[semIndex].sem;
    if(sem == NULL){
        print("Error sem es NULL\n");
    }
    while (_xchg(&sem->lock, 1) != 0)
        ;
    sem->value++;
    int pid;
    if (sem->sizeList > 0){
        if ((pid = dequeueProcess(sem)) == -1)
        {
            print("Error al sacar de la lista un proceso\n ");
            _xchg(&sem->lock, 0);
            return -1;
        }
        // int unblockValue = unblock(pid);
        // if (unblockValue != -1)
        // {
        //     forceTimer();
        // }
    }
    _xchg(&sem->lock, 0);//OJO CON ESTE CAMBIO 
    unblock(pid) ? : forceTimer();
    return 0;
}

// Retorna -1 en caso de no encontrar el sem
static uint64_t findSem(char *name)
{
    for (int i = 0; i < MAX_SEM; i++)
    {
        if (semSpaces[i].available == FALSE && strcmp(name, semSpaces[i].sem.name))
        {
            return i;
        }
    }
    return -1;
}

// Agrega un proceso a la lista, en caso de fallar retorna -1
uint64_t enqeueProcess(uint64_t pid, sem_t *sem)
{
    process_t *process = mallocFF(sizeof(process_t));
    if (process == NULL)
    {
        return -1;
    }
    process->pid = pid;
    if (sem->sizeList == 0)
    {
        sem->firstProcess = process;
        sem->lastProcess = sem->firstProcess;
        process->next = NULL;
    }
    else
    {
        sem->lastProcess->next = process;
        process->next = NULL;
        sem->lastProcess = process;
    }
    sem->sizeList++;
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
    sem->sizeList--;
    return pid;
}