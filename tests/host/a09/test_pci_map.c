#include "a09_test.h"
#include "fake.h"
#include "fake_fab.h"
#include "harness.h"
#include "pci_int.h"
#include "velum/err.h"
#include "velum/vmm.h"

static void	mappage_avec_decalage_dans_la_page(void)
{
	const t_pcidev	*d;
	uint8_t			*p;
	uint8_t			*page;

	d = fab_bars_device();
	h_eq_u64("BAR0 : base 0xfebf1010 (alignee sur 16)", d->bar[0], 0xfebf1010u);
	p = pci_map_bar(d, 0);
	h_true(p != NULL, "BAR0 mappee");
	page = fake_mmio(0xfebf1000u);
	h_true(page != NULL, "page entiere mappee");
	h_true(p == page + 0x10, "pointeur decale de 0x10 dans la page");
	h_eq_u64("longueur arrondie a une page", g_fake.io_last_len, 4096);
	h_eq_u64("sans cache", g_fake.io_last_flags, VM_NOCACHE);
}

static void	mappage_refuse_les_cas_invalides(void)
{
	const t_pcidev	*d;

	d = fab_bars_device();
	h_true(pci_map_bar(d, 1) == NULL, "BAR d'E/S");
	h_true(pci_map_bar(d, 2) == NULL, "base nulle (non attribuee)");
	h_true(pci_map_bar(d, 4) == NULL, "emplacement haut du 64 bits");
	h_true(pci_map_bar(d, 5) == NULL, "BAR de taille nulle");
	h_true(pci_map_bar(d, 6) == NULL, "indice 6");
	h_true(pci_map_bar(NULL, 0) == NULL, "appareil NULL");
	h_true(pci_map_bar(d, 3) == NULL, "512 Mio > 256 Mio");
	h_eq_u64("aucun mappage demande pour les refus", g_fake.io_map_calls, 0);
	g_fake.io_map_fail = 1;
	h_true(pci_map_bar(d, 0) == NULL, "vmm_io_map en echec");
}

static void	adresse_qui_deborde_en_64_bits(void)
{
	t_pcidev	copy;
	t_pcistate	*st;

	fab_bars_device();
	st = pci_state();
	copy = st->devs[0];
	st->devs[0].bar[0] = 0xfffffffffffff000ull;
	st->devs[0].bar_size[0] = 0x2000;
	h_true(pci_map_bar(&st->devs[0], 0) == NULL, "base + taille deborde");
	h_eq_u64("aucun mappage", g_fake.io_map_calls, 0);
	st->devs[0] = copy;
}

static void	port_d_es_et_limites(void)
{
	const t_pcidev	*d;

	d = fab_bars_device();
	h_eq_i64("port de la BAR1", pci_io_port(d, 1), 0xc040);
	h_eq_i64("BAR memoire : refus", pci_io_port(d, 0), E_INVAL);
	h_eq_i64("BAR absente : refus", pci_io_port(d, 5), E_INVAL);
	h_eq_i64("indice 6", pci_io_port(d, 6), E_INVAL);
	h_eq_i64("NULL", pci_io_port(NULL, 1), E_INVAL);
}

int	main(void)
{
	h_begin("a09/pci_map");
	h_run("map/decalage-dans-la-page", mappage_avec_decalage_dans_la_page);
	h_run("map/cas-refuses", mappage_refuse_les_cas_invalides);
	h_run("map/debordement-64-bits", adresse_qui_deborde_en_64_bits);
	h_run("map/port-d-es", port_d_es_et_limites);
	return (h_end());
}
