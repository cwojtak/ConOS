all:
	make clean
	make run

run: build/os-image.bin
	qemu-system-x86_64 -serial file:serial.log -hda $<

debug: build/os-image.bin
	qemu-system-x86_64 -S -s -serial file:serial.log -hda $<

build/os-image.bin:
	(cd src && make)
	dd if=/dev/zero of=build/os-image.bin bs=64K count=32
	mkfs.vfat -F12 -D0 -f1 -g2/18 -h0 -i0xa0a1a2a3 -M0xf0 -n"ConOS Disk " -r224 -R3 -s1 -S512 -v build/os-image.bin
	dd conv=notrunc if=src/boot/boot.bin of=build/os-image.bin
	sudo mkdir /mnt/ConOS
	sudo mount -o loop build/os-image.bin /mnt/ConOS
	sudo cp src/kernel.bin /mnt/ConOS
	sudo cp -r filesystem_addons/* /mnt/ConOS
	sudo umount /mnt/ConOS
	sudo rmdir /mnt/ConOS

clean:
	(cd src && make clean)
	rm -rf build/*.bin
	rm -rf *.log