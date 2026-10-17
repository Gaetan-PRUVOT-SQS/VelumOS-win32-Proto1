#ifndef LOGON_PAINT_H
# define LOGON_PAINT_H

# include <stddef.h>
# include <stdint.h>
# include "logon.h"

# define LOGON_HEAD_TEXT 48

int		logon_clock_text(uint64_t wall_ns, char *clock, char *date,
			size_t size);
void	logon_draw_head(t_logon *lg, uint64_t wall_ns);
void	logon_draw_avatars(t_logon *lg);
void	logon_draw_chips(t_logon *lg);
void	logon_paint_card(t_logon *lg);
void	logon_underlay(void *user, t_rect dirty);

#endif
