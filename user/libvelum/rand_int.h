#ifndef RAND_INT_H
# define RAND_INT_H

# include <stdint.h>
# include "velum/vsync.h"

# define RAND_POOL 256

typedef struct s_rand
{
	t_vspin		lock;
	uint32_t	seeded;
	uint32_t	pos;
	uint64_t	state;
	uint8_t		pool[RAND_POOL];
}	t_rand;

#endif
