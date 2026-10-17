#ifndef RENDER_EDGE_H
# define RENDER_EDGE_H

# include <stddef.h>
# include <stdint.h>
# include "velum/gfx.h"

# define EDGE_W 32
# define EDGE_H 16
# define EDGE_GUARD 8
# define EDGE_BG 0xff102030u
# define EDGE_INK 0xff0f7d83u
# define EDGE_SENTINEL 0xdeadbeefu

typedef struct s_edge
{
	uint32_t	*all;
	t_surface	s;
	size_t		total;
}	t_edge;

int	edge_open(t_edge *e);
int	edge_guard_ok(const t_edge *e);

#endif
