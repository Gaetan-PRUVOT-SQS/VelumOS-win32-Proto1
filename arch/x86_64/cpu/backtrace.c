#include "cpu_int.h"

static bool	frame_ok(uint64_t rbp, uint64_t lo)
{
	return (rbp != 0 && rbp >= lo && (rbp & 7) == 0
		&& rbp <= UINT64_MAX - 16);
}

int	bt_walk(uint64_t rbp, uint64_t lo, uint64_t *pcs, int max)
{
	const uint64_t	*frame;
	uint64_t		next;
	int				n;

	n = 0;
	while (n < max && frame_ok(rbp, lo))
	{
		frame = (const uint64_t *)rbp;
		if (!frame[1])
			break ;
		pcs[n] = frame[1];
		n++;
		next = frame[0];
		if (next <= rbp || next - rbp > BT_SPAN)
			break ;
		rbp = next;
	}
	return (n);
}

int	cpu_backtrace(uint64_t rbp, uint64_t *pcs, int max)
{
	if (!pcs || max <= 0)
		return (0);
	if (max > BT_MAX)
		max = BT_MAX;
	return (bt_walk(rbp, KERNEL_HALF, pcs, max));
}
