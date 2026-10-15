#include <string.h>
#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	char_limit(void)
{
	t_fakeui	u;
	t_ctl		*e;
	t_edit		*ed;

	fake_begin(&u, 200, 40);
	e = fake_edit(&u, "");
	ed = e->priv;
	h_eq_i64("limite fixee", ctl_edit_set_limit(&u.r, e, 5), 0);
	fake_type(&u.r, "a\xc3\xa9\xe2\x82\xac\xf0\x9f\x98\x80z!!!");
	h_eq_i64("5 caracteres au maximum", fake_utf8_chars(e->text), 5);
	h_eq_str("les 5 premiers", e->text,
		"a\xc3\xa9\xe2\x82\xac\xf0\x9f\x98\x80z");
	h_eq_i64("5 notifications", fake_cmd_find(1, CN_CHANGED), 5);
	h_true(fake_char(&u.r, 'x'), "plein : touche consommee");
	h_eq_i64("plein : pas de notification", fake_cmd_find(1, CN_CHANGED), 5);
	ed->anchor = 0;
	ed->caret = 1;
	fake_type(&u.r, "Q");
	h_eq_i64("remplacement si plein", fake_utf8_chars(e->text), 5);
	fake_done(&u);
}

static void	byte_capacity(void)
{
	t_fakeui	u;
	t_ctl		*e;
	int			i;

	fake_begin(&u, 200, 40);
	e = fake_edit(&u, "");
	i = 0;
	while (i < 70)
	{
		fake_type(&u.r, "\xf0\x9f\x98\x80");
		i++;
	}
	h_eq_i64("63 emojis de 4 octets", (int64_t)strlen(e->text), 252);
	fake_type(&u.r, "abcdef");
	h_eq_i64("complete jusqu'a 255 octets", (int64_t)strlen(e->text), 255);
	h_true(fake_utf8_valid(e->text), "jamais de caractere coupe");
	fake_done(&u);
}

static void	limit_api(void)
{
	t_fakeui	u;
	t_ctl		*e;
	t_ctl		*b;

	fake_begin(&u, 200, 40);
	e = fake_edit(&u, "\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9");
	b = fk_c(&u, CT_BUTTON, 2, rect_make(0, 30, 10, 10));
	h_eq_i64("limite 0 refusee", ctl_edit_set_limit(&u.r, e, 0), E_INVAL);
	h_eq_i64("pas un edit", ctl_edit_set_limit(&u.r, b, 3), E_INVAL);
	h_eq_i64("limite 3", ctl_edit_set_limit(&u.r, e, 3), 0);
	h_eq_str("texte tronque a 3 caracteres", e->text,
		"\xc3\xa9\xc3\xa9\xc3\xa9");
	h_true(((t_edit *)e->priv)->caret <= 6, "caret ramene dans le texte");
	h_eq_i64("limite enorme", ctl_edit_set_limit(&u.r, e, 1000000), 0);
	h_eq_i64("plafond a 255", ((t_edit *)e->priv)->max_chars, 255);
	fake_done(&u);
}

static void	set_text_cases(void)
{
	t_fakeui	u;
	t_ctl		*e;
	char		big[400];

	fake_begin(&u, 200, 40);
	e = fake_edit(&u, "");
	memset(big, 'k', sizeof(big));
	big[399] = '\0';
	ctl_set_text(&u.r, e, big);
	h_eq_i64("texte long tronque", (int64_t)strlen(e->text), 255);
	h_eq_i64("caret en fin", ((t_edit *)e->priv)->caret, 255);
	ctl_edit_set_limit(&u.r, e, 4);
	ctl_set_text(&u.r, e, "\xc3\xa9\xe2\x82\xac\xc3\xa9\xe2\x82\xac\xc3\xa9");
	h_eq_i64("limite appliquee au texte donne", fake_utf8_chars(e->text), 4);
	ctl_set_text(&u.r, e, "\xff\xfe");
	fake_press(&u.r, VK_BACK, 0);
	h_eq_i64("octet invalide : un par un", (int64_t)strlen(e->text), 1);
	h_eq_i64("set_text sans notification", fake_cmd_find(1, CN_CHANGED), 1);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/edit_limit");
	h_run("longueur maximale en caracteres", char_limit);
	h_run("capacite en octets : jamais de coupe", byte_capacity);
	h_run("ctl_edit_set_limit : erreurs et bornes", limit_api);
	h_run("ctl_set_text : troncature, limite, invalides", set_text_cases);
	return (h_end());
}
