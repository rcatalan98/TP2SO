
all:  bootloader kernel userland image

bootloader:
	cd Bootloader; make all

kernel:
	cd Kernel; make all

userland:
	cd Userland; make all

image: kernel bootloader userland
	cd Image; make all

clean:
	cd Bootloader; make clean
	cd Image; make clean
	cd Kernel; make clean
	cd Userland; make clean

SOURCES_CPP=$(shell find . -type f -name '*.c')
OUT_CPP=$(SOURCES_CPP:.c=.cpp)

test: cpp pvs

cpp: $(OUT_CPP)

pvs: clean 
	./pvs.sh

%.cpp: %.c
	cppcheck --quiet --enable=all --force --inconclusive $< 2>> output.cppOut

cleanTest:
	rm output.cppOut report.tasks

.PHONY: bootloader image collections kernel userland all clean
