#include <stdint.h>
#include "a04_fake.h"
#include "velum/err.h"

static void	unmap_refused_keeps_the_slab(void)
{
	void		*hold[600];
	uint32_t	n;
	t_cache		*k;

	a04_fresh();
	k = &g_heap.cls[0].cache[0];
	n = a04_grab(16, 0, hold, 500);
	h_eq_u64("E8 deux dalles", g_fake.mapped, 2);
	fake_pages_unmap_fail(3, E_NOMEM);
	a04_release(hold, n);
	h_eq_u64("E8 demontage refuse : dalle gardee", g_fake.mapped, 2);
	h_eq_u64("E8 refus compte", g_heap.unmap_refused, 1);
	h_eq_u64("E8 deux dalles vides dans le cache", k->empty, 2);
	h_eq_i64("E8 heap_check apres le refus", heap_check(), 0);
	fake_pages_unmap_fail(0, 0);
	heap_trim();
	h_eq_u64("E8 trim rend tout", g_fake.mapped, 0);
	a04_drain("E8 demontage de dalle refuse");
}

static void	unmap_refused_during_trim(void)
{
	void	*hold[600];

	a04_fresh();
	a04_release(hold, a04_grab(16, 0, hold, 500));
	fake_pages_unmap_fail(5, E_NOMEM);
	heap_trim();
	h_eq_u64("E8 trim refuse : dalle gardee", g_fake.mapped, 1);
	h_eq_i64("E8 heap_check apres trim refuse", heap_check(), 0);
	fake_pages_unmap_fail(0, 0);
	heap_trim();
	a04_drain("E8 trim refuse");
}

static void	unmap_refused_on_large_block_can_be_retried(void)
{
	t_heap_stats	st;
	void			*p;

	a04_fresh();
	p = kmalloc(9000);
	fake_pages_unmap_fail(1, E_NOMEM);
	kfree(p);
	heap_get_stats(&st);
	h_eq_u64("E8 bloc refuse : toujours vivant", st.allocs_live[0], 1);
	h_eq_u64("E8 bloc refuse : pages gardees", g_fake.mapped, 3);
	h_eq_i64("E8 heap_check apres le refus", heap_check(), 0);
	kfree(p);
	a04_drain("E8 gros bloc refuse puis libere");
}

static void	unmap_other_errors_are_kernel_bugs(void)
{
	void	*p;
	void	*hold[600];

	a04_fresh();
	p = kmalloc(9000);
	fake_pages_unmap_fail(1, E_INVAL);
	a04_expect_panic(kfree, p, "vmm_unmap a échoué");
	a04_fresh();
	a04_release(hold, a04_grab(16, 0, hold, 500));
	fake_pages_unmap_fail(1, E_INVAL);
	a04_expect_panic(a04_do_trim, NULL, "vmm_unmap a échoué");
}

int	main(void)
{
	h_begin("a04/demontage-refuse");
	h_run("E8 dalle gardee", unmap_refused_keeps_the_slab);
	h_run("E8 trim refuse", unmap_refused_during_trim);
	h_run("E8 gros bloc refuse", unmap_refused_on_large_block_can_be_retried);
	h_run("E8 autres erreurs", unmap_other_errors_are_kernel_bugs);
	return (h_end());
}
