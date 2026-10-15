#ifndef FAKE_H
# define FAKE_H

# include <stddef.h>
# include <stdint.h>
# include "velum/apk/dex.h"

t_span		fake_fixture(void);
uint8_t		*fake_copy(t_span s, size_t n);
void		fake_seal(uint8_t *p, size_t n);
int			fake_patched(size_t off, uint32_t v, uint32_t width);
uint32_t	fake_rand(uint32_t *state);
uint32_t	fake_walk(const t_dex *d);
int			fake_same(const char *a, const char *b);
int			fake_named(const t_dex *d, uint32_t string_idx, const char *want);
int			fake_string_idx(const t_dex *d, const char *want);

#endif
