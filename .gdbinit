add-symbol-file ./build/kernelfull.o 0x100000
break _start
target remote | qemu-system-i386 -S -gdb stdio -hda ./bin/os.bin
