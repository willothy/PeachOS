#FILES = ./build/kernel.asm.o ./build/vga_writer.o ./build/kernel.o ./build/idt/idt.asm.o ./build/idt/idt.o ./build/memory/memory.o ./build/memory/paging/paging.o ./build/memory/paging/paging.asm.o ./build/memory/heap/heap.o ./build/memory/heap/kheap.o ./build/io/io.asm.o
SRCS = $(shell find src/ -type f ! -name '*.ld' ! -path '*boot/*' | sed 's|^src/||')
FILES = $(patsubst %.asm, %.asm.o, $(patsubst %.c, %.o,$(addprefix build/,$(SRCS))))
#INCLUDES = -I./include -I./include/idt -I./include/memory
INCLUDES = $(addprefix -I,$(shell find ./include -type d -print))
BUILDDIRS = $(addprefix build/,$(shell find src/ -type d ! -path '*boot' | sed 's|^src/||'))
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

run:
	qemu-system-x86_64 -smp cpus=2,cores=2 -hda ./bin/os.bin


build: $(BUILDDIRS) ./bin/boot.bin ./bin/kernel.bin
	rm -f ./bin/os.bin
	dd if=./bin/boot.bin >> ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin
	dd if=/dev/zero bs=512 count=100 >> ./bin/os.bin
	@echo "Done."

$(BUILDDIRS):
	mkdir -p ./build
	find src/ -type d ! -path '*boot' | sed 's|^src/||' | xargs -I{} mkdir -p "./build/{}"

./bin/kernel.bin: $(FILES)
	i686-elf-ld -g -relocatable $(FILES) -o ./build/kernelfull.o
	i686-elf-gcc $(FLAGS) -T ./src/linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib ./build/kernelfull.o

./bin/boot.bin: ./src/boot/boot.asm
	nasm -f bin ./src/boot/boot.asm -o ./bin/boot.bin

./build/kernel.asm.o: ./src/kernel.asm
	nasm -f elf ./src/kernel.asm -o ./build/kernel.asm.o

./build/idt/idt.asm.o: ./src/idt/idt.asm
	nasm -f elf ./src/idt/idt.asm -o ./build/idt/idt.asm.o

./build/vga_writer.o: ./src/vga_writer.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/vga_writer.c -o ./build/vga_writer.o

./build/idt/idt.o: ./src/idt/idt.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/idt/idt.c -o ./build/idt/idt.o

./build/memory/memory.o: ./src/memory/memory.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/memory/memory.c -o ./build/memory/memory.o

./build/memory/heap/heap.o: ./src/memory/heap/heap.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/memory/heap/heap.c -o ./build/memory/heap/heap.o

./build/memory/heap/kheap.o: ./src/memory/heap/kheap.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/memory/heap/kheap.c -o ./build/memory/heap/kheap.o

./build/memory/paging/paging.o: ./src/memory/paging/paging.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/memory/paging/paging.c -o ./build/memory/paging/paging.o

./build/memory/paging/paging.asm.o: ./src/memory/paging/paging.asm
	nasm -f elf ./src/memory/paging/paging.asm -o ./build/memory/paging/paging.asm.o

./build/io/io.asm.o: ./src/io/io.asm
	nasm -f elf ./src/io/io.asm -o ./build/io/io.asm.o

./build/kernel.o: ./src/kernel.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/kernel.c -o ./build/kernel.o

clean:
	rm -f ./bin/*.bin
	rm -rf ./build/**
	rm -f ${FILES}
	@echo "Done."
