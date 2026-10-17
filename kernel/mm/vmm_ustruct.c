#include "velum/err.h"
#include "velum/libk.h"
#include "vmm_int.h"

#define TAIL_CHUNK 64u

static int	ustruct_claimed(t_uptr src, size_t min_size, size_t *claimed)
{
	uint32_t	size;
	uint64_t	end;
	int			rc;

	rc = copy_from_user(&size, src, sizeof(size));
	if (rc < 0)
		return (rc);
	if (size > UPTR_STRUCT_MAX)
		return (UPTR_E2BIG);
	if (size < min_size)
		return (E_INVAL);
	if (__builtin_add_overflow(src, size, &end))
		return (E_FAULT);
	*claimed = size;
	return (0);
}

static int	ustruct_tail_zero(t_uptr src, size_t from, size_t to)
{
	uint8_t	chunk[TAIL_CHUNK];
	size_t	n;
	size_t	i;
	int		rc;

	while (from < to)
	{
		n = min_u64(to - from, sizeof(chunk));
		rc = copy_from_user(chunk, src + from, n);
		if (rc < 0)
			return (rc);
		i = 0;
		while (i < n)
		{
			if (chunk[i++])
				return (UPTR_E2BIG);
		}
		from += n;
	}
	return (0);
}

static int	ustruct_fill(void *dst, size_t dst_size, size_t min_size,
				t_uptr src)
{
	size_t		claimed;
	size_t		known;
	uint32_t	retained;
	int			rc;

	rc = ustruct_claimed(src, min_size, &claimed);
	if (rc < 0)
		return (rc);
	known = min_u64(claimed, dst_size);
	rc = copy_from_user(dst, src, known);
	if (rc < 0)
		return (rc);
	if (claimed > dst_size)
		rc = ustruct_tail_zero(src, dst_size, claimed);
	if (rc < 0)
		return (rc);
	memset((uint8_t *)dst + known, 0, dst_size - known);
	retained = (uint32_t)known;
	memcpy(dst, &retained, sizeof(retained));
	return (0);
}

int	copy_struct_from_user(void *dst, size_t dst_size, size_t min_size,
		t_uptr src)
{
	int	rc;

	if (!dst)
		return (E_FAULT);
	if (dst_size > UPTR_STRUCT_MAX)
		return (E_INVAL);
	if (dst_size < UPTR_SIZE_FIELD || min_size < UPTR_SIZE_FIELD
		|| min_size > dst_size)
		rc = E_INVAL;
	else
		rc = ustruct_fill(dst, dst_size, min_size, src);
	if (rc < 0)
		memset(dst, 0, dst_size);
	return (rc);
}
