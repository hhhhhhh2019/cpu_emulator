#include "core.h"
#include "cpu.h"
#include "mmu.h"

#include <stdint.h>
#include <stdio.h>


static int opcode_len[] = {
	[sto]	 = 2,
	[loa]	 = 2,
	[add]	 = 1,
	[sub]	 = 1,
	[mul]	 = 1,
	[idiv]	 = 1,
	[addn]	 = 2,
	[subn]	 = 2,
	[muln]	 = 2,
	[divn]	 = 2,
	[adde]	 = 2,
	[addne]	 = 2,
	[addg]	 = 2,
	[addl]	 = 2,
	[addsg]	 = 2,
	[addsl]	 = 2,
	[notr]	 = 1,
	[andr]	 = 1,
	[orr]	 = 1,
	[xorr]	 = 1,
	[shl]	 = 1,
	[shr]	 = 1,
	[andn]	 = 2,
	[orn]	 = 2,
	[xorn]	 = 2,
	[shln]	 = 2,
	[shrn]	 = 2,
	[push]	 = 1,
	[pop]	 = 1,
	[call]	 = 1,
	[ret]	 = 1,
	[iint]	 = 1,
	[iret]	 = 1,
	[chst]	 = 1,
	[lost]	 = 1,
	[stou]	 = 2,
	[loau]	 = 2,
	[chtp]	 = 1,
	[lotp]	 = 1,
	[chflag] = 1,
	[loflag] = 1,
};


static uint64_t opcodes[] = {
	[sto]	 = 0,
	[loa]	 = 0,
	[add]	 = R1 | R2 | ALU_sum | W,
	[sub]	 = 0,
	[mul]	 = 0,
	[idiv]	 = 0,
	[addn]	 = R1 | num64_to_ro2 | ALU_sum | W,
	[subn]	 = 0,
	[muln]	 = 0,
	[divn]	 = 0,
	[adde]	 = 0,
	[addne]	 = 0,
	[addg]	 = 0,
	[addl]	 = 0,
	[addsg]	 = 0,
	[addsl]	 = 0,
	[notr]	 = 0,
	[andr]	 = 0,
	[orr]	 = 0,
	[xorr]	 = 0,
	[shl]	 = 0,
	[shr]	 = 0,
	[andn]	 = 0,
	[orn]	 = 0,
	[xorn]	 = 0,
	[shln]	 = 0,
	[shrn]	 = 0,
	[push]	 = 0,
	[pop]	 = 0,
	[call]	 = 0,
	[ret]	 = 0,
	[iint]	 = 0,
	[iret]	 = 0,
	[chst]	 = 0,
	[lost]	 = 0,
	[stou]	 = 0,
	[loau]	 = 0,
	[chtp]	 = 0,
	[lotp]	 = 0,
	[chflag] = 0,
	[loflag] = 0,
};


static inline char is_kernel_mode(struct Core* core) {
	return !(core->state & USERMODE) || (core->state & ISINTERRUPT);
}


void print_registers(struct Core* core) {
	printf("+-----------------------+\n");
	printf("| st : %016lx |\n", core->state);
	printf("| r0 : %016lx |\n", core->registers[0]);
	printf("| r1 : %016lx |\n", core->registers[1]);
	printf("| r2 : %016lx |\n", core->registers[2]);
	printf("| r3 : %016lx |\n", core->registers[3]);
	printf("| r4 : %016lx |\n", core->registers[4]);
	printf("| r5 : %016lx |\n", core->registers[5]);
	printf("| r6 : %016lx |\n", core->registers[6]);
	printf("| r7 : %016lx |\n", core->registers[7]);
	printf("| r8 : %016lx |\n", core->registers[8]);
	printf("| r9 : %016lx |\n", core->registers[9]);
	printf("| r10: %016lx |\n", core->registers[10]);
	printf("| r11: %016lx |\n", core->registers[11]);
	printf("| r12: %016lx |\n", core->registers[12]);
	printf("| r13: %016lx |\n", core->registers[13]);
	printf("| sp : %016lx |\n", core->registers[14]);
	printf("| pc : %016lx |\n", core->registers[15]);
	printf("| tp : %016lx |\n", core->registers[16]);
	printf("| fg : %016lx |\n", core->registers[17]);
	printf("+-----------------------+\n");
}


enum ALU_OP {
	SUM,
	SUB,
	MUL,
	DIV,
	NOT,
	AND,
	OR,
	XOR,
	SHL,
	SHR,
};

static inline void alu(struct Core* core, enum ALU_OP op) {
	switch (op) {
		case SUM: core->sdb = core->rout1 + core->rout2; break;
		case SUB: core->sdb = core->rout1 - core->rout2; break;
		case MUL: core->sdb = core->rout1 * core->rout2; break;
		case DIV: core->sdb = core->rout1 / core->rout2; break;
		case NOT: core->sdb = ~core->rout1; break;
		case AND: core->sdb = core->rout1 & core->rout2; break;
		case OR:  core->sdb = core->rout1 | core->rout2; break;
		case XOR: core->sdb = core->rout1 ^ core->rout2; break;
		case SHL: core->sdb = core->rout1 << core->rout2; break;
		case SHR: core->sdb = core->rout1 >> core->rout2; break;
	}
}


void core_step(struct Core* core) {
	if (!(core->state & ENABLED))
		return;

	if (is_kernel_mode(core))
		core->registers = core->registersk;
	else
		core->registers = core->registersu;

	core->registers[0] = 0;

	print_registers(core);

	char perm;
	unsigned long instruction = mmu_read(&core->cpu->mmu,
										 core->state & PAGING, core->registers[TP],
										 core->registers[PC], &perm);
	unsigned long num64 = mmu_read(&core->cpu->mmu,
										 core->state & PAGING, core->registers[TP],
										 core->registers[PC] + 8, &perm);

	uint8_t opcode   = instruction & 0xff;
	uint8_t r1       = (instruction >> 8) & 0xf;
	uint8_t r2       = (instruction >> 12) & 0xf;
	uint8_t r3       = (instruction >> 16) & 0xf;
	uint8_t num8     = (instruction >> 20) & 0xf;
	uint8_t bitwidth =((instruction >> 28) & 0b11) + 1;

	uint64_t bitmask = bitwidth == 4 ? -1 : (1 << (bitwidth * 8)) - 1;

	printf("%d %d %d %d %d %d\n", opcode, r1, r2, r3, num8, bitwidth);

	core->registers[PC] += opcode_len[opcode] * 8;


	uint64_t ucode = opcodes[opcode];


	// first stage

	if (ucode & R1)
		core->rout1 = core->registers[r2];

	if (ucode & R2)
		core->rout2 = core->registers[r3];

	if (ucode & num64_to_ro2)
		core->rout2 = num64;


	// second stage

	if (ucode & ALU_sum)
		alu(core, SUM);

	if (ucode & ALU_sub)
		alu(core, SUB);


	// third stage

	if (ucode & W)
		core->registers[r1] = core->sdb;
}
