#ifndef STRING_H
# define STRING_H

# include <stddef.h>
# include "velum/libk.h"

char	*strdup(const char *s);
char	*strndup(const char *s, size_t max);
void	explicit_bzero(void *s, size_t n);

#endif
