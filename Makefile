FILES = ./build/kernel.asm.o

run:
	qemu-system-x86_64 -hda ./bin/os.bin

build: ./bin/boot.bin ./bin/kernel.bin
	rm -f ./bin/os.bin
	dd if=./bin/boot.bin >> ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin
	dd if=/dev/zero bs=512 count=100 >> ./bin/os.bin
	@echo "Done."

./bin/kernel.bin: $(FILES)
	i686-elf-ld -g -relocatable $(FILES) -o ./build/kernelfull.o
	i686-elf-gcc -T ./src/linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib ./build/kernelfull.o

./bin/boot.bin: ./src/boot/boot.asm
	nasm -f bin ./src/boot/boot.asm -o ./bin/boot.bin

./build/kernel.asm.o: ./src/kernel.asm
	nasm -f elf ./src/kernel.asm -o ./build/kernel.asm.o

clean:
	rm -f ./bin/os.bin
	rm -f ./bin/boot.bin
	rm -f ./bin/kernel.bin
	rm -f ./build/kernelfull.o
	rm -f ${FILES}
	@echo "Done."
