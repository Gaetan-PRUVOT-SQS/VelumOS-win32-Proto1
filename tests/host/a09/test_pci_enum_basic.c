#include "a09_test.h"
#include "fake_fab.h"
#include "harness.h"
#include "pci_int.h"

static const t_fabspec	g_host = {0, 0, 0, 0x8086, 0x29c0, 6, 0, 0};
static const t_fabspec	g_multi0 = {0, 3, 0, 0x1af4, 0x1000, 2, 0, 0x80};
static const t_fabspec	g_multi1 = {0, 3, 1, 0x1af4, 0x1001, 1, 0, 0};
static const t_fabspec	g_multi3 = {0, 3, 3, 0x1af4, 0x1003, 3, 0, 0};
static const t_fabspec	g_single = {0, 4, 0, 0x10ec, 0x8139, 2, 0, 0};
static const t_fabspec	g_plain = {0, 3, 0, 0x1af4, 0x1001, 1, 0, 0};

static void	champs_d_un_appareil_type_0(void)
{
	t_fabdev		*d;
	const t_pcidev	*p;

	fab_reset();
	d = fab_make(&g_plain);
	fab_put(d, 0x08, 1, 0x03);
	fab_put(d, 0x09, 1, 0x30);
	fab_put(d, 0x2c, 4, 0x11001af4);
	fab_irq(d, 11, 2);
	pci_scan_root(0, 0);
	p = pci_at(0);
	h_eq_u64("nombre d'appareils", pci_state()->count, 1);
	h_eq_u64("fonction", p->fn, 0);
	h_eq_u64("vendeur", p->vendor, 0x1af4);
	h_eq_u64("appareil", p->device, 0x1001);
	h_eq_u64("classe", p->class_code, 1);
	h_eq_u64("revision", p->revision, 3);
	h_eq_u64("prog_if", p->prog_if, 0x30);
	h_eq_u64("sous-systeme vendeur", p->subsys_vendor, 0x1af4);
	h_eq_u64("sous-systeme appareil", p->subsys_device, 0x1100);
	h_eq_u64("ligne irq", p->irq_line, 11);
	h_eq_u64("broche irq", p->irq_pin, 2);
}

static void	pont_n_a_pas_de_sous_systeme(void)
{
	static const t_fabspec	br = {0, 1, 0, 0x8086, 0x244e, 6, 4, 1};
	t_fabdev				*d;

	fab_reset();
	d = fab_make(&br);
	fab_put(d, 0x2c, 4, 0xdeadbeefu);
	fab_bridge(d, 0, 0);
	pci_scan_root(0, 0);
	h_eq_u64("pont enumere", pci_state()->count, 1);
	h_eq_u64("sous-systeme vendeur non lu", pci_at(0)->subsys_vendor, 0);
	h_eq_u64("sous-systeme appareil non lu", pci_at(0)->subsys_device, 0);
}

static void	multifonction_et_ordre_de_decouverte(void)
{
	fab_reset();
	fab_make(&g_multi3);
	fab_make(&g_multi1);
	fab_make(&g_multi0);
	fab_make(&g_host);
	pci_scan_root(0, 0);
	h_eq_u64("quatre fonctions (la 2 est absente)", pci_state()->count, 4);
	h_eq_u64("premier : 00:00.0", pci_at(0)->dev, 0);
	h_eq_u64("deuxieme : 03:00", pci_at(1)->dev * 10 + pci_at(1)->fn, 30);
	h_eq_u64("troisieme : 03:01", pci_at(2)->dev * 10 + pci_at(2)->fn, 31);
	h_eq_u64("quatrieme : 03:03", pci_at(3)->dev * 10 + pci_at(3)->fn, 33);
}

static void	fonction_1_ignoree_si_pas_multifonction(void)
{
	static const t_fabspec	ghost = {0, 4, 1, 0x10ec, 0x9999, 2, 0, 0};

	fab_reset();
	fab_make(&g_single);
	fab_make(&ghost);
	pci_scan_root(0, 0);
	h_eq_u64("une seule fonction annoncee", pci_state()->count, 1);
	h_eq_u64("c'est la fonction 0", pci_at(0)->device, 0x8139);
}

int	main(void)
{
	h_begin("a09/pci_enum_basic");
	h_run("enum/champs-type-0", champs_d_un_appareil_type_0);
	h_run("enum/pont-sans-sous-systeme", pont_n_a_pas_de_sous_systeme);
	h_run("enum/multifonction-et-ordre", multifonction_et_ordre_de_decouverte);
	h_run("enum/fonction-1-ignoree-sans-bit-multifonction",
		fonction_1_ignoree_si_pas_multifonction);
	return (h_end());
}
