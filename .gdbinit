add-symbol-file ./build/kernelfull.o 0x100000
break _start
target remote | qemu-system-x86_64 -S -gdb stdio -hda ./bin/os.bin
