#include "mmu.h"
#include "cpu.h"
#include "motherboard.h"

#include <stdlib.h>
#include <stdio.h>


static inline void mmu_add_self(struct MMU* mmu) {
	mmu->mmio = realloc(mmu->mmio, sizeof(struct MMIO) * (++mmu->mmio_count));
	mmu->mmio[mmu->mmio_count - 1] = (struct MMIO){
		.addr_start = MMU_MMIO_ADDR,
		.size = sizeof(mmu->registers),
		.registers = mmu->registers
	};
}


void mmu_init(struct MMU* mmu, struct CPU* cpu) {
	mmu->cpu = cpu;

	mmu->mmio_count = 0;
	mmu->mmio = malloc(0);

	mmu_add_self(mmu);
}


void mmu_step(struct MMU* mmu) {
	unsigned char	cmd		  = *(unsigned char*)(mmu->registers + MMU_REG_CMD);
	unsigned int	addrstart = *(unsigned int*)(mmu->registers + MMU_REG_STARTADDR);
	unsigned int	size	  = *(unsigned int*)(mmu->registers + MMU_REG_SIZE);
	unsigned short	DEVICEID  = *(unsigned short*)(mmu->registers + MMU_REG_DEVICEID);

	if (cmd == 0)
		return;

	mmu->registers[0] = 0;

	if (cmd == MMU_CMD_RESET) {
		mmu->mmio_count = 0;
		free(mmu->mmio);
		mmu->mmio = malloc(0);

		mmu_add_self(mmu);
	}

	else if (cmd == MMU_CMD_ADD) {
		mmu->mmio = realloc(mmu->mmio, sizeof(struct MMIO) * (++mmu->mmio_count));
		mmu->mmio[mmu->mmio_count - 1] = (struct MMIO){
			.addr_start = addrstart,
			.size = size,
			/* .registers = */
		};
	}
}


uint64_t mmu_read(struct MMU* mmu, char vaddr, uint64_t tp, uint64_t addr, char* perm) {
	if (vaddr) {
		// TODO
	}

	for (int i = 0; i < mmu->mmio_count; i++) {
		if (mmu->mmio[i].addr_start < addr || addr >= mmu->mmio[i].addr_start + mmu->mmio[i].size)
			continue;

		addr -= mmu->mmio[i].addr_start;

		return *(uint64_t*)(mmu->mmio[i].registers + addr); // TODO: check for out of bound
	}

	printf("read: %016lx from %lx\n", *(uint64_t*)(mmu->cpu->motherboard->ram + addr), addr);

	return *(uint64_t*)(mmu->cpu->motherboard->ram + addr);
}


void mmu_write(struct MMU* mmu, char vaddr, uint64_t tp, uint64_t addr, char size, uint64_t value) {
	if (vaddr) {
		// TODO
	}

	for (int i = 0; i < mmu->mmio_count; i++) {
		if (mmu->mmio[i].addr_start < addr || addr >= mmu->mmio[i].addr_start + mmu->mmio[i].size)
			continue;

		addr -= mmu->mmio[i].addr_start;

		for (int j = 0; j < size; j++)
			mmu->mmio[i].registers[addr + j] = (value >> (j * 8)) & 0xff;
	}

	for (int j = 0; j < size; j++) {
		mmu->cpu->motherboard->ram[addr + j] = (value >> (j * 8)) & 0xff;
		printf("write: %02lx to %lx\n", (value >> (j * 8)) & 0xff, addr + j);
	}
}
