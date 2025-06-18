#define MMU_MMIO_ADDR 0x100

#define MMU_REG_CMD			0x0
#define MMU_REG_STARTADDR	0x1
#define MMU_REG_SIZE		0x5
#define MMU_REG_DEVICEID	0x9

#define MMU_CMD_RESET 1
#define MMU_CMD_ADD   2


#define TIMER_DEVICE_ID 1 // hardcoded
#define TIMER_MMIO_ADDR 0x110 // hardcoded

#define TIMER_REG_DELAY     0x0
#define TIMER_REG_INTERRUPT 0x8
#define TIMER_REG_HZ        0x9
