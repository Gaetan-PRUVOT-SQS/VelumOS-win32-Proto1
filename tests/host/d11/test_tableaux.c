#include "harness.h"
#include "d11.h"

static void	arraycopy_recouvrement(void)
{
	t_d11	t;
	t_dref	a;
	int32_t	*p;

	h_eq_i64("ouverture", d11_open(&t, "d11.dex", 0), 0);
	p = d11_entiers(&t, &a, 5);
	h_eq_i64("vers la droite", d11_copie(&t, (uint32_t[]){a, 0, a, 1, 4}), 0);
	h_eq_i64("recouvrement droit", p[1] * 1000 + p[2] * 100 + p[4], 1204);
	h_eq_i64("vers la gauche", d11_copie(&t, (uint32_t[]){a, 1, a, 0, 4}), 0);
	h_eq_i64("recouvrement gauche", p[0] * 1000 + p[3] * 10 + p[4], 1044);
	h_eq_i64("longueur nulle en fin",
		d11_copie(&t, (uint32_t[]){a, 5, a, 5, 0}), 0);
	d11_close(&t);
}

static void	arraycopy_bornes_et_nuls(void)
{
	t_d11	t;
	t_dref	a;

	h_eq_i64("ouverture", d11_open(&t, "d11.dex", 0), 0);
	d11_entiers(&t, &a, 5);
	h_eq_i64("source depassee",
		d11_copie(&t, (uint32_t[]){a, 2, a, 0, 4}), D11_THROWN);
	h_eq_str("ArrayIndexOutOfBounds", t.exc, X_AIOOBE);
	h_eq_i64("cible depassee",
		d11_copie(&t, (uint32_t[]){a, 0, a, 2, 4}), D11_THROWN);
	h_eq_i64("position negative",
		d11_copie(&t, (uint32_t[]){a, D11_MIN, a, 0, 1}), D11_THROWN);
	h_eq_i64("longueur negative",
		d11_copie(&t, (uint32_t[]){a, 0, a, 0, 0xffffffffu}), D11_THROWN);
	h_eq_i64("position 6",
		d11_copie(&t, (uint32_t[]){a, 6, a, 0, 0}), D11_THROWN);
	h_eq_i64("source nulle",
		d11_copie(&t, (uint32_t[]){0, 0, a, 0, 1}), D11_THROWN);
	h_eq_str("NullPointerException", t.exc, X_NPE);
	h_eq_i64("cible nulle",
		d11_copie(&t, (uint32_t[]){a, 0, 0, 0, 1}), D11_THROWN);
	d11_close(&t);
}

static void	arraycopy_types_primitifs(void)
{
	t_d11	t;
	t_dref	r[3];

	h_eq_i64("ouverture", d11_open(&t, "d11.dex", 0), 0);
	d11_entiers(&t, &r[0], 4);
	dvm_array_new(t.vm, "[S", 4, &r[1]);
	dvm_pin(t.vm, r[1]);
	dvm_array_new(t.vm, "[Ljava/lang/Object;", 2, &r[2]);
	dvm_pin(t.vm, r[2]);
	h_eq_i64("int vers short",
		d11_copie(&t, (uint32_t[]){r[0], 0, r[1], 0, 1}), D11_THROWN);
	h_eq_str("ArrayStoreException", t.exc, X_ASE);
	h_eq_i64("int vers objets",
		d11_copie(&t, (uint32_t[]){r[0], 0, r[2], 0, 1}), D11_THROWN);
	h_eq_i64("source pas un tableau", d11_copie(&t,
			(uint32_t[]){d11_str(&t, "x"), 0, r[2], 0, 1}), D11_THROWN);
	h_eq_str("ArrayStoreException sans tableau", t.exc, X_ASE);
	d11_close(&t);
}

static void	arraycopy_types_references(void)
{
	t_d11		t;
	t_dref		r[2];
	t_darrview	v;

	h_eq_i64("ouverture", d11_open(&t, "d11.dex", 0), 0);
	dvm_array_new(t.vm, "[Ljava/lang/Object;", 2, &r[0]);
	dvm_pin(t.vm, r[0]);
	dvm_array_new(t.vm, "[Ljava/lang/String;", 2, &r[1]);
	dvm_pin(t.vm, r[1]);
	dvm_array_view(t.vm, r[0], &v);
	((uint32_t *)v.data)[0] = d11_str(&t, "un");
	((uint32_t *)v.data)[1] = d11_new(&t, "Ljava/lang/Object;");
	h_eq_i64("element incompatible apres un prefixe",
		d11_copie(&t, (uint32_t[]){r[0], 0, r[1], 0, 2}), D11_THROWN);
	h_eq_str("ArrayStoreException a l'element", t.exc, X_ASE);
	dvm_array_view(t.vm, r[1], &v);
	h_true(((uint32_t *)v.data)[0] != 0, "le prefixe est copie");
	h_eq_u64("le second element reste nul", ((uint32_t *)v.data)[1], 0);
	h_eq_i64("String[] vers Object[]",
		d11_copie(&t, (uint32_t[]){r[1], 0, r[0], 1, 1}), 0);
	d11_close(&t);
}

int	main(void)
{
	h_begin("d11/tableaux");
	h_run("arraycopy_recouvrement", arraycopy_recouvrement);
	h_run("arraycopy_bornes_et_nuls", arraycopy_bornes_et_nuls);
	h_run("arraycopy_types_primitifs", arraycopy_types_primitifs);
	h_run("arraycopy_types_references", arraycopy_types_references);
	return (h_end());
}
