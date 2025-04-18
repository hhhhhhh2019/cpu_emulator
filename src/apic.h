#ifndef APIC_H
#define APIC_H


struct CPU;
struct APIC {
	struct CPU* cpu;
	char int_table[256]; // определяет на каком ядре вызывать прерывание
};


void apic_interrupt(struct APIC*, char id);


#endif // APIC_H
