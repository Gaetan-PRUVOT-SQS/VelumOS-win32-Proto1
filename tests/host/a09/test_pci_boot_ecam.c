#include "a09_test.h"
#include "fake.h"
#include "fake_fab.h"
#include "harness.h"
#include "pci_int.h"
#include "velum/pci.h"

static void	mcfg_une_fenetre(uint8_t bus_start, uint8_t bus_end)
{
	fake_reset();
	fab_reset();
	fake_mmio_reset();
	g_fake.acpi.nmcfg = 1;
	g_fake.acpi.mcfg[0] = (t_mcfg_entry){0xb0000000ull, 0, bus_start,
		bus_end, 0};
}

static void	amorcage_par_ecam(void)
{
	mcfg_une_fenetre(0, 255);
	fake_ecam_poke(0, 0, 0x29c08086u);
	fake_ecam_poke(0, 5, 0x11111234u);
	g_fake.io_map_calls = 0;
	h_eq_i64("pci_boot_init", pci_boot_init(), 0);
	h_eq_u64("deux appareils par ECAM", pci_count(), 2);
	h_eq_u64("hote en 00:00.0", pci_find(0x8086, 0x29c0, 0)->dev, 0);
	h_eq_u64("VGA en 00:05.0", pci_find(0x1234, 0x1111, 0)->dev, 5);
	h_true(fake_log_has("configuration par ECAM (MCFG)"), "backend annonce");
	h_eq_u64("un seul bus mappe", g_fake.io_map_calls, 1);
	h_eq_u64("depuis la base de la fenetre", g_fake.io_last_phys,
		0xb0000000ull);
}

static void	ecam_sans_appareil_repli_sur_les_ports(void)
{
	mcfg_une_fenetre(0, 255);
	fab_q35();
	h_eq_i64("pci_boot_init", pci_boot_init(), 0);
	h_eq_u64("appareils trouves par les ports", pci_count(), 3);
	h_true(fake_log_has("[warn] pci: ECAM sans appareil"), "avertissement");
	h_true(fake_log_has("configuration par ports"), "bilan par les ports");
}

static void	fenetre_ecam_qui_ne_demarre_pas_au_bus_0(void)
{
	mcfg_une_fenetre(16, 31);
	fake_ecam_poke(0, 0, 0x29c08086u);
	g_fake.io_map_calls = 0;
	h_eq_i64("pci_boot_init", pci_boot_init(), 0);
	h_eq_u64("racine = premier bus de la fenetre (16)", g_fake.io_last_phys,
		0xb0000000ull);
	h_eq_u64("appareil du bus 16 trouve", pci_count(), 1);
	h_eq_u64("numero de bus reporte", pci_at(0)->bus, 16);
}

int	main(void)
{
	h_begin("a09/pci_boot_ecam");
	h_run("boot-ecam/enumeration-par-mcfg", amorcage_par_ecam);
	h_run("boot-ecam/ecam-vide-repli-ports",
		ecam_sans_appareil_repli_sur_les_ports);
	h_run("boot-ecam/fenetre-du-bus-16",
		fenetre_ecam_qui_ne_demarre_pas_au_bus_0);
	return (h_end());
}
