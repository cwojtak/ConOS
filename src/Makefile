C_SOURCES = $(wildcard kernel/*.c drivers/*.c cpu/*.c libc/*.c)
CPP_SOURCES = $(wildcard kernel/*.cpp drivers/*.cpp cpu/*.cpp libc/*.cpp)
HEADERS = $(wildcard kernel/*.h drivers/*.h cpu/*.h libc/*.c)
CPP_HEADERS = $(wildcard kernel/*.hpp drivers/*.hpp cpu/*.hpp libc/*.hpp)
OBJ =  $(C_SOURCES:.c=.o cpu/interrupt.o)
CPP_OBJ = $(CPP_SOURCES:.cpp=.o)


CC = ${CONOS_CC}
GDB = /usr/bin/gdb

CFLAGS = -g

all: run

kernel.bin: boot/kernel_entry.o ${OBJ} ${CPP_OBJ}
	ld -melf_i386 -o $@ -Ttext 0x10000 $^ --oformat binary

kernel.elf: boot/kernel_entry.o ${OBJ} ${CPP_OBJ}
	ld  -melf_i386 -o $@ -Ttext 0x10000 $^

run: boot/boot.bin kernel.bin kernel.elf
	echo "Built bootloader and kernel!"

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -fno-PIC -m32 -ffreestanding -c $< -o $@

%.o: %.asm
	nasm $< -f elf -o $@

%.o: %.cpp ${CPP_HEADERS}
	g++ -c -std=c++11 -fno-PIC -m32 -ffreestanding -o $@ $<

%.bin: %.asm
	nasm $< -f bin -o $@

clean:
	rm -rf boot/*.bin kernel/*.bin *.bin *.elf drivers/*.bin
	rm -rf kernel/*.o boot/*.o drivers/*.o cpu/*.o cpu/*.bin
	rm -rf libc/*.o libc/*.bin
	rm -rf *.log
	