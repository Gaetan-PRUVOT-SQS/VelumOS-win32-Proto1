#include "a09_test.h"
#include "fake_fab.h"
#include "harness.h"
#include "pci_int.h"

static const t_fabspec	g_dev = {0, 3, 0, 0x1234, 0x1111, 3, 0, 0};

static void	lectures_par_largeur(void)
{
	const t_pcidev	*d;

	fab_reset();
	fab_make(&g_dev);
	pci_scan_root(0, 0);
	d = pci_at(0);
	h_eq_u64("32 bits", pci_cfg_read32(d, 0), 0x11111234u);
	h_eq_u64("16 bits", pci_cfg_read16(d, 2), 0x1111);
	h_eq_u64("8 bits", pci_cfg_read8(d, 3), 0x11);
	h_eq_u64("32 bits non aligne", pci_cfg_read32(d, 2), 0xffffffffu);
	h_eq_u64("16 bits non aligne", pci_cfg_read16(d, 1), 0xffff);
	h_eq_u64("8 bits hors borne", pci_cfg_read8(d, 256), 0xff);
}

static void	ecritures_a_la_largeur_demandee(void)
{
	const t_pcidev	*d;

	fab_reset();
	fab_make(&g_dev);
	pci_scan_root(0, 0);
	d = pci_at(0);
	g_fab.nlog = 0;
	pci_cfg_write16(d, 4, 0x0003);
	h_eq_u64("une ecriture", g_fab.nlog, 1);
	h_eq_u64("largeur 16 bits (statut W1C intact)", g_fab.log[0].width, 2);
	pci_cfg_write8(d, 0x3c, 0x0b);
	h_eq_u64("largeur 8 bits", g_fab.log[1].width, 1);
	pci_cfg_write32(d, 0x3c, 0x0102030b);
	h_eq_u64("largeur 32 bits", g_fab.log[2].width, 4);
	h_eq_u64("commande ecrite", fab_get(fab_find(0, 0, 3, 0), 4, 2), 3);
}

static void	pointeurs_d_appareil_invalides(void)
{
	t_pcidev		stranger;
	const t_pcidev	*d;

	fab_reset();
	fab_make(&g_dev);
	pci_scan_root(0, 0);
	d = pci_at(0);
	stranger = *d;
	g_fab.nlog = 0;
	h_eq_u64("NULL : lecture", pci_cfg_read32(NULL, 0), 0xffffffffu);
	h_eq_u64("etranger : lecture 32", pci_cfg_read32(&stranger, 0),
		0xffffffffu);
	h_eq_u64("etranger : lecture 16", pci_cfg_read16(&stranger, 0), 0xffff);
	h_eq_u64("etranger : lecture 8", pci_cfg_read8(&stranger, 0), 0xff);
	h_eq_u64("apres la fin de table : lecture", pci_cfg_read32(d + 1, 0),
		0xffffffffu);
	pci_cfg_write32(NULL, 4, 0);
	pci_cfg_write16(&stranger, 4, 0);
	pci_cfg_write8(d + 1, 4, 0);
	h_eq_u64("aucune ecriture emise", g_fab.nlog, 0);
}

int	main(void)
{
	h_begin("a09/pci_access");
	h_run("access/lectures-8-16-32", lectures_par_largeur);
	h_run("access/ecritures-8-16-32", ecritures_a_la_largeur_demandee);
	h_run("access/pointeurs-invalides", pointeurs_d_appareil_invalides);
	return (h_end());
}
