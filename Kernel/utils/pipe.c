#include "../include/pipe.h"

typedef struct
{
    uint64_t rIndex;
    uint64_t wIndex;
    int semRead;  //id del sem para leer en este pipe
    int semWrite; //id del sem para escribir en este pipe
    char buffer[BUFFER_SIZE];
    char name[MAX_LEN];
} pipe_t;

typedef struct
{
    pipe_t pipe;
    uint64_t available;
} space;
int semPipeManager; //id del sem para controlar el acceso al vetor de pipes.

static space pipes[MAX_PIPES];

static uint64_t indexValid(uint64_t pipeIndex);
static uint64_t createPipe(char *name);
static uint64_t findPipe(char *name);
static uint64_t findAvailableSpace();
uint64_t writeChar(uint64_t pipeIndex, char c);

//Retorna -1 en caso de error y 0 en caso contrario.
uint64_t initPipes()
{
    print("init pipes");
    if ((semPipeManager = semOpen("Pipe Manager", 1)) == -1)
    {
        print("Error en initPipe");
        return -1;
    }
    for (int i = 0; i < MAX_SEM; i++)
    {
        pipes[i].available = TRUE;
    }
    return 0;
}

//Retorna -1 en caso de error
uint64_t pipeOpen(char *name)
{
    if (semWait(semPipeManager))
    {
        print("Error semWait en pipeOpen\n");
        return -1;
    }
    //Busco si existe un pipe en nuestra estructura de datos

    int id = findPipe(name);
    if (id == 0)
    {
        //Si no existe un pipe con ese nombre
        id = createPipe(name);
    }
    if (id == -1)
    {
        print("Error en pipeOpen, id=-1\n");
        semPost(semPipeManager);
        return -1;
    }
    if (semPost(semPipeManager))
    {
        print("Error semPost en pipeClose\n");
        return -1;
    }
    return id;
}

uint64_t pipeClose(uint64_t pipeIndex)
{
    if (!indexValid(pipeIndex))
        return -1;

    if (semWait(semPipeManager) == -1)
    {
        print("Error semWait en pipeClose\n");
        return -1;
    }

    pipes[pipeIndex - 1].available = TRUE;
    int closeRead = semClose("semRead");
    int closeWrite = semClose("semWrite");

    if (closeRead == -1 || closeWrite == -1)
    {
        print("PipeClose: Error en los sem close del pipe\n");
        return -1;
    }

    if (semPost(semPipeManager) == -1)
    {
        print("Error semPost en pipeClose\n");
        return -1;
    }
    return 1;
}

uint64_t writePipe(uint64_t pipeIndex, char *string)
{
    if (!indexValid(pipeIndex))
        return -1;

    while (*string != 0)
    {
        if ((writeChar(pipeIndex, *string++)) == -1)
            return -1;
    }
    return 0;
}

uint64_t writeChar(uint64_t pipeIndex, char c)
{
    if (!indexValid(pipeIndex))
        return -1;

    pipe_t * pipe = &pipes[pipeIndex - 1].pipe;
    if (semWait(pipe->semWrite) == -1)
    {
        print("Error semWait en writeChar\n");
        return -1;
    }
    pipe->buffer[pipe->wIndex % BUFFER_SIZE] = c;
    pipe->wIndex++;
    if (semPost(pipe->semRead) == -1)
    {
        print("Error semPost en writeChar\n");
        return -1;
    }
    return 1;
}

char readPipe(uint64_t pipeIndex)
{
    if (!indexValid(pipeIndex))
        return -1;

    pipe_t * pipe = &pipes[pipeIndex - 1].pipe;
    if (semWait(pipe->semRead) == -1)
    {
        print("Error semWait en readPipe\n");
        return -1;
    }
    char c = pipe->buffer[pipe->rIndex % BUFFER_SIZE];
    pipe->rIndex++;
    if (semPost(pipe->semWrite) == -1)
    {
        print("Error semPost en readPipe\n");
        return -1;
    }
    return c;
}

//Retorna 0 en caso de no existir un pipe con el nombre, la pos + 1 en caso de que exista y -1 en caso de error.
static uint64_t findPipe(char *name)
{

    for (int i = 0; i < MAX_SEM; i++)
    {
        if (pipes[i].available == FALSE && strcmp(name, pipes[i].pipe.name))
        {
            return i + 1;
        }
    }
    return 0;
}

//Retorna el id del pipe en caso de exito y -1 si se encontro algun tipo de error
static uint64_t createPipe(char *name)
{
    int len = strlen(name);
    if (len <= 0 || len >= MAX_NAME)
    {
        print("Creando el pipe: Nombre demasiado largo\n");
        return -1;
    }
    uint64_t pos;
    if ((pos = findAvailableSpace()) != -1)
    {
        pipe_t *newPipe = &pipes[pos].pipe;
        // Inicializamos la estructura
        memcpy(newPipe->name, name, len);
        newPipe->rIndex = 0;
        newPipe->wIndex = 0;
        char nameR[MAX_NAME];
        memcpy(nameR, name, len);
        nameR[len] = 'R';
        nameR[len + 1] = 0;
        uint64_t semRead = semOpen(nameR, 0);
        char nameW[MAX_NAME];
        memcpy(nameW, name, len);
        nameW[len] = 'W';
        nameR[len + 1] = 0;
        uint64_t semWrite = semOpen(nameW, BUFFER_SIZE);
        if (semRead == -1 || semWrite == -1)
        {
            print("PipeOpen: Error en los sem del pipe");
            return -1;
        }
        newPipe->semRead = semRead;
        newPipe->semWrite = semWrite;
    }
    return pos + 1;
}

static uint64_t findAvailableSpace()
{
    for (int i = 0; i < MAX_PIPES; i++)
    {
        if (pipes[i].available == TRUE)
        {
            pipes[i].available = FALSE;
            return i;
        }
    }
    return -1; // No hay mas espacio en el vector para crear otro pipe
}

static uint64_t indexValid(uint64_t pipeIndex)
{
    return ((pipeIndex < 0 || pipeIndex > MAX_PIPES) && pipes[pipeIndex - 1].available == FALSE) ? 0 : 1;
}