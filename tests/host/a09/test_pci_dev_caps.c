#include "a09_test.h"
#include "fake_fab.h"
#include "harness.h"
#include "pci_int.h"
#include "velum/err.h"

static const t_fabspec	g_dev = {0, 5, 0, 0x1234, 0x1111, 3, 0, 0};
static const t_pciloc	g_loc = {0, 0, 5, 0};

static void	drapeaux_et_decalages_msi_msix_pcie(void)
{
	t_fabdev		*d;
	const t_pcidev	*p;
	const t_pcipriv	*v;

	fab_reset();
	d = fab_make(&g_dev);
	fab_put(d, 0x34, 1, 0x50);
	fab_cap(d, 0x50, PCI_CAP_MSI, 0x60);
	fab_cap(d, 0x60, PCI_CAP_MSIX, 0x70);
	fab_cap(d, 0x70, PCI_CAP_PCIE, 0);
	pci_scan_root(0, 0);
	p = pci_at(0);
	v = &pci_state()->priv[0];
	h_true(p->has_msi && p->has_msix, "MSI et MSI-X annonces");
	h_eq_u64("decalage MSI", v->msi_off, 0x50);
	h_eq_u64("decalage MSI-X", v->msix_off, 0x60);
	h_eq_u64("decalage PCIe", v->pcie_off, 0x70);
	h_eq_u64("type d'en-tete memorise", v->header_type, 0);
}

static void	sans_capacite_pas_de_msi(void)
{
	fab_reset();
	fab_make(&g_dev);
	pci_scan_root(0, 0);
	h_true(!pci_at(0)->has_msi, "pas de MSI");
	h_true(!pci_at(0)->has_msix, "pas de MSI-X");
	h_eq_u64("pas de decalage MSI", pci_state()->priv[0].msi_off, 0);
}

static void	pas_de_liste_si_le_bit_d_etat_est_nul(void)
{
	t_fabdev	*d;

	fab_reset();
	d = fab_make(&g_dev);
	fab_cap(d, 0x50, PCI_CAP_MSI, 0);
	fab_put(d, 0x34, 1, 0x50);
	fab_put(d, 0x06, 2, 0);
	h_eq_i64("bit 4 du statut nul", pcicap_next(&g_loc, PCI_CAP_MSI, 0),
		E_NOENT);
}

int	main(void)
{
	h_begin("a09/pci_dev_caps");
	h_run("dev-caps/drapeaux-et-decalages",
		drapeaux_et_decalages_msi_msix_pcie);
	h_run("dev-caps/aucune-capacite", sans_capacite_pas_de_msi);
	h_run("dev-caps/bit-de-liste-nul", pas_de_liste_si_le_bit_d_etat_est_nul);
	return (h_end());
}
