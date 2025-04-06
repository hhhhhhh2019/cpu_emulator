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
	iint   = 0x24,
	iret   = 0x25,
	chst   = 0x26,
	lost   = 0x27,
	stou   = 0x28,
	loau   = 0x29,
	chtp   = 0x2a,
	lotp   = 0x2b,
	chflag = 0x2c,
	loflag = 0x2d,
};


enum Microcodes {
	inter_off    = 1 << 1,
	inter_on     = 1 << 2,
	num64_to_pc  = 1 << 3,
	num8_to_ab   = 1 << 4,
	sdb_to_ab    = 1 << 5,
	sdb_to_flag  = 1 << 6,
	sdb_to_pc    = 1 << 7,
	sdb_to_r1    = 1 << 8,
	sdb_to_state = 1 << 9,
	sdb_to_tp    = 1 << 10,
	state_to_sdb = 1 << 11,
	tp_to_sdb    = 1 << 12,
	flag_to_sdb  = 1 << 13,
	write        = 1 << 14,
	read         = 1 << 15,
	is_usermode  = 1 << 16,
	bus_reset    = 1 << 17,
	read_num64   = 1 << 18,
	read_r2      = 1 << 19,
	read_r3      = 1 << 20,
	r3_to_sdb    = 1 << 21,
	read_sp      = 1 << 22,
	inc_sp       = 1 << 23,
	dec_sp       = 1 << 24,
	ALU_sum      = 1 << 25,
	ALU_sub      = 1 << 26,
	// TODO: other ALU operations
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
	uint64_t ab;
	uint64_t rout1;
	uint64_t rout2;
};


void print_registers(struct Core*);
void core_step(struct Core*);


#endif
