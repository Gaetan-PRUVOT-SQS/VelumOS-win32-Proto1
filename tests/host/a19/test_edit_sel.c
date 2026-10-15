#include <string.h>
#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static const char	*g_text = "a\xc3\xa9\xe2\x82\xac\xf0\x9f\x98\x80z";

static void	shift_select_straddle(void)
{
	t_fakeui	u;
	t_ctl		*e;
	t_edit		*ed;

	fake_begin(&u, 200, 40);
	e = fake_edit(&u, g_text);
	ed = e->priv;
	fake_press(&u.r, VK_HOME, 0);
	fake_keys(&u.r, VK_RIGHT, 0, 2);
	fake_keys(&u.r, VK_RIGHT, INPM_SHIFT, 2);
	h_eq_i64("ancre", ed->anchor, 3);
	h_eq_i64("caret apres l'euro et l'emoji", ed->caret, 10);
	fake_press(&u.r, VK_LEFT, INPM_SHIFT);
	h_eq_i64("Maj+Gauche : caret", ed->caret, 6);
	fake_type(&u.r, "Q");
	h_eq_str("la frappe remplace la selection", e->text,
		"a\xc3\xa9Q\xf0\x9f\x98\x80z");
	h_true(ed->anchor == ed->caret, "plus de selection");
	fake_done(&u);
}

static void	select_all_and_delete(void)
{
	t_fakeui	u;
	t_ctl		*e;
	t_edit		*ed;

	fake_begin(&u, 200, 40);
	e = fake_edit(&u, g_text);
	ed = e->priv;
	fake_press(&u.r, 'A', INPM_CTRL);
	h_true(ed->anchor == 0 && ed->caret == 11, "Ctrl+A");
	h_eq_i64("Ctrl+A ne modifie pas le texte", fake_cmd_find(1, CN_CHANGED), 0);
	fake_press(&u.r, VK_BACK, 0);
	h_eq_str("tout efface", e->text, "");
	h_eq_i64("une seule notification", fake_cmd_find(1, CN_CHANGED), 1);
	fake_press(&u.r, 'A', INPM_CTRL);
	h_true(ed->anchor == 0 && ed->caret == 0, "Ctrl+A sur texte vide");
	fake_done(&u);
}

static void	collapse_and_extend(void)
{
	t_fakeui	u;
	t_ctl		*e;
	t_edit		*ed;

	fake_begin(&u, 200, 40);
	e = fake_edit(&u, g_text);
	ed = e->priv;
	ed->anchor = 3;
	ed->caret = 10;
	fake_press(&u.r, VK_LEFT, 0);
	h_true(ed->caret == 3 && ed->anchor == 3, "Gauche : debut de selection");
	ed->anchor = 3;
	ed->caret = 10;
	fake_press(&u.r, VK_RIGHT, 0);
	h_true(ed->caret == 10 && ed->anchor == 10, "Droite : fin de selection");
	fake_press(&u.r, VK_HOME, INPM_SHIFT);
	h_true(ed->caret == 0 && ed->anchor == 10, "Maj+Debut garde l'ancre");
	fake_press(&u.r, VK_END, INPM_SHIFT);
	h_true(ed->caret == 11 && ed->anchor == 10, "Maj+Fin");
	fake_press(&u.r, VK_END, 0);
	h_true(ed->caret == 11 && ed->anchor == 11, "Fin efface la selection");
	h_true(e != NULL, "controle present");
	fake_done(&u);
}

static void	delete_selection_only(void)
{
	t_fakeui	u;
	t_ctl		*e;
	t_edit		*ed;

	fake_begin(&u, 200, 40);
	e = fake_edit(&u, g_text);
	ed = e->priv;
	ed->anchor = 1;
	ed->caret = 6;
	fake_press(&u.r, VK_BACK, 0);
	h_eq_str("Retour arriere : selection", e->text, "a\xf0\x9f\x98\x80z");
	ed->anchor = 1;
	ed->caret = 5;
	fake_press(&u.r, VK_DELETE, 0);
	h_eq_str("Suppr : la selection seule", e->text, "az");
	h_true(ed->caret == 1 && ed->anchor == 1, "caret au debut de la zone");
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/edit_sel");
	h_run("Maj+fleches a cheval, multi-octets", shift_select_straddle);
	h_run("Ctrl+A puis suppression", select_all_and_delete);
	h_run("fleches sur selection, Maj+Debut/Fin", collapse_and_extend);
	h_run("suppression de la selection seule", delete_selection_only);
	return (h_end());
}
