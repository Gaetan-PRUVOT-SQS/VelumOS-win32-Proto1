#include <stdint.h>
#include "a04_fake.h"

static void	life_second_empty_slab_is_released(void)
{
	void		*p[600];
	uint32_t	i;

	a04_fresh();
	i = 0;
	while (i < 500)
		p[i++] = kmalloc(16);
	h_eq_u64("E6 deux dalles pleines", g_fake.mapped, 2);
	while (i > 0)
	{
		i--;
		kfree(p[i]);
	}
	h_eq_u64("E6 la seconde dalle vide est rendue", g_fake.mapped, 1);
	h_eq_u64("E6 un seul demontage", g_fake.unmaps, 1);
	a04_drain("E6 seconde dalle vide");
}

int	main(void)
{
	h_begin("a04/dalles-liberation");
	h_run("E6 seconde dalle vide", life_second_empty_slab_is_released);
	return (h_end());
}
