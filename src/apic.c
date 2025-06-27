#include "apic.h"
#include "cpu.h"
#include "core.h"


void apic_interrupt(struct APIC* apic, uint64_t irq) {
	//char core_id = apic->int_table[(int)id]; // really, why gcc mark char id as warning?
	char core_id = 0;

	if (apic->cpu->cores[(int)core_id].state & INTERRUPTS)
		core_int(&apic->cpu->cores[(int)core_id], irq);
}
