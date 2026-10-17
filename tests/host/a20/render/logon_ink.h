#ifndef LOGON_INK_H
# define LOGON_INK_H

# include "render.h"

# define LGI_TOP 0
# define LGI_BOTTOM 1
# define LGI_LEFT 2
# define LGI_RIGHT 3
# define LGI_TOL 3

void	lgi_ref(void);
void	lgi_scan(const t_logon *lg, t_rect band, int32_t k[4]);

#endif
