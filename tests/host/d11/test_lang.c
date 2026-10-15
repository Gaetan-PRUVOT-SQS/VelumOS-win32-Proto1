#include "harness.h"
#include "d11.h"

static void	math_valeurs_limites(void)
{
	t_d11	t;

	h_eq_i64("ouverture", d11_open(&t, "d11.dex", 0), 0);
	h_eq_i64("abs(-5)", d11_i2(&t, M_ABS, (uint32_t)-5, 0), 5);
	h_eq_i64("abs(0)", d11_i2(&t, M_ABS, 0, 0), 0);
	h_eq_i64("abs(MAX)", d11_i2(&t, M_ABS, D11_MAX, 0), INT32_MAX);
	h_eq_i64("abs(MIN) reste MIN", d11_i2(&t, M_ABS, D11_MIN, 0), INT32_MIN);
	h_eq_i64("min(-1,1)", d11_i2(&t, M_MIN, (uint32_t)-1, 1), -1);
	h_eq_i64("min(MAX,MIN)", d11_i2(&t, M_MIN, D11_MAX, D11_MIN), INT32_MIN);
	h_eq_i64("min(3,3)", d11_i2(&t, M_MIN, 3, 3), 3);
	h_eq_i64("max(-1,1)", d11_i2(&t, M_MAX, (uint32_t)-1, 1), 1);
	h_eq_i64("max(MIN,MAX)", d11_i2(&t, M_MAX, D11_MIN, D11_MAX), INT32_MAX);
	t.now = 0x123456789abull;
	h_eq_i64("currentTimeMillis", d11_nat(&t, M_MILLIS, NULL), 0);
	h_eq_u64("horloge sur 64 bits", t.ret, 0x123456789abull);
	t.d.clock = NULL;
	h_eq_i64("sans horloge", d11_nat(&t, M_MILLIS, NULL), 0);
	h_eq_u64("zero sans horloge", t.ret, 0);
	d11_close(&t);
}

static void	parseint_signe_et_debordement(void)
{
	t_d11	t;

	h_eq_i64("ouverture", d11_open(&t, "d11.dex", 0), 0);
	h_eq_i64("0", d11_parse(&t, "0"), 0);
	h_eq_i64("+42", d11_parse(&t, "+42"), 42);
	h_eq_i64("-7", d11_parse(&t, "-7"), -7);
	h_eq_i64("MAX", d11_parse(&t, "2147483647"), INT32_MAX);
	h_eq_i64("MIN", d11_parse(&t, "-2147483648"), INT32_MIN);
	h_eq_i64("MAX+1", d11_parse(&t, "2147483648"), D11_THROWN);
	h_eq_str("NumberFormatException", t.exc, X_NFE);
	h_eq_i64("MIN-1", d11_parse(&t, "-2147483649"), D11_THROWN);
	h_eq_i64("vingt chiffres",
		d11_parse(&t, "99999999999999999999"), D11_THROWN);
	h_eq_i64("vide", d11_parse(&t, ""), D11_THROWN);
	h_eq_i64("signe seul", d11_parse(&t, "-"), D11_THROWN);
	h_eq_i64("lettre", d11_parse(&t, "12a"), D11_THROWN);
	h_eq_i64("nul", d11_i2(&t, M_PARSE, DVM_NULL, 0), D11_THROWN);
	h_eq_str("nul donne NumberFormatException", t.exc, X_NFE);
	d11_close(&t);
}

