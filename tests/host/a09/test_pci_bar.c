#include "a09_test.h"
#include "fake_fab.h"
#include "harness.h"
#include "pci_int.h"
#include "velum/err.h"

static const t_fabspec	g_dev = {0, 5, 0, 0x1234, 0x1111, 3, 0, 0};
static const t_pciloc	g_loc = {0, 0, 5, 0};

static void	memoire_32_bits_tailles_et_drapeaux(void)
{
	static const uint64_t	sizes[4] = {16, 4096, 0x100000, 0x80000000ull};
	t_fabdev				*d;
	t_pcibar				b;
	int						i;

	fab_reset();
	d = fab_make(&g_dev);
	i = 0;
	while (i < 4)
	{
		fab_bar(d, 0, FAB_MEM32, sizes[i]);
		fab_bar_base(d, 0, 0xfebf0000u & ~(sizes[i] - 1));
		h_eq_i64("sonde", pcibar_probe(&g_loc, 0, 6, &b), 0);
		h_eq_u64("taille", b.size, sizes[i]);
		h_eq_u64("base", b.base, 0xfebf0000u & ~(sizes[i] - 1));
		h_eq_u64("drapeaux : memoire 32 bits non prefetchable", b.flags, 0);
		i++;
	}
	d->bar[0].pref = 1;
	h_eq_i64("sonde prefetchable", pcibar_probe(&g_loc, 0, 6, &b), 0);
	h_eq_u64("drapeau prefetchable", b.flags, PCI_BAR_PREFETCH);
}

static void	memoire_64_bits_deux_emplacements(void)
{
	t_fabdev	*d;
	t_pcibar	b;

	fab_reset();
	d = fab_make(&g_dev);
	fab_bar(d, 1, FAB_MEM64_LO, 0x100000000ull);
	fab_bar_base(d, 1, 0x200000000ull);
	d->bar[1].pref = 1;
	h_eq_i64("sonde 4 Gio", pcibar_probe(&g_loc, 1, 6, &b), 0);
	h_eq_u64("taille 4 Gio", b.size, 0x100000000ull);
	h_eq_u64("base au-dela de 4 Gio", b.base, 0x200000000ull);
	h_eq_u64("drapeaux 64 bits prefetchable", b.flags,
		PCI_BAR_MEM64 | PCI_BAR_PREFETCH);
	h_eq_u64("deux emplacements", b.slots, 2);
	fab_bar(d, 3, FAB_MEM64_LO, 0x1000000);
	fab_bar_base(d, 3, 0x180000000ull);
	h_eq_i64("sonde 16 Mio", pcibar_probe(&g_loc, 3, 6, &b), 0);
	h_eq_u64("taille 16 Mio (mot haut cable a un)", b.size, 0x1000000);
	h_eq_u64("base 0x1_8000_0000", b.base, 0x180000000ull);
}

static void	es_inexistante_et_tout_a_un(void)
{
	t_fabdev	*d;
	t_pcibar	b;

	fab_reset();
	d = fab_make(&g_dev);
	fab_bar(d, 0, FAB_IO, 8);
	fab_bar_base(d, 0, 0xc040);
	h_eq_i64("sonde E/S", pcibar_probe(&g_loc, 0, 6, &b), 0);
	h_eq_u64("taille E/S", b.size, 8);
	h_eq_u64("base E/S", b.base, 0xc040);
	h_eq_u64("drapeau E/S", b.flags, PCI_BAR_IO);
	h_eq_i64("BAR inexistante : E_NODEV", pcibar_probe(&g_loc, 1, 6, &b),
		E_NODEV);
	h_eq_u64("une case", b.slots, 1);
	fab_bar(d, 2, FAB_ONES, 0);
	h_eq_i64("registre qui lit tout a un", pcibar_probe(&g_loc, 2, 6, &b),
		E_NODEV);
}

static void	base_nulle_non_attribuee(void)
{
	t_fabdev	*d;
	t_pcibar	b;

	fab_reset();
	d = fab_make(&g_dev);
	fab_bar(d, 0, FAB_MEM32, 0x1000);
	h_eq_i64("sonde", pcibar_probe(&g_loc, 0, 6, &b), 0);
	h_eq_u64("taille connue", b.size, 0x1000);
	h_eq_u64("base non attribuee", b.base, 0);
}

int	main(void)
{
	h_begin("a09/pci_bar");
	h_run("bar/memoire-32-tailles-16-a-2gio-et-prefetch",
		memoire_32_bits_tailles_et_drapeaux);
	h_run("bar/memoire-64-bits-4gio-et-16mio",
		memoire_64_bits_deux_emplacements);
	h_run("bar/es-inexistante-et-tout-a-un", es_inexistante_et_tout_a_un);
	h_run("bar/base-nulle", base_nulle_non_attribuee);
	return (h_end());
}
