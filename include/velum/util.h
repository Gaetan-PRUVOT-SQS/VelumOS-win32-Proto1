#ifndef UTIL_H
# define UTIL_H

# include <stddef.h>
# include <stdint.h>

# define PAGE_SIZE 4096
# define PAGE_SHIFT 12

static inline uint64_t	align_up(uint64_t v, uint64_t a)
{
	return ((v + a - 1) & ~(a - 1));
}

static inline uint64_t	align_down(uint64_t v, uint64_t a)
{
	return (v & ~(a - 1));
}

static inline int	is_aligned(uint64_t v, uint64_t a)
{
	return ((v & (a - 1)) == 0);
}

static inline uint64_t	min_u64(uint64_t a, uint64_t b)
{
	if (a < b)
		return (a);
	return (b);
}

static inline uint64_t	max_u64(uint64_t a, uint64_t b)
{
	if (a > b)
		return (a);
	return (b);
}

#endif
