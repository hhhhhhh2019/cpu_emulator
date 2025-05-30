#ifndef CPU_H
#define CPU_H


#include "core.h"
#include "mmu.h"
#include "apic.h"


struct Motherboard;
struct CPU {
	struct Motherboard* motherboard;

	int cores_number;
	struct Core* cores;

	struct MMU mmu;
	struct APIC apic;
};


#endif // CPU_H
