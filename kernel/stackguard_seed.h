#ifndef STACKGUARD_SEED_H
# define STACKGUARD_SEED_H

# include <stdbool.h>
# include <stdint.h>

# define GUARD_RDRAND_TRIES 10
# define GUARD_RDSEED_TRIES 64
# define GUARD_MIX_MULT_A 0xbf58476d1ce4e5b9ULL
# define GUARD_MIX_MULT_B 0x94d049bb133111ebULL
# define GUARD_FALLBACK 0x595e9fbd94fda700ULL

typedef enum e_guardsrc
{
	GUARD_SRC_TSC = 0,
	GUARD_SRC_RDSEED,
	GUARD_SRC_RDRAND,
	GUARD_SRC_BOTH
}	t_guardsrc;

typedef struct s_guardhw
{
	bool		has_rdrand;
	bool		has_rdseed;
	bool		(*rdrand)(uint64_t *out);
	bool		(*rdseed)(uint64_t *out);
	uint64_t	(*tsc)(void);
}	t_guardhw;

static inline bool	guard_value_usable(uint64_t v)
{
	return (v != 0 && v != ~0ULL);
}

static inline bool	guard_draw(bool (*fn)(uint64_t *), unsigned int tries,
	uint64_t *out)
{
	unsigned int	i;
	uint64_t		v;

	i = 0;
	while (i < tries)
	{
		v = 0;
		if (fn(&v) && guard_value_usable(v))
		{
			*out = v;
			return (true);
		}
		i++;
	}
	return (false);
}

static inline uint64_t	guard_rotl32(uint64_t v)
{
	return ((v << 32) | (v >> 32));
}

static inline uint64_t	guard_finalize(uint64_t z)
{
	z ^= z >> 30;
	z *= GUARD_MIX_MULT_A;
	z ^= z >> 27;
	z *= GUARD_MIX_MULT_B;
	z ^= z >> 31;
	return (z);
}

static inline uint64_t	guard_shape(uint64_t z)
{
	z &= ~0xffULL;
	if (z == 0)
		z = GUARD_FALLBACK;
	return (z);
}

static inline uint64_t	guard_hard(const t_guardhw *hw, t_guardsrc *src)
{
	uint64_t	seed;
	uint64_t	rnd;
	bool		got_seed;
	bool		got_rnd;

	seed = 0;
	rnd = 0;
	got_seed = hw->has_rdseed && guard_draw(hw->rdseed, GUARD_RDSEED_TRIES,
			&seed);
	got_rnd = hw->has_rdrand && guard_draw(hw->rdrand, GUARD_RDRAND_TRIES,
			&rnd);
	*src = GUARD_SRC_TSC;
	if (got_seed && got_rnd && seed != rnd)
	{
		*src = GUARD_SRC_BOTH;
		return (seed ^ rnd);
	}
	if (got_seed)
	{
		*src = GUARD_SRC_RDSEED;
		return (seed);
	}
	if (got_rnd)
	{
		*src = GUARD_SRC_RDRAND;
		return (rnd);
	}
	return (0);
}

static inline uint64_t	guard_seed(const t_guardhw *hw, t_guardsrc *src)
{
	uint64_t	first;
	uint64_t	second;
	uint64_t	hard;

	first = hw->tsc();
	hard = guard_hard(hw, src);
	second = hw->tsc();
	return (guard_shape(guard_finalize(hard ^ first ^ guard_rotl32(second))));
}

#endif
