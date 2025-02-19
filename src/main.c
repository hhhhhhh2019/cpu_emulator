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

	motherboard.cpu = (struct CPU){
		.motherboard = &motherboard,
		.cores_number = 1,
		.cores = malloc(sizeof(struct Core) * 1),
	};
	mmu_init(&motherboard.cpu.mmu, &motherboard.cpu);

	for (int i = 0; i < motherboard.cpu.cores_number; i++)
		motherboard.cpu.cores[i].cpu = &motherboard.cpu;
	motherboard.cpu.cores[0].state = ENABLED;
	for (int i = 0; i < 18; i++)
		motherboard.cpu.cores[0].registersk[i] = 0;
	motherboard.cpu.cores[0].registersk[PC] = BIOS_OFFSET;

	while (1) {
		getchar();
		for (int i = 0; i < motherboard.cpu.cores_number; i++)
			core_step(&motherboard.cpu.cores[i]);
	}
}
