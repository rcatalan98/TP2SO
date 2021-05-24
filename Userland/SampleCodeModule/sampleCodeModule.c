/**
 *  sampleCodeModule.c: Archivo original de barebones modificado para poder correr shell.
 */
#include "./include/shell.h"

int main()
{
	char *argv[] = {"Shell"};
	_createProcess(&intializeShell, 1, argv, 1, NULL);
	return 0;
}