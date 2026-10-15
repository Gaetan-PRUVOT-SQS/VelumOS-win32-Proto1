#include "blk_int.h"

bool	ebr_is_link(uint8_t type)
{
	return (type == 0x05 || type == 0x0f || type == 0x85);
}

int	ebr_note(const uint8_t *e, uint64_t nsectors, t_partlist *pl)
{
	uint64_t	first;
	uint64_t	count;

	pl->extended++;
	first = rd_le32(e + 8);
	count = rd_le32(e + 12);
	if (pl->extended > 1 || first == 0 || first >= nsectors
		|| count == 0 || count > nsectors - first)
		return (0);
	pl->ext_first = first;
	pl->ext_count = count;
	return (0);
}

static int	ebr_logical(const uint8_t *e, t_ebr *c, t_partlist *pl)
{
	t_part		p;
	uint64_t	end;
	uint64_t	rel;
	int			rc;

	c->min_next = c->cur + 1;
	p.count = rd_le32(e + 12);
	if (e[4] == 0 || p.count == 0)
		return (0);
	end = c->ext_first + c->ext_count;
	rel = rd_le32(e + 8);
	if (ebr_is_link(e[4]) || rel == 0 || rel >= end - c->cur)
		return (E_PROTO);
	p.first = c->cur + rel;
	p.num = c->num;
	if (p.count > end - p.first)
		return (E_PROTO);
	rc = part_add(pl, &p);
	if (rc < 0)
		return (rc);
	c->num++;
	c->min_next = p.first + p.count;
	return (0);
}

static int	ebr_link(const uint8_t *e, t_ebr *c)
{
	uint64_t	rel;

	if (!ebr_is_link(e[4]))
		return (0);
	rel = rd_le32(e + 8);
	if (rel >= c->ext_count || c->ext_first + rel < c->min_next)
		return (E_PROTO);
	c->cur = c->ext_first + rel;
	return (1);
}

int	ebr_step(const uint8_t *sec, t_ebr *c, t_partlist *pl)
{
	int	rc;

	if (sec[MBR_SIG_OFF] != 0x55 || sec[MBR_SIG_OFF + 1] != 0xaa)
		return (E_PROTO);
	rc = ebr_logical(sec + MBR_TABLE_OFF, c, pl);
	if (rc < 0)
		return (rc);
	return (ebr_link(sec + MBR_TABLE_OFF + MBR_ENTRY_SIZE, c));
}
