#include "velum/err.h"
#include "harness.h"
#include "fake.h"

static void	ecriture_exigee(void)
{
	t_aspace	*as;

	as = fake_user_env();
	h_true(user_range_wok(UVA, 2 * 4096), "deux pages RW : inscriptible");
	h_true(user_range_ok(UVA + 2 * 4096, 8), "page RO : lisible");
	h_true(!user_range_wok(UVA + 2 * 4096, 8), "page RO : non inscriptible");
	h_true(user_range_ok(UVA + 2 * 4096 - 8, 16), "RW puis RO : lisible");
	h_true(!user_range_wok(UVA + 2 * 4096 - 8, 16),
		"RW puis RO : non inscriptible");
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	bornes_communes(void)
{
	t_aspace	*as;

	as = fake_user_env();
	h_true(user_range_wok(0, 0), "plage vide");
	h_true(!user_range_wok(0, 8), "adresse nulle");
	h_true(!user_range_wok(KERNEL_BASE, 8), "espace noyau");
	h_true(!user_range_wok(UVA, UPTR_COPY_MAX + 1), "au dela du plafond");
	h_true(!user_range_wok(0xfffffffffffffff0ull, 0x20), "deborde 64 bits");
	h_true(!user_range_wok(UVA + 3 * 4096, 8), "non mappee");
	vmm_switch(NULL);
	h_true(!user_range_wok(UVA, 8), "espace noyau courant");
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

int	main(void)
{
	h_begin("a03/user_wr");
	h_run("ecriture exigee", ecriture_exigee);
	h_run("bornes communes", bornes_communes);
	return (h_end());
}
