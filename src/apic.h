#ifndef APIC_H
#define APIC_H


#include <stdint.h>


struct CPU;
struct APIC {
	struct CPU* cpu;
	// char int_table[256]; // определяет на каком ядре вызывать прерывание
};


void apic_interrupt(struct APIC*, uint64_t);


#endif // APIC_H
