#include <string.h>
#include "harness.h"
#include "d11.h"

static void	texte_tronque_sans_couper_un_caractere(void)
{
	t_d11	t;
	char	buf[400];
	t_dref	tv;

	h_eq_i64("ouverture", d11_open(&t, "d11.dex", 0), 0);
	tv = d11_view(&t, TV);
	h_eq_i64("300 octets",
		d11_i2(&t, M_SETTEXT, tv, d11_str(&t, d11_long(buf, 300, ""))), 0);
	h_eq_u64("255 octets gardes", strlen(t.d.views[0].text), 255);
	d11_i2(&t, M_SETTEXT, tv, d11_str(&t, d11_long(buf, 254, "é")));
	h_eq_u64("caractere de 2 octets a cheval retire",
		strlen(t.d.views[0].text), 254);
	d11_i2(&t, M_SETTEXT, tv, d11_str(&t, d11_long(buf, 253, "é")));
	h_eq_u64("caractere de 2 octets qui tient", strlen(t.d.views[0].text), 255);
	d11_i2(&t, M_SETTEXT, tv, d11_str(&t, d11_long(buf, 253, "€")));
	h_eq_u64("caractere de 3 octets a cheval", strlen(t.d.views[0].text), 253);
	d11_i2(&t, M_SETTEXT, tv, d11_str(&t, d11_long(buf, 252, "😀")));
	h_eq_u64("paire de substitution a cheval", strlen(t.d.views[0].text), 252);
	h_eq_i64("getText", d11_nat(&t, M_GETTEXT, &tv), 0);
	h_eq_i64("getText rend le texte garde",
		d11_i2(&t, M_LEN, (t_dref)t.ret, 0), 252);
	h_eq_i64("texte nul", d11_i2(&t, M_SETTEXT, tv, DVM_NULL), 0);
	h_eq_str("texte vide", t.d.views[0].text, "");
	d11_close(&t);
}

static void	arbre_cycles_et_placements(void)
{
	t_d11	t;
	t_dref	a;
	t_dref	b;
	t_dref	c;

	h_eq_i64("ouverture", d11_open(&t, "d11.dex", 0), 0);
	a = d11_view(&t, LL);
	b = d11_view(&t, LL);
	c = d11_view(&t, LL);
	h_eq_i64("a recoit b", d11_i2(&t, M_ADD, a, b), 0);
	h_eq_i64("b recoit c", d11_i2(&t, M_ADD, b, c), 0);
	h_eq_i64("cycle long", d11_i2(&t, M_ADD, c, a), D11_THROWN);
	h_eq_str("IllegalStateException", t.exc, X_ISE);
	h_eq_i64("soi-meme", d11_i2(&t, M_ADD, a, a), D11_THROWN);
	h_eq_i64("deja placee", d11_i2(&t, M_ADD, a, c), D11_THROWN);
	h_eq_i64("enfant nul", d11_i2(&t, M_ADD, a, DVM_NULL), D11_THROWN);
	h_eq_str("NullPointerException", t.exc, X_NPE);
	h_eq_i64("enfant chaine", d11_i2(&t, M_ADD, a, d11_str(&t, "x")),
		D11_THROWN);
	h_eq_str("ClassCastException", t.exc, X_CCE);
	h_eq_u64("un seul enfant", t.d.views[0].nkids, 1);
	h_eq_i64("orientation 1", d11_i2(&t, M_ORIENT, a, 1), 0);
	h_eq_i64("orientation 2", d11_i2(&t, M_ORIENT, a, 2), D11_THROWN);
	h_eq_str("IllegalArgumentException", t.exc, X_IAE);
	d11_close(&t);
}

static void	references_hostiles(void)
{
	t_d11	t;
	t_dref	tv;
	t_dref	brute;
	t_dref	s;

	h_eq_i64("ouverture", d11_open(&t, "d11.dex", 0), 0);
	tv = d11_view(&t, TV);
	brute = d11_new(&t, TV);
	s = d11_str(&t, "x");
	h_eq_i64("setText sur chaine", d11_i2(&t, M_SETTEXT, s, s), D11_THROWN);
	h_eq_str("ClassCastException", t.exc, X_CCE);
	h_eq_i64("setText sur nul", d11_i2(&t, M_SETTEXT, 0, s), D11_THROWN);
	h_eq_str("NullPointerException", t.exc, X_NPE);
	h_eq_i64("jamais construite", d11_i2(&t, M_SETTEXT, brute, s), D11_THROWN);
	h_eq_i64("addView sur TextView", d11_i2(&t, M_ADD, tv, tv), D11_THROWN);
	h_eq_i64("orientation TextView", d11_i2(&t, M_ORIENT, tv, 1), D11_THROWN);
	h_eq_i64("ecouteur chaine", d11_i2(&t, M_LISTEN, tv, s), D11_THROWN);
	h_eq_str("ecouteur refuse", t.exc, X_CCE);
	h_eq_i64("ecouteur nul accepte", d11_i2(&t, M_LISTEN, tv, 0), 0);
	h_eq_i64("double construction", d11_nat(&t, M_TVINIT, &tv), DVM_THROWN);
	h_eq_str("IllegalStateException", t.exc, X_ISE);
	h_eq_u64("une seule fiche", t.d.nviews, 1);
	d11_close(&t);
}

static void	journal_quatre_niveaux(void)
{
	t_d11	t;
	t_dref	tag;
	t_dref	msg;

	h_eq_i64("ouverture", d11_open(&t, "d11.dex", 0), 0);
	tag = d11_str(&t, "Étiq");
	msg = d11_str(&t, "été");
	h_eq_i64("Log.d rend la taille", d11_i2(&t, M_LOGD, tag, msg), 5);
	h_eq_u64("niveau debug", t.level, DROID_LOG_DEBUG);
	h_eq_str("UTF-8", t.log, "Étiq: été");
	d11_i2(&t, M_LOGI, tag, msg);
	h_eq_u64("niveau info", t.level, DROID_LOG_INFO);
	d11_i2(&t, M_LOGW, tag, msg);
	h_eq_u64("niveau warn", t.level, DROID_LOG_WARN);
	d11_i2(&t, M_LOGE, tag, msg);
	h_eq_u64("niveau error", t.level, DROID_LOG_ERROR);
	h_eq_u64("quatre lignes", t.nlogs, 4);
	h_eq_i64("message nul", d11_i2(&t, M_LOGE, tag, DVM_NULL), D11_THROWN);
	h_eq_str("NullPointerException", t.exc, X_NPE);
	t.d.log = NULL;
	h_eq_i64("sans journal", d11_i2(&t, M_LOGE, tag, msg), 5);
	h_eq_u64("toujours quatre lignes", t.nlogs, 4);
	d11_close(&t);
}

int	main(void)
{
	h_begin("d11/vues");
	h_run("texte_tronque_sans_couper_un_caractere",
		texte_tronque_sans_couper_un_caractere);
	h_run("arbre_cycles_et_placements", arbre_cycles_et_placements);
	h_run("references_hostiles", references_hostiles);
	h_run("journal_quatre_niveaux", journal_quatre_niveaux);
	return (h_end());
}
