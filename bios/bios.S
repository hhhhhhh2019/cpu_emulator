addn pc, pc, start - $

#define BIOS_OFFSET 0x10000

#include "core.S"
#include "devices.S"


vectors:
	dl irq0
	dl irq1
	dl irq2
vectors_end:


start:
	// write interrupt table to memory
	addn r1, pc, vectors - $
	add r2, r0, r0
	addn r3, pc, vectors_end - $
	addn r4, r0, 8

ivt_loop:
	loaL r5, r1, 0
	addn r5, r5, BIOS_OFFSET
	stoL r5, r2, 0

	add r1, r1, r4
	add r2, r2, r4

	sub r0, r1, r3
	addz pc, pc, ivt_loop_end - $
	addn pc, pc, ivt_loop - $
ivt_loop_end:

	// add TIMER MMIO
	addn r1, r0, MMU_MMIO_ADDR

	addn r2, r0, TIMER_DEVICE_ID
	stoS r2, r1, MMU_REG_DEVICEID

	addn r2, r0, TIMER_MMIO_ADDR
	stoS r2, r1, MMU_REG_STARTADDR

	addn r2, r0, 17
	stoS r2, r1, MMU_REG_SIZE

	addn r2, r0, MMU_CMD_ADD
	stoB r2, r1, MMU_REG_CMD

	// enable interrupts

	addn r1, r0, ENABLED | INTERRUPTS
	chst r1


	// set timer

	addn r1, r0, TIMER_MMIO_ADDR

	addn r2, r0, 1
	stoB r2, r1, TIMER_REG_INTERRUPT

	addn r2, r0, 10
	stoL r2, r1, TIMER_REG_DELAY

loop:
	addn pc, pc, loop - $


irq0:
	addn r1, r0, 1
	chst r0

irq1:
	addn r1, r0, 2
	chst r0

irq2:
	addn r1, r0, 3
	chst r0
