/**
 *  sampleCodeModule.c: Archivo original de barebones modificado para poder correr shell.
 */
#include "./include/shell.h"

int main()
{
	char *argv[] = {"Shell"};
	//int pid = _createProcess(&intializeShell, 1, argv, 1); PID no se usa
	_createProcess(&intializeShell, 1, argv, 1);
	return 0;
}