#include "cpio.h"
#include "velum/err.h"
#include "velum/heap.h"
#include "velum/libk.h"

static int	index_count(const uint8_t *b, uint64_t size, uint32_t *n)
{
	uint64_t	off;
	t_cpent		e;
	int			rc;

	off = 0;
	*n = 0;
	rc = cpio_next(b, size, &off, &e);
	while (rc > 0)
	{
		if (++*n > CPIO_MAX_ENTRIES)
			return (E_RANGE);
		rc = cpio_next(b, size, &off, &e);
	}
	return (rc);
}

static void	index_fill(const uint8_t *b, uint64_t size, t_initrd *rd)
{
	uint64_t	off;
	t_cpent		e;
	uint32_t	order;

	off = 0;
	order = 0;
	while (cpio_next(b, size, &off, &e) > 0)
	{
		e.order = order++;
		if (cpio_name_norm(&e) == 1)
			rd->ents[rd->n++] = e;
		else
			rd->skipped++;
	}
}

int	cpio_index(const uint8_t *base, uint64_t size, t_initrd *rd)
{
	uint32_t	n;
	int			rc;

	rd->ents = NULL;
	rd->n = 0;
	rd->skipped = 0;
	if (size == 0)
		return (0);
	if (base == NULL)
		return (E_INVAL);
	rc = index_count(base, size, &n);
	if (rc < 0)
		return (rc);
	if (n == 0)
		return (0);
	rd->ents = kmalloc_tag((size_t)n * sizeof(t_cpent), HEAP_FS);
	if (rd->ents == NULL)
		return (E_NOMEM);
	index_fill(base, size, rd);
	cpio_sort(rd->ents, rd->n);
	return (0);
}
