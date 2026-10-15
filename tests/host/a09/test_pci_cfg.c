#include "a09_test.h"
#include "fake.h"
#include "fake_fab.h"
#include "harness.h"
#include "pci_int.h"
#include "velum/err.h"

static const t_fabspec	g_dev = {0, 1, 0, 0x1234, 0x1111, 3, 0, 0};

static void	largeurs_et_alignements(void)
{
	t_pciloc	l;
	int			bad;

	fab_reset();
	fab_make(&g_dev);
	l = (t_pciloc){0, 0, 1, 0};
	bad = pcicfg_read(&l, 0, 4) != 0x11111234u;
	bad += pcicfg_read(&l, 0, 2) != 0x1234;
	bad += pcicfg_read(&l, 2, 2) != 0x1111;
	bad += pcicfg_read(&l, 3, 1) != 0x11;
	h_eq_i64("lectures valides", bad, 0);
	h_eq_u64("largeur 3", pcicfg_read(&l, 0, 3), 0xffffffffu);
	h_eq_u64("largeur 0", pcicfg_read(&l, 0, 0), 0xffffffffu);
	h_eq_u64("largeur 8", pcicfg_read(&l, 0, 8), 0xffffffffu);
	h_eq_u64("16 bits non alignes", pcicfg_read(&l, 1, 2), 0xffff);
	h_eq_u64("32 bits non alignes", pcicfg_read(&l, 2, 4), 0xffffffffu);
	h_eq_u64("8 bits hors borne (256)", pcicfg_read(&l, 256, 1), 0xff);
}

static void	limites_du_decalage(void)
{
	t_pciloc	l;

	fab_reset();
	fab_make(&g_dev);
	l = (t_pciloc){0, 0, 1, 0};
	h_eq_i64("16 bits en 254", pcicfg_write(&l, 254, 2, 0), 0);
	h_eq_i64("16 bits en 255", pcicfg_write(&l, 255, 2, 0), E_INVAL);
	h_eq_i64("32 bits en 252", pcicfg_write(&l, 252, 4, 0), 0);
	h_eq_i64("32 bits en 253", pcicfg_write(&l, 253, 4, 0), E_INVAL);
	h_eq_i64("8 bits en 255", pcicfg_write(&l, 255, 1, 0), 0);
	h_eq_i64("8 bits en 256", pcicfg_write(&l, 256, 1, 0), E_INVAL);
	h_eq_i64("8 bits en 0xffff", pcicfg_write(&l, 0xffff, 1, 0), E_INVAL);
	h_eq_u64("offset 0xffff en lecture", pcicfg_read(&l, 0xffff, 1), 0xff);
}

static void	appareil_fonction_segment_hors_limite(void)
{
	t_pciloc	l;

	fab_reset();
	fab_make(&g_dev);
	l = (t_pciloc){0, 0, 32, 0};
	h_eq_u64("appareil 32 : tout a un", pcicfg_read(&l, 0, 4), 0xffffffffu);
	h_eq_i64("appareil 32 : ecriture", pcicfg_write(&l, 4, 2, 0), E_INVAL);
	l = (t_pciloc){0, 0, 1, 8};
	h_eq_u64("fonction 8 : tout a un", pcicfg_read(&l, 0, 4), 0xffffffffu);
	l = (t_pciloc){1, 0, 1, 0};
	h_eq_u64("segment sans fenetre", pcicfg_read(&l, 0, 4), 0xffffffffu);
	h_eq_i64("segment sans fenetre : ecriture", pcicfg_write(&l, 4, 2, 0),
		E_INVAL);
	h_eq_i64("pointeur nul : ecriture", pcicfg_write(NULL, 4, 2, 0), E_INVAL);
	h_eq_u64("pointeur nul : lecture", pcicfg_read(NULL, 0, 4), 0xffffffffu);
}

static void	sans_pilote_de_configuration(void)
{
	t_pciloc	l;

	pci_state_reset();
	l = (t_pciloc){0, 0, 1, 0};
	h_eq_u64("lecture sans backend", pcicfg_read(&l, 0, 4), 0xffffffffu);
	h_eq_i64("ecriture sans backend", pcicfg_write(&l, 4, 2, 0), E_INVAL);
	h_eq_i64("sonde sans backend", pcicfg_probe(&l, NULL), E_INVAL);
}

int	main(void)
{
	h_begin("a09/pci_cfg");
	h_run("cfg/largeurs-1-2-4-et-alignement", largeurs_et_alignements);
	h_run("cfg/limites-254-255-252-253-256", limites_du_decalage);
	h_run("cfg/appareil-32-fonction-8-segment-1",
		appareil_fonction_segment_hors_limite);
	h_run("cfg/sans-backend", sans_pilote_de_configuration);
	return (h_end());
}
