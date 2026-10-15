#include <stdint.h>
#include "alloc_int.h"
#include "errno.h"
#include "fake_alloc.h"
#include "fake_sys.h"
#include "harness.h"
#include "stdlib.h"

#define STEPS_N 9

static const size_t	g_steps[STEPS_N] = {20, 3000, 100, 70000, 16, 5000, 5000,
	1, 2049};

static void	realloc_null_and_zero(void)
{
	char			*p;
	t_vheapstats	st;

	fake_reset();
	fake_kernel_on();
	fa_hook_install();
	p = realloc(NULL, 50);
	h_true(p != NULL, "realloc(NULL, n) == malloc(n)");
	h_eq_u64("capacite 64", alloc_capacity((t_ablock *)p - 1), 64);
	h_true(realloc(p, 0) == NULL, "realloc(p, 0) rend NULL");
	v_heap_stats(&st);
	h_eq_u64("bloc libere", st.live_blocks, 0);
	free(p);
	h_eq_i64("p etait bien libre", fa_hook_code(0), ALLOC_FAULT_DOUBLE_FREE);
}

static void	realloc_half_capacity_boundary(void)
{
	char	*p;
	char	*q;

	fake_reset();
	fake_kernel_on();
	p = malloc(100);
	fa_fill(p, 100, 5);
	h_true(realloc(p, 128) == p, "capacite entiere : meme bloc");
	h_true(realloc(p, 65) == p, "juste au-dessus de la moitie : meme bloc");
	h_true(realloc(p, 64) == p, "exactement la moitie : meme bloc");
	q = realloc(p, 63);
	h_true(q != p, "sous la moitie : nouveau bloc");
	h_true(fa_intact(q, 63, 5), "contenu conserve");
	h_eq_u64("capacite 64", alloc_capacity((t_ablock *)q - 1), 64);
	free(q);
}

static char	*resize_check(char *p, size_t cur, size_t next, uint8_t seed)
{
	size_t	keep;

	p = realloc(p, next);
	h_true(p != NULL, "realloc non nul");
	keep = cur;
	if (next < keep)
		keep = next;
	h_true(fa_intact(p, keep, seed), "contenu conserve");
	return (p);
}

static void	realloc_grow_shrink(void)
{
	char	*p;
	size_t	i;
	size_t	cur;

	fake_reset();
	fake_kernel_on();
	cur = g_steps[0];
	p = malloc(cur);
	fa_fill(p, cur, 0);
	i = 1;
	while (i < STEPS_N)
	{
		p = resize_check(p, cur, g_steps[i], (uint8_t)(i - 1));
		cur = g_steps[i];
		fa_fill(p, cur, (uint8_t)i);
		i++;
	}
	free(p);
	h_eq_u64("aucun vfree invalide", g_fsys.vfree_bad, 0);
}

int	main(void)
{
	h_begin("a14/malloc_realloc");
	h_run("realloc/partition : NULL et taille 0", realloc_null_and_zero);
	h_run("realloc/limite : moitie de capacite",
		realloc_half_capacity_boundary);
	h_run("realloc/partition : agrandir et reduire", realloc_grow_shrink);
	return (h_end());
}
