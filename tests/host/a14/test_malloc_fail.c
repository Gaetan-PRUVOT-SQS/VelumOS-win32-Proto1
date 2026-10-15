#include <stdint.h>
#include "alloc_int.h"
#include "errno.h"
#include "fake_alloc.h"
#include "fake_sys.h"
#include "harness.h"
#include "stdlib.h"

#define FAIL_MAX 4

static int	fail_alloc_run(void **p)
{
	int	n;

	n = 0;
	while (n <= FAIL_MAX)
	{
		p[n] = malloc(3000 + 4096 * (size_t)n);
		if (!p[n])
			break ;
		n++;
	}
	return (n);
}

static void	fail_large_until_budget(void)
{
	void	*p[FAIL_MAX + 1];
	int		k;
	int		n;

	k = 0;
	while (k <= FAIL_MAX)
	{
		fake_reset();
		fake_kernel_on();
		g_fsys.valloc_budget = k;
		errno = 0;
		n = fail_alloc_run(p);
		h_eq_i64("autant de succes que de budget", n, k);
		h_eq_i64("errno ENOMEM a l'echec", errno, ENOMEM);
		while (n--)
			free(p[n]);
		h_eq_u64("large_bytes", g_alloc.stats.large_bytes, 0);
		k++;
	}
}

static void	fail_small_chunk(void)
{
	void	*p;
	void	*q;

	fake_reset();
	fake_kernel_on();
	g_fsys.valloc_budget = 0;
	h_eq_u64("aucune tranche au depart", g_alloc.stats.chunk_bytes, 0);
	errno = 0;
	h_true(malloc(16) == NULL, "pas de tranche et pas de budget : refus");
	h_eq_i64("errno ENOMEM", errno, ENOMEM);
	g_fsys.valloc_budget = -1;
	p = malloc(16);
	h_true(p != NULL, "reprise apres remise du budget");
	g_fsys.valloc_budget = 0;
	q = malloc(16);
	h_true(q != NULL, "la tranche courante sert sans noyau");
	free(p);
	free(q);
}

int	main(void)
{
	h_begin("a14/malloc_fail");
	h_run("malloc/injection : gros blocs", fail_large_until_budget);
	h_run("malloc/injection : tranche de petits blocs", fail_small_chunk);
	return (h_end());
}
