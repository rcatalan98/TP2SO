include Makefile.inc

KERNEL=kernel.bin
SOURCES=$(wildcard *.c)
SOURCES_LIBASM=$(wildcard libasm/*.asm)
SOURCES_LIBC=$(wildcard libc/*.c)
SOURCES_DRIVERS =$(wildcard drivers/*.c)
SOURCES_IDT=$(wildcard idt/*.c)
SOURCES_UTILS=$(wildcard UTILS/*.c)

OBJECTS=$(SOURCES:.c=.o)
OBJECTS_LIBASM=$(SOURCES_LIBASM:.asm=.o)
OBJECTS_LIBC=$(SOURCES_LIBC:.c=.o)
OBJECTS_DRIVERS=$(SOURCES_DRIVERS:.c=.o)
OBJECTS_IDT=$(SOURCES_IDT:.c=.o)
OBJECTS_UTILS=$(SOURCES_UTILS:.c=.o)

LOADERSRC=loader.asm

LOADEROBJECT=$(LOADERSRC:.asm=.o)
STATICLIBS=

all: $(KERNEL)

$(KERNEL): $(LOADEROBJECT) $(OBJECTS) $(STATICLIBS) $(OBJECTS_LIBASM) $(OBJECTS_LIBC) $(OBJECTS_DRIVERS) $(OBJECTS_IDT) $(OBJECTS_UTILS)
	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_LIBASM) $(OBJECTS_LIBC) $(STATICLIBS) $(OBJECTS_DRIVERS) $(OBJECTS_IDT) $(OBJECTS_UTILS)

%.o: %.c
	$(GCC) $(GCCFLAGS) -I./include -I./drivers -I./idt -I./libc -c $< -o $@

%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(LOADEROBJECT):
	$(ASM) $(ASMFLAGS) $(LOADERSRC) -o $(LOADEROBJECT)

clean:
	rm -rf libasm/*.o *.o *.bin drivers/*.o idt/*.o libc/*.o utils/*.o

.PHONY: all clean
