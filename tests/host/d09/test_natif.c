#include <string.h>
#include "harness.h"
#include "d09.h"

static int	rappel(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	uint64_t	r;
	int			rc;

	r = 0;
	rc = dvm_call(vm, fk_method("Ld09/Rec;", "descend"), args, &r);
	*ret = r + 100;
	return (rc);
}

static void	natif_rappelle_dvm_call(void)
{
	uint64_t	ret;

	fk_init("d09.dex", 64, 0);
	g_fk.native = rappel;
	g_fk.native_ins = 1;
	ret = 0;
	h_eq_i64("rc", fk_calc("Ld09/Div;", "natif", &ret), 0);
	h_eq_u64("20 + 100 + 1", ret, 121);
	fk_check("pile");
	fk_end();
	fk_init("d09.dex", 10, 0);
	g_fk.native = rappel;
	g_fk.native_ins = 1;
	h_eq_i64("plafond dans le rappel", fk_calc("Ld09/Div;", "natif", &ret),
		DVM_THROWN);
	h_eq_str("erreur", fk_pending(), "Ljava/lang/StackOverflowError;");
	fk_check("pile apres exception dans le natif");
	fk_end();
}

static void	natif_mauvais_nombre_de_mots(void)
{
	uint64_t	ret;

	fk_init("d09.dex", 64, 0);
	g_fk.native = rappel;
	g_fk.native_ins = 2;
	ret = 0;
	h_eq_i64("rc", fk_calc("Ld09/Div;", "natif", &ret), DVM_THROWN);
	h_eq_str("erreur", fk_pending(), "Ljava/lang/VerifyError;");
	fk_check("pile");
	fk_end();
}

static void	methode_sans_code(void)
{
	t_dmethod	m;
	uint64_t	ret;

	fk_init(NULL, 8, 0);
	memset(&m, 0, sizeof(m));
	m.access = ACC_ABSTRACT;
	ret = 0;
	h_eq_i64("rc", dvm_call(&g_fk.vm, &m, NULL, &ret), DVM_THROWN);
	h_eq_str("erreur", fk_pending(), "Ljava/lang/AbstractMethodError;");
	h_eq_i64("vm nulle", dvm_call(NULL, &m, NULL, &ret), E_INVAL);
	fk_check("pile");
	fk_end();
}

int	main(void)
{
	h_begin("d09/natif");
	h_run("natif_rappelle_dvm_call", natif_rappelle_dvm_call);
	h_run("natif_mauvais_nombre_de_mots", natif_mauvais_nombre_de_mots);
	h_run("methode_sans_code", methode_sans_code);
	return (h_end());
}
