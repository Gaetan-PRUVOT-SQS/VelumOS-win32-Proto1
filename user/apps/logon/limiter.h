#ifndef LIMITER_H
# define LIMITER_H

# include <stdint.h>
# include "accounts.h"

# define LIM_DELAY_FIRST_S 1
# define LIM_DELAY_MAX_S 30
# define LIM_FAILURES_MAX 1000000

typedef struct s_limiter
{
	uint32_t	failures;
	uint64_t	blocked_until_ns;
}	t_limiter;

typedef struct s_limiters
{
	t_limiter	slot[ACC_MAX];
}	t_limiters;

void		lim_init(t_limiters *l);
uint64_t	lim_delay_ns(uint32_t failures);
uint32_t	lim_remaining_s(const t_limiters *l, uint32_t idx, uint64_t now_ns);
void		lim_fail(t_limiters *l, uint32_t idx, uint64_t now_ns);
void		lim_reset(t_limiters *l, uint32_t idx);

#endif
