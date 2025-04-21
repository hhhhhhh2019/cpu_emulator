#include "timer.h"
#include "apic.h"
#include "core.h"
#include "motherboard.h"

#include <stdio.h>


void timer_init(struct Timer* timer, struct Motherboard* motherboard, unsigned long hz) {
	for (int i = 0; i < 17; i++)
		timer->registers[i] = 0;
	timer->type = TIMER;
	timer->motherboard = motherboard;
	timer->hz = hz;

	timer->registers[9]  = (timer->hz >> 0 * 8) & 0xff;
	timer->registers[10] = (timer->hz >> 1 * 8) & 0xff;
	timer->registers[11] = (timer->hz >> 2 * 8) & 0xff;
	timer->registers[12] = (timer->hz >> 3 * 8) & 0xff;
	timer->registers[13] = (timer->hz >> 4 * 8) & 0xff;
	timer->registers[14] = (timer->hz >> 5 * 8) & 0xff;
	timer->registers[15] = (timer->hz >> 6 * 8) & 0xff;
	timer->registers[16] = (timer->hz >> 7 * 8) & 0xff;
}


void timer_step(struct Timer* timer) {
	timer->registers[9]  = (timer->hz >> 0 * 8) & 0xff;
	timer->registers[10] = (timer->hz >> 1 * 8) & 0xff;
	timer->registers[11] = (timer->hz >> 2 * 8) & 0xff;
	timer->registers[12] = (timer->hz >> 3 * 8) & 0xff;
	timer->registers[13] = (timer->hz >> 4 * 8) & 0xff;
	timer->registers[14] = (timer->hz >> 5 * 8) & 0xff;
	timer->registers[15] = (timer->hz >> 6 * 8) & 0xff;
	timer->registers[16] = (timer->hz >> 7 * 8) & 0xff;

	unsigned long delay =
		((long)timer->registers[0] << 0 * 8) |
		((long)timer->registers[1] << 1 * 8) |
		((long)timer->registers[2] << 2 * 8) |
		((long)timer->registers[3] << 3 * 8) |
		((long)timer->registers[4] << 4 * 8) |
		((long)timer->registers[5] << 5 * 8) |
		((long)timer->registers[6] << 6 * 8) |
		((long)timer->registers[7] << 7 * 8);

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
		apic_interrupt(&timer->motherboard->cpu.apic, timer->registers[8]);
}
