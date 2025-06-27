addn pc, pc, start - $

#define BIOS_OFFSET 0x10000

#include "core.S"
#include "devices.S"


start:
	// add timer mmio
	addn r1, r0, MMU_MMIO_ADDR

	addn r2, r0, TIMER_DEVICE_ID
	stoS r2, r1, MMU_REG_DEVICEID

	addn r2, r0, TIMER_MMIO_ADDR
	stoI r2, r1, MMU_REG_STARTADDR

	addn r2, r0, 24
	stoI r2, r1, MMU_REG_SIZE

	addn r2, r0, MMU_CMD_ADD
	stoB r2, r1, MMU_REG_CMD


	// setup timer
	addn r1, r0, TIMER_MMIO_ADDR

	addn r2, pc, timer_irq - $
	stoL r2, r1, TIMER_REG_INTERRUPT

	// enable timer
	addn r2, r0, 10
	stoL r2, r1, TIMER_REG_DELAY


	// enable interrupts
	addn r1, r0, ENABLED | INTERRUPTS
	chst r1

	add pc, pc, r0


timer_irq:
	addn r10, r10, 1

	addn r2, r0, 10
	stoL r2, r0, TIMER_MMIO_ADDR + TIMER_REG_DELAY

	iret
