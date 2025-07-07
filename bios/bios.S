addn pc, pc, start - $

#define BIOS_OFFSET 0x10000

#include "core.S"
#include "devices.S"


start:
	// enable interrupts

	addn r1, r0, ENABLED | INTERRUPTS
	chst r1

	addn r1, pc, syscall_irq - $
	setsyscall r1

	syscall

	addn pc, pc, 0


syscall_irq:
	chst r0
