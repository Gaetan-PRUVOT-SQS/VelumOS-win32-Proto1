#ifndef E05_FAKE_H
# define E05_FAKE_H

# include <stdint.h>
# include "fsys.h"

# define FK_BASE 0x10000ull
# define FK_SIZE 0x300000ull
# define FK_FILE 0x280000ull

typedef struct s_fk
{
	uint8_t		umem[FK_SIZE];
	uint64_t	ro_from;
	uint64_t	pos;
	uint64_t	written;
	uint32_t	rights;
}	t_fk;

extern t_fk	g_fk;

void		fk_reset(uint64_t ro_from);
uint8_t		fk_pattern(uint64_t i);
t_sysargs	*fk_args(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3);

#endif
