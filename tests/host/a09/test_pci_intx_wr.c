#include "a09_test.h"
#include "fake_fab.h"
#include "harness.h"
#include "pci_int.h"
#include "velum/err.h"

static int	g_reads;
static int	g_writes;

static uint32_t	flaky_read(const t_pciloc *l, uint16_t off, uint8_t width)
{
	(void)l;
	(void)off;
	(void)width;
	g_reads++;
	return (0x0007);
}

static void	flaky_write(const t_pciloc *l, uint16_t off, uint8_t width,
	uint32_t v)
{
	(void)l;
	(void)off;
	(void)width;
	(void)v;
	g_writes++;
}

static bool	bus_gone_after_first_read(uint16_t seg, uint8_t bus)
{
	(void)seg;
	(void)bus;
	return (g_reads == 0);
}

static void	ecriture_refusee_apres_lecture(void)
{
	static const t_fabspec	dev = {0, 3, 0, 0x1234, 0x1111, 3, 0, 0};
	static const t_pciops	ops = {flaky_read, flaky_write,
		bus_gone_after_first_read, PCI_LEGACY_CFG_SIZE};
	const t_pcidev			*d;

	fab_reset();
	fab_make(&dev);
	pci_scan_root(0, 0);
	d = pci_at(0);
	pcicfg_set_ops(&ops);
	h_eq_i64("erreur d'ecriture remontee", pci_intx_disable(d), E_INVAL);
	h_eq_i64("une lecture", g_reads, 1);
	h_eq_i64("aucune ecriture materielle", g_writes, 0);
	h_eq_i64("verrous equilibres", pci_state()->lock.ticket
		- pci_state()->lock.serving, 0);
}

int	main(void)
{
	h_begin("a09/pci_intx_wr");
	h_run("intx/ecriture-qui-echoue", ecriture_refusee_apres_lecture);
	return (h_end());
}
