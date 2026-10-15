#include "heap_int.h"
#include "heap_st.h"
#include "velum/pmm.h"

extern void	pmm_fail_after(int64_t n) __attribute__((weak));

int	st_realloc(void)
{
	uint8_t	*p;
	uint8_t	*q;

	p = kmalloc(20);
	st_fill(p, 20, 4);
	q = krealloc(p, 3000);
	if (!q || st_verify(q, 20, 4))
		return (1);
	p = krealloc(q, 40);
	if (!p || st_verify(p, 20, 4))
		return (2);
	if (krealloc(p, 0) || kcalloc(SIZE_MAX / 2 + 1, 2))
		return (3);
	p = kcalloc(10, 10);
	q = kzalloc(5000);
	if (!p || !q || p[0] || p[99] || q[0] || q[4999])
		return (4);
	kfree(p);
	kfree(q);
	return (0);
}

static int	st_pmm_refusal(void)
{
	void	*p;

	if (!pmm_fail_after)
		return (0);
	pmm_fail_after(0);
	p = kmalloc(100000);
	pmm_fail_after(-1);
	if (p)
		return (1);
	p = kmalloc(100000);
	if (!p)
		return (2);
	kfree(p);
	return (0);
}

int	st_failures(void)
{
	void	*p[3];

	heap_fail_after(2);
	p[0] = kmalloc(8);
	p[1] = kmalloc(8);
	p[2] = kmalloc(8);
	heap_fail_after(-1);
	if (!p[0] || !p[1] || p[2])
		return (1);
	kfree(p[0]);
	kfree(p[1]);
	heap_fail_after(0);
	p[0] = kmalloc(5000);
	heap_fail_after(-1);
	if (p[0])
		return (2);
	return (st_pmm_refusal());
}
