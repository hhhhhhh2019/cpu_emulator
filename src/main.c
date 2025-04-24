#include "core.h"
#include "mmu.h"
#include "motherboard.h"
#include "timer.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>


#define BIOS_OFFSET 0x10000


struct Motherboard motherboard;

#define ADD_DEVICE(device) \
	motherboard.devices = realloc( \
		motherboard.devices, \
		sizeof(struct Device*) * (++motherboard.devices_count) \
	); \
	motherboard.devices[motherboard.devices_count - 1] = (struct Device*)device; \


static unsigned long gcd(unsigned long a, unsigned long b) {
	if (a % b == 0)
		return b;
	if (b % a == 0)
		return a;
	if (a > b)
		return gcd(a % b, a);
	else
		return gcd(b, b % a);
}


int main() {
	char* bios_filename = "bios";

	motherboard.ram_size = 1024 * 1024 * 1024;  // 1 GiB
	motherboard.ram = malloc(motherboard.ram_size);

	FILE* bios_fd = fopen(bios_filename, "rb");

	if (bios_fd == NULL) {
		perror("fopen");
		return errno;
	}

	fseek(bios_fd, 0, SEEK_END);
	motherboard.bios_size = ftell(bios_fd);
	rewind(bios_fd);
	motherboard.bios = malloc(motherboard.bios_size);
	fread(motherboard.bios, motherboard.bios_size, 1, bios_fd);
	fclose(bios_fd);

	memcpy(motherboard.ram + BIOS_OFFSET, motherboard.bios, motherboard.bios_size);


	// MMU init
	motherboard.cpu = (struct CPU){
		.motherboard = &motherboard,
		.cores_number = 1,
		.cores = malloc(sizeof(struct Core) * 1),
	};
	mmu_init(&motherboard.cpu.mmu, &motherboard.cpu, 1000);


	// APIC init
	for (int i = 0; i < 256; i++)
		motherboard.cpu.apic.int_table[i] = 0;
	motherboard.cpu.apic.cpu = &motherboard.cpu;

	ADD_DEVICE(malloc(sizeof(struct Device)));
	((struct Device*)motherboard.devices[motherboard.devices_count - 1])->type = APIC;
	((struct Device*)motherboard.devices[motherboard.devices_count - 1])->hz = 1000;
	((struct Device*)motherboard.devices[motherboard.devices_count - 1])->registers = motherboard.cpu.apic.int_table;


	// CPU init
	motherboard.cpu.cores[0].hz = 1000;
	for (int i = 0; i < motherboard.cpu.cores_number; i++) {
		motherboard.cpu.cores[i].cpu = &motherboard.cpu;
		for (int j = 0; j < 18; j++)
			motherboard.cpu.cores[i].registersk[j] = 0;
		motherboard.cpu.cores[i].int_queue_head = 0;
		motherboard.cpu.cores[i].int_queue_tail = 0;
	}
	motherboard.cpu.cores[0].state = ENABLED;
	motherboard.cpu.cores[0].registersk[PC] = BIOS_OFFSET;


	// devices

	// timer
	struct Timer* timer = malloc(sizeof(struct Timer));
	timer_init(timer, &motherboard, 10000);
	ADD_DEVICE(timer);


	// setup hz

	int hz_count = 1 + motherboard.devices_count + motherboard.cpu.cores_number;
	unsigned long** hz = malloc(sizeof(unsigned long*) * hz_count);
	hz[0] = &motherboard.cpu.mmu.hz;
	for (int i = 0; i < motherboard.cpu.cores_number; i++)
		hz[i + 1] = &motherboard.cpu.cores[i].hz;
	for (int i = 0; i < motherboard.devices_count; i++)
		hz[i + 1 + motherboard.cpu.cores_number] = &motherboard.devices[i]->hz;

	int del = *hz[0];

	for (int i = 1; i < hz_count; i++)
		del = gcd(del, *hz[i]);

	for (int i = 0; i < hz_count; i++)
		*hz[i] = *hz[i] / del;


	unsigned long ticks_in_sec = 1;

	for (int i = 0; i < hz_count; i++)
		ticks_in_sec *= *hz[i];


	unsigned long tick = 0;

	while (1) {
		getchar();

		for (int i = 0; i < motherboard.devices_count; i++) {
			printf("%d: %lu\n", i, tick % (ticks_in_sec / motherboard.devices[i]->hz));
			if (tick % (ticks_in_sec / motherboard.devices[i]->hz) != 0)
				continue;

			switch (motherboard.devices[i]->type) {
				case (APIC): break;
				case (TIMER): timer_step((struct Timer*)motherboard.devices[i]); break;
			}
		}

		if (tick % (ticks_in_sec / motherboard.cpu.mmu.hz) == 0)
			mmu_step(&motherboard.cpu.mmu);

		for (int i = 0; i < motherboard.cpu.cores_number; i++) {
			if (tick % (ticks_in_sec / motherboard.cpu.cores[i].hz) == 0)
				core_step(&motherboard.cpu.cores[i]);
		}

		tick++;
	}
}
