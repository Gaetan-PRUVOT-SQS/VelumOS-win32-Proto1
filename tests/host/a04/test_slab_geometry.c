#include <stdint.h>
#include <stdio.h>
#include "a04_fake.h"

static const uint16_t	g_sizes[HEAP_CLASSES] = {16, 32, 48, 64, 96, 128, 192,
	256, 384, 512, 768, 1024, 1536, 2048};
static const uint16_t	g_nobj[HEAP_CLASSES] = {252, 126, 84, 63, 42, 31, 21,
	15, 10, 15, 5, 15, 5, 7};
static const uint8_t	g_arena[HEAP_CLASSES] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	0, 2, 1, 2};

static void	geometry_matches_the_table(void)
{
	uint32_t	i;

	a04_fresh();
	i = 0;
	while (i < HEAP_CLASSES)
	{
		h_eq_u64("E1 taille de classe", g_heap.cls[i].size, g_sizes[i]);
		h_eq_u64("E1 objets par dalle", g_heap.cls[i].nobj, g_nobj[i]);
		h_eq_u64("E1 arene de la classe", g_heap.cls[i].arena, g_arena[i]);
		h_eq_u64("E1 decalage 64", g_heap.cls[i].obj_off % 64, 0);
		i++;
	}
	a04_drain("E1 geometrie");
}

static void	geometry_fits_in_the_slab(void)
{
	const t_class	*c;
	uint32_t		i;
	uint32_t		bytes;
	uint32_t		meta;

	a04_fresh();
	i = 0;
	while (i < HEAP_CLASSES)
	{
		c = &g_heap.cls[i];
		bytes = PAGE_SIZE << c->arena;
		meta = HEAP_SLAB_HDR + 8 * ((c->nobj + 63) / 64);
		h_true(meta <= c->obj_off, "E1 en-tete et bitmap avant les objets");
		h_true(c->obj_off + c->nobj * c->size <= bytes, "E1 dans la dalle");
		h_true((c->obj_off + (c->nobj + 1) * c->size > bytes), "E1 maximal");
		printf("a04 mesure : classe %4u dalle %5u objets %4u en-tete %3u "
			"octets (%.3f par objet) reste %u\n", c->size, bytes, c->nobj,
			c->obj_off, (double)c->obj_off / c->nobj,
			bytes - c->obj_off - c->nobj * c->size);
		i++;
	}
	a04_drain("E1 dans la dalle");
}

static void	geometry_class_lookup_is_exhaustive(void)
{
	size_t		need;
	uint32_t	i;
	uint32_t	want;

	a04_fresh();
	need = 0;
	while (need <= 2049)
	{
		want = 0;
		while (want < HEAP_CLASSES && g_sizes[want] < need)
			want++;
		i = slab_class_find(need, 16);
		h_eq_u64("E1 recherche de classe", i, want);
		need++;
	}
	a04_drain("E1 recherche exhaustive");
}

static void	geometry_aligned_lookup_is_exhaustive(void)
{
	size_t		need;
	uint32_t	al;
	uint32_t	want;

	a04_fresh();
	al = 32;
	while (al <= 64)
	{
		need = 0;
		while (need <= 2049)
		{
			want = 0;
			while (want < HEAP_CLASSES && (g_sizes[want] < need
					|| g_sizes[want] % al))
				want++;
			h_eq_u64("E2 classe alignee", slab_class_find(need, al), want);
			need++;
		}
		al *= 2;
	}
	a04_drain("E2 recherche alignee exhaustive");
}

int	main(void)
{
	h_begin("a04/geometrie");
	h_run("E1 geometrie des classes", geometry_matches_the_table);
	h_run("E1 contenance des dalles", geometry_fits_in_the_slab);
	h_run("E1 recherche de classe exhaustive",
		geometry_class_lookup_is_exhaustive);
	h_run("E2 classe alignee exhaustive",
		geometry_aligned_lookup_is_exhaustive);
	return (h_end());
}
