#include "velum/libk.h"
#include "reglages.h"

static uint32_t	checked_index(t_ctlroot *r, uint32_t base, uint32_t n)
{
	uint32_t	i;
	t_ctl		*c;

	i = 0;
	while (i < n)
	{
		c = ctl_find(r, base + i);
		if (c && (c->flags & CTL_CHECKED))
			return (i);
		i++;
	}
	return (0);
}

void	reg_pull(t_reglages *app)
{
	t_ctl	*check;

	app->cfg.theme = checked_index(&app->ui.root, REG_ID_THEME, 3);
	app->cfg.layout = checked_index(&app->ui.root, REG_ID_LAYOUT, 2);
	app->cfg.mode = checked_index(&app->ui.root, REG_ID_MODE, SET_MODES);
	check = ctl_find(&app->ui.root, REG_ID_TRANSP);
	app->cfg.reduced = (check && (check->flags & CTL_CHECKED)) ? 1 : 0;
}

static void	show_saved(t_reglages *app, bool ok)
{
	t_ctl	*label;

	label = ctl_find(&app->ui.root, REG_ID_SAVED);
	if (!label)
		return ;
	ctl_set_text(&app->ui.root, label, ok
		? "Réglages enregistrés."
		: "Enregistrement impossible, réglages appliqués pour cette session.");
}

void	reg_on_command(void *c, uint32_t code, void *user)
{
	t_reglages	*app;
	const t_ctl	*ctl;
	t_settings	before;

	ctl = c;
	app = user;
	if (!app || !ctl || (code != CN_CLICKED && code != CN_CHANGED)
		|| ctl->id < REG_ID_THEME || ctl->id >= REG_ID_NOTE)
		return ;
	before = app->cfg;
	reg_pull(app);
	if (memcmp(&before, &app->cfg, sizeof(before)) == 0)
		return ;
	show_saved(app, set_save(SET_PATH, &app->cfg) >= 0);
	reg_apply(app, 0);
}
