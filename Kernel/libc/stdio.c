/**
 * stdio.c: Librería dedicada a la impresión y manejo de salida a pantalla.
*/
#include "../include/stdio.h"

int height = HEIGHT / CHAR_HEIGHT, width = WIDTH / CHAR_WIDTH;
int x = 0, y = (HEIGHT / CHAR_HEIGHT) - FONT_SIZE, blink = 0, cursorColor = GREEN;

void backspace();
void enter();
void tab();
void blinkCursor();
static char buffer[64] = {'0'};

void setCursor(unsigned int new_x, unsigned int new_y, int color)
{
    if (new_x > width || new_x < 0 || new_y > height || new_y < 0)
        return;
    x = new_x;
    y = new_y;
    cursorColor = color;
    blinkCursor();
}

void blinkCursor()
{
    drawCursor(x * CHAR_WIDTH, y * CHAR_HEIGHT, blink = !blink, cursorColor);
}
void atomicPutChar(char c, int color){
    switch (c)
    {
    case '\n':
        enter();
        break;
    case '\b':
        backspace();
        break;
    case '\t':
        tab();
        break;
    default:
        // Se debe ver si saltar de linea o quedarse en la misma.
        drawChar(x * CHAR_WIDTH, y * CHAR_HEIGHT, c, FONT_SIZE, color, BLACK);
        ((x += FONT_SIZE) > width) ? enter() : setCursor(x, y, cursorColor);
    }
}
void putchar(char c, int color)
{
    int fd = getFdOut();
    if(fd > 0){
        writePipe(fd, &c);
    }else{
        atomicPutChar(c, color);
    }
}

uint64_t sWrite(char *buffer, int size, int color)
{
    while (size-- && *buffer != 0)
    {
        putchar(*buffer, color);
        buffer++;
    }
    return size == 0;
}

void print(char *string)
{
    sWrite(string, strlen(string), WHITE);
}

void printInt(int num)
{
    char buffer[40];
    uintToBase(num, buffer, 10);
    print(buffer);
}

void backspace()
{
    if (x == 0 && y == 0)
        return;
    setCursor(x - FONT_SIZE, y, cursorColor);
    putchar(' ', BLACK);
    if (blink)
    {
        blinkCursor();
    }
    x -= FONT_SIZE;
}

void enter()
{
    if (blink)
    {
        blinkCursor();
    }
    (y < height) ? scrollUpScreen() : clearScreen();
    setCursor(0, y, cursorColor);
}

void tab()
{
    if (blink)
    {
        blinkCursor();
    }
    ((x += (4 * FONT_SIZE)) < width) ? setCursor(x, y, cursorColor) : enter();
}
void printBase(uint64_t value, uint32_t base)
{
    uintToBase(value, buffer, base);
    print(buffer);
}
void printHex(uint64_t value)
{
    printBase(value, 16);
}
uint64_t sWritePriv(char *buffer, int size, int color)
{
    while (size-- && *buffer != 0)
    {
        atomicPutChar(*buffer, color);
        buffer++;
    }
    return size == 0;
}
void printError2(char *string)
{
    sWritePriv(string, strlen(string), RED);
}
