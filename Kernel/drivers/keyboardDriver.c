/**
 * Driver de teclado, llamado del metodo getKey() desde syscall 4.
*/
#include "../include/keyboardDriver.h"
#define IS_LOWER_CASE(n) ((n) >= 'a' && (n) <= 'z')




void putInBuffer(char c);
char keyToAscii(int scancode);

/** 
 *  Matriz que representa los ASCII del teclado, en primer lugar se encuentran
 * los caracteres sin tecla especial y en segundo aquellos cuando se presiona shift.
 * En caso de que la tecla no tenga un ASCII asociado decidimos devolver 0. 
 */
static char pressCodes[KEYS][2] = {
    {0, 0}, {ESC, ESC}, {'1', '!'}, {'2', '@'}, {'3', '#'}, {'4', '$'}, {'5', '%'}, {'6', '^'}, {'7', '&'}, {'8', '*'}, {'9', '('}, {'0', ')'}, {'-', '_'}, {'=', '+'}, {'\b', '\b'}, {'\t', '\t'}, {'q', 'Q'}, {'w', 'W'}, {'e', 'E'}, {'r', 'R'}, {'t', 'T'}, {'y', 'Y'}, {'u', 'U'}, {'i', 'I'}, {'o', 'O'}, {'p', 'P'}, {'[', '{'}, {']', '}'}, {'\n', '\n'}, {0, 0}, {'a', 'A'}, {'s', 'S'}, {'d', 'D'}, {'f', 'F'}, {'g', 'G'}, {'h', 'H'}, {'j', 'J'}, {'k', 'K'}, {'l', 'L'}, {';', ':'}, {'\'', '\"'}, {'`', '~'}, {0, 0}, {'\\', '|'}, {'z', 'Z'}, {'x', 'X'}, {'c', 'C'}, {'v', 'V'}, {'b', 'B'}, {'n', 'N'}, {'m', 'M'}, {',', '<'}, {'.', '>'}, {'/', '?'}, {0, 0}, {0, 0}, {0, 0}, {' ', ' '}};
static uint64_t shift = 0;
static uint64_t capsLock = 0;
static unsigned int bsize = 0;
static char keyBuffer[MAX_SIZE];
static int semId  = 0;
int initializeKeyboard()
{
    if((semId = semOpen("semKey", 0)) == -1){
        print("Error opening sem in Keyboard");
        return -1;
    }
    return 0;
}
int keyboard_handler()
{
    int scanCode;

    while (keyboardActivated())
    {
        scanCode = getKeyboardScancode();
        if (scanCode == SHIFT1 || scanCode == SHIFT2)
            shift = 1;
        else if (scanCode == SHIFT1_FREE || scanCode == SHIFT2_FREE)
            shift = 0;
        else if (scanCode == CAPS_LOCK)
            capsLock = !capsLock;
        if (scanCode >= 0 && scanCode < KEYS && pressCodes[scanCode][0] != 0)
        {
            putInBuffer(keyToAscii(scanCode));
            //print("antes del semPost\n");
            semPost(semId);
            //print("despues del semPost\n");
            return 1;
        }
    }
    return 0;
}

void putInBuffer(char c)
{
    if (bsize <= MAX_SIZE)
    {
        keyBuffer[bsize++] = c;
    }
}

char keyToAscii(int scancode)
{
    return pressCodes[scancode][capsLock || shift];
}

char sGetChar()
{
    // print("antes del semWait\n");
    if(semWait(semId)==-1)
        print("Error en el semWait");
    // print("despues del semWait\n");
    if (bsize <= 0)
    {
        return 0;
    }
    char key = keyBuffer[0];
    if (bsize > 0)
    {
        // removemos el primero.
        for (int i = 1; i < bsize; i++)
        {
            keyBuffer[i - 1] = keyBuffer[i];
        }
    }
    bsize--;
    return key;
}