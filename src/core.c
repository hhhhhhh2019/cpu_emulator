#include "core.h"
#include "cpu.h"
#include "mmu.h"

#include <stdint.h>
#include <stdio.h>


static int opcode_len[] = { // in bytes
	[sto]    = 4 * 3,
	[loa]    = 4 * 3,
	[add]    = 4 * 1,
	[sub]    = 4 * 1,
	[mul]    = 4 * 1,
	[idiv]   = 4 * 1,
	[addn]   = 4 * 3,
	[subn]   = 4 * 3,
	[muln]   = 4 * 3,
	[divn]   = 4 * 3,
	[addz]   = 4 * 3,
	[addc]   = 4 * 3,
	[adds]   = 4 * 3,
	[notr]   = 4 * 1,
	[andr]   = 4 * 1,
	[orr]    = 4 * 1,
	[xorr]   = 4 * 1,
	[shl]    = 4 * 1,
	[shr]    = 4 * 1,
	[andn]   = 4 * 3,
	[orn]    = 4 * 3,
	[xorn]   = 4 * 3,
	[shln]   = 4 * 3,
	[shrn]   = 4 * 3,
	[push]   = 4 * 1,
	[pop]    = 4 * 1,
	[call]   = 4 * 1,
	[iint]   = 4 * 1,
	[iret]   = 4 * 1,
	[chst]   = 4 * 1,
	[lost]   = 4 * 1,
	[chtp]   = 4 * 1,
	[lotp]   = 4 * 1,
	[chflag] = 4 * 1,
	[loflag] = 4 * 1,
	[utok]   = 4 * 1,
	[ktou]   = 4 * 1,
};


