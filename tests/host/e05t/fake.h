#ifndef E05T_FAKE_H
# define E05T_FAKE_H

# include <stdint.h>
# include "proc_int.h"
# include "proc_sys.h"

typedef struct s_ft
{
	uint64_t	pages;
	uint64_t	last_size;
	int			allocs;
	int			unmaps;
	int			fail_add;
}	t_ft;

extern t_ft	g_ft;

void		ft_reset(uint64_t limit, uint64_t pages);
int64_t		ft_create(uint64_t size);

#endif
