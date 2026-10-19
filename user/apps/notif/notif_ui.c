#include "velum/err.h"
#include "velum/libk.h"
#include "velum/font.h"
#include "../common/platform.h"
#include "notif_app.h"

static t_ctl	*add(t_notifapp *app, t_ctlspec s, t_fontid font)
{
	t_ctlroot	*r;
	t_ctl		*c;

	r = &app->ui.root;
	c = ctl_add(r, r->root, &s);
	if (c != NULL)
		ctl_set_font(r, c, font);
	return (c);
}

static void	clear_children(t_notifapp *app)
{
	t_ctlroot	*r;

	r = &app->ui.root;
	while (r->root->first != NULL)
		ctl_remove(r, r->root->first);
}

static int	build_head(t_notifapp *app, int32_t w)
{
	uint32_t	f;

	f = CTL_VISIBLE | CTL_ENABLED;
	if (add(app, (t_ctlspec){CT_LABEL, NA_ID_HEAD, f | CTL_BOLD,
			{16, 12, 180, 22}, NA_TITLE}, FONT_INS_BOLD_14) == NULL)
		return (-1);
	if (add(app, (t_ctlspec){CT_BUTTON, NA_ID_CLEAR, f | CTL_FOCUSABLE,
			{w - 124, 8, 112, 28}, "Tout effacer"}, FONT_INS_SEMI_12) == NULL)
		return (-1);
	return (0);
}

static int	build_card(t_notifapp *app, uint32_t i, int32_t y, int32_t w)
{
	const t_notif	*n;
	char			age[NOTIF_AGE_MAX];
	uint32_t		f;
	uint32_t		id;

	n = notif_store_get(&app->store, i);
	f = CTL_VISIBLE | CTL_ENABLED;
	id = NA_ID_CARD + i * NA_ID_STRIDE;
	notif_age_text(os_mono_ns(), n->stamp_ns, age, sizeof(age));
	if (add(app, (t_ctlspec){CT_LABEL, id, f | CTL_ELLIPSIS | CTL_BOLD,
			{16, y + 8, w - 150, 20}, n->title}, FONT_INS_SEMI_13) == NULL)
		return (-1);
	if (add(app, (t_ctlspec){CT_LABEL, id + 1, f | CTL_ALIGN_RIGHT,
			{w - 130, y + 8, 114, 20}, age}, FONT_INS_11) == NULL)
		return (-1);
	if (add(app, (t_ctlspec){CT_LABEL, id + 2, f | CTL_ELLIPSIS,
			{16, y + 32, w - 32, 20}, n->text}, FONT_INS_13) == NULL)
		return (-1);
	if (add(app, (t_ctlspec){CT_BUTTON, NA_ID_DISMISS + i,
			f | CTL_FOCUSABLE, {16, y + 58, 88, 26}, "Ignorer"},
		FONT_INS_SEMI_12) == NULL)
		return (-1);
	return (0);
}

static int	build_cards(t_notifapp *app, int32_t w)
{
	uint32_t	i;
	uint32_t	n;
	int32_t		y;

	n = notif_store_count(&app->store);
	if (n == 0)
		return (add(app, (t_ctlspec){CT_LABEL, NA_ID_EMPTY,
				CTL_VISIBLE | CTL_ENABLED, {16, NA_HEAD_H + 12, w - 32, 22},
				"Aucune notification"}, FONT_INS_13) == NULL ? -1 : 0);
	if (n > NA_VISIBLE_MAX)
		n = NA_VISIBLE_MAX;
	i = 0;
	y = NA_HEAD_H;
	while (i < n)
	{
		if (build_card(app, i, y, w) < 0)
			return (-1);
		y += NA_CARD_H + NA_CARD_GAP;
		i++;
	}
	return (0);
}

int	notifui_rebuild(t_notifapp *app)
{
	int32_t	w;

	w = app->ui.area.w;
	clear_children(app);
	if (build_head(app, w) < 0 || build_cards(app, w) < 0)
		return (E_NOMEM);
	app->dirty = 0;
	ctl_invalidate(&app->ui.root, app->ui.root.root);
	uiwin_flush(&app->ui);
	return (0);
}

void	notifui_show(t_notifapp *app, int visible)
{
	uint32_t	state;

	state = visible ? WSTATE_NORMAL : WSTATE_HIDDEN;
	if (wmc_set_state_id(app->ui.win.id, state) < 0)
		return ;
	app->visible = visible;
	if (visible)
	{
		notifui_rebuild(app);
		wmc_activate(app->ui.win.id);
	}
}

void	notifui_toggle(t_notifapp *app)
{
	notifui_show(app, !app->visible);
}

void	notifui_command(void *c, uint32_t code, void *user)
{
	const t_ctl	*ctl;
	t_notifapp	*app;
	const t_notif	*n;

	ctl = c;
	app = user;
	if (app == NULL || code != CN_CLICKED)
		return ;
	if (ctl->id == NA_ID_CLEAR)
		notif_store_clear(&app->store);
	else if (ctl->id >= NA_ID_DISMISS
		&& ctl->id < NA_ID_DISMISS + NA_VISIBLE_MAX)
	{
		n = notif_store_get(&app->store, ctl->id - NA_ID_DISMISS);
		if (n == NULL)
			return ;
		notif_store_dismiss(&app->store, n->id);
	}
	else
		return ;
	app->dirty = 1;
}
