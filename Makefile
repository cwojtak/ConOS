C_SOURCES = $(wildcard kernel/*.c drivers/*.c cpu/*.c libc/*.c)
CPP_SOURCES = $(wildcard kernel/*.cpp drivers/*.cpp cpu/*.cpp libc/*.cpp)
HEADERS = $(wildcard kernel/*.h drivers/*.h cpu/*.h libc/*.c)
CPP_HEADERS = $(wildcard kernel/*.hpp drivers/*.hpp cpu/*.hpp libc/*.hpp)
OBJ =  $(C_SOURCES:.c=.o cpu/interrupt.o)


CC = /usr/bin/gcc
GDB = /usr/bin/gdb

CFLAGS = -g

all: run

os-image.bin: boot/boot.bin kernel.bin
	cat $^ > os-image.bin

kernel.bin: boot/kernel_entry.o ${OBJ}
	ld -melf_i386 -o $@ -Ttext 0x1000 $^ --oformat binary

kernel.elf: boot/kernel_entry.o ${OBJ}
	ld -melf_i386 -o $@ -Ttext 0x1000 $^

run: os-image.bin
	qemu-system-x86_64 -serial file:serial.log -fda $<

debug: os-image.bin kernel.elf
	qemu-system-x86_64 -S -s -fda os-image.bin

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -fno-pie -m32 -ffreestanding -c $< -o $@

%.o: %.asm
	nasm $< -f elf -o $@

%.o: %.cpp ${CPP_HEADERS}
	g++ -c -std=c++0x -m32 -ffreestanding -o $< $@

%.bin: %.asm
	nasm $< -f bin -o $@

clean:
	rm -rf boot/*.bin kernel/*.bin kernel.bin *.elf drivers/*.bin
	rm -rf kernel/*.o boot/*.o drivers/*.o cpu/*.o cpu/*.bin
	rm -rf libc/*.o libc/*.bin
