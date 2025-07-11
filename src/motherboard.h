#ifndef MOTHERBOARD_H
#define MOTHERBOARD_H


#include "cpu.h"

#include <stdint.h>


enum Device_type {
	APIC,
	TIMER,
};

struct Device {
	enum Device_type type;
	uint64_t hz;
	uint8_t registers[];
};


struct Motherboard {
	uint64_t ram_size;
	uint8_t* ram;

	uint64_t bios_size;
	uint8_t* bios;

	struct CPU cpu;

	int devices_count;
	struct Device** devices;
};


#endif // MOTHERBOARD_H
