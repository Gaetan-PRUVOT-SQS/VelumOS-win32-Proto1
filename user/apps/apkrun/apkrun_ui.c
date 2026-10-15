#include "velum/libk.h"
#include "../common/apkglue.h"
#include "apkrun.h"

int	apkrun_open(t_apkrun *app)
{
	char		title[APK_LABEL_MAX];
	t_wmhello	info;
	t_wmcreate	rq;
	int			r;

	r = ui_connect(&info);
	if (r < 0)
		return (r);
	title[0] = '\0';
	if (app->known)
		apkglue_label(title, sizeof(title), app->man.label);
	if (!title[0])
		apkglue_label(title, sizeof(title), APKRUN_TITLE);
	ui_request(&rq, rect_make(180, 140, APKRUN_W, APKRUN_H_PX), WS_DEFAULT,
		title);
	r = uiwin_open(&app->ui, &rq, apkrun_on_command);
	if (r < 0)
		return (r);
	app->ui.root.user = app;
	app->open = 1;
	return (0);
}

static void	clear(t_ctlroot *r)
{
	int	n;

	n = 0;
	while (r->root && r->root->first && n < APKRUN_CTL_MAX)
	{
		ctl_remove(r, r->root->first);
		n++;
	}
}

static void	add_message(t_apkrun *app)
{
	t_ctlroot	*r;
	t_ctlspec	s;
	int32_t		w;

	r = &app->ui.root;
	w = app->ui.area.w - 2 * APKLAY_MARGIN;
	s = (t_ctlspec){CT_LABEL, APKRUN_ID_HEAD, CTL_VISIBLE | CTL_ENABLED
		| CTL_BOLD, {APKLAY_MARGIN, APKLAY_MARGIN, w, APKLAY_TEXT_H}, T_HEAD};
	ctl_add(r, r->root, &s);
	s = (t_ctlspec){CT_LABEL, APKRUN_ID_MSG, CTL_VISIBLE | CTL_ENABLED
		| CTL_ELLIPSIS, {APKLAY_MARGIN, 2 * APKLAY_MARGIN + APKLAY_TEXT_H, w,
		APKLAY_TEXT_H}, app->msg};
	ctl_add(r, r->root, &s);
}

static void	add_boxes(t_apkrun *app, t_ctlroot *r)
{
	const t_droidview	*v;
	char				text[DROID_TEXT_MAX];
	t_ctlspec			s;
	uint32_t			i;

	i = 0;
	while (i < app->lay.n)
	{
		v = droid_view(app->d, app->lay.box[i].view);
		text[0] = '\0';
		if (v)
			apkglue_label(text, sizeof(text), v->text);
		s = (t_ctlspec){CT_LABEL, APKRUN_ID_BASE + app->lay.box[i].view,
			CTL_VISIBLE | CTL_ENABLED | CTL_ELLIPSIS, app->lay.box[i].r, text};
		if (app->lay.box[i].kind == DV_BUTTON)
			s.type = CT_BUTTON;
		if (app->lay.box[i].kind == DV_BUTTON)
			s.flags = CTL_VISIBLE | CTL_ENABLED | CTL_FOCUSABLE;
		ctl_add(r, r->root, &s);
		i++;
	}
}

void	apkrun_rebuild(t_apkrun *app)
{
	clear(&app->ui.root);
	app->laid = app->ui.area;
	if (app->msg[0])
		add_message(app);
	else if (app->d && app->started)
	{
		apklay_run(app->d, rect_make(0, 0, app->ui.area.w, app->ui.area.h),
			&app->lay);
		add_boxes(app, &app->ui.root);
	}
	if (app->d)
		app->d->dirty = 0;
	app->redraw = 0;
	uiwin_flush(&app->ui);
}
