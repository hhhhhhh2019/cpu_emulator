#include "core.h"
#include "mmu.h"
#include "motherboard.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>


#define BIOS_OFFSET 0x10000


struct Motherboard motherboard;


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
	mmu_init(&motherboard.cpu.mmu, &motherboard.cpu);


	// APIC init
	for (int i = 0; i < 256; i++)
		motherboard.cpu.apic.int_table[i] = 0;

	motherboard.devices = realloc(
		motherboard.devices,
		sizeof(void*) * (++motherboard.devices_count)
	);
	((struct Device*)motherboard.devices[motherboard.devices_count - 1])->type = APIC;
	((struct Device*)motherboard.devices[motherboard.devices_count - 1])->registers = motherboard.cpu.apic.int_table;


	// CPU init
	for (int i = 0; i < motherboard.cpu.cores_number; i++) {
		motherboard.cpu.cores[i].cpu = &motherboard.cpu;
		for (int j = 0; j < 18; j++)
			motherboard.cpu.cores[i].registersk[j] = 0;
		motherboard.cpu.cores[i].int_queue_head = 0;
		motherboard.cpu.cores[i].int_queue_tail = 0;
	}
	motherboard.cpu.cores[0].state = ENABLED;
	motherboard.cpu.cores[0].registersk[PC] = BIOS_OFFSET;


	while (1) {
		getchar();
		mmu_step(&motherboard.cpu.mmu);
		for (int i = 0; i < motherboard.cpu.cores_number; i++)
			core_step(&motherboard.cpu.cores[i]);
	}
}
