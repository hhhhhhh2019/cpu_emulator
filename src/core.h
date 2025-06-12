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
	add    = 0x2,
	sub    = 0x3,
	mul    = 0x4,
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
	orr    = 0xf,
	xorr   = 0x10,
	shl    = 0x11,
	shr    = 0x12,
	andn   = 0x13,
	orn    = 0x14,
	xorn   = 0x15,
	shln   = 0x16,
	shrn   = 0x17,
	push   = 0x18,
	pop    = 0x19,
	call   = 0x1d,
	iint   = 0x1e,
	iret   = 0x1f,
	chst   = 0x20,
	lost   = 0x21,
	chtp   = 0x22,
	lotp   = 0x23,
	chflag = 0x24,
	loflag = 0x25,
	utok   = 0x26,
	ktou   = 0x27,
};


#define inter_off        (1l << 0)
#define num8_to_core_int (1l << 1)
#define pc_to_sdb        (1l << 2)
#define r3_to_pc         (1l << 3)
#define sdb_to_ab        (1l << 4)
#define sdb_to_flag      (1l << 5)
#define sdb_to_r1        (1l << 6)
#define sdb_to_r1_u      (1l << 7)
#define sdb_to_state     (1l << 8)
#define sdb_to_tp        (1l << 9)
#define state_to_sdb     (1l << 10)
#define tp_to_sdb        (1l << 11)
#define flag_to_sdb      (1l << 12)
#define _write           (1l << 13)
#define _read            (1l << 14)
#define is_usermode      (1l << 15)
#define is_zero          (1l << 16)
#define is_carry         (1l << 17)
#define is_sign          (1l << 18)
#define bus_reset        (1l << 19)
#define read_num64       (1l << 20)
#define read_r2          (1l << 21)
#define read_r3          (1l << 22)
#define r3_to_sdb        (1l << 23)
#define r3_u_to_sdb      (1l << 24)
#define read_sp          (1l << 25)
#define inc_sp           (1l << 26)
#define dec_sp           (1l << 27)
#define ALU_sum          (1l << 28)
#define ALU_sub          (1l << 29)
// TODO: other ALU operations


struct Opcodes {
	uint8_t count;
	long* microcodes;
};

#define INSTRUCTION(...) \
	(struct Opcodes){ \
		VA_ARGS_COUNT(__VA_ARGS__), \
		(enum Microcodes[]){__VA_ARGS__}}


struct CPU;
struct Core {
	struct CPU* cpu;
	unsigned long hz;

	uint64_t registersk[18];
	uint64_t registersu[18];
	uint64_t* registers;

	uint64_t state;

	uint64_t sdb;
	uint64_t ab;
	uint64_t rout1;
	uint64_t rout2;

	char int_queue[256];
	int int_queue_head;
	int int_queue_tail;
};


void print_registers(struct Core*);
void core_step(struct Core*);
void core_int(struct Core*, char id);
void core_handle_interrupt(struct Core*);


#endif
