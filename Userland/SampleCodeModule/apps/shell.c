/**
 * shell.c: Intérprete de comandos de funciones de kernel.
*/
#include "../include/shell.h"

#define MAX_INPUT 30
#define MAX_SIZE 40
#define MAX_ARGUMENTS 5
#define REG_SIZE 17
#define ESC 27
#define LOOP_TIME 1
t_command commands[MAX_SIZE];
static int sizeC = 0;
void test();
void cat(int argSize, char *args[]);
void wc(int argSize, char *args[]);
void filter(int argSize, char *args[]);
static int isVowel(char c);

void intializeShell()
{
    char input[MAX_INPUT];
    loadCommands();
    _setCursor(0, HEIGHT - 1, GREEN);
    while (1) // !exit
    {
        printUser();
        readInput(input, MAX_INPUT, ESC);
        putChar('\n');
        processInput(input);
    }
}
// TRUE-> es builtin FALSE-> necesita ser un proceso extra
void loadCommands()
{
    loadCommand(&help, "help", "Displays the description of all functions available.\n", TRUE);
    loadCommand(&getRegs, "inforeg", "Displays all the information regarding the registers.\n", TRUE);
    loadCommand(&printmem, "printmem", "Makes a 32 Bytes memory dump to screen from the address passed by argument.\n", TRUE);
    loadCommand(&printCurrentTime, "time", "Displays the current time and date.\n", TRUE);
    loadCommand(&invalidOpCodeException, "invalidOpCodeException", "Displays exception of an invalid operation code.\n", TRUE);
    loadCommand(&invalidZeroDivisionException, "invalidZeroDivisionException", "Displays exception of an invalid division by zero.\n", TRUE);
    loadCommand(&chess, "chess", "Play a 1v1 match against a friend or yourself!.\nType 'chess -c' to continue the previous match.\nType 'chess -man' to display instructions.\n", FALSE);
    loadCommand(&_clearScreen, "clear", "Clears the whole screen.\n", TRUE);
    loadCommand(&wkill, "kill", "Kills a running process.\n", TRUE);
    loadCommand(&wblock, "block", "Blocks a running process.\n", TRUE);
    loadCommand(&wunblock, "unblock", "Unlocks a running process.\n", TRUE);
    loadCommand(&_mem, "mem", "Prints the current memory state.\n", TRUE);
    loadCommand(&_ps, "ps", "Prints running processes information.\n", TRUE);
    loadCommand(&loop, "loop", "Prints the current process ID and a message.\n", FALSE);
    loadCommand(&sleep, "sleep", "Delay for a specified amount of time.\n", TRUE);
    loadCommand(&test, "test", "Prints a loop of hello world as a built-in.\n", FALSE);
    loadCommand(&wnice, "nice", "Changes a process' priority.\n", TRUE);
    loadCommand(&_yield, "yield", "The current process resigns to the CPU.\n", TRUE);
    loadCommand((void *)&cat, "cat", "Prints entered text.\n", FALSE);
    loadCommand((void *)&wc, "wc", "Prints word count of the entered text.\n", FALSE);
    loadCommand(&filter, "filter", "Filters the vowels of the entered text.\n", FALSE);
    loadCommand(&test_mm, "test_mm", "Function to test the memory manager.\n", FALSE);
    loadCommand(&test_prio, "test_prio", "Function to test the priority scheduler.\n", FALSE);
    loadCommand(&test_processes, "test_processes", "Function to test the creation of processes.\n", FALSE);
    loadCommand(&test_sync, "test_sync", "Function to test the synchronization of processes with sem.\n", FALSE);
    loadCommand(&test_no_sync, "test_no_sync", "Function to test the synchronization of processes without sem.\n", FALSE);
}

void loadCommand(void (*fn)(), char *name, char *desc, int builtIn)
{
    commands[sizeC].command = fn;
    commands[sizeC].name = name;
    commands[sizeC].description = desc;
    commands[sizeC].builtIn = builtIn;
    sizeC++;
}

int readInput(char *inputBuffer, int maxSize, char token)
{
    int size = 0;
    uint64_t c;
    while (size < (maxSize - 1) && (c = getChar()) != '\n' && c != token)
    {
        if (c) // Verificamos que se presiona una letra.
        {
            if (c != '\b')
            {
                putChar(c);
                inputBuffer[size++] = c;
            }
            else if (size > 0)
            {
                putChar('\b');
                size--;
            }
        }
    }
    // Ponemos la marca de final al string.
    inputBuffer[size++] = 0;
    return c != token;
}

int processInput(char *inputBuffer)
{
    char *args[MAX_ARGUMENTS];
    int argSize = strtok(inputBuffer, ' ', args, MAX_ARGUMENTS);
    // Verificamos la cant de args antes de compararlo con los existentes.
    if (argSize <= 0 || argSize > 5)
    {
        print("Invalid amount of arguments, try again.\n");
        return 0;
    }
    for (int i = 0; i < sizeC; i++)
    {
        if (strcmp(args[0], commands[i].name))
        {
            if (!commands[i].builtIn)
            {
                context cxt = FOREGROUND;
                if (argSize == 2 && args[1][0] == '&')
                {
                    cxt = BACKGROUND;
                    // print("background\n");
                }
                // Se agrega la funcion como un proceso nuevo si no es built-in.
                _createProcess(commands[i].command, argSize, args, cxt);
            }
            else
            {
                commands[i].command(argSize - 1, args + 1);
            }
            return 1;
        }
    }
    print("Invalid command, try again.\n");
    return 0;
}

