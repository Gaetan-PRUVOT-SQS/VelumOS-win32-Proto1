#ifndef SORT_INT_H
# define SORT_INT_H

# include <stddef.h>

typedef int	(*t_cmpfn)(const void *, const void *);

typedef struct s_sortctx
{
	unsigned char	*base;
	size_t			size;
	t_cmpfn			cmp;
}	t_sortctx;

typedef struct s_bsearch
{
	const void	*key;
	const void	*base;
	size_t		nmemb;
	size_t		size;
	t_cmpfn		cmp;
}	t_bsearch;

void	*vbsearch_impl(const t_bsearch *args);

#endif
