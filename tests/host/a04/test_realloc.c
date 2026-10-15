#include <stdint.h>
#include "a04_fake.h"
#include "velum/libk.h"

static const uint32_t	g_moves[][2] = {{1, 1}, {10, 16}, {16, 17}, {100, 2000},
{2000, 100}, {1000, 3000}, {3000, 1000}, {5000, 5200}, {5200, 5000},
{5000, 9000}, {9000, 5000}, {3000, 100000}, {100000, 3000}, {100000, 1000},
{100, 100}, {2048, 2049}, {4096, 4097}, {20000, 20000}};

static void	realloc_null_and_zero(void)
{
	void	*p;

	a04_fresh();
	p = krealloc(NULL, 40);
	h_true(p != NULL, "E5 NULL = kmalloc");
	h_eq_u64("E5 NULL : compteur", a04_bytes(), a04_slot(40));
	h_true(krealloc(p, 0) == NULL, "E5 taille 0 libere");
	h_eq_u64("E5 taille 0 : compteur", a04_bytes(), 0);
	p = krealloc(NULL, 0);
	h_true(p != NULL, "E5 NULL et 0 : objet valide");
	kfree(p);
	a04_drain("E5 realloc NULL et 0");
}

static void	realloc_content_and_counters(void)
{
	size_t	i;
	uint8_t	*p;
	uint8_t	*q;
	size_t	keep;

	a04_fresh();
	i = 0;
	while (i < sizeof(g_moves) / sizeof(g_moves[0]))
	{
		p = kmalloc(g_moves[i][0]);
		a04_fill(p, g_moves[i][0], (uint8_t)(i + 5));
		q = krealloc(p, g_moves[i][1]);
		h_true(q != NULL, "E5 deplacement : resultat");
		keep = (size_t)min_u64(g_moves[i][0], g_moves[i][1]);
		h_eq_i64("E5 deplacement : contenu conserve", a04_verify(q, keep,
				(uint8_t)(i + 5)), 0);
		h_eq_u64("E5 deplacement : compteur", a04_bytes(),
			a04_slot(g_moves[i][1]));
		a04_fill(q, g_moves[i][1], 77);
		kfree(q);
		i++;
	}
	a04_drain("E5 contenu et compteurs");
}

static void	realloc_in_place_when_same_unit(void)
{
	void	*p;
	void	*q;

	a04_fresh();
	p = kmalloc(10);
	q = krealloc(p, 16);
	h_true(p == q, "E5 meme classe : meme adresse");
	p = krealloc(q, 17);
	h_true(p != q || a04_slot(17) == 16, "E5 autre classe : nouvelle adresse");
	kfree(p);
	p = kmalloc(5000);
	q = krealloc(p, 5200);
	h_true(p == q, "E5 meme nombre de pages : meme adresse");
	q = krealloc(p, 9000);
	h_true(q != NULL, "E5 plus de pages");
	kfree(q);
	a04_drain("E5 sur place");
}

static void	realloc_keeps_the_tag(void)
{
	t_heap_stats	st;
	void			*p;

	a04_fresh();
	p = kmalloc_tag(100, HEAP_PROC);
	p = krealloc(p, 3000);
	p = krealloc(p, 40);
	heap_get_stats(&st);
	h_eq_u64("E5 etiquette : octets PROC", st.bytes_live[HEAP_PROC],
		a04_slot(40));
	h_eq_u64("E5 etiquette : objets PROC", st.allocs_live[HEAP_PROC], 1);
	h_eq_u64("E5 etiquette : rien en GENERIC", st.allocs_live[HEAP_GENERIC],
		0);
	kfree(p);
	a04_drain("E5 etiquette conservee");
}

int	main(void)
{
	h_begin("a04/realloc");
	h_run("E5 NULL et zero", realloc_null_and_zero);
	h_run("E5 contenu et compteurs", realloc_content_and_counters);
	h_run("E5 sur place", realloc_in_place_when_same_unit);
	h_run("E5 etiquette conservee", realloc_keeps_the_tag);
	return (h_end());
}
