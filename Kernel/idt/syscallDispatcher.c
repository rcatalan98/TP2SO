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
        // void freeFF(void *ap)
        freeFF((void *)rsi);
        return 1;
    case 11:
        return createProcess((void(*) (int, char **)) rsi, rdx, (char **) rcx);
    default:
        break;
    }
    return 0;
}