/**
 *  _loader.c: Implementación de barebones sin modificar.
*/
#include <stdint.h>
#include "./include/stdlib.h"
extern char bss;
extern char endOfBinary;

int main();

int _start()
{
	//Clean BSS
	memset(&bss, 0, &endOfBinary - &bss);

	return main();
}
