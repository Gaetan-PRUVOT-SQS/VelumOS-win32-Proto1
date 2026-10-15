#ifndef HARNESS_H
# define HARNESS_H

# include <stdint.h>

typedef struct s_hstate
{
	const char	*suite;
	const char	*name;
	int			fail;
	int			checks;
}	t_hstate;

extern t_hstate	g_h;

void	h_begin(const char *suite);
void	h_run(const char *name, void (*fn)(void));
void	h_true(int cond, const char *what);
int		h_end(void);
void	h_eq_i64(const char *what, int64_t got, int64_t want);
void	h_eq_u64(const char *what, uint64_t got, uint64_t want);
void	h_eq_str(const char *what, const char *got, const char *want);

#endif
