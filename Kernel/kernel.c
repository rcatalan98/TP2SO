// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 *  kernel.c: Archivo original de barebones. Se limpia lo no utilizado de modo texto.
 */
#include "./include/lib.h"
#include <moduleLoader.h>
#include <idtLoader.h>
#include <naiveConsole.h>
#include <rtcDriver.h>
#include <keyboardDriver.h>
#include <videoDriver.h>
#include <stdio.h>
#include <exceptions.h>
#include <memoryManager.h>
#include <scheduler.h>
#include "include/semaphore.h"
#include "../include/pipe.h"

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

static void *const sampleCodeModuleAddress = (void *)0x400000;
static void *const sampleDataModuleAddress = (void *)0x500000;
static void *const heapBaseAddress = (void *)0x600000;

#define HEAP_SIZE (1024*1024*128) // 128 MB
typedef int (*EntryPoint)();

void clearBSS(void *bssAddress, uint64_t bssSize)
{
	memset(bssAddress, 0, bssSize);
}

void *getStackBase()
{
	return (void *)((uint64_t)&endOfKernel + PageSize * 8 //The size of the stack itself, 32KiB
					- sizeof(uint64_t)					  //Begin at the top of the stack
	);
}

void *initializeKernelBinary()
{
	void *moduleAddresses[] = {
		sampleCodeModuleAddress,
		sampleDataModuleAddress};

	loadModules(&endOfKernelBinary, moduleAddresses);

	clearBSS(&bss, &endOfKernel - &bss);

	return getStackBase();
}

int main()
{
	initialStateSnapshot((uint64_t)sampleCodeModuleAddress, getSP());
	initSems();
	initializeMem(heapBaseAddress, HEAP_SIZE);
	initPipes();
	initializeKeyboard();
	initializeScheduler();
	load_idt();
	// char *argv[] = {"./sample"};
	// createProcess(sampleCodeModuleAddress, 1, argv, 1); // complica el caso donde tenemos la lista de readys vacia
	((EntryPoint)sampleCodeModuleAddress)();
	_sti();
	_hlt();
	print("Warning: Out of Kernel.\n");
	return ((EntryPoint)sampleCodeModuleAddress)();
}
