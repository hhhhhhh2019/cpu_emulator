#include "mmu.h"
#include "core.h"
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
			    ((struct Device*)mmu->cpu->motherboard->devices[DEVICEID])->registers
		};
	}
}


uint64_t virt_to_phys(struct MMU* mmu, uint64_t vaddr, uint64_t tp, char* perm) {
	uint64_t offset = (vaddr >>  0) & 0xffff;
	uint64_t t1     = (vaddr >> 16) & 0xffff;
	uint64_t t2     = (vaddr >> 24) & 0xffff;
	uint64_t t3     = (vaddr >> 34) & 0xffff;
	uint64_t t4     = (vaddr >> 44) & 0xffff;
	uint64_t t5     = (vaddr >> 54) & 0xffff;

	uint8_t* ram = mmu->cpu->motherboard->ram;

	uint64_t t5_addr   = tp;
	uint64_t t4_addr   = *(uint64_t*)(ram + t5_addr + t5 * 8); // i hope this will work everywhere
	uint64_t t3_addr   = *(uint64_t*)(ram + t4_addr + t4 * 8);
	uint64_t t2_addr   = *(uint64_t*)(ram + t3_addr + t3 * 8);
	uint64_t t1_addr   = *(uint64_t*)(ram + t2_addr + t2 * 8);
	uint64_t page_addr = *(uint64_t*)(ram + t1_addr + t1 * 8);

	*perm = (page_addr >> 60) & 0b1111;

	return page_addr & 0xfffffffffffffff0 + offset;
}


uint64_t mmu_read(struct MMU* mmu, char vaddr, uint64_t tp, uint64_t addr, char* perm) {
	if (vaddr) {
		addr = virt_to_phys(mmu, addr, tp, perm);

		if ((*perm & READ) == 0) {
			// TODO
		}
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
		char perm;
		addr = virt_to_phys(mmu, addr, tp, &perm);

		if ((perm & WRITE) == 0) {
			// TODO
		}
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
