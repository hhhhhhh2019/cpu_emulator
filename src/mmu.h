#ifndef MMU_H
#define MMU_H


#include <stdint.h>


#define MMU_MMIO_ADDR 0x100

#define MMU_REG_CMD			0x0
#define MMU_REG_STARTADDR	0x1
#define MMU_REG_SIZE		0x5
#define MMU_REG_DEVICEID	0x9

#define MMU_CMD_RESET 1
#define MMU_CMD_ADD   2


struct MMIO {
	uint64_t addr_start;
	uint64_t size;
	char* registers;
};


struct CPU;
struct MMU {
	struct CPU* cpu;
	unsigned long hz;

	char registers[11];

	uint64_t mmio_count;
	struct MMIO* mmio;
};


void mmu_init(struct MMU*, struct CPU*, unsigned long hz);
void mmu_step(struct MMU*);


uint64_t mmu_read(struct MMU*, char vaddr, uint64_t tp, uint64_t addr, char* perm);
void mmu_write(struct MMU*, char vaddr, uint64_t tp, uint64_t addr, char size, uint64_t value);


#endif // MMU_H
