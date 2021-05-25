// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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