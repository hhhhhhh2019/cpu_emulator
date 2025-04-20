#ifndef TIMER_H
#define TIMER_H


#include "motherboard.h"


struct Timer {
	enum Device_type type;
	unsigned long hz;
	char registers[17];
	struct Motherboard* motherboard;
};

void timer_init(struct Timer*, struct Motherboard*, unsigned long hz);
void timer_step(struct Timer*);


#endif // TIMER_H
