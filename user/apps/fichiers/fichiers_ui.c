#include "velum/libk.h"
#include "../common/tbuf.h"
#include "fichiers.h"

static int	add(t_fichiers *app, t_ctlspec s, t_fontid font)
{
	t_ctlroot	*r;
	t_ctl		*c;

	r = &app->ui.root;
	c = ctl_add(r, r->root, &s);
	if (!c)
		return (-1);
	ctl_set_font(r, c, font);
	return (0);
}

static int	add_button(t_fichiers *app, uint32_t id, t_rect rect,
				const char *text)
{
	return (add(app, (t_ctlspec){CT_BUTTON, id, CTL_VISIBLE | CTL_ENABLED
			| CTL_FOCUSABLE, rect, text}, FF_FONT_BUTTON));
}

static int	add_sidebar(t_fichiers *app)
{
	static const char	*names[] = {"Ordinateur", "Système", "Données"};
	int					i;

	if (add(app, (t_ctlspec){CT_LABEL, 0, CTL_VISIBLE | CTL_ENABLED,
		{FF_SIDE_X + 4, FF_SIDE_LABEL_Y, FF_SIDE_W, FF_LINE_H}, "Favoris"},
		FF_FONT_BUTTON) < 0)
		return (-1);
	i = 0;
	while (i < 3)
	{
		if (add_button(app, FF_ID_FAV_ROOT + i, rect_make(FF_SIDE_X,
					FF_SIDE_FIRST_Y + i * FF_SIDE_STEP, FF_SIDE_W,
					FF_SIDE_BTN_H), names[i]) < 0)
			return (-1);
		i++;
	}
	return (0);
}

static int	add_header(t_fichiers *app)
{
	if (add_button(app, FF_ID_UP, rect_make(FF_MAIN_X, FF_UP_Y, FF_UP_W,
				FF_UP_H), "Dossier parent") < 0)
		return (-1);
	if (add(app, (t_ctlspec){CT_LABEL, FF_ID_PATH, CTL_VISIBLE | CTL_ENABLED
		| CTL_ELLIPSIS, {FF_PATH_X, FF_PATH_Y, FF_PATH_W, FF_LINE_H}, "/"},
		FF_FONT_TEXT) < 0)
		return (-1);
	if (add(app, (t_ctlspec){CT_LABEL, FF_ID_TITLE, CTL_VISIBLE | CTL_ENABLED
		| CTL_ELLIPSIS, {FF_MAIN_X, FF_TITLE_Y, FF_TITLE_W, FF_TITLE_H}, ""},
		FF_FONT_TITLE) < 0)
		return (-1);
	return (add(app, (t_ctlspec){CT_LABEL, FF_ID_COUNT, CTL_VISIBLE
		| CTL_ENABLED | CTL_ALIGN_RIGHT, {FF_COUNT_X, FF_COUNT_Y, FF_COUNT_W,
		FF_LINE_H}, ""}, FF_FONT_TEXT));
}

static int	add_body(t_fichiers *app)
{
	if (add(app, (t_ctlspec){CT_LIST, FF_ID_LIST, CTL_VISIBLE | CTL_ENABLED
		| CTL_FOCUSABLE, {FF_MAIN_X, FF_LIST_Y, FF_MAIN_W, FF_LIST_H}, ""},
		FF_FONT_TEXT) < 0)
		return (-1);
	if (add(app, (t_ctlspec){CT_EDIT, FF_ID_NAME, CTL_VISIBLE | CTL_ENABLED
		| CTL_FOCUSABLE, {FF_MAIN_X, FF_NAME_Y, FF_MAIN_W, FF_NAME_H}, ""},
		FF_FONT_TEXT) < 0)
		return (-1);
	return (add(app, (t_ctlspec){CT_LABEL, FF_ID_STATUS, CTL_VISIBLE
		| CTL_ENABLED | CTL_ELLIPSIS, {FF_MAIN_X, FF_STATUS_Y, FF_MAIN_W,
		FF_LINE_H}, ""}, FF_FONT_TEXT));
}

static int	add_actions(t_fichiers *app)
{
	static const char	*names[] = {"Ouvrir", "Nouveau dossier",
		"Nouveau fichier", "Renommer", "Supprimer", "Propriétés"};
	int					i;

	i = 0;
	while (i < 6)
	{
		if (add_button(app, FF_ID_OPEN + i, rect_make(FF_MAIN_X + i
					% FF_ACTION_COLS * FF_ACTION_STEP, FF_ACTIONS_Y + i
					/ FF_ACTION_COLS * FF_ACTION_ROW, FF_ACTION_W,
					FF_ACTION_H), names[i]) < 0)
			return (-1);
		i++;
	}
	return (0);
}

int	ff_app_build(t_fichiers *app)
{
	if (add_sidebar(app) < 0 || add_header(app) < 0 || add_body(app) < 0
		|| add_actions(app) < 0)
		return (-1);
	ctl_focus(&app->ui.root, ctl_find(&app->ui.root, FF_ID_LIST));
	return (0);
}
