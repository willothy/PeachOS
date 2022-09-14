#ifndef IDT_H_
#define IDT_H_

#include <stdint.h>

struct idt_desc {
    uint16_t offset_1; // offset bits 0..15
    uint16_t selector; // a code segment selector in GDT or LDT
    uint8_t zero;      // unused, set to 0
    uint8_t type_attr; // type and attributes, see below
    uint16_t offset_2; // offset bits 16..31
} __attribute__((packed));

struct idtr_desc {
    uint16_t limit; // Size of desc table - 1
    uint32_t base;  // Base address of desc table
} __attribute__((packed));

void idt_init();

#endif