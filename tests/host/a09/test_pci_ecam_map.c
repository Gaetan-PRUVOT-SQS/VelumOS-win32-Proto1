#include "a09_test.h"
#include "fake.h"
#include "harness.h"
#include "pci_int.h"
#include "velum/vmm.h"

static void	mappage_adresse_longueur_et_cache(void)
{
	void	*first;

	fake_ecam_window(0, 255);
	first = pci_ecam_bus_base(0, 5);
	h_true(first != NULL, "bus 5 mappe");
	h_eq_u64("adresse physique = base + 5 Mio", g_fake.io_last_phys,
		0xb0500000ull);
	h_eq_u64("longueur 1 Mio", g_fake.io_last_len, 0x100000);
	h_eq_u64("sans cache d'acces (VM_NOCACHE)", g_fake.io_last_flags,
		VM_NOCACHE);
	h_true(pci_ecam_bus_base(0, 5) == first, "meme pointeur au second appel");
	h_eq_u64("un seul mappage", g_fake.io_map_calls, 1);
	h_true(pci_ecam_bus_base(0, 6) != first, "autre bus, autre fenetre");
	h_eq_u64("deux mappages", g_fake.io_map_calls, 2);
}

static void	mappage_relatif_au_premier_bus_de_la_fenetre(void)
{
	fake_ecam_window(100, 199);
	h_true(pci_ecam_bus_base(0, 101) != NULL, "bus 101 mappe");
	h_eq_u64("base + (101 - 100) Mio", g_fake.io_last_phys, 0xb0100000ull);
	h_true(pci_ecam_bus_base(0, 99) == NULL, "bus 99 hors fenetre");
	h_true(pci_ecam_bus_base(0, 200) == NULL, "bus 200 hors fenetre");
	h_true(pci_ecam_bus_base(1, 101) == NULL, "segment 1 hors fenetre");
	h_eq_u64("aucun mappage pour les refus", g_fake.io_map_calls, 1);
}

static void	echec_de_mappage_non_memorise(void)
{
	fake_ecam_window(0, 255);
	g_fake.io_map_fail = 1;
	h_true(pci_ecam_bus_base(0, 1) == NULL, "mappage refuse : NULL");
	g_fake.io_map_fail = 0;
	h_true(pci_ecam_bus_base(0, 1) != NULL, "reessai reussi");
	h_eq_u64("deux appels vmm_io_map", g_fake.io_map_calls, 2);
	h_eq_u64("rien a demapper", g_fake.io_unmap_calls, 0);
}

static void	cache_plein_rend_le_mappage_excedentaire(void)
{
	int	bus;
	int	nulls;

	fake_ecam_window(0, 255);
	nulls = 0;
	bus = 0;
	while (bus < PCI_ECAM_MAP_MAX)
		nulls += pci_ecam_bus_base(0, (uint8_t)bus++) == NULL;
	h_eq_i64("64 bus mappes sans refus", nulls, 0);
	h_true(pci_ecam_bus_base(0, 64) == NULL, "65e bus : refuse");
	h_eq_u64("mappage excedentaire rendu", g_fake.io_unmap_calls, 1);
	h_true(pci_ecam_bus_base(0, 3) != NULL, "bus deja mappes toujours servis");
}

int	main(void)
{
	h_begin("a09/pci_ecam_map");
	h_run("ecam-map/adresse-longueur-nocache-et-cache",
		mappage_adresse_longueur_et_cache);
	h_run("ecam-map/relatif-au-premier-bus-de-la-fenetre",
		mappage_relatif_au_premier_bus_de_la_fenetre);
	h_run("ecam-map/echec-non-memorise", echec_de_mappage_non_memorise);
	h_run("ecam-map/cache-plein-64-bus",
		cache_plein_rend_le_mappage_excedentaire);
	return (h_end());
}
