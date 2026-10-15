#include "../../../kernel/time/time_int.h"

#define HPET_COUNTER 0x0f0

bool	hpet_present(void)
{
	return (hpet_state()->present);
}

uint64_t	hpet_hz(void)
{
	return (hpet_state()->hz);
}

bool	hpet_is_wide(void)
{
	return (hpet_state()->wide);
}

uint64_t	hpet_read(void)
{
	t_hpet		*h;
	uint32_t	lo;
	uint64_t	fl;
	uint64_t	v;

	h = hpet_state();
	if (h->wide)
		return (*(volatile uint64_t *)(h->base + HPET_COUNTER));
	fl = a05_lock(&h->lock);
	lo = *(volatile uint32_t *)(h->base + HPET_COUNTER);
	if (lo < h->last_lo)
		h->high += 1ull << 32;
	h->last_lo = lo;
	v = h->high | lo;
	a05_unlock(&h->lock, fl);
	return (v);
}
