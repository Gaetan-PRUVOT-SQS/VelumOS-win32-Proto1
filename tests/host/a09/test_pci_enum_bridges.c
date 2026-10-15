#include "a09_test.h"
#include "fake_fab.h"
#include "harness.h"
#include "pci_int.h"

static void	un_pont_un_appareil_derriere(void)
{
	fab_reset();
	fab_pci_bridge(0, 1, 1, 1);
	fab_endpoint(1, 0, 0x1111);
	fab_endpoint(2, 0, 0x2222);
	pci_scan_root(0, 0);
	h_eq_u64("pont + appareil du bus 1", pci_state()->count, 2);
	h_true(pci_find(0x1111, 1, 0) != NULL, "appareil du bus 1 trouve");
	h_true(pci_find(0x2222, 1, 0) == NULL, "bus 2 non joint : absent");
}

static void	ponts_imbriques_sur_trois_niveaux(void)
{
	fab_reset();
	fab_pci_bridge(0, 1, 1, 3);
	fab_pci_bridge(1, 0, 2, 3);
	fab_pci_bridge(2, 4, 3, 3);
	fab_endpoint(3, 7, 0x3333);
	fab_endpoint(1, 5, 0x1555);
	pci_scan_root(0, 0);
	h_eq_u64("trois ponts + deux appareils", pci_state()->count, 5);
	h_true(pci_find(0x3333, 1, 0) != NULL, "appareil du bus 3 trouve");
	h_eq_u64("bus de l'appareil profond", pci_find(0x3333, 1, 0)->bus, 3);
	h_eq_u64("appareil profond : slot 7", pci_find(0x3333, 1, 0)->dev, 7);
}

static void	configurations_de_pont_incoherentes(void)
{
	fab_reset();
	fab_pci_bridge(0, 1, 0, 0);
	fab_pci_bridge(0, 2, 5, 4);
	fab_pci_bridge(0, 3, 1, 0);
	fab_pci_bridge(0, 4, 0, 255);
	fab_endpoint(1, 0, 0x4444);
	fab_endpoint(5, 0, 0x5555);
	pci_scan_root(0, 0);
	h_eq_u64("quatre ponts seulement, rien derriere", pci_state()->count, 4);
	h_true(pci_find(0x4444, 1, 0) == NULL, "bus 1 : subordonne < secondaire");
	h_true(pci_find(0x5555, 1, 0) == NULL, "bus 5 : subordonne 4 < 5");
}

static void	deux_ponts_vers_le_meme_bus(void)
{
	fab_reset();
	fab_pci_bridge(0, 1, 1, 1);
	fab_pci_bridge(0, 2, 1, 1);
	fab_endpoint(1, 0, 0x6666);
	pci_scan_root(0, 0);
	h_eq_u64("2 ponts + 1 appareil, sans doublon", pci_state()->count, 3);
	h_eq_u64("appareil derriere : unique",
		pci_find(0x6666, 1, 1) == NULL, 1);
}

int	main(void)
{
	h_begin("a09/pci_enum_bridges");
	h_run("bridges/un-pont-un-appareil", un_pont_un_appareil_derriere);
	h_run("bridges/imbrication-3-niveaux", ponts_imbriques_sur_trois_niveaux);
	h_run("bridges/configurations-incoherentes",
		configurations_de_pont_incoherentes);
	h_run("bridges/meme-bus-secondaire", deux_ponts_vers_le_meme_bus);
	return (h_end());
}
