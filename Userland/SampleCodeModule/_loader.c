// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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
