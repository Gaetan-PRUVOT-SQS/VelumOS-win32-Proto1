#include <stdio.h>
#include <string.h>
#include "a09_test.h"
#include "fake.h"
#include "fake_fab.h"
#include "harness.h"
#include "pci_int.h"
#include "velum/boot.h"
#include "velum/pci.h"

static void	boot_q35(const char *cmdline)
{
	fake_reset();
	fab_reset();
	fab_q35();
	snprintf(boot_info_rw()->cmdline, BOOT_CMDLINE_MAX, "%s", cmdline);
	pci_boot_init();
}

static void	autotest_passe_sur_un_q35_factice(void)
{
	boot_q35("selftest pci.q35");
	h_eq_i64("pci_selftest", pci_selftest(), 0);
	h_true(!fake_log_has("[err]"), "aucune erreur journalisee");
	boot_q35("selftest");
	h_eq_i64("sans le mot pci.q35", pci_selftest(), 0);
}

static void	autotest_detecte_les_incoherences(void)
{
	t_pcistate	*st;

	boot_q35("selftest");
	st = pci_state();
	st->devs[1].bar_size[0] = 3;
	h_eq_i64("taille de BAR non puissance de 2", pci_selftest(), 3);
	boot_q35("selftest");
	st->devs[1].bar[0] += 0x1000;
	h_eq_i64("base non alignee sur la taille", pci_selftest(), 3);
	boot_q35("selftest");
	st->devs[2].vendor = 0x1111;
	h_eq_i64("identifiants differents de la config", pci_selftest(), 2);
	boot_q35("selftest");
	st->devs[1].has_msi = true;
	h_eq_i64("MSI annonce sans capacite", pci_selftest(), 4);
	boot_q35("selftest");
	st->count = 0;
	h_eq_i64("aucun appareil", pci_selftest(), 1);
}

static void	autotest_q35_exige_hote_et_vga(void)
{
	boot_q35("selftest pci.q35");
	pci_state()->devs[0].device = 0x1237;
	pci_state()->devs[0].vendor = 0x8086;
	fab_find(0, 0, 0, 0)->cfg[2] = 0x37;
	fab_find(0, 0, 0, 0)->cfg[3] = 0x12;
	h_eq_i64("hote i440fx au lieu de q35", pci_selftest(), 7);
	boot_q35("selftest pci.q35");
	pci_state()->devs[1].device = 0x1112;
	fab_find(0, 0, 1, 0)->cfg[2] = 0x12;
	h_eq_i64("VGA absente", pci_selftest(), 8);
	boot_q35("selftest pci.q35");
	pci_state()->devs[1].bar_flags[0] = 0;
	h_eq_i64("VGA : BAR0 non prefetchable", pci_selftest(), 9);
	boot_q35("selftest pci.edu");
	h_eq_i64("pci.edu sans appareil edu", pci_selftest(), 10);
}

int	main(void)
{
	h_begin("a09/pci_selftest");
	h_run("selftest-pci/passe-sur-q35-factice",
		autotest_passe_sur_un_q35_factice);
	h_run("selftest-pci/incoherences-detectees",
		autotest_detecte_les_incoherences);
	h_run("selftest-pci/exigences-q35-et-edu", autotest_q35_exige_hote_et_vga);
	return (h_end());
}
