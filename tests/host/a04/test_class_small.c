#include "a04_fake.h"
#include "velum/libk.h"

static const uint16_t	g_sizes[] = {1, 15, 16, 17, 31, 32, 33, 47, 48, 49, 63,
	64, 65, 95, 96, 97, 127, 128, 129, 191, 192, 193, 255, 256, 257, 383, 384,
	385, 511, 512, 513, 767, 768, 769, 1023, 1024, 1025, 1535, 1536, 1537,
	2000, 2032, 2033, 2047, 2048};

static void	class_zero_size_is_valid(void)
{
	void	*p;

	a04_fresh();
	p = kmalloc(0);
	h_true(p != NULL, "E1 taille 0 : pointeur non nul");
	h_true(((uintptr_t)p & 15) == 0, "E1 taille 0 : aligne sur 16");
	h_eq_u64("E1 taille 0 : une classe de 16", a04_bytes(), 16);
	kfree(p);
	a04_drain("E1 taille 0");
}

static void	class_boundary_sizes(void)
{
	void		*p;
	size_t		i;
	uint64_t	before;

	a04_fresh();
	i = 0;
	while (i < sizeof(g_sizes) / sizeof(g_sizes[0]))
	{
		before = a04_bytes();
		p = kmalloc(g_sizes[i]);
		h_true(p != NULL, "E1 limites : allocation");
		h_true(((uintptr_t)p & 15) == 0, "E1 limites : alignement 16");
		h_eq_u64("E1 limites : octets de la classe", a04_bytes() - before,
			a04_slot(g_sizes[i]));
		a04_fill(p, g_sizes[i], (uint8_t)i);
		h_eq_i64("E1 limites : donnees intactes", a04_verify(p, g_sizes[i],
				(uint8_t)i), 0);
		kfree(p);
		h_eq_u64("E1 limites : retour au compteur initial", a04_bytes(),
			before);
		i++;
	}
	a04_drain("E1 limites");
}

static void	class_neighbours_do_not_overlap(void)
{
	void	*p[64];
	size_t	i;

	a04_fresh();
	i = 0;
	while (i < 64)
	{
		p[i] = kmalloc(48);
		a04_fill(p[i], 48, (uint8_t)(i * 3));
		i++;
	}
	i = 0;
	while (i < 64)
	{
		h_eq_i64("E1 voisins : aucun recouvrement", a04_verify(p[i], 48,
				(uint8_t)(i * 3)), 0);
		kfree(p[i]);
		i++;
	}
	a04_drain("E1 voisins");
}

static void	class_small_stays_in_slab_arena(void)
{
	void	*p;

	a04_fresh();
	p = kmalloc(1000);
	h_true(a04_is_slab(p), "E1 1000 octets : voie des dalles");
	kfree(p);
	a04_drain("E1 voie des dalles");
}

int	main(void)
{
	h_begin("a04/classes-petites");
	h_run("E1 taille zero", class_zero_size_is_valid);
	h_run("E1 valeurs limites des classes", class_boundary_sizes);
	h_run("E1 objets voisins", class_neighbours_do_not_overlap);
	h_run("E1 voie des dalles", class_small_stays_in_slab_arena);
	return (h_end());
}
