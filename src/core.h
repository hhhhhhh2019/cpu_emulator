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


enum Flag {
	zero  = 1 << 0,
	carry = 1 << 1,
	sign  = 1 << 2,
};


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
	addz   = 0xa,
	addc   = 0xb,
	adds   = 0xc,
	notr   = 0xd,
	andr   = 0xe,
	orr	   = 0xf,
	xorr   = 0x10,
	shl	   = 0x11,
	shr	   = 0x12,
	andn   = 0x13,
	orn	   = 0x14,
	xorn   = 0x15,
	shln   = 0x16,
	shrn   = 0x17,
	push   = 0x18,
	pop    = 0x1c,
	call   = 0x20,
	iint   = 0x21,
	iret   = 0x22,
	chst   = 0x23,
	lost   = 0x24,
	stou   = 0x25,
	loau   = 0x26,
	chtp   = 0x27,
	lotp   = 0x28,
	chflag = 0x29,
	loflag = 0x2a,
};


enum Microcodes {
	inter_off    = 1 << 1,
	inter_on     = 1 << 2,
	pc_to_sdb    = 1 << 3,
	r3_to_pc     = 1 << 4,
	num8_to_ab   = 1 << 5,
	sdb_to_ab    = 1 << 6,
	sdb_to_flag  = 1 << 7,
	sdb_to_pc    = 1 << 8,
	sdb_to_r1    = 1 << 9,
	sdb_to_state = 1 << 10,
	sdb_to_tp    = 1 << 11,
	state_to_sdb = 1 << 12,
	tp_to_sdb    = 1 << 13,
	flag_to_sdb  = 1 << 14,
	write        = 1 << 15,
	read         = 1 << 16,
	is_usermode  = 1 << 17,
	is_zero      = 1 << 18,
	is_carry     = 1 << 19,
	is_sign      = 1 << 20,
	bus_reset    = 1 << 21,
	read_num64   = 1 << 22,
	read_r2      = 1 << 23,
	read_r3      = 1 << 24,
	r3_to_sdb    = 1 << 25,
	read_sp      = 1 << 26,
	inc_sp       = 1 << 27,
	dec_sp       = 1 << 28,
	ALU_sum      = 1 << 29,
	ALU_sub      = 1 << 30,
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
