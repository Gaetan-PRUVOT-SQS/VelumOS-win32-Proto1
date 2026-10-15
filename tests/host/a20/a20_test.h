#ifndef A20_TEST_H
# define A20_TEST_H

# include <stddef.h>
# include <stdint.h>

# define NS_SEC 1000000000ull
# define V_SALT_A "000102030405060708090a0b0c0d0e0f"
# define V_SALT_B "a1b2c3d4e5f60718"
# define V_SALT_C1 "a5a5a5a5a5a5a5a5a5a5a5a5a5a5a5a5"
# define V_SALT_C2 "a5a5a5a5a5a5a5a5a5a5a5a5a5a5a5a5"
# define V_SALT_D "0f0e0d0c0b0a09080706050403020100"
# define V_SALT_E "1111111111111111111111111111111111111111"
# define V_SALT_N "2222222222222222"
# define V_HA1 "e418c26f08c4729d239abd46eb0b9655"
# define V_HA2 "4467e23dfb06f2ae503f27e8793af0c0"
# define V_HB1 "3563762196466d34da637d9504efa61e"
# define V_HB2 "0c460ccc2551632d83949eb8722c3415"
# define V_HC1 "409b28f29e23bafbe0571762e611b5d1"
# define V_HC2 "631ee7ecd2e391d64b5fd00f41323825"
# define V_HD1 "4e70affdfe26f8006da6182a1fe2026c"
# define V_HD2 "42386401bc635416db3978d5b12027d1"
# define V_HE1 "1f5effc58bec1291e8d7d53ede45340d"
# define V_HE2 "8a3c1177e3485b5b963076254a912f40"
# define V_HN1 "360caa24efe843fb5de1c774e8b6be78"
# define V_HN2 "b60320417a9f2a6715f11026d833348c"
# define V_HS1 "389de76e65b8d69933697614bd43dde7"
# define V_HS2 "8594e8cd52fbca7933d0ca8b17e75f25"

static inline uint32_t	next_rand(uint32_t *s)
{
	*s ^= *s << 13;
	*s ^= *s >> 17;
	*s ^= *s << 5;
	return (*s);
}

typedef struct s_clockrow
{
	uint64_t	ns;
	const char	*want;
}	t_clockrow;

typedef struct s_linerow
{
	const char	*line;
	int			want;
	const char	*what;
}	t_linerow;

typedef struct s_utfrow
{
	const char	*s;
	size_t		len;
	int			want;
}	t_utfrow;

typedef struct s_runrow
{
	const char	*in;
	int			want;
	const char	*path;
}	t_runrow;

typedef struct s_pwrow
{
	const char	*pw;
	size_t		len;
	int			want;
	const char	*what;
}	t_pwrow;

#endif
