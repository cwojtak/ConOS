C_SOURCES = $(wildcard kernel/*.c drivers/*.c cpu/*.c kernel_lib/*.c)
CPP_SOURCES = $(wildcard kernel/*.cpp drivers/*.cpp cpu/*.cpp kernel_lib/*.cpp)
HEADERS = $(wildcard kernel/*.h drivers/*.h cpu/*.h kernel_lib/*.c)
CPP_HEADERS = $(wildcard kernel/*.hpp drivers/*.hpp cpu/*.hpp kernel_lib/*.hpp)
OBJ =  $(C_SOURCES:.c=.o cpu/interrupt.o)
CPP_OBJ = $(CPP_SOURCES:.cpp=.o)


CC = ${CONOS_CC}
LD = ${CONOS_LD}

all: run

kernel.bin: boot/kernel_entry.o ${OBJ} ${CPP_OBJ}
	${LD} -melf_i386 -o $@ -Ttext 0x10000 $^ --oformat binary

kernel.elf: boot/kernel_entry.o ${OBJ} ${CPP_OBJ}
	${LD} -melf_i386 -o $@ -Ttext 0x10000 $^

run: boot/boot.bin kernel.bin kernel.elf
	echo "Built bootloader and kernel!"

%.o: %.c ${HEADERS}
	${CC} -g -fno-PIC -m32 -ffreestanding -c $< -o $@

%.o: %.asm
	nasm $< -f elf -o $@

%.o: %.cpp ${CPP_HEADERS}
	g++ -c -std=c++11 -fno-PIC -m32 -ffreestanding -o $@ $<

%.bin: %.asm
	nasm $< -f bin -o $@

clean:
	rm -rf boot/*.bin kernel/*.bin *.bin *.elf drivers/*.bin
	rm -rf kernel/*.o boot/*.o drivers/*.o cpu/*.o cpu/*.bin
	rm -rf kernel_lib/*.o kernel_lib/*.bin
	rm -rf *.log
	
