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


static struct Opcodes opcodes[] = {
	[sto]	 = INSTRUCTION(),
	[loa]	 = INSTRUCTION(),
	[add]	 = INSTRUCTION(R2_to_Rd, Reg_read, Ro_to_A1, R3_to_Rd, Reg_read, Ro_to_A2, ALU_sum, Ao_to_sdb, R1_to_Rd, Reg_write),
	[sub]	 = INSTRUCTION(R2_to_Rd, Reg_read, Ro_to_A1, R3_to_Rd, Reg_read, Ro_to_A2, ALU_sub, Ao_to_sdb, R1_to_Rd, Reg_write),
	[mul]	 = INSTRUCTION(),
	[idiv]	 = INSTRUCTION(),
	[addn]	 = INSTRUCTION(R2_to_Rd, Reg_read, Ro_to_A1, num64_to_sdb, sdb_to_A2, ALU_sum, Ao_to_sdb, R1_to_Rd, Reg_write),
	[subn]	 = INSTRUCTION(R2_to_Rd, Reg_read, Ro_to_A1, num64_to_sdb, sdb_to_A2, ALU_sub, Ao_to_sdb, R1_to_Rd, Reg_write),
	[muln]	 = INSTRUCTION(),
	[divn]	 = INSTRUCTION(),
	[adde]	 = INSTRUCTION(),
	[addne]	 = INSTRUCTION(),
	[addg]	 = INSTRUCTION(),
	[addl]	 = INSTRUCTION(),
	[addsg]	 = INSTRUCTION(),
	[addsl]	 = INSTRUCTION(),
	[notr]	 = INSTRUCTION(),
	[andr]	 = INSTRUCTION(),
	[orr]	 = INSTRUCTION(),
	[xorr]	 = INSTRUCTION(),
	[shl]	 = INSTRUCTION(),
	[shr]	 = INSTRUCTION(),
	[andn]	 = INSTRUCTION(),
	[orn]	 = INSTRUCTION(),
	[xorn]	 = INSTRUCTION(),
	[shln]	 = INSTRUCTION(),
	[shrn]	 = INSTRUCTION(),
	[push]	 = INSTRUCTION(),
	[pop]	 = INSTRUCTION(),
	[call]	 = INSTRUCTION(),
	[ret]	 = INSTRUCTION(),
	[iint]	 = INSTRUCTION(),
	[iret]	 = INSTRUCTION(),
	[chst]	 = INSTRUCTION(),
	[lost]	 = INSTRUCTION(),
	[stou]	 = INSTRUCTION(),
	[loau]	 = INSTRUCTION(),
	[chtp]	 = INSTRUCTION(),
	[lotp]	 = INSTRUCTION(),
	[chflag] = INSTRUCTION(),
	[loflag] = INSTRUCTION(),
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
		case SUM: core->alu_l3 = core->alu_l1 + core->alu_l2; break;
		case SUB: core->alu_l3 = core->alu_l1 - core->alu_l2; break;
		case MUL: core->alu_l3 = core->alu_l1 * core->alu_l2; break;
		case DIV: core->alu_l3 = core->alu_l1 / core->alu_l2; break;
		case NOT: core->alu_l3 = ~core->alu_l1; break;
		case AND: core->alu_l3 = core->alu_l1 & core->alu_l2; break;
		case OR:  core->alu_l3 = core->alu_l1 | core->alu_l2; break;
		case XOR: core->alu_l3 = core->alu_l1 ^ core->alu_l2; break;
		case SHL: core->alu_l3 = core->alu_l1 << core->alu_l2; break;
		case SHR: core->alu_l3 = core->alu_l1 >> core->alu_l2; break;
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

	for (int i = 0; i < opcodes[opcode].count; i++) {
		switch (opcodes[opcode].microcodes[i]) {
			case (num64_to_sdb):
				core->sdb = num64 & bitmask;
				break;
			case (R1_to_Rd):
				core->reg_id = r1;
				break;
			case (R2_to_Rd):
				core->reg_id = r2;
				break;
			case (R3_to_Rd):
				core->reg_id = r3;
				break;
			case (sdb_to_A2):
				core->alu_l2 = core->sdb;
				break;
			case (Reg_read):
				core->reg_out = core->registers[core->reg_id] & bitmask;
				break;
			case (Reg_write):
				core->registers[core->reg_id] = core->sdb;
				break;
			case (Ro_to_A1):
				core->alu_l1 = core->reg_out;
				break;
			case (Ro_to_A2):
				core->alu_l2 = core->reg_out;
				break;
			case (Ao_to_sdb):
				core->sdb = core->alu_l3 & bitmask;
				break;
			case (ALU_sum):
				alu(core, SUM);
				break;
			case (ALU_sub):
				alu(core, SUB);
				break;
		}
	}
}
