#ifndef XLATE_H
# define XLATE_H

# include <stdint.h>
# include "inp_types.h"
# include "kbd.h"
# include "mouse.h"
# include "velum/sync.h"

typedef struct s_xlate
{
	t_spinlock	lock;
	t_kbd		kbd;
	t_mouse		mouse;
}	t_xlate;

extern t_xlate	g_xlate;

void	xlate_init(void);
int		xlate_key(const t_keyraw *raw, t_inpevent *out, uint8_t *locks);
int		xlate_mouse(const t_mousepkt *p, t_inpevent *out);
uint8_t	xlate_locks(void);
void	xlate_locks_set(uint8_t mask);
int		xlate_keys_lost(t_inpevent *out);

#endif
