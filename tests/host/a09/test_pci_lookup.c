#include "a09_test.h"
#include "fake_fab.h"
#include "harness.h"
#include "pci_int.h"

static void	peupler(void)
{
	static const t_fabspec	a = {0, 0, 0, 0x8086, 0x29c0, 6, 0, 0};
	static const t_fabspec	b = {0, 1, 0, 0x1234, 0x1111, 3, 0, 0};
	static const t_fabspec	c = {0, 2, 0, 0x1af4, 0x1000, 2, 0, 0};
	static const t_fabspec	d = {0, 3, 0, 0x1af4, 0x1000, 2, 0, 0};

	fab_reset();
	fab_make(&a);
	fab_make(&b);
	fab_make(&c);
	fab_make(&d);
	pci_scan_root(0, 0);
}

static void	comptage_et_acces_par_indice(void)
{
	peupler();
	h_eq_u64("quatre appareils", pci_count(), 4);
	h_eq_u64("indice 0", pci_at(0)->vendor, 0x8086);
	h_eq_u64("indice 3", pci_at(3)->dev, 3);
	h_true(pci_at(4) == NULL, "indice 4 : NULL");
	h_true(pci_at(0xffffffffu) == NULL, "indice maximal : NULL");
}

static void	recherche_par_identifiants(void)
{
	peupler();
	h_eq_u64("premier 1af4:1000", pci_find(0x1af4, 0x1000, 0)->dev, 2);
	h_eq_u64("second 1af4:1000", pci_find(0x1af4, 0x1000, 1)->dev, 3);
	h_true(pci_find(0x1af4, 0x1000, 2) == NULL, "troisieme : NULL");
	h_true(pci_find(0x1af4, 0x1001, 0) == NULL, "appareil inconnu");
	h_true(pci_find(0xffff, 0xffff, 0) == NULL, "0xffff:0xffff");
}

static void	recherche_par_classe(void)
{
	peupler();
	h_eq_u64("classe 06:00", pci_find_class(6, 0, 0)->device, 0x29c0);
	h_eq_u64("classe 02:00 premier", pci_find_class(2, 0, 0)->dev, 2);
	h_eq_u64("classe 02:00 second", pci_find_class(2, 0, 1)->dev, 3);
	h_true(pci_find_class(2, 0, 2) == NULL, "troisieme : NULL");
	h_true(pci_find_class(2, 1, 0) == NULL, "sous-classe differente");
}

int	main(void)
{
	h_begin("a09/pci_lookup");
	h_run("lookup/comptage-et-indices", comptage_et_acces_par_indice);
	h_run("lookup/vendeur-appareil-et-index", recherche_par_identifiants);
	h_run("lookup/classe-sous-classe-et-index", recherche_par_classe);
	return (h_end());
}
