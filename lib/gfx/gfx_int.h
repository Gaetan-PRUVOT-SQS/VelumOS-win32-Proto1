#ifndef GFX_INT_H
# define GFX_INT_H

# include "velum/gfx.h"
# include "velum/libk.h"
# include "gfx_px.h"

# define GFX_RADIUS_MAX 1048576
# define GFX_REGION_LIMIT 1073741823
# define GFX_SMOOTH_MAX 65536
# define GFX_COVER_SIDE 8

typedef struct s_box
{
	int64_t	x0;
	int64_t	y0;
	int64_t	x1;
	int64_t	y1;
}	t_box;

typedef struct s_dda
{
	int64_t	m;
	int64_t	q;
	int64_t	r;
	int64_t	sq;
	int64_t	sr;
}	t_dda;

typedef struct s_ramp
{
	int64_t	den;
	int64_t	from[4];
	int64_t	delta[4];
	t_dda	c[4];
}	t_ramp;

typedef struct s_job
{
	t_surface		*dst;
	const t_surface	*src;
	t_box			d;
	int64_t			ox;
	int64_t			oy;
}	t_job;

typedef struct s_line
{
	int64_t		maj0;
	int64_t		min0;
	int64_t		len;
	uint64_t	dmin;
	int64_t		sgn;
	bool		steep;
	t_box		clip;
	int64_t		i;
	int64_t		hi;
	uint64_t	q;
	uint64_t	r;
}	t_line;

typedef struct s_rrctx
{
	t_surface	*s;
	t_box		r;
	t_box		clip;
	t_color		color;
	int64_t		y;
	int64_t		rad[4];
}	t_rrctx;

typedef struct s_tap_axis
{
	int64_t		i0;
	int64_t		i1;
	uint32_t	frac;
}	t_tap_axis;

typedef void	(*t_rowfn)(uint32_t *, const uint32_t *, size_t);

static inline int64_t	gfx_max64(int64_t a, int64_t b)
{
	if (a > b)
		return (a);
	return (b);
}

static inline int64_t	gfx_min64(int64_t a, int64_t b)
{
	if (a < b)
		return (a);
	return (b);
}

static inline int64_t	gfx_abs64(int64_t a)
{
	if (a < 0)
		return (-a);
	return (a);
}

t_box		gfx_box_make(int64_t x0, int64_t y0, int64_t x1, int64_t y1);
t_box		gfx_box_of(t_rect r);
t_box		gfx_box_clip(t_box a, t_box b);
bool		gfx_box_empty(t_box b);
t_rect		gfx_box_rect(t_box b);
t_box		gfx_bounds(const t_surface *s);
t_box		gfx_clipbox(const t_surface *s);
uint32_t	*gfx_px_at(const t_surface *s, int64_t x, int64_t y);
void		gfx_span_fill(uint32_t *d, size_t n, t_color c);
void		gfx_span_copy(uint32_t *d, const uint32_t *s, size_t n);
void		gfx_span_move(uint32_t *d, const uint32_t *s, size_t n);
void		gfx_span_blend(uint32_t *d, const uint32_t *s, size_t n);
void		gfx_span_color(uint32_t *d, size_t n, t_color c);
bool		gfx_span_overlap(const uint32_t *d, const uint32_t *s, size_t n);
void		gfx_fill_box(t_surface *s, t_box b, t_color c);
void		gfx_line_walk(t_surface *s, t_line *c, t_color col);
void		gfx_dda_make(t_dda *d, int64_t m, int64_t step);
void		gfx_dda_seek(t_dda *d, int64_t num);
void		gfx_dda_next(t_dda *d);
void		gfx_ramp_init(t_ramp *rp, t_color from, t_color to, int64_t n);
void		gfx_ramp_seek(t_ramp *rp, int64_t i);
void		gfx_ramp_next(t_ramp *rp);
t_color		gfx_ramp_color(const t_ramp *rp);
bool		gfx_blit_job(t_job *j, const t_blit *b);
void		gfx_job_run(const t_job *j, t_rowfn row);
void		gfx_tap_axis(t_tap_axis *a, int64_t pos256, int64_t lo, int64_t hi);
t_color		gfx_bilinear(const uint32_t p[4], const uint32_t w[4]);
uint32_t	gfx_corner_cover(int64_t radius, int64_t u, int64_t v);
int64_t		gfx_rr_side(const t_rrctx *c, int side);
void		gfx_region_check(t_region *rg);
int			gfx_rect_split(t_rect a, t_rect cut, t_rect out[4]);
bool		gfx_rect_join(t_rect a, t_rect b, t_rect *out);
bool		gfx_region_cut(t_region *rg, t_rect cut);
void		gfx_region_merge(t_region *rg);

#endif
