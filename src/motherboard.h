#ifndef MOTHERBOARD_H
#define MOTHERBOARD_H


#include "cpu.h"

#include <stdint.h>


struct Motherboard {
	uint64_t ram_size;
	uint8_t* ram;

	uint64_t bios_size;
	uint8_t* bios;

	struct CPU cpu;
};


#endif // MOTHERBOARD_H
