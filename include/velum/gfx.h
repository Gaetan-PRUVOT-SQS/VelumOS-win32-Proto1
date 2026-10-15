#ifndef GFX_H
# define GFX_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>

# define GFX_REGION_MAX 32

typedef uint32_t	t_color;

typedef struct s_point
{
	int32_t	x;
	int32_t	y;
}	t_point;

typedef struct s_rect
{
	int32_t	x;
	int32_t	y;
	int32_t	w;
	int32_t	h;
}	t_rect;

typedef struct s_surface
{
	uint32_t	*px;
	int32_t		w;
	int32_t		h;
	int32_t		stride;
	t_rect		clip;
}	t_surface;

typedef struct s_region
{
	t_rect		r[GFX_REGION_MAX];
	uint32_t	n;
}	t_region;

typedef struct s_gradient
{
	t_rect	r;
	t_color	from;
	t_color	to;
	bool	horizontal;
}	t_gradient;

typedef struct s_blit
{
	t_surface		*dst;
	const t_surface	*src;
	t_rect			dr;
	t_rect			sr;
}	t_blit;

typedef struct s_rrect
{
	t_rect	r;
	t_color	color;
	int32_t	tl;
	int32_t	tr;
	int32_t	br;
	int32_t	bl;
}	t_rrect;

t_color		gfx_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b);
t_color		gfx_blend(t_color dst, t_color src);
t_color		gfx_lerp(t_color a, t_color b, uint32_t t256);
t_rect		rect_make(int32_t x, int32_t y, int32_t w, int32_t h);
t_rect		rect_intersect(t_rect a, t_rect b);
t_rect		rect_union(t_rect a, t_rect b);
bool		rect_empty(t_rect r);
bool		rect_contains(t_rect r, t_point p);
void		gfx_surface_init(t_surface *s, uint32_t *px, int32_t w, int32_t h);
t_surface	gfx_surface_sub(const t_surface *s, t_rect r);
void		gfx_set_clip(t_surface *s, t_rect r);
void		gfx_put(t_surface *s, t_point p, t_color c);
t_color		gfx_get(const t_surface *s, t_point p);
void		gfx_fill(t_surface *s, t_rect r, t_color c);
void		gfx_hline(t_surface *s, t_point p, int32_t len, t_color c);
void		gfx_vline(t_surface *s, t_point p, int32_t len, t_color c);
void		gfx_line(t_surface *s, t_point a, t_point b, t_color c);
void		gfx_frame(t_surface *s, t_rect r, t_color c);
void		gfx_gradient(t_surface *s, const t_gradient *g);
void		gfx_rrect_fill(t_surface *s, const t_rrect *rr);
void		gfx_blit(const t_blit *b);
void		gfx_blit_alpha(const t_blit *b);
void		gfx_blit_scaled(const t_blit *b);
void		gfx_scroll(t_surface *s, t_rect r, t_point delta);
void		region_clear(t_region *rg);
void		region_add(t_region *rg, t_rect r);
void		region_subtract(t_region *rg, t_rect r);
void		region_intersect(t_region *rg, t_rect r);
t_rect		region_bounds(const t_region *rg);
bool		region_empty(const t_region *rg);
void		gfx_blit_smooth(const t_blit *b);
int			gfx_selftest(void);

#endif
