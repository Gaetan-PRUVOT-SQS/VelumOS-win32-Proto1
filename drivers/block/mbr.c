#include "blk_int.h"

static bool	mbr_looks_valid(const uint8_t *sec)
{
	int		i;
	uint8_t	boot;

	if (sec[MBR_SIG_OFF] != 0x55 || sec[MBR_SIG_OFF + 1] != 0xaa)
		return (false);
	i = 0;
	while (i < MBR_ENTRIES)
	{
		boot = sec[MBR_TABLE_OFF + i * MBR_ENTRY_SIZE];
		if (boot != 0x00 && boot != 0x80)
			return (false);
		i++;
	}
	return (true);
}

static int	mbr_entry(const uint8_t *e, uint64_t nsectors, uint32_t num,
	t_partlist *pl)
{
	t_part	p;

	if (e[4] == 0)
		return (0);
	if (ebr_is_link(e[4]))
		return (ebr_note(e, nsectors, pl));
	p.first = rd_le32(e + 8);
	p.count = rd_le32(e + 12);
	p.num = num;
	if (p.count == 0)
		return (0);
	if (p.first == 0 || p.first >= nsectors || p.count > nsectors - p.first)
		return (E_PROTO);
	return (part_add(pl, &p));
}

static bool	mbr_protective(const uint8_t *sec)
{
	int	i;

	i = 0;
	while (i < MBR_ENTRIES)
	{
		if (sec[MBR_TABLE_OFF + i * MBR_ENTRY_SIZE + 4] == MBR_TYPE_GPT)
			return (true);
		i++;
	}
	return (false);
}

static void	mbr_fence_extended(t_partlist *pl)
{
	uint32_t	i;

	i = 0;
	while (pl->ext_count && i < pl->n)
	{
		if (pl->ext_first <= pl->p[i].first + (pl->p[i].count - 1)
			&& pl->p[i].first <= pl->ext_first + (pl->ext_count - 1))
		{
			pl->ext_count = 0;
			pl->ext_overlap = true;
		}
		i++;
	}
}

int	mbr_parse(const uint8_t *sec, uint64_t nsectors, t_partlist *pl)
{
	int	i;
	int	rc;

	memset(pl, 0, sizeof(*pl));
	if (!mbr_looks_valid(sec))
		return (0);
	if (mbr_protective(sec))
	{
		pl->gpt = true;
		return (0);
	}
	i = 0;
	while (i < MBR_ENTRIES)
	{
		rc = mbr_entry(sec + MBR_TABLE_OFF + i * MBR_ENTRY_SIZE, nsectors,
				(uint32_t)i + 1, pl);
		if (rc < 0)
		{
			pl->n = 0;
			return (rc);
		}
		i++;
	}
	mbr_fence_extended(pl);
	return (0);
}
