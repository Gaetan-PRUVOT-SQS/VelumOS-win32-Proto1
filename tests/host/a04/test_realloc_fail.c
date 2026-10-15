#include <stdint.h>
#include "a04_fake.h"
#include "velum/libk.h"

static void	realloc_growth_failure_keeps_block(void)
{
	uint8_t		*p;
	void		*q;
	uint64_t	before;

	a04_fresh();
	p = kmalloc(100);
	a04_fill(p, 100, 3);
	before = a04_bytes();
	heap_fail_after(0);
	q = krealloc(p, 5000);
	heap_fail_after(-1);
	h_true(q == NULL, "E5 echec : NULL");
	h_eq_u64("E5 echec : compteur inchange", a04_bytes(), before);
	h_eq_i64("E5 echec : contenu inchange", a04_verify(p, 100, 3), 0);
	kfree(p);
	a04_drain("E5 echec de croissance");
}

static void	realloc_shrink_never_fails(void)
{
	uint8_t	*p;
	uint8_t	*q;

	a04_fresh();
	p = kmalloc(3000);
	a04_fill(p, 3000, 8);
	heap_fail_after(0);
	q = krealloc(p, 100);
	heap_fail_after(-1);
	h_true(q == p, "E5 reduction refusee : meme bloc");
	h_eq_i64("E5 reduction : contenu", a04_verify(q, 100, 8), 0);
	kfree(q);
	a04_drain("E5 reduction sans memoire");
}

static void	calloc_cases(void)
{
	uint8_t	*p;
	size_t	i;

	a04_fresh();
	h_true(kcalloc(SIZE_MAX / 2 + 1, 2) == NULL, "E5 calloc : debordement");
	h_true(kcalloc(SIZE_MAX, SIZE_MAX) == NULL, "E5 calloc : maxi");
	h_true(kcalloc(1ull << 33, 1ull << 33) == NULL, "E5 calloc : 2^66");
	p = kcalloc(0, 8);
	h_true(p != NULL, "E5 calloc : 0 x 8");
	kfree(p);
	p = kcalloc(8, 0);
	h_true(p != NULL, "E5 calloc : 8 x 0");
	kfree(p);
	p = kmalloc(100);
	memset(p, 0xee, 100);
	kfree(p);
	p = kcalloc(10, 10);
	i = 0;
	while (i < 100)
		h_true(p[i++] == 0, "E5 calloc : zero sur memoire sale");
	kfree(p);
	a04_drain("E5 calloc");
}

static void	zeroing_over_dirty_pages(void)
{
	uint8_t	*p;
	size_t	i;

	a04_fresh();
	p = kzalloc(9000);
	i = 0;
	while (i < 9000)
	{
		if (p[i] != 0)
			h_true(0, "E5 kzalloc : octet non nul");
		i++;
	}
	kfree(p);
	p = kcalloc(3, 3000);
	h_true(p[0] == 0 && p[8999] == 0, "E5 kcalloc gros : zero");
	kfree(p);
	a04_drain("E5 mise a zero");
}

int	main(void)
{
	h_begin("a04/realloc-echec");
	h_run("E5 echec de croissance", realloc_growth_failure_keeps_block);
	h_run("E5 reduction sans memoire", realloc_shrink_never_fails);
	h_run("E5 kcalloc", calloc_cases);
	h_run("E5 mise a zero", zeroing_over_dirty_pages);
	return (h_end());
}
