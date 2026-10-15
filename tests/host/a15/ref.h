#ifndef REF_H
# define REF_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "velum/gfx.h"

# define GUARD_WORDS 64
# define GUARD_CANARY 0xc0ffee00u
# define CLIP_VARIANTS 8
# define RGN_G 16
# define FX_KINDS 10

typedef struct s_guard
{
	uint32_t	*raw;
	uint32_t	*px;
	size_t		count;
}	t_guard;

typedef struct s_scene
{
	t_guard		g;
	t_surface	s;
	uint32_t	*exp;
	int32_t		w;
	int32_t		h;
	t_rect		cv;
}	t_scene;

typedef struct s_cstat
{
	int64_t		area;
	int64_t		abs_err;
	int64_t		max_err;
}	t_cstat;

typedef struct s_pair
{
	t_rect		dr;
	t_rect		sr;
	int			mode;
}	t_pair;

typedef struct s_axis
{
	int64_t		i0;
	int64_t		i1;
	int64_t		frac;
}	t_axis;

typedef struct s_rgstat
{
	int64_t		steps;
	int64_t		exact;
	int64_t		fallback;
	uint32_t	max_n;
}	t_rgstat;

typedef struct s_fx
{
	t_guard		g;
	uint32_t	*snap;
	t_surface	view;
	t_rect		cv;
	int32_t		ww;
	int32_t		wh;
	int32_t		ox;
	int32_t		oy;
	bool		valid;
}	t_fx;

typedef struct s_grid
{
	int32_t		w;
	int32_t		h;
	uint8_t		*on;
}	t_grid;

void		rng_seed(uint64_t seed);
uint64_t	rng_next(void);
int64_t		rng_range(int64_t lo, int64_t hi);
int32_t		rng_coord(int32_t span);
t_color		rng_color(void);
int32_t		rng_size(int32_t span);
t_rect		rng_rect(int32_t span);
t_point		rng_point(int32_t span);
t_point		pt(int32_t x, int32_t y);
bool		in_rect(t_rect r, int64_t x, int64_t y);
bool		in_view(const t_surface *s, int64_t x, int64_t y);
size_t		img_diff(const uint32_t *a, const uint32_t *b, size_t n);
void		img_pattern(uint32_t *p, size_t n, uint32_t seed);
t_rect		clip_variant(int k, int32_t w, int32_t h);
void		check_empty(const char *what, const t_surface *s);
void		scene_open(t_scene *sc, int32_t w, int32_t h, int clip);
bool		scene_in(const t_scene *sc, int64_t x, int64_t y);
void		scene_blend(t_scene *sc, int64_t x, int64_t y, t_color c);
void		scene_rect(t_scene *sc, t_rect r, t_color c);
void		scene_grid(t_scene *sc, const t_grid *g, t_color c);
void		scene_exact_line(t_scene *sc, t_point a, t_point b, t_color c);
void		scene_gradient(t_scene *sc, const t_gradient *g);
void		scene_clear(t_scene *sc, t_color c);
t_color		scene_px(const t_scene *sc, int32_t x, int32_t y);
void		scene_adopt(t_scene *sc, t_rect r);
void		scene_set(t_scene *sc, int64_t x, int64_t y, t_color c);
void		scene_blit(t_scene *sc, const t_surface *src, t_pair p);
uint32_t	*scene_snapshot(const t_scene *sc);
void		scene_scroll(t_scene *sc, t_rect r, t_point delta);
void		scale_case(int32_t dw, int32_t sw, int clip, t_pair p);
void		run_smooth(t_scene *dst, t_scene *src, t_rect dr, t_rect sr);
void		scene_scaled(t_scene *sc, const t_surface *src, t_pair p);
void		scene_smooth(t_scene *sc, const t_surface *src, t_pair p);
int64_t		ref_floor_div(__int128 a, __int128 b);
int64_t		ref_clamp(int64_t v, int64_t lo, int64_t hi);
t_axis		ref_axis(int64_t i, int64_t sw, int64_t dw);
int			rg_paint(uint8_t cells[RGN_G * RGN_G], const t_region *rg);
void		rg_model(uint8_t cells[RGN_G * RGN_G], int op, t_rect r);
void		rg_apply(t_region *rg, int op, t_rect r);
bool		rg_joinable(t_rect a, t_rect b);
bool		rg_valid(const t_region *rg);
bool		rg_covers(const t_region *rg, int64_t x, int64_t y);
bool		rg_superset(const uint8_t got[RGN_G * RGN_G],
				const uint8_t want[RGN_G * RGN_G]);
void		rg_sequences(int count, int length, t_rgstat *st);
t_rect		rg_cells_bounds(const uint8_t cells[RGN_G * RGN_G]);
void		rg_step(t_region *rg, int op, t_rect r, t_rgstat *st);
void		fx_open(t_fx *fx, int kind);
void		fx_close(t_fx *fx);
bool		fx_allowed(const t_fx *fx, int64_t wx, int64_t wy);
void		fx_clip(t_fx *fx);
void		fx_verify(t_fx *fx, const char *what);
void		fuzz_op(t_fx *dst, t_fx *src, int op);
t_gradient	grad_make(t_rect r, t_color from, t_color to, bool horizontal);
void		scene_copy_in(t_scene *a, const t_scene *b);
void		scene_close(t_scene *sc, const char *what);
void		guard_new(t_guard *g, size_t count);
bool		guard_ok(const t_guard *g);
void		guard_free(t_guard *g);
void		guard_randomize(t_guard *g);
uint32_t	ref_div255(uint64_t x);
t_color		ref_blend(t_color dst, t_color src);
t_color		ref_spread(uint32_t a, uint32_t v, uint32_t mul);
uint32_t	ref_cover(int64_t radius, int64_t u, int64_t v);
t_cstat		corner_stat(const t_scene *sc, t_rect r, int corner, int32_t rad);
int32_t		ref_cap(int32_t radius, t_rect r);
bool		hard_inside(t_rect r, const int32_t rad[4], int32_t x, int32_t y);
int64_t		ref_ramp(int64_t from, int64_t to, int64_t n, int64_t i);
void		ref_line(t_grid *g, t_point a, t_point b);
bool		ref_axes(t_point a, t_point b, int64_t p[2], int64_t q[2]);
bool		ref_on_line(t_point a, t_point b, int64_t x, int64_t y);
t_grid		*grid_new(int32_t w, int32_t h);
void		grid_free(t_grid *g);
bool		grid_get(const t_grid *g, int32_t x, int32_t y);
void		grid_set(t_grid *g, int32_t x, int32_t y, bool v);

#endif
