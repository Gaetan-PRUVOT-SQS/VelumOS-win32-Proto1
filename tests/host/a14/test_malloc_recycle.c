#include <stdint.h>
#include "alloc_int.h"
#include "fake_alloc.h"
#include "fake_sys.h"
#include "harness.h"
#include "stdlib.h"

static void	recycle_large(void)
{
	void	*p;
	void	*q;

	fake_reset();
	fake_kernel_on();
	fa_hook_install();
	g_fsys.valloc_reuse = 1;
	p = malloc(10000);
	free(p);
	q = malloc(10000);
	h_true(q == p, "le noyau redonne la meme adresse");
	free(q);
	h_eq_i64("liberation valide, aucune faute", fa_hook_count(), 0);
	h_eq_u64("deux vfree", g_fsys.vfree_calls, 2);
}

static void	recycle_chunk_over_large(void)
{
	void	*p;
	void	*q;

	fake_reset();
	fake_kernel_on();
	fa_hook_install();
	g_fsys.valloc_reuse = 1;
	p = malloc(70000);
	free(p);
	g_alloc.bump = g_alloc.bump_end;
	q = malloc(16);
	h_true(q == p, "la tranche recouvre l'ancien gros bloc");
	free(q);
	h_eq_i64("petit bloc libere sans faute", fa_hook_count(), 0);
}

static void	recycle_double_free_still_caught(void)
{
	void	*p;

	fake_reset();
	fake_kernel_on();
	fa_hook_install();
	g_fsys.valloc_reuse = 1;
	p = malloc(10000);
	free(p);
	free(p);
	h_eq_i64("double liberation encore detectee", fa_hook_count(), 1);
}

int	main(void)
{
	h_begin("a14/malloc_recycle");
	h_run("free/regression I1 : adresse recyclee (gros)", recycle_large);
	h_run("free/regression I1 : adresse recyclee (tranche)",
		recycle_chunk_over_large);
	h_run("free/transition : double liberation apres recyclage",
		recycle_double_free_still_caught);
	return (h_end());
}
