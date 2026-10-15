#ifndef RANDOM_H
# define RANDOM_H

# include <stddef.h>
# include <stdint.h>

int			random_boot_init(void);
void		krandom(void *buf, size_t n);
uint64_t	krandom_u64(void);
uint64_t	krandom_below(uint64_t bound);
void		random_add_entropy(const void *data, size_t n);
int			random_selftest(void);

#endif
