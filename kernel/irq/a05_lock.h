#ifndef A05_LOCK_H
# define A05_LOCK_H

# include <stdint.h>

# define A05_LOCK_SPIN_MAX 2000000000ull

typedef struct s_a05lock
{
	volatile uint8_t	taken;
	const char			*name;
}	t_a05lock;

uint64_t	a05_lock(t_a05lock *l);
void		a05_unlock(t_a05lock *l, uint64_t flags);

#endif
