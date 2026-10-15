#include <string.h>
#include "harness.h"
#include "fake.h"

static void	root_init_destroy(void)
{
	t_fakeui	u;
	t_rect		all;

	fake_mem_reset(0);
	h_eq_i64("init", fake_ui_open(&u, 200, 100), 0);
	h_true(u.r.root != NULL, "racine creee");
	h_eq_i64("type racine", u.r.root->type, CT_PANEL);
	all = rect_make(0, 0, 200, 100);
	h_true(fake_dirty_is(&u.r, &all, 1), "tout est sale apres init");
	h_true(u.r.focus == NULL && u.r.hot == NULL, "pas de focus");
	fake_ui_close(&u);
	h_eq_i64("aucune fuite", fake_mem_live(), 0);
	ctl_root_destroy(&u.r);
	ctl_root_destroy(NULL);
	h_eq_i64("init sans racine", ctl_root_init(NULL, &u.s, NULL), E_INVAL);
	h_eq_i64("init sans surface", ctl_root_init(&u.r, NULL, NULL), E_INVAL);
}

static void	add_and_find(void)
{
	t_fakeui	u;
	t_ctl		*b;
	t_ctl		*l;

	fake_mem_reset(0);
	fake_ui_open(&u, 200, 100);
	b = fake_add(&u, fake_spec(CT_BUTTON, 7, rect_make(5, 5, 50, 20), "OK"));
	l = fake_add(&u, fake_spec(CT_LABEL, 0, rect_make(5, 30, 50, 12), "Nom"));
	h_true(b && l, "ajouts");
	h_true(ctl_find(&u.r, 7) == b, "find par id");
	h_true(ctl_find(&u.r, 8) == NULL, "find absent");
	h_true(ctl_find(&u.r, 0) == NULL, "id 0 anonyme jamais trouve");
	h_true(b->parent == u.r.root && u.r.root->first == b, "parent racine");
	h_true(b->next == l && l->next == NULL, "ordre d'ajout");
	h_eq_str("texte copie", b->text, "OK");
	l = fake_add(&u, fake_spec(CT_LABEL, 0, rect_make(0, 0, 5, 5), ""));
	h_true(l != NULL, "deux anonymes autorises");
	fake_ui_close(&u);
	h_eq_i64("aucune fuite", fake_mem_live(), 0);
}

static void	text_truncation(void)
{
	t_fakeui	u;
	char		big[400];
	t_ctl		*c;

	memset(big, 'a', sizeof(big));
	memcpy(big + 254, "\xc3\xa9", 2);
	big[256] = '\0';
	fake_mem_reset(0);
	fake_ui_open(&u, 100, 100);
	c = fake_add(&u, fake_spec(CT_LABEL, 1, rect_make(0, 0, 50, 12), big));
	h_eq_i64("coupe avant le caractere partiel", (int64_t)strlen(c->text), 254);
	h_true(fake_utf8_valid(c->text), "utf-8 valide apres coupe");
	ctl_set_text(&u.r, c, "x");
	h_eq_str("set_text", c->text, "x");
	fake_ui_close(&u);
}

int	main(void)
{
	h_begin("a19/tree");
	h_run("racine : init, destroy, arguments invalides", root_init_destroy);
	h_run("ajout, find, ordre, id anonyme", add_and_find);
	h_run("texte tronque a la frontiere utf-8", text_truncation);
	return (h_end());
}
