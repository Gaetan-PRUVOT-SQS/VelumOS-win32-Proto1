#ifndef FAKE_F3_H
# define FAKE_F3_H

# include <stddef.h>
# include <stdint.h>

# define F3_SENTINEL 4242

typedef struct s_f3_strtol
{
	const char	*in;
	int			base;
	long long	want;
	int			end_off;
	int			want_errno;
}	t_f3_strtol;

typedef struct s_f3_strtoul
{
	const char			*in;
	int					base;
	unsigned long long	want;
	int					end_off;
	int					want_errno;
}	t_f3_strtoul;

typedef struct s_f3_ctype
{
	const char	*name;
	int			(*fn)(int);
	const char	*ref;
	int			lo;
	int			hi;
	int			extra;
}	t_f3_ctype;

void		f3_strtol_run(const t_f3_strtol *c, size_t n, int ll);
void		f3_strtoul_run(const t_f3_strtoul *c, size_t n, int ull);
void		f3_ctype_build(const t_f3_ctype *d, uint8_t *exp);
int			f3_cmp_int(const void *a, const void *b);
int			f3_cmp_int_desc(const void *a, const void *b);
int			f3_cmp_counting(const void *a, const void *b);
uint64_t	f3_cmp_calls(void);
void		f3_cmp_reset(void);
int			f3_cmp_b1(const void *a, const void *b);
int			f3_cmp_b3(const void *a, const void *b);
int			f3_cmp_b8(const void *a, const void *b);
int			f3_cmp_b24(const void *a, const void *b);
int			f3_is_sorted(const void *base, size_t n, size_t size,
				int (*cmp)(const void *, const void *));
void		f3_hash(const void *base, size_t n, size_t size, uint64_t *out);
void		f3_fill(void *base, size_t bytes, uint64_t *rng);
int			f3_survives(void (*fn)(void *), void *arg);
void		f3_pattern(int *v, size_t n, int mode, uint64_t seed);
int			f3_same_ints(const int *a, const int *b, size_t n);
void		f3_key_expect(const void *key);
int			f3_fmt(char *buf, size_t size, const char *fmt, ...);
int			f3_cmp_keyed(const void *a, const void *b);
uint64_t	f3_key_violations(void);

#endif
