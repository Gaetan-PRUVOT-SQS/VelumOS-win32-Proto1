#ifndef MOUSE_DEC_H
# define MOUSE_DEC_H

# include <stdint.h>
# include "../../kernel/input/inp_types.h"

# define MD_TIMEOUT_NS 50000000ull
# define MD_SYNC_BIT 0x08

typedef struct s_mousedec
{
	uint8_t		buf[4];
	uint8_t		len;
	uint8_t		wheel;
	uint64_t	last_ns;
}	t_mousedec;

void	mousedec_reset(t_mousedec *m, int wheel);
int		mousedec_feed(t_mousedec *m, uint8_t b, uint64_t now,
			t_mousepkt *out);

#endif
