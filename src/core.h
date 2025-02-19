#ifndef CORE_H
#define CORE_H


#include <stdint.h>
#include "va_args_count.h"


#define ZERO 0
#define SP   14
#define PC   15
#define TP   16
#define FLAG 17

#define ENABLED     (1 << 0)
#define INTERRUPTS  (1 << 1)
#define PAGING      (1 << 2)
#define USERMODE    (1 << 3)
#define ISINTERRUPT (1 << 4)


enum Instruction {
	sto    = 0x0,
	loa    = 0x1,
	add	   = 0x2,
	sub	   = 0x3,
	mul	   = 0x4,
	idiv   = 0x5,
	addn   = 0x6,
	subn   = 0x7,
	muln   = 0x8,
	divn   = 0x9,
	adde   = 0xa,
	addne  = 0xb,
	addg   = 0xc,
	addl   = 0xd,
	addsg  = 0xe,
	addsl  = 0xf,
	notr   = 0x10,
	andr   = 0x11,
	orr	   = 0x12,
	xorr   = 0x13,
	shl	   = 0x14,
	shr	   = 0x15,
	andn   = 0x16,
	orn	   = 0x17,
	xorn   = 0x18,
	shln   = 0x19,
	shrn   = 0x1a,
	push   = 0x1b,
	pop    = 0x1f,
	call   = 0x23,
	ret	   = 0x24,
	iint   = 0x25,
	iret   = 0x26,
	chst   = 0x27,
	lost   = 0x28,
	stou   = 0x29,
	loau   = 0x2a,
	chtp   = 0x2b,
	lotp   = 0x2c,
	chflag = 0x2d,
	loflag = 0x2e,
};


enum Microcodes {
	num64_to_sdb,
	Reg_read,
	Reg_write,
	R1_to_Rd,
	R2_to_Rd,
	R3_to_Rd,
	sdb_to_Ri,
	sdb_to_A2,
	Ro_to_A1,
	Ro_to_A2,
	Ro_to_sdb,
	ALU_sum,
	ALU_sub,
	ALU_mul,
	ALU_div,
	ALU_not,
	ALU_or,
	ALU_and,
	ALU_xor,
	Ao_to_sdb,
	sdb_to_Ad,
};


struct Opcodes {
	uint8_t count;
	enum Microcodes* microcodes;
};

#define INSTRUCTION(...) \
	(struct Opcodes){ \
		VA_ARGS_COUNT(__VA_ARGS__), \
		(enum Microcodes[]){__VA_ARGS__}}


struct CPU;
struct Core {
	struct CPU* cpu;

	uint64_t registersk[18];
	uint64_t registersu[18];
	uint64_t* registers;

	uint64_t state;

	uint64_t sdb;

	uint64_t alu_l1; // first input
	uint64_t alu_l2; // second input
	uint64_t alu_l3; // output

	uint64_t reg_id;
	uint64_t reg_inp;
	uint64_t reg_out;

	uint64_t ram_addr;
	/* uint64_t ram_value; */
};


void print_registers(struct Core*);
void core_step(struct Core*);


#endif
