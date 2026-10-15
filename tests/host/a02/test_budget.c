#include <stdio.h>
#include "a02_fake.h"

static void	budget_case(const char *label, uint64_t mib, uint64_t pages)
{
	uint64_t	meta_kib;
	uint64_t	ram_kib;

	h_eq_i64(label, fake_simple(1, mib), 0);
	h_eq_u64(label, g_pmm.span, mib * 256);
	h_eq_u64(label, g_pmm.meta_pages, pages);
	meta_kib = g_pmm.meta_pages * 4;
	ram_kib = mib * 1024;
	printf("budget %-8s : span %9llu pages, métadonnées %6llu Kio"
		" (%llu.%03llu %%)\n", label, (unsigned long long)g_pmm.span,
		(unsigned long long)meta_kib,
		(unsigned long long)(meta_kib * 100 / ram_kib),
		(unsigned long long)(meta_kib * 100000 / ram_kib % 1000));
}

static void	budget_256_mio(void)
{
	budget_case("256Mio", 256, 18);
}

static void	budget_4_gio(void)
{
	budget_case("4Gio", 4096, 288);
}

static void	budget_64_gio(void)
{
	budget_case("64Gio", 65536, 4608);
}

int	main(void)
{
	h_begin("a02/budget");
	h_run("budget/256Mio : 72 Kio", budget_256_mio);
	h_run("budget/4Gio : 1152 Kio", budget_4_gio);
	h_run("budget/64Gio : 18432 Kio", budget_64_gio);
	return (h_end());
}
