#include <string.h>
#include "a07_fake.h"
#include "harness.h"

void	gen_set_dyn(t_gen *g, int i, int64_t tag, uint64_t val)
{
	t_elf64_dyn	d;

	d.tag = tag;
	d.val = val;
	memcpy(g->buf + GEN_DYN_OFF + i * sizeof(d), &d, sizeof(d));
}

void	gen_set_rela(t_gen *g, int i, uint64_t off, uint64_t info)
{
	t_elf64_rela	r;

	r.offset = off;
	r.info = info;
	r.addend = 0x100 + 0x1000 * i;
	memcpy(g->buf + GEN_RELA_OFF + i * sizeof(r), &r, sizeof(r));
}

void	gen_patch(t_gen *g, uint32_t off, uint32_t len, uint64_t val)
{
	uint32_t	i;

	i = 0;
	while (i < len && off + i < GEN_SIZE)
	{
		g->buf[off + i] = (uint8_t)(val >> (8 * i));
		i++;
	}
}

void	gen_run(const t_patch *t, uint32_t n, uint16_t type)
{
	t_gen		g;
	t_elfinfo	info;
	uint32_t	i;
	uint32_t	k;

	i = 0;
	while (i < n)
	{
		gen_valid(&g, type);
		k = 0;
		while (k < PATCH_MAX)
		{
			gen_patch(&g, t[i].off[k], t[i].len[k], t[i].val[k]);
			k++;
		}
		h_eq_i64(t[i].name, gen_check(&g, &info), t[i].want);
		i++;
	}
}
