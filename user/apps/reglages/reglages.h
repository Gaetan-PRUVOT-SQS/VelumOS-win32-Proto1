#ifndef REGLAGES_H
# define REGLAGES_H

# include <stdint.h>
# include "velum/font.h"
# include "velum/settings.h"
# include "../common/uiwin.h"

# define REG_TITLE "Réglages"
# define REG_W 820
# define REG_H 600
# define REG_Y 36
# define REG_SIDE_W 210
# define REG_PAD 24
# define REG_X0 234
# define REG_CONTENT_W 560
# define REG_TICK_NS 60000000000ull
# define REG_HOUR_NS 3600000000000ull

# define REG_ID_TITLE 10
# define REG_ID_SUB 11
# define REG_ID_THEME 20
# define REG_ID_TRANSP 30
# define REG_ID_LAYOUT 40
# define REG_ID_MODE 50
# define REG_ID_NOTE 60
# define REG_ID_SAVED 61

typedef struct s_reglages
{
	t_uiwin		ui;
	t_handle	timer;
	t_settings	cfg;
	uint32_t	sent_dark;
	uint32_t	sent_reduced;
	int			quit;
}	t_reglages;

int		reg_open(t_reglages *app);
void	reg_run(t_reglages *app);
int		reg_build(t_reglages *app);
void	reg_on_command(void *c, uint32_t code, void *user);
void	reg_pull(t_reglages *app);
void	reg_apply(t_reglages *app, int force);
uint32_t	reg_hour(void);

#endif
