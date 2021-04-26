/**
 *  sampleCodeModule.c: Archivo original de barebones modificado para poder correr shell.
 */
#include "./include/shell.h"

int main()
{
	char *argv[] = {"Shell"};
	int pid = _createProcess(&intializeShell, 1, argv);
	printInt(pid);
	//inforeg(getRegs());
	//intializeShell();
	return 0;
}