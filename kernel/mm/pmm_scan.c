#include "pmm_int.h"

static uint64_t	run_from(const t_pmm_req *q, uint64_t from, uint64_t limit)
{
	uint64_t	f;
	uint64_t	c;
	uint64_t	u;
	uint64_t	known;

	while (from < limit)
	{
		f = pmm_bits_next(from, q->hi, 0);
		c = align_up(f, q->align);
		if (c >= limit)
			return (PMM_NONE);
		known = c;
		if (c == f)
			known = c + 1;
		u = pmm_bits_next(known, c + q->count, 1);
		if (u == c + q->count)
			return (c);
		from = u + 1;
	}
	return (PMM_NONE);
}

uint64_t	pmm_find_run(const t_pmm_req *q)
{
	uint64_t	limit;
	uint64_t	found;

	if (q->lo >= q->hi || q->count > q->hi - q->lo)
		return (PMM_NONE);
	limit = q->hi - q->count + 1;
	found = run_from(q, q->start, limit);
	if (found == PMM_NONE && q->start > q->lo)
		found = run_from(q, q->lo, min_u64(q->start, limit));
	return (found);
}
