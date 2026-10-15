#ifndef STDLIB_H
# define STDLIB_H

# include <stddef.h>
# include "velum/cnames/bsearch_decl.h"

# define EXIT_SUCCESS 0
# define EXIT_FAILURE 1
# define RAND_MAX 2147483647
# define ATEXIT_MAX 32

void				*malloc(size_t size);
void				free(void *ptr);
void				*calloc(size_t nmemb, size_t size);
void				*realloc(void *ptr, size_t size);
void				*reallocarray(void *ptr, size_t nmemb, size_t size);
int					abs(int n);
long				labs(long n);
long long			llabs(long long n);
int					atoi(const char *s);
long				atol(const char *s);
long				strtol(const char *s, char **end, int base);
long long			strtoll(const char *s, char **end, int base);
unsigned long		strtoul(const char *s, char **end, int base);
unsigned long long	strtoull(const char *s, char **end, int base);
void				qsort(void *base, size_t nmemb, size_t size,
						int (*cmp)(const void *, const void *));
int					rand(void);
void				srand(unsigned int seed);
int					atexit(void (*fn)(void));
_Noreturn void		exit(int status);
_Noreturn void		abort(void);

#endif
