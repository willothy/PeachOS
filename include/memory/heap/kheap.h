#ifndef KHEAP_H
#define KHEAP_H

#include <stddef.h>
#include <stdint.h>

void kheap_init();
void* kmalloc(int size);
void* kzalloc(int size);
void kfree(void* ptr);

#endif // KHEAP_H
