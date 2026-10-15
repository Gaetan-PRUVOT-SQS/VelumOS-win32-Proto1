#include "blk_int.h"

static bool	part_overlap(const t_part *a, const t_part *b)
{
	return (a->first <= b->first + (b->count - 1)
		&& b->first <= a->first + (a->count - 1));
}

int	part_add(t_partlist *pl, const t_part *p)
{
	uint32_t	i;

	if (p->count == 0 || p->num == 0 || p->num > BLK_PART_NUM_MAX)
		return (E_PROTO);
	if (p->first + (p->count - 1) < p->first)
		return (E_PROTO);
	i = 0;
	while (i < pl->n)
	{
		if (part_overlap(&pl->p[i], p))
			return (E_PROTO);
		i++;
	}
	if (pl->n >= BLK_PARTS_MAX)
		return (E_NOTSUP);
	pl->p[pl->n] = *p;
	pl->n++;
	return (0);
}

static size_t	num_digits(uint32_t num, char *rev)
{
	size_t	n;

	n = 0;
	while (num)
	{
		rev[n] = (char)('0' + num % 10);
		num /= 10;
		n++;
	}
	return (n);
}

int	blk_part_name(char *out, const char *disk, uint32_t num)
{
	char	rev[12];
	size_t	len;
	size_t	nd;
	size_t	need;

	if (num == 0 || num > BLK_PART_NUM_MAX)
		return (E_INVAL);
	len = strnlen(disk, BLK_NAME_MAX);
	nd = num_digits(num, rev);
	need = len + nd;
	if (len && disk[len - 1] >= '0' && disk[len - 1] <= '9')
		need++;
	if (len == 0 || need >= BLK_NAME_MAX)
		return (E_RANGE);
	memcpy(out, disk, len);
	if (need > len + nd)
		out[len++] = 'p';
	while (nd)
		out[len++] = rev[--nd];
	out[len] = '\0';
	return (0);
}
