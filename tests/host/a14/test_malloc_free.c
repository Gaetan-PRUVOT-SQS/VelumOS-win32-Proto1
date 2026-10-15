#include <stdint.h>
#include "alloc_int.h"
#include "fake_alloc.h"
#include "fake_sys.h"
#include "harness.h"
#include "stdlib.h"

static void	free_double_small(void)
{
	void	*p;
	void	*a;
	void	*b;

	fake_reset();
	fake_kernel_on();
	fa_hook_install();
	p = malloc(40);
	free(p);
	free(p);
	h_eq_i64("une faute signalee", fa_hook_count(), 1);
	h_eq_i64("code double liberation", fa_hook_code(0),
		ALLOC_FAULT_DOUBLE_FREE);
	a = malloc(40);
	b = malloc(40);
	h_true(a == p, "le bloc libere est reutilise");
	h_true(b != a, "liste libre intacte apres la faute");
	free(a);
	free(b);
	h_eq_i64("pas d'autre faute", fa_hook_count(), 1);
}

static void	free_double_large(void)
{
	void	*p;

	fake_reset();
	fake_kernel_on();
	fa_hook_install();
	p = malloc(10000);
	free(p);
	free(p);
	h_eq_i64("une faute signalee", fa_hook_count(), 1);
	h_eq_i64("code double liberation", fa_hook_code(0),
		ALLOC_FAULT_DOUBLE_FREE);
	h_eq_u64("un seul vfree", g_fsys.vfree_calls, 1);
	h_eq_u64("aucun vfree invalide", g_fsys.vfree_bad, 0);
}

static void	free_bad_pointer(void)
{
	char	*p;

	fake_reset();
	fake_kernel_on();
	fa_hook_install();
	p = malloc(64);
	fa_fill(p, 64, 9);
	free(p + 1);
	free(p + 8);
	h_eq_i64("deux fautes", fa_hook_count(), 2);
	h_eq_i64("code pointeur invalide", fa_hook_code(1),
		ALLOC_FAULT_BAD_POINTER);
	h_true(fa_intact(p, 64, 9), "bloc intact");
	free(p);
	h_eq_i64("liberation normale sans faute", fa_hook_count(), 2);
}

static void	free_null(void)
{
	fake_reset();
	fake_kernel_on();
	fa_hook_install();
	free(NULL);
	h_eq_i64("free(NULL) sans faute", fa_hook_count(), 0);
	h_eq_u64("free(NULL) sans appel noyau", g_fsys.calls, 0);
}

int	main(void)
{
	h_begin("a14/malloc_free");
	h_run("free/transition : double liberation petit bloc", free_double_small);
	h_run("free/transition : double liberation gros bloc", free_double_large);
	h_run("free/partition : pointeur non aligne", free_bad_pointer);
	h_run("free/partition : NULL", free_null);
	return (h_end());
}
