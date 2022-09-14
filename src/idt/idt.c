#include "idt.h"
#include "config.h"
#include "io.h"
#include "memory.h"
#include "vga_writer.h"

struct idt_desc idt_descriptors[PEACHOS_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

extern void idt_load(void *ptr);
extern void int21h();
extern void no_interrupt();

void no_interrupt_handler() { outb(0x20, 0x20); }

void int21h_handler() {
    // Int 21h
    vga_print("Key pressedn\n", VGA_WHITE);
    outb(0x20, 0x20);
}

void idt_zero() {
    // log the error
    vga_print("Divide by zero error", VGA_RED);
}

void idt_set(int interrupt_no, void *address) {
    struct idt_desc *desc = &idt_descriptors[interrupt_no];
    desc->offset_1 = (uint32_t)address & 0x0000ffff;
    desc->selector = KERNEL_CODE_SELECTOR;
    desc->zero = 0x00;
    desc->type_attr = 0xEE; // Set P, DPL, Storage Segment, and Type
    desc->offset_2 = (uint32_t)address >> 16;
}

void idt_init() {
    memset(idt_descriptors, 0, sizeof(idt_descriptors));
    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
    idtr_descriptor.base = (uint32_t)idt_descriptors;

    for (int i = 0; i < PEACHOS_TOTAL_INTERRUPTS; i++)
        idt_set(i, no_interrupt);

    idt_set(0, idt_zero);
    idt_set(0x21, int21h);

    // load idt descriptor table
    idt_load(&idtr_descriptor);
}