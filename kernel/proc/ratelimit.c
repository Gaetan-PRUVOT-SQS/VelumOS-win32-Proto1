#include "proc_pure.h"

int	rl_allow(t_ratelimit *rl, uint64_t now_ns, uint32_t max_per_s)
{
	if (now_ns < rl->start_ns || now_ns - rl->start_ns >= RL_WINDOW_NS)
	{
		rl->start_ns = now_ns;
		rl->count = 0;
		rl->dropped = 0;
	}
	if (rl->count >= max_per_s)
	{
		if (rl->dropped < UINT32_MAX)
			rl->dropped++;
		return (0);
	}
	rl->count++;
	return (1);
}

void	log_sanitize(char *s, uint64_t n)
{
	uint64_t	i;

	i = 0;
	while (i < n)
	{
		if ((unsigned char)s[i] < 0x20 || (unsigned char)s[i] == 0x7f)
			s[i] = '?';
		i++;
	}
}
