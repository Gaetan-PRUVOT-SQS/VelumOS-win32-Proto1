#include "a09_test.h"
#include "fake.h"
#include "fake_fab.h"
#include "harness.h"
#include "pci_int.h"

static void	profondeur_bornee_a_32_niveaux(void)
{
	fab_reset();
	fab_bridge_chain(40);
	pci_scan_root(0, 0);
	h_true(pci_find(0x1000 + 32, 1, 0) != NULL, "bus 32 : atteint");
	h_true(pci_find(0x1000 + 33, 1, 0) == NULL, "bus 33 : au-dela de la borne");
	h_eq_u64("33 ponts + 33 appareils", pci_state()->count, 66);
}

static void	table_pleine_a_128_appareils(void)
{
	fab_reset();
	fab_multifunction_slots(17);
	pci_scan_root(0, 0);
	h_eq_u64("128 appareils conserves", pci_state()->count, PCI_MAX_DEVS);
	h_eq_u64("8 appareils ignores", pci_state()->overflow, 8);
	h_true(pci_at(PCI_MAX_DEVS) == NULL, "indice 128 : NULL");
	h_true(pci_at(PCI_MAX_DEVS - 1) != NULL, "indice 127 : present");
}

static void	double_balayage_sans_doublon(void)
{
	static const t_fabspec	s = {0, 5, 0, 0x1234, 0x5678, 2, 0, 0};

	fab_reset();
	fab_make(&s);
	pci_scan_root(0, 0);
	pci_scan_root(0, 0);
	h_eq_u64("un seul appareil", pci_state()->count, 1);
	h_eq_u64("pas de debordement", pci_state()->overflow, 0);
}

static void	vendeur_nul_considere_absent(void)
{
	static const t_fabspec	s = {0, 6, 0, 0, 0x5678, 2, 0, 0};

	fab_reset();
	fab_make(&s);
	pci_scan_root(0, 0);
	h_eq_u64("aucun appareil", pci_state()->count, 0);
}

int	main(void)
{
	h_begin("a09/pci_enum_limits");
	h_run("limits/profondeur-32", profondeur_bornee_a_32_niveaux);
	h_run("limits/table-128-appareils", table_pleine_a_128_appareils);
	h_run("limits/double-balayage", double_balayage_sans_doublon);
	h_run("limits/vendeur-0", vendeur_nul_considere_absent);
	return (h_end());
}
