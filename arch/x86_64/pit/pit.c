#include "../../../kernel/time/time_int.h"
#include "velum/io.h"

#define PIT_CH2_DATA 0x42
#define PIT_CMD 0x43
#define PIT_CMD_CH2_MODE0 0xb0
#define PORT_B 0x61
#define PORT_B_GATE2 0x01
#define PORT_B_SPEAKER 0x02
#define PORT_B_OUT2 0x20
#define PIT_CAL_COUNT 35795u
#define PIT_SPIN_MAX 1000000u

uint64_t	tsc_calibrate_pit(void)
{
	uint8_t		saved;
	uint64_t	t0;
	uint64_t	t1;
	uint32_t	spins;

	saved = inb(PORT_B);
	outb(PORT_B, (uint8_t)((saved & ~PORT_B_SPEAKER) | PORT_B_GATE2));
	outb(PIT_CMD, PIT_CMD_CH2_MODE0);
	outb(PIT_CH2_DATA, (uint8_t)(PIT_CAL_COUNT & 0xff));
	outb(PIT_CH2_DATA, (uint8_t)(PIT_CAL_COUNT >> 8));
	t0 = tsc_read();
	spins = 0;
	while (!(inb(PORT_B) & PORT_B_OUT2) && spins < PIT_SPIN_MAX)
		spins++;
	t1 = tsc_read();
	outb(PORT_B, saved);
	if (spins >= PIT_SPIN_MAX)
		return (0);
	return (hz_from_ref(t1 - t0, PIT_CAL_COUNT, PIT_HZ));
}
