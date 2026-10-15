#include <stdint.h>
#include <string.h>
#include "a04_fake.h"
#include "velum/err.h"

static const uint32_t	g_shifts[] = {10, 13, 27, 40, 22, 22, 22};
static const uint32_t	g_larges[] = {64, 64, 64, 64, 0, 3, 65537};

static void	layout_backend_refuses(void)
{
	fake_pages_setup(A04_SHIFT, A04_LARGE);
	memset(&g_heap, 0, sizeof(g_heap));
	fake_pages_busy(1);
	h_eq_i64("E14 region deja mappee : E_BUSY", heap_boot_init(), E_BUSY);
	h_eq_i64("E14 tas non pret", g_heap.ready, 0);
	fake_pages_busy(0);
	h_eq_i64("E14 puis succes", heap_boot_init(), 0);
	h_eq_i64("E14 tas pret", g_heap.ready, 1);
	a04_drain("E14 E_BUSY");
}

static void	layout_invalid_values(void)
{
	size_t	i;

	i = 0;
	while (i < sizeof(g_shifts) / sizeof(g_shifts[0]))
	{
		fake_pages_setup(A04_SHIFT, A04_LARGE);
		g_fake.slab_shift = g_shifts[i];
		g_fake.large_pages = g_larges[i];
		memset(&g_heap, 0, sizeof(g_heap));
		h_eq_i64("E14 disposition invalide : E_INVAL", heap_boot_init(),
			E_INVAL);
		h_eq_i64("E14 tas non pret", g_heap.ready, 0);
		i++;
	}
}

static void	layout_limits_are_accepted(void)
{
	fake_pages_setup(14, 4);
	memset(&g_heap, 0, sizeof(g_heap));
	h_eq_i64("E14 minimum accepte", heap_boot_init(), 0);
	h_eq_u64("E14 une dalle de 16 Kio", g_heap.arena[2].slots, 1);
	kfree(kmalloc(1024));
	a04_drain("E14 minimum");
	a04_fresh_with(26, 65536);
	h_eq_u64("E14 maximum : dalles de 4 Kio", g_heap.arena[0].slots, 16384);
	h_eq_u64("E14 maximum : pages", g_heap.arena[3].slots, 65536);
	a04_drain("E14 limites");
}

static void	layout_init_is_idempotent(void)
{
	void		*p;
	uint64_t	bytes;

	a04_fresh();
	p = kmalloc(100);
	bytes = a04_bytes();
	h_eq_i64("E14 second appel : 0", heap_boot_init(), 0);
	h_eq_u64("E14 etat intact", a04_bytes(), bytes);
	kfree(p);
	a04_drain("E14 idempotence");
}

int	main(void)
{
	h_begin("a04/disposition");
	h_run("E14 refus du fond", layout_backend_refuses);
	h_run("E14 dispositions invalides", layout_invalid_values);
	h_run("E14 bornes acceptees", layout_limits_are_accepted);
	h_run("E14 idempotence", layout_init_is_idempotent);
	return (h_end());
}
