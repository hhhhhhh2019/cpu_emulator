#include "apic.h"
#include "cpu.h"
#include "core.h"


void apic_interrupt(struct APIC* apic, char id) {
	char cpu_id = apic->int_table[id];

	if (apic->cpu->cores[cpu_id].state & INTERRUPTS)
		core_int(&apic->cpu->cores[cpu_id], id);
}
