#include <string.h>
#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static const char	*g_text = "a\xc3\xa9\xe2\x82\xac\xf0\x9f\x98\x80z";

static void	insert_multibyte(void)
{
	t_fakeui	u;
	t_ctl		*e;
	t_edit		*ed;

	fake_begin(&u, 200, 40);
	e = fake_edit(&u, "");
	ed = e->priv;
	h_eq_i64("5 caracteres tapes", fake_type(&u.r, g_text), 5);
	h_eq_str("octets exacts", e->text, g_text);
	h_eq_i64("longueur en octets", (int64_t)strlen(e->text), 11);
	h_eq_i64("caret en fin", ed->caret, 11);
	h_true(ed->anchor == ed->caret, "pas de selection");
	h_true(fake_utf8_valid(e->text), "utf-8 valide");
	h_eq_i64("une notification par caractere", fake_cmd_find(1, CN_CHANGED), 5);
	h_eq_i64("callback aussi", fake_cmd_find(1, CN_CHANGED | FAKE_CB_MARK), 5);
	fake_done(&u);
}

static void	backspace_each(void)
{
	t_fakeui	u;
	t_ctl		*e;
	const int	want[5] = {10, 6, 3, 1, 0};
	int			i;

	fake_begin(&u, 200, 40);
	e = fake_edit(&u, g_text);
	i = 0;
	while (i < 5)
	{
		h_true(fake_press(&u.r, VK_BACK, 0), "Retour arriere consomme");
		h_eq_i64("longueur apres retour arriere", (int64_t)strlen(e->text),
			want[i]);
		h_true(fake_utf8_valid(e->text), "utf-8 valide");
		i++;
	}
	h_true(fake_press(&u.r, VK_BACK, 0), "au bord : consomme");
	h_eq_i64("au bord : rien ne change", fake_cmd_find(1, CN_CHANGED), 5);
	fake_done(&u);
}

static void	delete_forward(void)
{
	t_fakeui	u;
	t_ctl		*e;
	const int	want[5] = {10, 8, 5, 1, 0};
	int			i;

	fake_begin(&u, 200, 40);
	e = fake_edit(&u, g_text);
	fake_press(&u.r, VK_HOME, 0);
	i = 0;
	while (i < 5)
	{
		fake_press(&u.r, VK_DELETE, 0);
		h_eq_i64("longueur apres Suppr", (int64_t)strlen(e->text), want[i]);
		h_true(fake_utf8_valid(e->text), "utf-8 valide");
		i++;
	}
	h_true(fake_press(&u.r, VK_DELETE, 0), "Suppr en fin : consomme");
	h_eq_i64("Suppr en fin : inchange", fake_cmd_find(1, CN_CHANGED), 5);
	fake_done(&u);
}

static void	navigation_and_middle_insert(void)
{
	t_fakeui	u;
	t_ctl		*e;
	t_edit		*ed;
	const int	pos[6] = {0, 1, 3, 6, 10, 11};
	int			i;

	fake_begin(&u, 200, 40);
	e = fake_edit(&u, g_text);
	ed = e->priv;
	fake_press(&u.r, VK_HOME, 0);
	i = 0;
	while (i < 6)
	{
		h_eq_i64("Droite : frontiere", ed->caret, pos[i]);
		fake_press(&u.r, VK_RIGHT, 0);
		i++;
	}
	fake_keys(&u.r, VK_LEFT, 0, 4);
	h_eq_i64("Gauche x4", ed->caret, 1);
	fake_type(&u.r, "X");
	h_eq_str("insertion au milieu", e->text,
		"aX\xc3\xa9\xe2\x82\xac\xf0\x9f\x98\x80z");
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/edit");
	h_run("insertion e acute, euro, emoji", insert_multibyte);
	h_run("retour arriere sur chaque caractere", backspace_each);
	h_run("suppression avant sur chaque caractere", delete_forward);
	h_run("navigation, insertion au milieu", navigation_and_middle_insert);
	return (h_end());
}
