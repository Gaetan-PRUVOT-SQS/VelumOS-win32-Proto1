#include "velum/libk.h"
#include "logon.h"

t_ctl	*logon_add_ctl(t_logon *lg, t_ctlspec s, t_fontid font)
{
	t_ctlroot	*r;
	t_ctl		*c;

	r = &lg->ui.root;
	c = ctl_add(r, r->root, &s);
	if (c)
		ctl_set_font(r, c, font);
	return (c);
}

static int	add_tiles(t_logon *lg)
{
	t_ctlspec	s;
	uint32_t	i;
	const char	*name;

	i = 0;
	while (i < lg->lay.count)
	{
		name = lg->set.list[lg->shown[i]].name;
		if (lg->lay.count == 1)
			name = "";
		s = (t_ctlspec){CT_BUTTON, LOGON_ID_TILE + i, CTL_VISIBLE | CTL_ENABLED
			| CTL_FOCUSABLE, lg->lay.tile[i], name};
		if (!logon_add_ctl(lg, s, FONT_INS_SEMI_14))
			return (-1);
		i++;
	}
	return (0);
}

static int	add_login_row(t_logon *lg)
{
	t_ctlspec	s;
	t_ctl		*edit;

	s = (t_ctlspec){CT_EDIT, LOGON_ID_PW, CTL_ENABLED | CTL_FOCUSABLE
		| CTL_PASSWORD, lg->lay.password, ""};
	edit = logon_add_ctl(lg, s, CTL_FONT_AUTO);
	if (!edit)
		return (-1);
	ctl_edit_set_limit(&lg->ui.root, edit, LOGON_PW_LIMIT);
	s = (t_ctlspec){CT_BUTTON, LOGON_ID_GO, CTL_ENABLED | CTL_FOCUSABLE
		| CTL_DEFAULT, lg->lay.go, "OK"};
	if (!logon_add_ctl(lg, s, FONT_INS_SEMI_13))
		return (-1);
	return (0);
}

static int	add_foot_row(t_logon *lg)
{
	t_ctlspec	s;

	s = (t_ctlspec){CT_LABEL, LOGON_ID_MSG, CTL_VISIBLE | CTL_ENABLED
		| CTL_ELLIPSIS, lg->lay.message, ""};
	if (!logon_add_ctl(lg, s, FONT_INS_13))
		return (-1);
	s = (t_ctlspec){CT_BUTTON, LOGON_ID_SHUT, CTL_VISIBLE | CTL_ENABLED
		| CTL_FOCUSABLE, lg->lay.shutdown, "Éteindre"};
	if (!logon_add_ctl(lg, s, FONT_INS_SEMI_13))
		return (-1);
	return (0);
}

int	logon_build(t_logon *lg)
{
	if (add_tiles(lg) < 0 || add_login_row(lg) < 0 || add_foot_row(lg) < 0
		|| logon_add_dialog(lg) < 0)
		return (-1);
	logon_sync(lg);
	return (0);
}
