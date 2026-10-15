#include "a09_test.h"
#include "fake.h"
#include "harness.h"
#include "pci_int.h"
#include "velum/err.h"

static void	arguments_invalides_et_verrous(void)
{
	const t_pcidev	*d;
	t_pcidev		stranger;

	d = fab_plain_device(10, 1);
	stranger = *d;
	h_eq_i64("fonction nulle", pci_irq_setup(d, NULL, NULL), E_INVAL);
	h_eq_i64("appareil NULL", pci_irq_setup(NULL, fake_nop_isr, NULL),
		E_INVAL);
	h_eq_i64("appareil etranger", pci_irq_setup(&stranger, fake_nop_isr, NULL),
		E_INVAL);
	pci_irq_free(NULL);
	pci_irq_free(&stranger);
	h_eq_u64("aucune liberation", g_fake.irq_frees + g_fake.vec_frees, 0);
	h_eq_i64("verrous", g_fake.lock_errors + g_fake.lock_depth, 0);
}

int	main(void)
{
	h_begin("a09/pci_irq_args");
	h_run("irq-args/pointeurs-fonction-et-verrous",
		arguments_invalides_et_verrous);
	return (h_end());
}
