#include "a09_test.h"
#include "fake_fab.h"
#include "harness.h"
#include "pci_int.h"
#include "velum/err.h"

static const t_fabspec	g_dev = {0, 3, 0, 0x1234, 0x1111, 3, 0, 0};

static void	bits_de_commande_valides(void)
{
	t_fabdev		*f;
	const t_pcidev	*d;

	fab_reset();
	f = fab_make(&g_dev);
	fab_put(f, 4, 2, 0x0100);
	pci_scan_root(0, 0);
	d = pci_at(0);
	h_eq_i64("memoire seule", pci_enable(d, PCI_CMD_MEM), 0);
	h_eq_u64("bit memoire pose, SERR conserve", fab_get(f, 4, 2), 0x0102);
	h_eq_i64("E/S et maitre", pci_enable(d, PCI_CMD_IO | PCI_CMD_MASTER), 0);
	h_eq_u64("trois bits poses", fab_get(f, 4, 2), 0x0107);
	h_eq_i64("aucun bit", pci_enable(d, 0), 0);
	h_eq_u64("commande inchangee", fab_get(f, 4, 2), 0x0107);
}

static void	bits_et_arguments_refuses(void)
{
	const t_pcidev	*d;
	t_pcidev		stranger;

	fab_reset();
	fab_make(&g_dev);
	pci_scan_root(0, 0);
	d = pci_at(0);
	stranger = *d;
	g_fab.nlog = 0;
	h_eq_i64("bit 3 (cycles speciaux)", pci_enable(d, 0x8), E_INVAL);
	h_eq_i64("bit 10 (INTx desactive)", pci_enable(d, 0x400), E_INVAL);
	h_eq_i64("bit 15", pci_enable(d, 0x8000), E_INVAL);
	h_eq_i64("valide + invalide", pci_enable(d, PCI_CMD_MEM | 0x8), E_INVAL);
	h_eq_i64("NULL", pci_enable(NULL, PCI_CMD_MEM), E_INVAL);
	h_eq_i64("appareil etranger", pci_enable(&stranger, PCI_CMD_MEM), E_INVAL);
	h_eq_u64("aucune ecriture emise", g_fab.nlog, 0);
}

static void	appareil_qui_ignore_l_ecriture(void)
{
	t_fabdev		*f;
	const t_pcidev	*d;

	fab_reset();
	f = fab_make(&g_dev);
	pci_scan_root(0, 0);
	f->cmd_ro = 1;
	d = pci_at(0);
	h_eq_i64("bits non retenus par le materiel", pci_enable(d, PCI_CMD_MEM),
		E_IO);
	h_eq_i64("verrous equilibres", pci_state()->lock.ticket
		- pci_state()->lock.serving, 0);
}

int	main(void)
{
	h_begin("a09/pci_enable");
	h_run("enable/bits-valides-et-conservation", bits_de_commande_valides);
	h_run("enable/bits-et-pointeurs-refuses", bits_et_arguments_refuses);
	h_run("enable/ecriture-ignoree-par-le-materiel",
		appareil_qui_ignore_l_ecriture);
	return (h_end());
}
