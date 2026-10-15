#ifndef FX_H
# define FX_H

# include "velum/apk/arsc.h"
# include "velum/err.h"

# define FX_POOL 0
# define FX_AXML 1
# define FX_ARSC 2

t_span		fx_load(const char *name);
t_span		fx_dup(t_span s, size_t len);
void		fx_free(t_span s);
void		fx_poke(t_span s, size_t off, uint8_t v);
uint32_t	fx_rnd(uint32_t *seed);
int			fx_walk(int kind, t_span s);
int			fx_walk_axml(t_span s);

#endif
