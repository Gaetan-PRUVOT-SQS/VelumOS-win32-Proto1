#ifndef STARTMENU_H
# define STARTMENU_H

# include <stdint.h>
# include "velum/gfx.h"
# include "velum/luna_launcher.h"
# include "sm_progs.h"

# define SM_ITEMS 6
# define SM_NO_ITEM -1
# define SM_IDX_RUN 0
# define SM_IDX_LOGOFF 4
# define SM_IDX_SHUTDOWN 5

typedef enum e_smcmd
{
	SMC_NONE = 0,
	SMC_RUN,
	SMC_LOGOFF,
	SMC_SHUTDOWN,
	SMC_LAUNCH_HELLO,
	SMC_LAUNCH_B,
	SMC_LAUNCH_C
}	t_smcmd;

typedef struct s_smitem
{
	uint32_t	cmd;
	uint32_t	icon;
	const char	*label;
}	t_smitem;

typedef struct s_startmenu
{
	int			open;
	int32_t		hot;
	int32_t		focus;
}	t_startmenu;

typedef t_launcherlayout	t_smlayout;

const t_smitem	*sm_item(uint32_t index);
int				sm_visible(uint32_t index);
uint32_t		sm_tiles(void);
void			sm_init(t_startmenu *m);
void			sm_open(t_startmenu *m);
void			sm_close(t_startmenu *m);
int32_t			sm_hover(t_startmenu *m, int32_t idx);
int32_t			sm_move(t_startmenu *m, int dir);
int				sm_activate(t_startmenu *m, int32_t idx);
int				sm_key(t_startmenu *m, uint32_t vk);
t_rect			sm_item_rect(const t_smlayout *l, int32_t idx);

#endif
