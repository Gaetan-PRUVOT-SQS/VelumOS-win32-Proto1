#ifndef LOGON_GLASS_H
# define LOGON_GLASS_H

# include <stddef.h>
# include <stdint.h>
# include "velum/gfx.h"

# define LOGON_GLASS_PX 25600

int		logon_glass_attach(uint32_t *buf, size_t cap_px, t_rect card);
int		logon_glass_open(t_rect card);
void	logon_glass_close(void);

#endif
