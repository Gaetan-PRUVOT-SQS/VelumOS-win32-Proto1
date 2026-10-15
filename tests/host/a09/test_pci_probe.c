#include "a09_test.h"
#include "fake_fab.h"
#include "harness.h"
#include "pci_int.h"
#include "velum/err.h"

static const t_fabspec	g_dev = {0, 2, 0, 0x8086, 0x29c0, 6, 0, 0};

static void	sonde_present_absent_vendeur_invalide(void)
{
	t_pciloc	l;
	uint32_t	id;

	fab_reset();
	fab_make(&g_dev);
	l = (t_pciloc){0, 0, 2, 0};
	id = 0;
	h_eq_i64("appareil present", pcicfg_probe(&l, &id), 0);
	h_eq_u64("identifiants rendus", id, 0x29c08086u);
	h_eq_i64("sans pointeur de sortie", pcicfg_probe(&l, NULL), 0);
	l.dev = 3;
	h_eq_i64("appareil absent (0xffff)", pcicfg_probe(&l, &id), E_NODEV);
	l.dev = 2;
	fab_find(0, 0, 2, 0)->cfg[0] = 0;
	fab_find(0, 0, 2, 0)->cfg[1] = 0;
	h_eq_i64("vendeur 0x0000 refuse", pcicfg_probe(&l, &id), E_NODEV);
}

static void	sonde_arguments_invalides(void)
{
	t_pciloc	l;

	fab_reset();
	fab_make(&g_dev);
	l = (t_pciloc){0, 0, 32, 0};
	h_eq_i64("appareil 32", pcicfg_probe(&l, NULL), E_INVAL);
	l = (t_pciloc){0, 0, 2, 8};
	h_eq_i64("fonction 8", pcicfg_probe(&l, NULL), E_INVAL);
	l = (t_pciloc){2, 0, 2, 0};
	h_eq_i64("segment inconnu", pcicfg_probe(&l, NULL), E_INVAL);
	h_eq_i64("pointeur nul", pcicfg_probe(NULL, NULL), E_INVAL);
}

int	main(void)
{
	h_begin("a09/pci_probe");
	h_run("probe/present-absent-vendeur-0xffff-et-0",
		sonde_present_absent_vendeur_invalide);
	h_run("probe/arguments-invalides", sonde_arguments_invalides);
	return (h_end());
}