static void	chaines_valeurs_limites(void)
{
	t_d11	t;
	t_dref	abc;

	h_eq_i64("ouverture", d11_open(&t, "d11.dex", 0), 0);
	abc = d11_str(&t, "abc");
	h_eq_i64("length vide", d11_i2(&t, M_LEN, d11_str(&t, ""), 0), 0);
	h_eq_i64("length en UTF-16", d11_i2(&t, M_LEN, d11_str(&t, "héé"), 0), 3);
	h_eq_i64("charAt(0)", d11_i2(&t, M_CHARAT, abc, 0), 'a');
	h_eq_i64("charAt(2)", d11_i2(&t, M_CHARAT, abc, 2), 'c');
	h_eq_i64("charAt(3)", d11_i2(&t, M_CHARAT, abc, 3), D11_THROWN);
	h_eq_str("StringIndexOutOfBounds", t.exc, X_SIOOBE);
	h_eq_i64("charAt(-1)", d11_i2(&t, M_CHARAT, abc, D11_MIN), D11_THROWN);
	h_eq_i64("hashCode abc", d11_i2(&t, M_SHASH, abc, 0), 96354);
	h_eq_i64("hashCode vide", d11_i2(&t, M_SHASH, d11_str(&t, ""), 0), 0);
	h_eq_i64("equals", d11_i2(&t, M_SEQ, abc, d11_str(&t, "abc")), 1);
	h_eq_i64("equals autre", d11_i2(&t, M_SEQ, abc, d11_str(&t, "abd")), 0);
	h_eq_i64("equals nul", d11_i2(&t, M_SEQ, abc, DVM_NULL), 0);
	h_eq_i64("concat", d11_i2(&t, M_CONCAT, abc, d11_str(&t, "de")) != 0, 1);
	h_eq_str("abcde", d11_text(&t, (t_dref)t.ret), "abcde");
	h_eq_i64("concat nul", d11_i2(&t, M_CONCAT, abc, DVM_NULL), D11_THROWN);
	h_eq_str("NullPointerException", t.exc, X_NPE);
	h_eq_i64("length sur nul", d11_i2(&t, M_LEN, DVM_NULL, 0), D11_THROWN);
	d11_close(&t);
}

static void	entiers_en_texte_et_objet(void)
{
	t_d11	t;
	t_dref	o;

	h_eq_i64("ouverture", d11_open(&t, "d11.dex", 0), 0);
	h_eq_i64("toString(0)", d11_nat(&t, M_ITOS, (uint32_t[]){0}), 0);
	h_eq_str("0", d11_text(&t, (t_dref)t.ret), "0");
	h_eq_i64("toString(MIN)", d11_nat(&t, M_ITOS, (uint32_t[]){D11_MIN}), 0);
	h_eq_str("MIN", d11_text(&t, (t_dref)t.ret), "-2147483648");
	h_eq_i64("valueOf(MAX)", d11_nat(&t, M_VALUEOF, (uint32_t[]){D11_MAX}), 0);
	h_eq_str("MAX", d11_text(&t, (t_dref)t.ret), "2147483647");
	o = d11_new(&t, "Ljava/lang/Object;");
	h_eq_i64("equals identite", d11_i2(&t, M_OEQ, o, o), 1);
	h_eq_i64("equals autre", d11_i2(&t, M_OEQ, o, d11_str(&t, "x")), 0);
	h_eq_i64("hashCode stable", d11_i2(&t, M_OHASH, o, 0), (int64_t)o);
	h_eq_i64("toString", d11_nat(&t, M_OSTR, &o), 0);
	h_true(d11_has(d11_text(&t, (t_dref)t.ret), "java.lang.Object@"),
		"Classe@indice");
	h_eq_i64("exception creee", dvm_throw(t.vm, X_ISE, "motif"), DVM_THROWN);
	o = t.vm->pending;
	t.vm->pending = DVM_NULL;
	h_eq_i64("getMessage", d11_nat(&t, M_GETMSG, &o), 0);
	h_eq_str("message", d11_text(&t, (t_dref)t.ret), "motif");
	d11_close(&t);
}

int	main(void)
{
	h_begin("d11/lang");
	h_run("math_valeurs_limites", math_valeurs_limites);
	h_run("parseint_signe_et_debordement", parseint_signe_et_debordement);
	h_run("chaines_valeurs_limites", chaines_valeurs_limites);
	h_run("entiers_en_texte_et_objet", entiers_en_texte_et_objet);
	return (h_end());
}
