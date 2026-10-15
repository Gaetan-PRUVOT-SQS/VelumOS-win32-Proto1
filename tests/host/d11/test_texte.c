#include "harness.h"
#include "d11.h"

static void	sept_append(void)
{
	t_d11		t;
	uint32_t	a[3];

	h_eq_i64("ouverture", d11_open(&t, "d11.dex", 0), 0);
	a[0] = d11_new(&t, SB);
	h_eq_i64("append String", d11_i2(&t, M_SBS, a[0], d11_str(&t, "Vé")), a[0]);
	h_eq_i64("append int", d11_i2(&t, M_SBI, a[0], D11_MIN), a[0]);
	a[1] = 0;
	a[2] = D11_MIN;
	h_eq_i64("append long", d11_nat(&t, M_SBJ, a), 0);
	h_eq_i64("append char", d11_i2(&t, M_SBC, a[0], 'x'), a[0]);
	h_eq_i64("append true", d11_i2(&t, M_SBZ, a[0], 1), a[0]);
	h_eq_i64("append false", d11_i2(&t, M_SBZ, a[0], 0), a[0]);
	h_eq_i64("append objet nul", d11_i2(&t, M_SBO, a[0], DVM_NULL), a[0]);
	h_eq_i64("append chaine nulle", d11_i2(&t, M_SBS, a[0], DVM_NULL), a[0]);
	h_eq_i64("append lui-meme", d11_i2(&t, M_SBO, a[0], a[0]), a[0]);
	h_eq_i64("length", d11_i2(&t, M_SBLEN, a[0], 0), 102);
	h_eq_i64("toString", d11_nat(&t, M_SBSTR, a), 0);
	h_eq_str("contenu", d11_text(&t, (t_dref)t.ret),
		"Vé-2147483648-9223372036854775808xtruefalsenullnull"
		"Vé-2147483648-9223372036854775808xtruefalsenullnull");
	d11_close(&t);
}

static void	tampon_plein_et_references_hostiles(void)
{
	t_d11		t;
	t_dref		sb;
	t_dref		dix;
	uint32_t	i;

	h_eq_i64("ouverture", d11_open(&t, "d11.dex", 0), 0);
	sb = d11_new(&t, SB);
	dix = d11_str(&t, "0123456789");
	i = 0;
	while (i++ < 102)
		d11_i2(&t, M_SBS, sb, dix);
	h_eq_i64("1020 caracteres", d11_i2(&t, M_SBLEN, sb, 0), 1020);
	h_eq_i64("1024 atteint", d11_i2(&t, M_SBS, sb, d11_str(&t, "abcd")), sb);
	h_eq_i64("1025 refuse", d11_i2(&t, M_SBC, sb, 'x'), D11_THROWN);
	h_eq_str("OutOfMemoryError", t.exc, X_OOME);
	h_eq_i64("longueur intacte", d11_i2(&t, M_SBLEN, sb, 0), 1024);
	h_eq_i64("receveur nul", d11_i2(&t, M_SBI, DVM_NULL, 1), D11_THROWN);
	h_eq_str("NullPointerException", t.exc, X_NPE);
	h_eq_i64("receveur chaine", d11_i2(&t, M_SBI, dix, 1), D11_THROWN);
	h_eq_str("ClassCastException", t.exc, X_CCE);
	h_eq_i64("reference morte", d11_i2(&t, M_SBI, 0x7ffffff0u, 1), D11_THROWN);
	h_eq_i64("length sur chaine", d11_i2(&t, M_SBLEN, dix, 0), D11_THROWN);
	d11_close(&t);
}

int	main(void)
{
	h_begin("d11/texte");
	h_run("sept_append", sept_append);
	h_run("tampon_plein_et_references_hostiles",
		tampon_plein_et_references_hostiles);
	return (h_end());
}
