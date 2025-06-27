#include "timer.h"
#include "apic.h"
#include "core.h"
#include "motherboard.h"

#include <stdio.h>
#include <string.h>


void timer_init(struct Timer* timer, struct Motherboard* motherboard, uint64_t hz) {
	memset(timer->registers, 0, sizeof(timer->registers));
	timer->type = TIMER;
	timer->motherboard = motherboard;
	timer->hz = hz;

	timer->registers[16] = (timer->hz >> 0 * 8) & 0xff;
	timer->registers[17] = (timer->hz >> 1 * 8) & 0xff;
	timer->registers[18] = (timer->hz >> 2 * 8) & 0xff;
	timer->registers[19] = (timer->hz >> 3 * 8) & 0xff;
	timer->registers[20] = (timer->hz >> 4 * 8) & 0xff;
	timer->registers[21] = (timer->hz >> 5 * 8) & 0xff;
	timer->registers[22] = (timer->hz >> 6 * 8) & 0xff;
	timer->registers[23] = (timer->hz >> 7 * 8) & 0xff;
}


void timer_step(struct Timer* timer) {
	timer->registers[16] = (timer->hz >> 0 * 8) & 0xff;
	timer->registers[17] = (timer->hz >> 1 * 8) & 0xff;
	timer->registers[18] = (timer->hz >> 2 * 8) & 0xff;
	timer->registers[19] = (timer->hz >> 3 * 8) & 0xff;
	timer->registers[20] = (timer->hz >> 4 * 8) & 0xff;
	timer->registers[21] = (timer->hz >> 5 * 8) & 0xff;
	timer->registers[22] = (timer->hz >> 6 * 8) & 0xff;
	timer->registers[23] = (timer->hz >> 7 * 8) & 0xff;

	uint64_t delay =
		((uint64_t)timer->registers[0] << 0 * 8) |
		((uint64_t)timer->registers[1] << 1 * 8) |
		((uint64_t)timer->registers[2] << 2 * 8) |
		((uint64_t)timer->registers[3] << 3 * 8) |
		((uint64_t)timer->registers[4] << 4 * 8) |
		((uint64_t)timer->registers[5] << 5 * 8) |
		((uint64_t)timer->registers[6] << 6 * 8) |
		((uint64_t)timer->registers[7] << 7 * 8);


	uint64_t irq =
		((uint64_t)timer->registers[8]  << 0 * 8) |
		((uint64_t)timer->registers[9]  << 1 * 8) |
		((uint64_t)timer->registers[10] << 2 * 8) |
		((uint64_t)timer->registers[11] << 3 * 8) |
		((uint64_t)timer->registers[12] << 4 * 8) |
		((uint64_t)timer->registers[13] << 5 * 8) |
		((uint64_t)timer->registers[14] << 6 * 8) |
		((uint64_t)timer->registers[15] << 7 * 8);

	if (delay == 0)
		return;

	delay -= 1;

	printf("timer: %lu\n", delay);

	timer->registers[0] = (delay >> 0 * 8) & 0xff;
	timer->registers[1] = (delay >> 1 * 8) & 0xff;
	timer->registers[2] = (delay >> 2 * 8) & 0xff;
	timer->registers[3] = (delay >> 3 * 8) & 0xff;
	timer->registers[4] = (delay >> 4 * 8) & 0xff;
	timer->registers[5] = (delay >> 5 * 8) & 0xff;
	timer->registers[6] = (delay >> 6 * 8) & 0xff;
	timer->registers[7] = (delay >> 7 * 8) & 0xff;

	if (delay == 0)
		apic_interrupt(&timer->motherboard->cpu.apic, irq);
}
