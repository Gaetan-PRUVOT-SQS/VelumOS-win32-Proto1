#include "kcon_int.h"
#include "velum/libk.h"

void	bsod_text(t_bsod *b, int32_t x, const t_bsod_line *ln, t_color c)
{
	t_textreq	rq;

	if (ln->len <= 0 || b->row >= b->rows)
		return ;
	rq.font = b->font;
	rq.at.x = x;
	rq.at.y = b->row * b->cell_h;
	rq.color = c;
	rq.text = ln->s;
	rq.len = ln->len;
	font_draw(b->surf, &rq);
}

void	bsod_title(t_bsod *b, const char *title)
{
	t_bsod_line	ln;
	t_rect		bar;
	int32_t		px;

	if (!title)
		title = "";
	ln.s = title;
	ln.len = bsod_cut(title, (int32_t)strlen(title), b->width - 2);
	px = font_text_width(b->font, ln.s, ln.len) + 2 * b->cell_w;
	bar = rect_make((b->surf->w - px) / 2, b->row * b->cell_h, px, b->cell_h);
	gfx_fill(b->surf, bar, BSOD_WHITE);
	bsod_text(b, bar.x + b->cell_w, &ln, BSOD_BLUE);
	b->row++;
}

void	bsod_para(t_bsod *b, const char *text, int32_t max)
{
	t_bsod_wrap	w;
	t_bsod_line	dots;
	int32_t		i;
	int32_t		x;

	w.text = text;
	w.cols = b->width;
	w.max = max;
	bsod_wrap(&w);
	dots.s = g_bsod_text.dots;
	dots.len = BSOD_DOTS;
	i = 0;
	while (i < w.count && b->row < b->rows)
	{
		bsod_text(b, BSOD_MARGIN * b->cell_w, &w.lines[i], BSOD_WHITE);
		if (w.truncated && i == w.count - 1)
		{
			x = BSOD_MARGIN * b->cell_w;
			x += font_text_width(b->font, w.lines[i].s, w.lines[i].len);
			bsod_text(b, x, &dots, BSOD_WHITE);
		}
		b->row++;
		i++;
	}
}

uint32_t	bsod_hash(const char *msg)
{
	uint32_t	h;

	h = 2166136261u;
	while (*msg)
	{
		h ^= (uint8_t)(*msg);
		h *= 16777619u;
		msg++;
	}
	return (h);
}

void	bsod_code(t_bsod *b, const char *msg)
{
	char		buf[48];
	t_bsod_line	ln;
	uint32_t	h;
	size_t		n;
	int			i;

	n = strlcpy(buf, g_bsod_text.code_label, sizeof(buf));
	h = bsod_hash(msg);
	i = 8;
	while (i > 0 && n + 1 < sizeof(buf))
	{
		i--;
		buf[n++] = "0123456789ABCDEF"[(h >> (4 * i)) & 15];
	}
	buf[n] = '\0';
	ln.s = buf;
	ln.len = (int32_t)n;
	bsod_text(b, BSOD_MARGIN * b->cell_w, &ln, BSOD_WHITE);
	b->row++;
}
