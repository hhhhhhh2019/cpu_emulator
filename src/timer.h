#ifndef TIMER_H
#define TIMER_H


#include "motherboard.h"


struct Timer {
	enum Device_type type;
	uint64_t hz;
	char registers[24];
	struct Motherboard* motherboard;
};

void timer_init(struct Timer*, struct Motherboard*, uint64_t hz);
void timer_step(struct Timer*);


#endif // TIMER_H