static uint64_t opcodes[] = {
	[sto]	 = read_r2 | read_num64 | ALU_sum | sdb_to_ab | bus_reset | r3_to_sdb | _write,
	[loa]	 = read_r2 | read_num64 | ALU_sum | sdb_to_ab | bus_reset | _read | sdb_to_r1,
	[add]	 = read_r2 | read_r3 | ALU_sum | sdb_to_r1,
	[sub]	 = read_r2 | read_r3 | ALU_sub | sdb_to_r1,
	[mul]	 = 0,
	[idiv]	 = 0,
	[addn]	 = read_r2 | read_num64 | ALU_sum | sdb_to_r1,
	[subn]	 = read_r2 | read_num64 | ALU_sub | sdb_to_r1,
	[muln]	 = 0,
	[divn]	 = 0,
	[addz]   = is_zero | read_r2 | read_num64 | ALU_sum | sdb_to_r1,
	[addc]   = is_carry | read_r2 | read_num64 | ALU_sum | sdb_to_r1,
	[adds]   = is_sign | read_r2 | read_num64 | ALU_sum | sdb_to_r1,
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
	[push]	 = read_sp | ALU_sum | sdb_to_ab | r3_to_sdb | _write | dec_sp,
	[pop]	 = read_sp | ALU_sum | sdb_to_ab | _read | sdb_to_r1 | inc_sp,
	[call]	 = read_sp | ALU_sum | sdb_to_ab | pc_to_sdb | _write | dec_sp | r3_to_pc,
	[iint]	 = num8_to_core_int,
	[iret]	 = inter_off,
	[chst]	 = is_usermode | read_r2 | ALU_sum | sdb_to_state,
	[lost]	 = is_usermode | state_to_sdb | sdb_to_r1,
	[chtp]	 = is_usermode | read_r2 | ALU_sum | sdb_to_tp,
	[lotp]	 = is_usermode | tp_to_sdb | sdb_to_r1,
	[chflag] = is_usermode | read_r2 | ALU_sum | sdb_to_flag,
	[loflag] = is_usermode | flag_to_sdb | sdb_to_r1,
	[utok]   = is_usermode | r3_u_to_sdb | sdb_to_r1,
	[ktou]   = is_usermode | r3_to_sdb | sdb_to_r1_u,
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

static inline void alu(struct Core* core, enum ALU_OP op, int bitwidth) {
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

	core->registers[FLAG] &= ~zero;
	core->registers[FLAG] &= ~carry;
	core->registers[FLAG] &= ~sign;

	if (core->sdb == 0)
		core->registers[FLAG] |= zero;

	if (core->sdb & (1 << (bitwidth - 1)))
		core->registers[FLAG] |= sign;

	switch (op) {
		case SUM: core->registers[FLAG] |=
			((((uint64_t)-1) - core->rout1) > core->rout2) ? carry : 0; break;
		case SUB: core->registers[FLAG] |=
			(core->rout2 > core->rout1) ? carry : 0; break;
		case (MUL): // TODO
		case (DIV):
		case (NOT):
		case (AND):
		case (OR):
		case (XOR):
		case (SHL):
		case (SHR):
			break;
	}
}


static void update_registers(struct Core* core) {
	if (is_kernel_mode(core))
		core->registers = core->registersk;
	else
		core->registers = core->registersu;
}


void core_step(struct Core* core) {
	if (!(core->state & ENABLED))
		return;

	if (core->state & INTERRUPTS && core->int_queue_head != core->int_queue_tail) {
		core_handle_interrupt(core);
		return;
	}

	update_registers(core);

	core->registers[0] = 0;
	core->sdb = 0;
	core->ab = 0;
	core->rout1 = 0;
	core->rout2 = 0;

	print_registers(core);

	char perm;
	unsigned int instruction = mmu_read(&core->cpu->mmu,
										 core->state & PAGING, core->registers[TP],
										 core->registers[PC], &perm);
	uint64_t num64 = mmu_read(&core->cpu->mmu,
										 core->state & PAGING, core->registers[TP],
										 core->registers[PC] + 4, &perm);

	uint8_t opcode   = instruction & 0xff;
	uint8_t r1       = (instruction >> 8) & 0xf;
	uint8_t r2       = (instruction >> 12) & 0xf;
	uint8_t r3       = (instruction >> 16) & 0xf;
	uint8_t num8     = (instruction >> 20) & 0xf;
	uint8_t bitwidth = (1 << ((instruction >> 28) & 0b11)) * 8;

	uint64_t bitmask = bitwidth == 64 ? -1 : ((uint64_t)1 << bitwidth) - 1;

	printf("%d %d %d %d %d %d %016lx\n", opcode, r1, r2, r3, num8, bitwidth, bitmask);


	uint64_t ucode = opcodes[opcode];


	// 0 stage

	if (ucode & is_usermode) {
		if (!is_kernel_mode(core)) {
			core->registers[PC] += opcode_len[opcode];
			return;
		}
	}

	if (ucode & inter_off) {
		core->state &= ~ISINTERRUPT;
		core->registersk[PC] = core->int_ret;
		update_registers(core);
	}

	if (ucode & dec_sp)
		core->registers[SP] -= bitwidth;

	if (ucode & is_zero) {
		if ((core->registers[FLAG] & zero) == 0) {
			core->registers[PC] += opcode_len[opcode];
			return;
		}
	}

	if (ucode & is_carry) {
		if ((core->registers[FLAG] & carry) == 0) {
			core->registers[PC] += opcode_len[opcode];
			return;
		}
	}

	if (ucode & is_sign) {
		if ((core->registers[FLAG] & sign) == 0) {
			core->registers[PC] += opcode_len[opcode];
			return;
		}
	}

	if (ucode & num8_to_core_int)
		core_int(core, num8);


	// 1 stage

	if (ucode & read_num64)
		core->rout2 = num64;

	if (ucode & read_r2)
		core->rout1 = core->registers[r2];

	if (ucode & read_r3)
		core->rout2 = core->registers[r3];

	if (ucode & read_sp)
		core->rout1 = core->registers[SP];


	// 2 stage

	if (ucode & ALU_sum)
		alu(core, SUM, bitwidth);

	if (ucode & ALU_sub)
		alu(core, SUB, bitwidth);

	// TODO: other ALU operations


	// 3 stage

	if (ucode & sdb_to_ab)
		core->ab = core->sdb;


	// 4 stage

	if (ucode & bus_reset) {
		core->rout1 = 0;
		core->rout2 = 0;
		core->sdb = 0;
	}


	// 5 stage

	if (ucode & r3_to_sdb)
		core->sdb = core->registers[r3];

	if (ucode & r3_u_to_sdb)
		core->sdb = core->registersu[r3];

	if (ucode & pc_to_sdb)
		core->sdb = core->registers[PC];


	core->registers[PC] += opcode_len[opcode];


	// 6 stage

	if (ucode & _write)
		mmu_write(&core->cpu->mmu,
		          core->state & PAGING, core->registers[TP],
		          core->ab, bitwidth, core->sdb);

	if (ucode & _read)
		core->sdb = mmu_read(&core->cpu->mmu,
		          core->state & PAGING, core->registers[TP],
		          core->ab, &perm) & bitmask;

	if (ucode & inc_sp)
		core->registers[SP] += bitwidth;

	if (ucode & flag_to_sdb)
		core->sdb = core->registersu[FLAG];

	if (ucode & state_to_sdb)
		core->sdb = core->state;

	if (ucode & tp_to_sdb)
		core->sdb = core->registersu[TP];

	if (ucode & r3_to_pc)
		core->registers[PC] = core->registers[r3];


	// 7 stage

	if (ucode & sdb_to_r1)
		core->registers[r1] = core->sdb;

	if (ucode & sdb_to_r1_u)
		core->registersu[r1] = core->sdb;

	if (ucode & sdb_to_flag)
		core->registersu[FLAG] = core->sdb;

	if (ucode & sdb_to_state)
		core->state = core->sdb;

	if (ucode & sdb_to_tp)
		core->registersu[TP] = core->sdb;
}


void core_int(struct Core* core, uint64_t irq) {
	core->int_queue[core->int_queue_head++] = irq;
	core->int_queue_head %= 256;
}


void core_handle_interrupt(struct Core* core) {
	uint64_t irq = core->int_queue[core->int_queue_tail++];
	printf("int: %016lx\n", irq);
	core->int_queue_tail %= 256;

	core->state |= ISINTERRUPT;

	core->int_ret = core->registersk[PC];
	core->registersk[PC] = irq;
}
