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


void mmu_init(struct MMU* mmu, struct CPU* cpu, uint64_t hz) {
	mmu->cpu = cpu;
	mmu->hz = hz;

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
		printf("MMU reset devices\n");
		mmu->mmio_count = 0;
		free(mmu->mmio);
		mmu->mmio = malloc(0);

		mmu_add_self(mmu);
	}

	else if (cmd == MMU_CMD_ADD) {
		printf("MMU map device: %d %d %d\n", DEVICEID, addrstart, size);
		mmu->mmio = realloc(mmu->mmio, sizeof(struct MMIO) * (++mmu->mmio_count));
		mmu->mmio[mmu->mmio_count - 1] = (struct MMIO){
			.addr_start = addrstart,
			.size = size,
			.registers =
			    (char*)&((struct Device*)mmu->cpu->motherboard->devices[DEVICEID])->registers
			// честно, я не понимаю почему надо брать ссылку на registers,
			// потому что registers и так должен быть ссылкой, но без этого выпадает segfault
		};
	}
}


uint64_t mmu_read(struct MMU* mmu, char vaddr, uint64_t tp, uint64_t addr, char* perm) {
	if (vaddr) {
		// TODO
	}

	for (int i = 0; i < mmu->mmio_count; i++) {
		if (addr < mmu->mmio[i].addr_start || addr >= mmu->mmio[i].addr_start + mmu->mmio[i].size)
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
		if (addr < mmu->mmio[i].addr_start || addr >= mmu->mmio[i].addr_start + mmu->mmio[i].size)
			continue;

		addr -= mmu->mmio[i].addr_start;

		for (int j = 0; j < size / 8; j++)
			mmu->mmio[i].registers[addr + j] = (value >> (j * 8)) & 0xff;

		return;
	}

	for (int j = 0; j < size / 8; j++) {
		printf("write: %02lx to %lx\n", (value >> (j * 8)) & 0xff, addr + j);
		mmu->cpu->motherboard->ram[addr + j] = (value >> (j * 8)) & 0xff;
	}
}
