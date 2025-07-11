addn pc, pc, start - $

#define BIOS_OFFSET 0x10000

#include "core.S"
#include "devices.S"


process:

process_var:
	dl 0

process_start:
	addn r1, r0, 0

	loop:
		loaL r2, r1, 0
		addn r2, r2, 1
		stoL r2, r1, 0
	addn pc, pc, loop - $



t5:
	dl t4 + BIOS_OFFSET

t4:
	dl t3 + BIOS_OFFSET

t3:
	dl t2 + BIOS_OFFSET

t2:
	dl t1 + BIOS_OFFSET

t1:
	dl (process + BIOS_OFFSET) | (0b1111 << 60)


start:
	addn r1, pc, t5 - $
	chtp r1

	addn r1, r0, process_start - process
	ktou pc, r1

	addn r1, r0, ENABLED | PAGING | USERMODE
	chst r1

	chst r0
