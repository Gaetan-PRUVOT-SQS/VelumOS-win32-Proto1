#ifndef BSEARCH_DECL_H
# define BSEARCH_DECL_H

# include <stddef.h>

void	*bsearch(const void *key, const void *base, size_t nmemb, size_t size,
			int (*cmp)(const void *, const void *));

#endif
