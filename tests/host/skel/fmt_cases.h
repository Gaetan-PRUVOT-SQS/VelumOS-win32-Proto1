#ifndef FMT_CASES_H
# define FMT_CASES_H

# include <stddef.h>
# include <stdint.h>

# define SP15 "               "
# define SP14 "              "
# define ZE15 "000000000000000"
# define ZE14 "00000000000000"
# define ALT_CASES 24
# define BUDGET_APPELS 1000
# define BUDGET_SECONDES 2

typedef struct s_altcase
{
	const char		*fmt;
	unsigned int	val;
	const char		*want;
}	t_altcase;

typedef struct s_bigcase
{
	const char	*fmt;
	int			arg;
	int64_t		len;
	const char	*want;
}	t_bigcase;

#endif
