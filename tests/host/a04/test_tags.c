#include <stdint.h>
#include "a04_fake.h"

static void	tags_distinct(const uint64_t *idx)
{
	uint32_t	t;
	uint32_t	u;

	t = 0;
	while (t < HEAP_TAGS)
	{
		u = t + 1;
		while (u < HEAP_TAGS)
			h_true(idx[t] != idx[u++], "E4 deux etiquettes, deux dalles");
		t++;
	}
}

static void	tags_use_separate_slabs(void)
{
	void		*p[HEAP_TAGS];
	uint32_t	t;
	uint64_t	idx[HEAP_TAGS];

	a04_fresh();
	t = 0;
	while (t < HEAP_TAGS)
	{
		p[t] = kmalloc_tag(24, (t_heap_tag)t);
		idx[t] = arena_index(&g_heap.arena[0], (uintptr_t)p[t]);
		t++;
	}
	tags_distinct(idx);
	h_eq_u64("E4 une dalle par etiquette", g_fake.mapped, HEAP_TAGS);
	a04_release(p, HEAP_TAGS);
	a04_drain("E4 dalles separees");
}

static void	tags_unknown_tag_is_a_kernel_bug(void)
{
	t_tagcall	c;

	a04_fresh();
	c.size = 8;
	c.tag = HEAP_TAGS;
	a04_expect_panic(a04_do_tag, &c, "étiquette de propriétaire inconnue");
	c.tag = -1;
	a04_expect_panic(a04_do_tag, &c, "étiquette de propriétaire inconnue");
	c.tag = HEAP_TAGS - 1;
	a04_do_tag(&c);
	h_true(c.out != NULL, "E4 derniere etiquette valide");
	kfree(c.out);
	a04_drain("E4 etiquette inconnue");
}

static void	tags_large_blocks_are_counted_per_tag(void)
{
	t_heap_stats	st;
	void			*p;
	void			*q;

	a04_fresh();
	p = kmalloc_tag(10000, HEAP_FS);
	q = kmalloc_tag(300000, HEAP_BLOCK);
	heap_get_stats(&st);
	h_eq_u64("E4 FS : octets", st.bytes_live[HEAP_FS], 3 * 4096);
	h_eq_u64("E4 BLOCK : octets", st.bytes_live[HEAP_BLOCK],
		(300000 + 64 + 4095) / 4096 * 4096);
	h_eq_u64("E4 GENERIC vide", st.allocs_live[HEAP_GENERIC], 0);
	kfree(p);
	kfree(q);
	a04_drain("E4 gros blocs par etiquette");
}

int	main(void)
{
	h_begin("a04/etiquettes");
	h_run("E4 dalles separees par etiquette", tags_use_separate_slabs);
	h_run("E4 etiquette inconnue", tags_unknown_tag_is_a_kernel_bug);
	h_run("E4 gros blocs par etiquette", tags_large_blocks_are_counted_per_tag);
	return (h_end());
}
