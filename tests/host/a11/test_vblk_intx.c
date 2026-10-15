#include "harness.h"
#include "vblk.h"
#include "fake.h"

static void	intx_coupee_par_le_lot_pci(void)
{
	t_fkdev	*f;

	f = &g_fkdev[0];
	fk_dev_init(f, 1024, VBLK_PCI_DEVICE);
	h_eq_i64("sonde", vblk_probe(&f->pci, 0), 0);
	h_eq_i64("pci_intx_disable appelee une fois", f->intx_calls, 1);
	h_eq_i64("aucune ecriture directe de Command", f->raw_cmd_writes, 0);
	h_true(f->cmd & 0x400, "bit 10 pose");
	h_true(f->cmd & PCI_CMD_MEM, "decodage memoire conserve");
	fk_dev_free(f);
}

static void	refus_de_coupure_tolere(void)
{
	t_fkdev	*f;

	f = &g_fkdev[0];
	fk_dev_init(f, 1024, VBLK_PCI_DEVICE);
	f->modes = FK_INTX_FAIL;
	h_eq_i64("sonde malgre le refus", vblk_probe(&f->pci, 1), 0);
	h_eq_i64("coupure tentee", f->intx_calls, 1);
	h_eq_i64("aucune ecriture directe de Command", f->raw_cmd_writes, 0);
	h_true(!(f->cmd & 0x400), "bit 10 absent");
	fk_dev_free(f);
}

int	main(void)
{
	h_begin("a11/vblk_intx");
	h_run("intx/coupure-deleguee-au-lot-pci", intx_coupee_par_le_lot_pci);
	h_run("intx/refus-du-materiel-tolere", refus_de_coupure_tolere);
	return (h_end());
}
