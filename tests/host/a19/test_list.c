#include <string.h>
#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	empty_list(void)
{
	t_fakeui	u;
	t_ctl		*l;
	char		out[16];

	fake_begin(&u, 200, 100);
	l = fake_list(&u, 0);
	h_eq_i64("vide", ctl_list_count(l), 0);
	h_eq_i64("rien de selectionne", ctl_list_selected(l), -1);
	fake_keys(&u.r, VK_DOWN, 0, 2);
	fake_keys(&u.r, VK_END, 0, 1);
	fake_keys(&u.r, VK_NEXT, 0, 1);
	h_true(fake_press(&u.r, VK_RETURN, 0), "Entree consomme");
	fake_click(&u.r, 20, 20);
	h_eq_i64("aucune notification", fake_cmd_count(), 0);
	h_eq_i64("remove(0)", ctl_list_remove(&u.r, l, 0), E_RANGE);
	h_eq_i64("select(0)", ctl_list_select(&u.r, l, 0), E_RANGE);
	h_eq_i64("select(-1)", ctl_list_select(&u.r, l, -1), 0);
	h_eq_i64("text(0)", ctl_list_text(l, 0, out, 16), E_RANGE);
	ctl_list_clear(&u.r, l);
	fake_done(&u);
}

static void	add_and_text(void)
{
	t_fakeui	u;
	t_ctl		*l;
	char		big[400];
	char		out[CTL_TEXT_MAX];

	fake_begin(&u, 200, 100);
	l = fake_list(&u, 0);
	h_eq_i64("index 0", ctl_list_add(&u.r, l, "un"), 0);
	h_eq_i64("index 1", ctl_list_add(&u.r, l, "deux"), 1);
	memset(big, 'k', sizeof(big));
	memcpy(big + 254, "\xc3\xa9", 2);
	big[300] = '\0';
	h_eq_i64("index 2", ctl_list_add(&u.r, l, big), 2);
	h_eq_i64("texte long : 254 octets", ctl_list_text(l, 2, out, sizeof(out)),
		254);
	h_eq_i64("lecture", ctl_list_text(l, 1, out, sizeof(out)), 4);
	h_eq_str("contenu", out, "deux");
	h_eq_i64("tampon trop petit", ctl_list_text(l, 1, out, 3), 2);
	h_eq_str("tronque", out, "de");
	h_eq_i64("indice negatif", ctl_list_text(l, -1, out, 16), E_RANGE);
	fake_done(&u);
}

static void	index_limits(void)
{
	t_fakeui	u;
	t_ctl		*l;

	fake_begin(&u, 200, 100);
	l = fake_list(&u, 5);
	h_eq_i64("select(count)", ctl_list_select(&u.r, l, 5), E_RANGE);
	h_eq_i64("select(-2)", ctl_list_select(&u.r, l, -2), E_RANGE);
	h_eq_i64("select(count-1)", ctl_list_select(&u.r, l, 4), 0);
	h_eq_i64("selected", ctl_list_selected(l), 4);
	h_eq_i64("remove(selectionne)", ctl_list_remove(&u.r, l, 4), 0);
	h_eq_i64("selection perdue", ctl_list_selected(l), -1);
	ctl_list_select(&u.r, l, 2);
	ctl_list_remove(&u.r, l, 0);
	h_eq_i64("selection decalee", ctl_list_selected(l), 1);
	ctl_list_remove(&u.r, l, 2);
	h_eq_i64("selection inchangee", ctl_list_selected(l), 1);
	h_eq_i64("remove(count)", ctl_list_remove(&u.r, l, 3), E_RANGE);
	ctl_list_clear(&u.r, l);
	h_true(ctl_list_count(l) == 0 && ctl_list_selected(l) == -1, "clear");
	fake_done(&u);
}

static void	ten_thousand(void)
{
	t_fakeui	u;
	t_ctl		*l;
	char		out[32];

	fake_begin(&u, 200, 100);
	l = fake_list(&u, 10000);
	h_eq_i64("10000 elements", ctl_list_count(l), 10000);
	ctl_list_text(l, 9999, out, sizeof(out));
	h_eq_str("dernier", out, "item 9999");
	fake_press(&u.r, VK_END, 0);
	h_eq_i64("Fin", ctl_list_selected(l), 9999);
	h_eq_i64("defilement au maximum", ((t_list *)l->priv)->sb.pos, 10000 - 3);
	fake_press(&u.r, VK_HOME, 0);
	h_eq_i64("Debut", ctl_list_selected(l), 0);
	h_eq_i64("defilement a 0", ((t_list *)l->priv)->sb.pos, 0);
	fake_log_clear();
	ctl_paint(&u.r);
	h_true(fake_log_kind(FK_TEXT) <= 5, "lignes visibles seulement");
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/list");
	h_run("liste vide : touches, clic, indices", empty_list);
	h_run("ajout, indices, texte tronque", add_and_text);
	h_run("indices limites et ajustement de la selection", index_limits);
	h_run("10000 elements", ten_thousand);
	return (h_end());
}
