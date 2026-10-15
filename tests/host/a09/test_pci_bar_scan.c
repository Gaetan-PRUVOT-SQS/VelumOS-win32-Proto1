#include "a09_test.h"
#include "fake_fab.h"
#include "harness.h"
#include "pci_int.h"

static const t_fabspec	g_dev = {0, 5, 0, 0x1234, 0x1111, 3, 0, 0};
static const t_fabspec	g_bridge = {0, 6, 0, 0x8086, 0x244e, 6, 4, 1};
static const t_pciloc	g_loc = {0, 0, 5, 0};

static void	decodage_coupe_pendant_la_mesure_puis_restaure(void)
{
	t_fabdev	*d;
	t_pcidev	out;

	fab_reset();
	d = fab_make(&g_dev);
	fab_bar(d, 0, FAB_MEM32, 0x10000);
	fab_bar_base(d, 0, 0xfe000000u);
	fab_bar(d, 2, FAB_IO, 32);
	fab_bar_base(d, 2, 0xc000);
	fab_put(d, 0x04, 2, 0x0007);
	pcibar_scan(&g_loc, 6, &out);
	h_eq_u64("aucune ecriture de 1 avec decodage actif",
		g_fab.decode_violations, 0);
	h_eq_u64("commande restauree", fab_get(d, 0x04, 2), 0x0007);
	h_eq_u64("BAR0 restauree", fab_bar_reg(d, 0), 0xfe000000u);
	h_eq_u64("BAR2 restauree", fab_bar_reg(d, 2), 0xc001);
	h_eq_u64("taille BAR0", out.bar_size[0], 0x10000);
	h_eq_u64("taille BAR2", out.bar_size[2], 32);
	h_eq_u64("drapeaux BAR2", out.bar_flags[2], PCI_BAR_IO);
}

static void	un_bar_64_bits_saute_son_emplacement_haut(void)
{
	t_fabdev	*d;
	t_pcidev	out;

	fab_reset();
	d = fab_make(&g_dev);
	fab_bar(d, 0, FAB_MEM64_LO, 0x4000);
	fab_bar_base(d, 0, 0x1fe000000ull);
	fab_bar(d, 2, FAB_MEM32, 0x100);
	fab_bar_base(d, 2, 0xfebf1000u);
	pcibar_scan(&g_loc, 6, &out);
	h_eq_u64("BAR0 : base 64 bits", out.bar[0], 0x1fe000000ull);
	h_eq_u64("BAR1 : emplacement haut vide", out.bar_size[1], 0);
	h_eq_u64("BAR2 : suivante lue normalement", out.bar_size[2], 0x100);
	h_eq_u64("BAR0 : drapeau 64 bits", out.bar_flags[0] & PCI_BAR_MEM64,
		PCI_BAR_MEM64);
}

static void	en_tete_de_pont_deux_bars_sans_toucher_aux_bus(void)
{
	t_fabdev	*d;
	t_pcidev	out;
	t_pciloc	l;

	fab_reset();
	d = fab_make(&g_bridge);
	fab_bridge(d, 1, 3);
	fab_bar(d, 0, FAB_MEM32, 0x1000);
	fab_bar(d, 2, FAB_MEM32, 0x1000);
	l = (t_pciloc){0, 0, 6, 0};
	pcibar_scan(&l, 2, &out);
	h_eq_u64("aucune ecriture 0x18-0x27 (bus et BAR suivantes)",
		fab_writes_at(0x18, 0x28), 0);
	h_eq_u64("secondaire intact", fab_get(d, 0x19, 1), 1);
	h_eq_u64("BAR0 mesuree", out.bar_size[0], 0x1000);
	h_eq_u64("BAR2 ignoree", out.bar_size[2], 0);
}

static void	bar_64_bits_sur_le_dernier_emplacement(void)
{
	t_fabdev	*d;
	t_pcidev	out;

	fab_reset();
	d = fab_make(&g_dev);
	fab_bar(d, 5, FAB_MEM64_LO, 0x1000);
	fab_bar_base(d, 5, 0xfebf2000u);
	pcibar_scan(&g_loc, 6, &out);
	h_eq_u64("aucune ecriture au-dela de 0x27", fab_writes_at(0x28, 0x100), 0);
	h_eq_u64("BAR5 traitee comme 32 bits", out.bar_size[5], 0x1000);
	h_eq_u64("sans drapeau 64 bits", out.bar_flags[5] & PCI_BAR_MEM64, 0);
}

int	main(void)
{
	h_begin("a09/pci_bar_scan");
	h_run("bar-scan/decodage-coupe-et-valeurs-restaurees",
		decodage_coupe_pendant_la_mesure_puis_restaure);
	h_run("bar-scan/64-bits-saute-l-emplacement-haut",
		un_bar_64_bits_saute_son_emplacement_haut);
	h_run("bar-scan/pont-2-bars-bus-intacts",
		en_tete_de_pont_deux_bars_sans_toucher_aux_bus);
	h_run("bar-scan/64-bits-en-bar5", bar_64_bits_sur_le_dernier_emplacement);
	return (h_end());
}
