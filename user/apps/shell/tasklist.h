#ifndef TASKLIST_H
# define TASKLIST_H

# include <stdint.h>
# include "velum/wm.h"

# define TASK_MAX 64
# define TASK_TITLE_MAX 64

typedef struct s_task
{
	uint32_t	id;
	uint32_t	pid;
	uint32_t	style;
	uint32_t	state;
	uint32_t	icon;
	int			active;
	char		title[TASK_TITLE_MAX];
}	t_task;

typedef struct s_tasklist
{
	t_task		list[TASK_MAX];
	uint32_t	count;
}	t_tasklist;

void			tl_init(t_tasklist *l);
int32_t			tl_find(const t_tasklist *l, uint32_t id);
int				tl_shown(uint32_t style, uint32_t state);
void			tl_remove(t_tasklist *l, uint32_t id);
int				tl_apply(t_tasklist *l, uint32_t type, const t_wminfo *info);
const t_task	*tl_active(const t_tasklist *l);

#endif
