/**
 *  sampleCodeModule.c: Archivo original de barebones modificado para poder correr shell.
 */
#include "./include/shell.h"

static void dummy()
{
	int i = 10000;
	while ((i--) > 0){
		print("fn\n");
	}
}

int main()
{
	char *argv2[] = {"Dummy"};
	int pid2 = _createProcess(&dummy, 1, argv2);
	_block(pid2);
	char *argv[] = {"Shell"};
	int pid = _createProcess(&intializeShell, 1, argv);
	// intializeShell();
	return 0;
}