int getScreenHeight()
{
    return _getPixelHeight() / CHAR_HEIGHT;
}

int getScreenWidth()
{
    return _getPixelWidth() / CHAR_WIDTH;
}

/*
 ************************************ 
 * COMANDOS 
 ************************************
*/
void printUser()
{
    char s[] = "User@TP2_SO:$ ";
    printWithColor(s, LIGHT_BLUE);
}

void help()
{
    for (int i = 0; i < sizeC; i++)
    {
        printWithColor(commands[i].name, YELLOW);
        print(": ");
        print(commands[i].description);
        // putChar('\n');
    }
}

void inforeg(uint64_t *reg)
{
    static char *regs[REG_SIZE] = {
        "RAX", "RBX", "RCX", "RDX", "RBP", "RDI", "RSI",
        "R08", "R09", "R10", "R11", "R12", "R13", "R14",
        "R15", "IP ", "RSP"};

    char toPrint[30];
    for (int i = 0; i < REG_SIZE; i++)
    {
        printWithColor(regs[i], YELLOW);
        uintToBase(reg[i], toPrint, 16);
        print(":");
        print(toPrint);
        putChar('\n');
    }
}

void printCurrentTime()
{
    printInt(_getTime(HOURS));
    print(":");
    printInt(_getTime(MINUTES));
    print(", ");
    printInt(_getTime(DAY_OF_THE_MONTH));
    print("/");
    printInt(_getTime(MONTH));
    print("/");
    printInt(_getTime(YEAR) + 2000);
    putChar('\n');
}

void printmem(int argSize, char *args[])
{
    uint64_t num = hexaToInt(args[0]);
    if (argSize < 1 || num == 0)
    {
        print("Invalid argument. Try again.\n");
        return;
    }
    char toPrint[32];
    uint8_t *mem_address = (uint8_t *)num;
    for (int i = 0; i < 32; i++)
    {
        uintToBase(mem_address[i], toPrint, 16);
        print(toPrint);
        putChar(' ');
    }
    putChar('\n');
}

// source: https://www.felixcloutier.com/x86/ud.
void invalidOpCodeException()
{
    __asm__("ud2");
}

void invalidZeroDivisionException()
{
    int a = 0, b = (1 / a); // dividimos por 0.
    if (b)
    {
    }
}

void chess(int argSize, char *args[])
{
    _clearScreen();
    if (argSize == 0)
        startGame(NEW_GAME);
    if (strcmp(args[0], "-c"))
        startGame(CONTINUE_GAME);
    if (strcmp(args[0], "-man"))
    {
        printWithColor("CHESS MANUAL.\n", BEIGE);
        // Separado en más llamados a print ya que al usar un string muy largo genera errores.
        print("- Valid moves: 'FROM_X''FROM_Y' 'TO_X''TO_Y', caps lock should be enabled and the move should be valid to end your turn.\n");
        print("- Castling: short 'e 2' or 'e 3'.\n");
        print("- Coronation: By default a pawn transforms into a queen.\n");
        print("- Rotate: 'r' to rotate 90 degrees the board.\n");
        print("- Exit: 'ESC' to leave the game.\n");
    }
}

void sleep(int seconds)
{
    int secondsElapsed = _secondsElapsed();
    int finalTime = seconds + secondsElapsed;
    while (_secondsElapsed() <= finalTime)
        ;
}

void loop()
{
    int currentPid = _getPid();
    while (1)
    {
        sleep(LOOP_TIME);
        print("Soy el proceso: ");
        printInt(currentPid);
        print("\n");
    }
}

void test()
{
    while (1)
    {
        sleep(LOOP_TIME);
        print("Hello World\n");
    }
}

void wblock(int argSize, char *args[])
{
    if (argSize != 1)
    {
        print("Wrong amount of parameters\n");
        return;
    }
    _block(atoi2(args[0]));
}

void wkill(int argSize, char *args[])
{
    if (argSize != 1)
    {
        print("Wrong amount of parameters\n");
        return;
    }
    _kill(atoi2(args[0]));
}

void wunblock(int argSize, char *args[])
{
    if (argSize != 1)
    {
        print("Wrong amount of parameters\n");
        return;
    }
    _unblock(atoi2(args[0]));
}

void wnice(int argSize, char *args[])
{
    if (argSize != 2)
    {
        print("Wrong amount of parameters\n");
        return;
    }
    _nice(atoi2(args[0]), atoi2(args[1]));
}

void cat(int argSize, char *args[])
{
    char c;
    while ((c = getChar()) != 0)
        putChar(c);
}

void wc(int argSize, char *args[])
{
    int lines = 0;
    char c;
    while ((c = getChar()) != 0)
    {
        if (c == '\n')
            lines++;
    }
    print("Amount of lines: ");
    printInt(lines);
    print("\n");
}

void filter(int argSize, char *args[])
{
    char c;
    while ((c = _sGetChar()) != 0)
    {
        if (!isVowel(c))
            print(&c);
    }
    return;
}

static int isVowel(char c)
{
    return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' ||
            c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U')
               ? 1
               : 0;
}