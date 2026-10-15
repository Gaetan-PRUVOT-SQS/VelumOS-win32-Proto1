#include <stdlib.h>
#include "harness.h"
#include "d00.h"
#include "velum/apk/droid.h"

static int	fill_heap(t_dvm *vm)
{
	t_dref	ref;
	int		rc;
	int		n;

	rc = 0;
	n = 0;
	while (rc == 0 && n < 4096)
	{
		rc = dvm_array_new(vm, "[I", 4096, &ref);
		if (rc == 0)
			rc = dvm_pin(vm, ref);
		n++;
	}
	return (rc);
}

static void	tas_plein_leve_la_bonne_classe(void)
{
	t_droid		*d;
	t_dvm		*vm;
	t_dlimits	lim;

	d = calloc(1, sizeof(*d));
	lim = (t_dlimits){262144, 0, 0, 0, 0};
	h_eq_i64("dvm_create", dvm_create(&vm, &lim), 0);
	h_eq_i64("droid_install", droid_install(d, vm), 0);
	h_eq_i64("tas plein", fill_heap(vm), DVM_THROWN);
	h_true(vm->pending != DVM_NULL, "exception en attente");
	h_eq_str("classe de l'exception",
		dvm_class_name(dvm_class_of(vm, vm->pending)),
		"Ljava/lang/OutOfMemoryError;");
	dvm_destroy(vm);
	free(d);
}

int	main(void)
{
	h_begin("d00/croise-oom");
	h_run("tas_plein_leve_la_bonne_classe", tas_plein_leve_la_bonne_classe);
	return (h_end());
}
