C_SOURCES = $(wildcard kernel/*.c drivers/*.c cpu/*.c libc/*.c)
CPP_SOURCES = $(wildcard kernel/*.cpp drivers/*.cpp cpu/*.cpp libc/*.cpp)
HEADERS = $(wildcard kernel/*.h drivers/*.h cpu/*.h libc/*.c)
CPP_HEADERS = $(wildcard kernel/*.hpp drivers/*.hpp cpu/*.hpp libc/*.hpp)
OBJ =  $(C_SOURCES:.c=.o cpu/interrupt.o)
CPP_OBJ = $(CPP_SOURCES:.cpp=.o)


CC = /usr/bin/gcc
GDB = /usr/bin/gdb

CFLAGS = -g

all: run

#os-image.bin: boot/boot.bin kernel.bin
#	cat $^ > os-image.bin

os-image.bin: boot/boot.bin kernel.bin
	dd if=/dev/zero of=./os-image.bin bs=64K count=32
	mkfs.vfat -F12 -D0 -f1 -g2/18 -h0 -i0xa0a1a2a3 -M0xf0 -n"ConOS Disk " -r224 -R3 -s1 -S512 -v os-image.bin
	dd conv=notrunc if=boot/boot.bin of=os-image.bin
	sudo mkdir /mnt/ConOS
	sudo mount -o loop os-image.bin /mnt/ConOS
	sudo cp kernel.bin /mnt/ConOS
	sudo umount /mnt/ConOS
	sudo rmdir /mnt/ConOS

kernel.bin: boot/kernel_entry.o ${OBJ} ${CPP_OBJ}
	ld -melf_i386 -o $@ -Ttext 0x1000 $^ --oformat binary

kernel.elf: boot/kernel_entry.o ${OBJ} ${CPP_OBJ}
	ld  -melf_i386 -o $@ -Ttext 0x1000 $^

run: os-image.bin
	qemu-system-x86_64 -serial file:serial.log -hda $<

debug: os-image.bin kernel.elf
	qemu-system-x86_64 -S -s -fda os-image.bin

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
