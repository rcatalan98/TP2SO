/**
 *  sampleCodeModule.c: Archivo original de barebones modificado para poder correr shell.
 */
#include "./include/shell.h"

static void dummy()
{
	int i = 100000000000000000000;
	while ((i--) > 0){
		//print("fn\n");
	}
	print("Done\n");
}

int main()
{
	//char *argv2[] = {"Dummy"};
	//int pid2 = _createProcess(&dummy, 1, argv2);
	//_block(pid2);
	char *argv[] = {"Shell"};
	int pid = _createProcess(&intializeShell, 1, argv);
	print("fn\n");
	// intializeShell();
	return 0;
}