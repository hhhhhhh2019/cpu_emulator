#include "apic.h"
#include "cpu.h"
#include "core.h"


void apic_interrupt(struct APIC* apic, char id) {
	char cpu_id = apic->int_table[(int)id]; // really, why gcc mark char id as warning?

	if (apic->cpu->cores[(int)cpu_id].state & INTERRUPTS)
		core_int(&apic->cpu->cores[(int)cpu_id], id);
}
