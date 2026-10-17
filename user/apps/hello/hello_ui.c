#include "velum/libk.h"
#include "../common/platform.h"
#include "../common/tbuf.h"
#include "../common/timefmt.h"
#include "../common/typo.h"
#include "hello.h"

static int	add_label(t_ctlroot *r, t_ctlspec s, t_fontid font)
{
	t_ctl	*c;

	c = ctl_add(r, r->root, &s);
	if (!c)
		return (-1);
	ctl_set_font(r, c, font);
	return (0);
}

static int	add_labels(t_ctlroot *r)
{
	t_ctlspec	s;

	s = (t_ctlspec){CT_LABEL, HELLO_ID_TITLE, CTL_VISIBLE
		| CTL_ENABLED, {HELLO_MARGIN, HELLO_TITLE_Y, HELLO_TEXT_W,
		HELLO_TITLE_H}, "Bonjour de VelumOS" NBSP "!"};
	if (add_label(r, s, HELLO_FONT_TITLE) < 0)
		return (-1);
	s = (t_ctlspec){CT_LABEL, HELLO_ID_UPTIME, CTL_VISIBLE | CTL_ENABLED
		| CTL_ELLIPSIS, {HELLO_MARGIN, HELLO_UPTIME_Y, HELLO_TEXT_W,
		HELLO_LINE_H}, ""};
	if (add_label(r, s, HELLO_FONT_UPTIME) < 0)
		return (-1);
	s = (t_ctlspec){CT_LABEL, HELLO_ID_NOTE, CTL_VISIBLE | CTL_ENABLED
		| CTL_ELLIPSIS, {HELLO_MARGIN, HELLO_NOTE_Y, HELLO_TEXT_W,
		HELLO_LINE_H}, "Application de démonstration de la session."};
	if (add_label(r, s, HELLO_FONT_NOTE) < 0)
		return (-1);
	return (0);
}

int	hello_build(t_hello *app)
{
	t_ctlroot	*r;
	t_ctlspec	s;
	t_ctl		*btn;

	r = &app->ui.root;
	if (add_labels(r) < 0)
		return (-1);
	s = (t_ctlspec){CT_BUTTON, CTL_ID_CANCEL, CTL_VISIBLE | CTL_ENABLED
		| CTL_FOCUSABLE | CTL_DEFAULT, {HELLO_BTN_X, HELLO_BTN_Y,
		HELLO_BTN_W, HELLO_BTN_H}, "Fermer"};
	btn = ctl_add(r, r->root, &s);
	if (!btn)
		return (-1);
	ctl_set_font(r, btn, HELLO_FONT_BUTTON);
	ctl_focus(r, btn);
	return (0);
}

void	hello_on_command(void *c, uint32_t code, void *user)
{
	const t_ctl	*ctl;
	t_hello		*app;

	ctl = c;
	app = user;
	if (app && code == CN_CLICKED && ctl->id == CTL_ID_CANCEL)
		app->quit = 1;
}

void	hello_tick(t_hello *app)
{
	t_sysinfo	si;
	char		text[96];
	char		up[48];
	t_tbuf		b;
	t_ctl		*label;

	label = ctl_find(&app->ui.root, HELLO_ID_UPTIME);
	if (!label || os_sysinfo(&si) < 0)
		return ;
	fmt_uptime(si.uptime_ns, up, sizeof(up));
	tb_init(&b, text, sizeof(text));
	tb_str(&b, "Temps de fonctionnement" NBSP ": ");
	tb_str(&b, up);
	ctl_set_text(&app->ui.root, label, text);
	uiwin_flush(&app->ui);
}
