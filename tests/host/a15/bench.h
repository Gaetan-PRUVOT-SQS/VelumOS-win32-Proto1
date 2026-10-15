#ifndef BENCH_H
# define BENCH_H

# include <stddef.h>
# include <stdint.h>
# include "velum/gfx.h"

# define BENCH_W 1024
# define BENCH_H_PX 768
# define BENCH_REPS 150

typedef struct s_bench
{
	t_surface	dst;
	t_surface	src;
	t_surface	half;
	uint32_t	*mem;
}	t_bench;

typedef void	(*t_benchfn)(t_bench *);

typedef struct s_bcase
{
	const char	*name;
	t_benchfn	fn;
}	t_bcase;

uint64_t	bench_now(void);
void		bench_run(const t_bcase *c, t_bench *b);
void		bench_open(t_bench *b);
void		bench_close(t_bench *b);
void		op_fill(t_bench *b);
void		op_fill_alpha(t_bench *b);
void		op_blit(t_bench *b);
void		op_blit_alpha(t_bench *b);
void		op_scaled(t_bench *b);
void		op_smooth(t_bench *b);
void		op_gradient_v(t_bench *b);
void		op_gradient_h(t_bench *b);
void		op_rrect(t_bench *b);
void		op_scroll(t_bench *b);
void		op_lines(t_bench *b);
void		op_fill_rep(t_bench *b);
void		op_fill_loop(t_bench *b);
void		op_copy_rep(t_bench *b);
void		op_copy_memcpy(t_bench *b);
void		op_copy_loop(t_bench *b);

#endif
