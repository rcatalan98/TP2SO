#ifndef __keyboardDriver_H_
#define __keyboardDriver_H_
#include "stdint.h"
#include "font.h"
#include "videoDriver.h"
#include "semaphore.h"
#include "pipe.h"
#define SHIFT1 42
#define SHIFT2 54
#define SHIFT1_FREE (SHIFT1 + 128)
#define SHIFT2_FREE (SHIFT2 + 128)
#define ENTER 10
#define SPACE 32
#define CAPS_LOCK 58
#define DEL 127
#define TAB 09
#define ESC 27
#define KEYS 58
#define MAX_SIZE 10
/**
 * Handler que se llama desde syscall para la lectura de teclado.
 * Retorna 1 si lo pudo leer correctamente, 0 si no.
 */
int keyboard_handler();
/**
 * Retorna la tecla presionada del buffer, se lo llama desde syscalls.
*/
char sGetChar();
/**
 * Inicializa el sem necesario para el funcionamiento bloqueante del sGetChar. La idea es un semWait antes de pedir y un semPost al llegar una tecla.
*/
int initializeKeyboard();
/**
 * Detecta si hay un codigo de teclado en el buffer.
 */
extern int keyboardActivated();
/**
 * Devuelve el codigo de teclado del buffer.
 */
extern unsigned char getKeyboardScancode();
#endif