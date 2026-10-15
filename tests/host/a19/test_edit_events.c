#include <string.h>
#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	char_filters(void)
{
	t_fakeui	u;
	t_ctl		*e;
	t_inpevent	ev;

	fake_begin(&u, 200, 40);
	e = fake_edit(&u, "");
	memset(&ev, 0, sizeof(ev));
	ev.type = INP_CHAR;
	ev.code = 'a';
	ev.mods = INPM_ALT;
	h_true(!ctl_key(&u.r, &ev), "Alt+a : pas insere");
	ev.mods = INPM_CTRL;
	h_true(!ctl_key(&u.r, &ev), "Ctrl+a : pas insere");
	ev.mods = INPM_CTRL | INPM_ALT;
	ev.code = '@';
	h_true(ctl_key(&u.r, &ev), "AltGr+0 : @ insere");
	h_eq_str("AltGr", e->text, "@");
	fake_done(&u);
}

static void	invalid_code_points(void)
{
	t_fakeui	u;
	t_ctl		*e;
	uint32_t	cp;

	fake_begin(&u, 200, 40);
	e = fake_edit(&u, "");
	cp = 0;
	while (cp < 0x20)
		fake_char(&u.r, cp++);
	fake_char(&u.r, 0x7f);
	fake_char(&u.r, 0x80);
	fake_char(&u.r, 0x9f);
	fake_char(&u.r, 0xd800);
	fake_char(&u.r, 0xdfff);
	fake_char(&u.r, 0x110000);
	fake_char(&u.r, 0xffffffffu);
	h_eq_str("rien d'insere", e->text, "");
	fake_char(&u.r, 0xa0);
	fake_char(&u.r, 0x10ffff);
	h_eq_i64("U+00A0 et U+10FFFF acceptes", fake_utf8_chars(e->text), 2);
	fake_done(&u);
}

static void	keys_not_consumed(void)
{
	t_fakeui	u;
	t_ctl		*e;

	fake_begin(&u, 200, 40);
	e = fake_edit(&u, "ab");
	h_true(!fake_press(&u.r, VK_UP, 0), "Haut non consomme");
	h_true(!fake_press(&u.r, VK_DOWN, 0), "Bas non consomme");
	h_true(!fake_press(&u.r, VK_ESCAPE, 0), "Echap non consomme");
	h_true(!fake_press(&u.r, 'A', 0), "lettre sans INP_CHAR : non consomme");
	h_true(!fake_press(&u.r, 'A', INPM_ALT), "Alt+lettre : non consomme");
	h_true(fake_press(&u.r, VK_RETURN, 0), "Entree consomme");
	h_eq_i64("CN_ENTER", fake_cmd_find(1, CN_ENTER), 1);
	h_eq_i64("pas de CN_CHANGED", fake_cmd_find(1, CN_CHANGED), 0);
	h_eq_str("texte intact", e->text, "ab");
	fake_done(&u);
}

static void	disabled_and_callback_removal(void)
{
	t_fakeui	u;
	t_ctl		*e;

	fake_begin(&u, 200, 40);
	e = fake_edit(&u, "ab");
	ctl_set_flag(&u.r, e, CTL_ENABLED, false);
	fake_type(&u.r, "z");
	h_eq_str("desactive : pas d'edition", e->text, "ab");
	ctl_set_flag(&u.r, e, CTL_ENABLED, true);
	ctl_focus(&u.r, e);
	e->cb = fake_cb_kill;
	e->user = &u.r;
	h_true(fake_char(&u.r, 'z'), "frappe consommee");
	h_true(ctl_find(&u.r, 1) == NULL, "retire par le callback");
	h_eq_i64("pas de commande apres retrait", fake_cmd_count(), 0);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/edit_events");
	h_run("INP_CHAR : Alt, Ctrl, AltGr", char_filters);
	h_run("points de code invalides, controles", invalid_code_points);
	h_run("touches non consommees, Entree", keys_not_consumed);
	h_run("desactive, retrait dans le callback", disabled_and_callback_removal);
	return (h_end());
}
