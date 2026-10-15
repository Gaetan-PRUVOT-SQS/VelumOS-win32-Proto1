#ifndef STRTO_INT_H
# define STRTO_INT_H

# include <stdint.h>

typedef struct s_strto
{
	uint64_t	mag;
	const char	*end;
	int			neg;
	int			overflow;
	int			status;
}	t_strto;

void	strto_parse(const char *s, int base, t_strto *r);

#endif
