#include "blk_int.h"

#define GPT_GUID_SIZE 16
#define GPT_E_FIRST 32
#define GPT_E_LAST 40

static bool	guid_is_zero(const uint8_t *g)
{
	int	i;

	i = 0;
	while (i < GPT_GUID_SIZE)
	{
		if (g[i])
			return (false);
		i++;
	}
	return (true);
}

static int	gpt_entry(const uint8_t *e, const t_gpthdr *h, uint32_t num,
	t_partlist *pl)
{
	t_part		p;
	uint64_t	last;

	if (guid_is_zero(e))
		return (0);
	p.first = rd_le64(e + GPT_E_FIRST);
	last = rd_le64(e + GPT_E_LAST);
	if (p.first > last || p.first < h->first_usable
		|| last > h->last_usable)
		return (E_PROTO);
	p.count = last - p.first + 1;
	p.num = num;
	return (part_add(pl, &p));
}

int	gpt_entries(const uint8_t *a, const t_gpthdr *h, t_partlist *pl)
{
	uint32_t	i;
	int			rc;

	memset(pl, 0, sizeof(*pl));
	pl->gpt = true;
	if (crc32_calc(a, h->array_bytes) != h->array_crc)
		return (E_PROTO);
	i = 0;
	while (i < h->nentries)
	{
		rc = gpt_entry(a + (size_t)i * h->entry_size, h, i + 1, pl);
		if (rc < 0)
		{
			pl->n = 0;
			return (rc);
		}
		i++;
	}
	return (0);
}
