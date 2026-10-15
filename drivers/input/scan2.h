#ifndef SCAN2_H
# define SCAN2_H

# include <stdint.h>
# include "../../kernel/input/inp_types.h"

# define S2_IDLE 0
# define S2_E0 1
# define S2_BRK 2
# define S2_E0BRK 3
# define S2_E1 4
# define S2_PAUSE_TAIL 7
# define S2_PAUSE_MAKE 2
# define S2_MAX_CODE 0x84

typedef struct s_scan2
{
	uint8_t		state;
	uint8_t		e1pos;
	uint16_t	last;
	uint8_t		skip;
}	t_scan2;

void	scan2_reset(t_scan2 *s);
int		scan2_feed(t_scan2 *s, uint8_t b, t_keyraw *out);
int		scan2_emit(t_scan2 *s, uint16_t code, uint8_t release,
			t_keyraw *out);
int		scan2_idle(t_scan2 *s, uint8_t b, t_keyraw *out);
int		scan2_e1(t_scan2 *s, uint8_t b, t_keyraw *out);

#endif
