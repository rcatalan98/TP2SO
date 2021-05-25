/**
 * shell.c: Contrato para interprete de comandos del usuario.
*/
#ifndef _SHELL_H_
#define _SHELL_H_
#include <stdlib.h>
#include "chess.h"
#include "test.h"
#include "phylo.h"
typedef enum
{
    BACKGROUND,
    FOREGROUND
} context;
/**
 * Decidimos usar defines para el tamaño de los chars ya que una syscall seria innecesaria.
 */
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16

#define HEIGHT getScreenHeight()
#define WIDTH getScreenWidth()

#define SECONDS 0
#define MINUTES 2
#define HOURS 4
#define DAY_OF_THE_WEEK 6
#define DAY_OF_THE_MONTH 7
#define MONTH 8
#define YEAR 9

#define TRUE 1
#define FALSE 0

typedef struct
{
    void (*command)(int argSize, char *args[]);
    char *name;
    char *description;
    int builtIn; //flag for declaring if the function is built in.
} t_command;

extern void _setCursor(int x, int y, int color);

extern void _clearScreen();

extern void _drawFigure(char *toDraw, int color, int size, int x, int y);

extern uint64_t _createProcess(void (*fn)(int, char **), int argc, char **argv, context cxt, int fd[2]);

extern void *_mallocFF(uint32_t size);

extern void _freeFF(void *ap);

extern uint64_t _kill(uint64_t pid);

extern void _mem();

extern void _ps();

extern uint64_t _unblock(uint64_t pid);

extern uint64_t _block(uint64_t pid);

extern uint64_t _getPid();

extern uint64_t _secondsElapsed();

extern uint64_t _nice(uint64_t pid, uint64_t newPriority);

extern void _yield();

extern uint64_t _semOpen(char *name, uint64_t initValue);

extern uint64_t _semClose(char *name);

extern uint64_t _semPost(uint64_t semIndex);

extern uint64_t _semWait(uint64_t semIndex);

extern uint64_t _pipeOpen(char *name);

extern uint64_t _pipeClose(uint64_t pipeIndex);

extern uint64_t _writePipe(uint64_t pipeIndex, char *string);

extern char _readPipe(uint64_t pipeIndex);

extern void _sem();

extern void _pipe();

extern void getRegs(int argcount, char *args[]);

/**
 * Obtiene la cantidad de renglones en pantalla.
 */
int getScreenHeight();
/**
 * Obtiene la cantidad de posiciones de caracteres disponibles en pantalla.
 */
int getScreenWidth();
/**
 * Inicializa la estructura que ejecuta los programas del usuario.
*/
void intializeShell();
/**
 * Carga los camandos posibles para el usuario a traves de .
*/
void loadCommands();
/**
 * Actua de 
*/
void loadCommand(void (*fn)(), char *name, char *desc, int builtIn);

int readInput(char *inputBuffer, int maxSize, char token);
int processInput(char *inputBuffer);

/****************** COMANDOS ******************/
void printUser();
void help();
void inforeg(uint64_t *reg);
void printCurrentTime();
void printmem();
void invalidOpCodeException();
void invalidZeroDivisionException();
void chess(int argSize, char *args[]);
void loop(); //Falta revisar aca el tema de como le pasamos los argumentos.
void sleep(int seconds);
void wblock(int argSize, char *args[]);   // Wrappers para convertir el char en numero y llamar a la syscall
void wunblock(int argSize, char *args[]); // Wrapper de _unblock
void wkill(int argSize, char *args[]);    // Wrapper de _kill
void wnice(int argSize, char *args[]);    // Wrapper de _nice

#endif