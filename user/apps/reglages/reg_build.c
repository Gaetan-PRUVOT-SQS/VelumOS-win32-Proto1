#include "velum/libk.h"
#include "reglages.h"

static t_ctl	*add(t_reglages *app, t_ctlspec s, t_fontid font)
{
	t_ctl	*c;

	c = ctl_add(&app->ui.root, app->ui.root.root, &s);
	if (c)
		ctl_set_font(&app->ui.root, c, font);
	return (c);
}

static t_ctl	*add_in(t_reglages *app, t_ctl *parent, t_ctlspec s)
{
	t_ctl	*c;

	c = ctl_add(&app->ui.root, parent, &s);
	if (c)
		ctl_set_font(&app->ui.root, c, FONT_INS_14);
	return (c);
}

static int	radios(t_reglages *app, t_ctl *grp, uint32_t base, int32_t y0,
	const char *const *names, uint32_t n, uint32_t checked)
{
	uint32_t	i;
	uint32_t	flags;
	t_ctl		*c;

	i = 0;
	while (i < n)
	{
		flags = CTL_VISIBLE | CTL_ENABLED | CTL_FOCUSABLE;
		if (i == checked)
			flags |= CTL_CHECKED;
		c = add_in(app, grp, (t_ctlspec){CT_RADIO, base + i, flags,
			{REG_X0 + 18 + (int32_t)(i % 3) * 180, y0 + (int32_t)(i / 3) * 28,
			170, 24}, names[i]});
		if (!c)
			return (-1);
		i++;
	}
	return (0);
}

static int	group(t_reglages *app, t_ctlspec s, const char *const *names,
	uint32_t n, uint32_t base, uint32_t checked)
{
	t_ctl	*grp;

	grp = add(app, s, FONT_INS_SEMI_13);
	if (!grp)
		return (-1);
	return (radios(app, grp, base, s.rect.y + 26, names, n, checked));
}

static int	side(t_reglages *app)
{
	static const char	*names[] = {"Système", "Apparence", "Stockage",
		"Utilisateurs", "À propos de VelumOS"};
	uint32_t			i;

	i = 0;
	while (i < 5)
	{
		if (!add(app, (t_ctlspec){CT_LABEL, 70 + i, CTL_VISIBLE | CTL_ENABLED,
				{REG_PAD, 24 + (int32_t)i * 36 + (i == 4 ? 300 : 0),
				REG_SIDE_W - 2 * REG_PAD, 24}, names[i]},
			i == 1 ? FONT_INS_SEMI_14 : FONT_INS_14))
			return (-1);
		i++;
	}
	return (0);
}

int	reg_build(t_reglages *app)
{
	static const char	*themes[] = {"Clair", "Sombre", "Selon l'heure"};
	static const char	*layouts[] = {"Azerty", "Qwerty"};
	static const char	*modes[SET_MODES] = {"1024 × 768", "1280 × 720",
		"1366 × 768", "1600 × 900", "1920 × 1080"};
	uint32_t			flags;

	flags = CTL_VISIBLE | CTL_ENABLED;
	if (side(app) < 0 || !add(app, (t_ctlspec){CT_LABEL, REG_ID_TITLE, flags,
			{REG_X0, 18, REG_CONTENT_W, 36}, "Apparence"}, FONT_INS_BOLD_28)
		|| !add(app, (t_ctlspec){CT_LABEL, REG_ID_SUB, flags | CTL_ELLIPSIS,
			{REG_X0, 58, REG_CONTENT_W, 20},
			"Thème, transparence, clavier et écran."}, FONT_INS_14)
		|| group(app, (t_ctlspec){CT_GROUP, 12, flags, {REG_X0, 96,
			REG_CONTENT_W, 70}, "Thème"}, themes, 3, REG_ID_THEME,
			app->cfg.theme) < 0
		|| group(app, (t_ctlspec){CT_GROUP, 13, flags, {REG_X0, 180,
			REG_CONTENT_W, 70}, "Clavier"}, layouts, 2, REG_ID_LAYOUT,
			app->cfg.layout) < 0
		|| group(app, (t_ctlspec){CT_GROUP, 14, flags, {REG_X0, 264,
			REG_CONTENT_W, 100}, "Résolution d'écran"}, modes, SET_MODES,
			REG_ID_MODE, app->cfg.mode) < 0)
		return (-1);
	if (!add(app, (t_ctlspec){CT_CHECK, REG_ID_TRANSP, flags | CTL_FOCUSABLE
			| (app->cfg.reduced ? CTL_CHECKED : 0), {REG_X0, 384, REG_CONTENT_W,
			26}, "Réduire la transparence"}, FONT_INS_14)
		|| !add(app, (t_ctlspec){CT_LABEL, REG_ID_NOTE, flags | CTL_ELLIPSIS,
			{REG_X0, 428, REG_CONTENT_W, 40},
			"La disposition du clavier et la résolution s'appliquent au prochain démarrage."},
			FONT_INS_13)
		|| !add(app, (t_ctlspec){CT_LABEL, REG_ID_SAVED, flags | CTL_ELLIPSIS,
			{REG_X0, 480, REG_CONTENT_W, 20}, ""}, FONT_INS_13))
		return (-1);
	return (0);
}
