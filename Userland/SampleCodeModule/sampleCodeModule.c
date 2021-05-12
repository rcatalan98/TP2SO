/**
 *  sampleCodeModule.c: Archivo original de barebones modificado para poder correr shell.
 */
#include "./include/shell.h"

static void dummy()
{
	int i = 100000000000000000000;
	while (1){
		sleep(5);
		print("fn\n");
	}
	print("Done\n");
}

int main()
{
	// char *argv2[] = {"Dummy"};
	// int pid2 = _createProcess(&dummy, 1, argv2);
	// _kill(pid2);
	char *argv[] = {"Shell"};
	int pid = _createProcess(&intializeShell, 1, argv);
	// _kill(pid);
	print("fn\n");
	// intializeShell();
	return 0;
}