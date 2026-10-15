#include "a09_test.h"
#include "fake.h"
#include "fake_fab.h"
#include "harness.h"
#include "pci_int.h"
#include "velum/pci.h"

static void	amorcage_par_les_ports_sans_mcfg(void)
{
	fake_reset();
	fab_reset();
	fab_q35();
	h_eq_i64("pci_boot_init", pci_boot_init(), 0);
	h_eq_u64("trois appareils", pci_count(), 3);
	h_true(fake_log_has("pci: 3 appareil(s), configuration par ports"),
		"journal du bilan");
	h_true(fake_log_has("0000:00:00.0 8086:29c0 060000 host bridge"),
		"classe lisible : pont hote");
	h_true(fake_log_has("0000:00:01.0 1234:1111 030000 VGA compatible"),
		"classe lisible : VGA");
	h_true(fake_log_has("0000:00:1f.0 8086:2918 060100 ISA bridge"),
		"classe lisible : pont ISA");
	h_true(fake_log_has("bar0 mem32 prefetch 0xfd000000 taille 0x1000000"),
		"BAR journalisee");
}

static void	acpi_absente_ou_sans_mcfg(void)
{
	fake_reset();
	fab_reset();
	fab_q35();
	g_fake.acpi_absent = 1;
	h_eq_i64("sans ACPI", pci_boot_init(), 0);
	h_eq_u64("appareils par les ports", pci_count(), 3);
	h_eq_u64("aucun mappage ECAM", g_fake.io_map_calls, 0);
}

static void	aucun_appareil_ni_panique(void)
{
	fake_reset();
	fab_reset();
	h_eq_i64("pci_boot_init", pci_boot_init(), 0);
	h_eq_u64("zero appareil", pci_count(), 0);
	h_true(fake_log_has("pci: 0 appareil(s)"), "bilan a zero");
	h_true(pci_at(0) == NULL, "pci_at(0) : NULL");
	h_true(pci_find(0x8086, 0x29c0, 0) == NULL, "pci_find : NULL");
}

static void	table_pleine_signalee_au_journal(void)
{
	fake_reset();
	fab_reset();
	fab_multifunction_slots(17);
	h_eq_i64("pci_boot_init", pci_boot_init(), 0);
	h_eq_u64("128 appareils", pci_count(), 128);
	h_true(fake_log_has("[warn] pci: table pleine, 8 appareil(s) ignore(s)"),
		"avertissement");
}

int	main(void)
{
	h_begin("a09/pci_boot");
	h_run("boot/ports-0xcf8-sans-mcfg-et-journal",
		amorcage_par_les_ports_sans_mcfg);
	h_run("boot/acpi-absente", acpi_absente_ou_sans_mcfg);
	h_run("boot/aucun-appareil", aucun_appareil_ni_panique);
	h_run("boot/table-pleine-signalee", table_pleine_signalee_au_journal);
	return (h_end());
}
