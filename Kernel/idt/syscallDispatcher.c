/**
 * syscallDispatcher.c: Recibe los llamados a las system calls usadas en Userland.
 * Preservamos syscalls similares o inspiradas en las existentes de Linux.
*/
#include "../include/stdint.h"
#include "../include/stdio.h"
#include "../include/keyboardDriver.h"
#include "../include/rtcDriver.h"
#include "../include/time.h"
#include "../include/memoryManager.h"
#include "../include/scheduler.h"
#include "../include/semaphore.h"

uint64_t syscallDispatcher(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t r9)
{
    switch (rdi)
    {
    case 0:
        // void timerFunc(void (*f), int toDo);
        timerFunc((void *)rsi, rdx);
        return 1;
    case 1:
        // uint8_t getTime(int descriptor);
        return getTime(rsi);
    case 2:
        // void drawFigure(char *toDraw, int color, int size, int x, int y);
        drawFigure((char *)rsi, rdx, rcx, r8, r9);
        return 1;
    case 3:
        // uint64_t sWrite(char *buffer, int size, int color);
        return sWrite((char *)rsi, rdx, rcx);
    case 4:
        // char sGetChar()
        return sGetChar();
    case 5:
        // setCursor(unsigned int new_x, unsigned int new_y, int color);
        setCursor(rsi, rdx, rcx);
        return 1;
    case 6:
        // void clearScreen();
        clearScreen();
        return 1;
    case 7:
        // void getPixelHeight();
        return getPixelHeight();
    case 8:
        // void getPixelWidth();
        return getPixelWidth();
    case 9:
        // void *mallocFF(uint32_t size)
        return (uint64_t)mallocFF(rsi);
    case 10:
        // void freeFF(void *ap) falta progamar algo para tema de errores.
        freeFF((void *)rsi);
        return 1;
    case 11:
        // uint64_t mem();
        mem();
        return 1;
    case 12:
        // uint64_t createProcess(void (*fn)(int, char **), int argc, char **argv, context cxt, int fd[2])
        return createProcess((void (*)(int, char **))rsi, (int)rdx, (char **)rcx, (context)r8, (int *)r9);
    case 13:
        // uint64_t kill(uint64_t pid);
        return kill((uint64_t)rsi);
    case 14:
        // uint64_t block(uint64_t pid);
        return block((uint64_t)rsi);
    case 15:
        // uint64_t ps();
        ps();
        return 1;
    case 16:
        // uint64_t getPid();
        return getPid();
    case 17:
        // uint64_t secondsElapsed()
        return secondsElapsed();
    case 18:
        // uint64_t unblock(uint64_t pid);
        return unblock((uint64_t)rsi);
    case 19:
        // uint64_t nice(uint64_t pid, uint64_t newPriority);
        return nice((uint64_t)rsi, (uint64_t)rdx);
    case 20:
        yield();
        return 1;
    case 21:
        // uint64_t semWait(uint64_t semIndex);
        return semWait((uint64_t)rsi);
    case 22:
        // uint64_t semPost(uint64_t semIndex);
        return semPost((uint64_t)rsi);
    case 23:
        // uint64_t semClose(char *name);
        return semClose((char *)rsi);
    case 24:
        // uint64_t semOpen(char *name, uint64_t initValue);
        return semOpen((char *)rsi, (uint64_t)rdx);
    case 25:
        // uint64_t pipeOpen(char *name);
        return pipeOpen((char *)rsi);
    case 26:
        // uint64_t pipeClose(uint64_t pipeIndex);
        return pipeClose((uint64_t)rsi);
    case 27:
        // char readPipe(uint64_t pipeIndex);
        return readPipe((uint64_t)rsi);
    case 28:
        // uint64_t writePipe(uint64_t pipeIndex, char *string);
        return writePipe((uint64_t)rsi, (char *)rdx);
    case 29:
        // void sem();
        sem();
        return 1;
    case 30:
        // void pipe();
        pipe();
        return 1;
    default:
        break;
    }
    return 0;
}