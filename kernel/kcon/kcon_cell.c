#include "kcon_int.h"
#include "velum/err.h"
#include "velum/libk.h"

static int	cell_size(const t_font *f, int32_t *cw, int32_t *ch)
{
	*cw = font_text_width(f, "M", 1);
	*ch = f->height;
	if (*cw < 1 || *cw > KCON_CELL_MAX)
		return (E_INVAL);
	if (*ch < 1 || *ch > KCON_CELL_MAX)
		return (E_INVAL);
	return (E_OK);
}

int	kcon_setup(t_kcon *k, const t_surface *s, const t_font *f)
{
	int32_t	cw;
	int32_t	ch;
	int		rc;

	memset(k, 0, sizeof(*k));
	if (!s || !s->px || !f || s->w < 1 || s->h < 1 || s->stride < s->w)
		return (E_INVAL);
	rc = cell_size(f, &cw, &ch);
	if (rc < 0)
		return (rc);
	if (s->w < cw || s->h < ch)
		return (E_RANGE);
	k->surf = *s;
	k->font = f;
	k->cell_w = cw;
	k->cell_h = ch;
	k->cols = s->w / cw;
	k->rows = s->h / ch;
	k->fg = KCON_FG;
	k->bg = KCON_BG;
	k->ready = true;
	return (E_OK);
}

void	kcon_cell_draw(t_kcon *k, const char *text, int32_t len)
{
	t_textreq	rq;
	t_rect		cell;

	cell = rect_make(k->col * k->cell_w, k->row * k->cell_h, k->cell_w,
			k->cell_h);
	gfx_fill(&k->surf, cell, k->bg);
	rq.font = k->font;
	rq.at.x = cell.x;
	rq.at.y = cell.y;
	rq.color = k->fg;
	rq.text = text;
	rq.len = len;
	font_draw(&k->surf, &rq);
}

void	kcon_scroll(t_kcon *k)
{
	t_rect	area;
	t_rect	last;
	t_point	delta;

	area = rect_make(0, 0, k->cols * k->cell_w, k->rows * k->cell_h);
	last = rect_make(0, (k->rows - 1) * k->cell_h, area.w, k->cell_h);
	delta.x = 0;
	delta.y = -k->cell_h;
	gfx_scroll(&k->surf, area, delta);
	gfx_fill(&k->surf, last, k->bg);
}

void	kcon_clear_all(t_kcon *k)
{
	gfx_fill(&k->surf, rect_make(0, 0, k->surf.w, k->surf.h), k->bg);
	k->col = 0;
	k->row = 0;
	k->cursor_on = false;
}
