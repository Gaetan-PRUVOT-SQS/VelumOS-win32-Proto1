#include <stdint.h>
#include "alloc_int.h"
#include "errno.h"
#include "fake_alloc.h"
#include "fake_sys.h"
#include "harness.h"
#include "stdlib.h"
#include "string.h"

static int	all_zero(const void *p, size_t n)
{
	const uint8_t	*b;

	b = p;
	while (n--)
	{
		if (*b++)
			return (0);
	}
	return (1);
}

static void	calloc_zero_recycled(void)
{
	void	*p;
	void	*q;

	fake_reset();
	fake_kernel_on();
	p = malloc(100);
	memset(p, 0xaa, 100);
	free(p);
	q = calloc(100, 1);
	h_true(q == p, "bloc recycle");
	h_true(all_zero(q, 100), "bloc recycle remis a zero");
	free(q);
	q = calloc(10, 10000);
	h_true(q != NULL && all_zero(q, 100000), "gros bloc a zero");
	free(q);
	q = calloc(0, 5);
	h_true(q != NULL, "calloc(0, 5) rend un bloc");
	free(q);
}

static void	calloc_overflow(void)
{
	void	*p;

	fake_reset();
	fake_kernel_on();
	errno = 0;
	h_true(calloc(SIZE_MAX / 2, 3) == NULL, "n * size deborde");
	h_eq_i64("errno ENOMEM", errno, ENOMEM);
	p = malloc(10);
	errno = 0;
	h_true(reallocarray(p, SIZE_MAX / 2 + 1, 4) == NULL,
		"reallocarray deborde");
	h_eq_i64("errno ENOMEM", errno, ENOMEM);
	h_true(calloc(1UL << 31, 1UL << 31) == NULL, "produit 2^62 refuse");
	free(p);
}

static void	realloc_failure_keeps_block(void)
{
	char	*p;
	void	*q;

	fake_reset();
	fake_kernel_on();
	fa_hook_install();
	p = malloc(40);
	fa_fill(p, 40, 11);
	g_fsys.valloc_budget = 0;
	errno = 0;
	q = realloc(p, 100000);
	h_true(q == NULL, "realloc echoue sans memoire");
	h_eq_i64("errno ENOMEM", errno, ENOMEM);
	h_true(fa_intact(p, 40, 11), "ancien bloc intact");
	g_fsys.valloc_budget = -1;
	free(p);
	h_eq_i64("ancien bloc liberable sans faute", fa_hook_count(), 0);
}

int	main(void)
{
	h_begin("a14/malloc_calloc");
	h_run("calloc/partition : zero sur bloc recycle", calloc_zero_recycled);
	h_run("calloc/limite : n * size deborde", calloc_overflow);
	h_run("realloc/injection : panne du noyau", realloc_failure_keeps_block);
	return (h_end());
}
