#include "harness.h"
#include "d00.h"
#include "velum/apk/dvm.h"

static int64_t	calcul(t_dvm *vm, const char *desc)
{
	t_dclass		*c;
	const t_dmethod	*m;
	t_dname			name;
	uint64_t		ret;

	name = (t_dname){"calcul", "()I"};
	ret = 0;
	if (dvm_class(vm, desc, &c) != 0)
		return (-1001);
	if (dvm_method(vm, c, &name, &m) != 0)
		return (-1002);
	if (dvm_call(vm, m, NULL, &ret) != 0)
		return (-1003);
	return ((int32_t)ret);
}

static void	essais_sans_bibliotheque(void)
{
	t_span	file;
	t_dvm	*vm;

	file = d00_load("essais.dex");
	h_eq_i64("dvm_create", dvm_create(&vm, NULL), 0);
	h_eq_i64("dvm_load_dex", dvm_load_dex(vm, file), 0);
	h_eq_i64("Arithmetique",
		calcul(vm, "Lcom/velum/essais/Arithmetique;"), 1410065813);
	h_eq_i64("Boucles", calcul(vm, "Lcom/velum/essais/Boucles;"), 5215);
	h_eq_i64("Tableaux", calcul(vm, "Lcom/velum/essais/Tableaux;"), 1153);
	h_eq_i64("Aiguillages",
		calcul(vm, "Lcom/velum/essais/Aiguillages;"), 306);
	h_eq_u64("pile rendue", vm->sp, 0);
	h_eq_u64("profondeur rendue", vm->depth, 0);
	h_eq_u64("aucune exception en attente", vm->pending, DVM_NULL);
	dvm_destroy(vm);
	d00_free(file);
}

int	main(void)
{
	h_begin("d00/croise-vm");
	h_run("essais_sans_bibliotheque", essais_sans_bibliotheque);
	return (h_end());
}
