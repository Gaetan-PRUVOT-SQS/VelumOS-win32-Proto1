#include "velum/font.h"
#include "velum/gfx.h"
#include "velum/libk.h"
#include "con_app.h"

#define BG 0xff1a1e2bu
#define CURSOR 0xff4fd1ccu
#define STATUS_RULE 0x0fffffffu
#define STATUS_TEXT 0xff8a93a8u

static const t_color	g_palette[CC_COUNT] = {0xffd6dce7u, 0xff4fd1ccu,
	0xffaab2c3u, 0xffa99cf0u, 0xfff3cd68u, 0xff7de673u, 0xff8a93a8u,
	0xffff8a8au};

static void	draw_run(t_conapp *a, t_point at, const char *text, int32_t len,
				t_color color)
{
	t_textreq	rq;

	rq.font = font_get(FONT_JBM_14);
	rq.at = at;
	rq.color = color;
	rq.text = text;
	rq.len = len;
	font_draw(&a->ui.win.surface, &rq);
}

static void	draw_cells(t_conapp *a, int32_t y, const char *text,
				const uint8_t *color, uint32_t bytes)
{
	uint32_t	start;
	uint32_t	i;
	int32_t		x;

	x = CON_PAD_X;
	start = 0;
	while (start < bytes)
	{
		i = start;
		while (i < bytes && color[i] == color[start])
			i++;
		draw_run(a, (t_point){x, y}, text + start, (int32_t)(i - start),
			g_palette[color[start]]);
		x += a->cell_w * (int32_t)line_cells(text + start, i - start);
		start = i;
	}
}

static void	draw_cursor(t_conapp *a, int32_t y, const t_conlive *l)
{
	t_rect		r;
	uint32_t	i;
	uint32_t	n;

	r = rect_make(CON_PAD_X + a->cell_w * (int32_t)l->cursor, y,
		a->cell_w, 17);
	gfx_fill(&a->ui.win.surface, r, CURSOR);
	i = 0;
	n = 0;
	while (i < l->bytes && n < l->cursor)
	{
		i++;
		while (i < l->bytes && ((uint8_t)l->text[i] & 0xc0) == 0x80)
			i++;
		n++;
	}
	if (i >= l->bytes || l->text[i] == ' ')
		return ;
	n = 1;
	while (i + n < l->bytes && ((uint8_t)l->text[i + n] & 0xc0) == 0x80)
		n++;
	draw_run(a, (t_point){r.x, y}, l->text + i, (int32_t)n, BG);
}

static void	draw_status(t_conapp *a)
{
	t_surface	*s;
	t_textreq	rq;
	char		left[32];
	const char	*right;
	int32_t		y;

	s = &a->ui.win.surface;
	y = s->h - CON_STATUS_H;
	gfx_fill(s, rect_make(0, y, s->w, 1), STATUS_RULE);
	strlcpy(left, "UTF-8 · ", sizeof(left));
	con_fmt_num(left + strlen(left), 8, a->con.screen.cols, 0);
	strlcat(left, "x", sizeof(left));
	con_fmt_num(left + strlen(left), 8, a->rows_vis, 0);
	right = "Ctrl+L efface · Ctrl+C annule";
	rq.font = font_get(FONT_INS_12);
	rq.color = STATUS_TEXT;
	rq.at = (t_point){18, y + 9};
	rq.text = left;
	rq.len = (int32_t)strlen(left);
	font_draw(s, &rq);
	rq.text = right;
	rq.len = (int32_t)strlen(right);
	rq.at.x = s->w - 18 - font_text_width(rq.font, right, rq.len);
	font_draw(s, &rq);
}

static uint32_t	history_rows(const t_console *c)
{
	const t_conrow	*last;
	uint32_t		n;

	n = scr_count(&c->screen);
	last = scr_row(&c->screen, n - 1);
	if (last && last->bytes == 0)
		n--;
	return (n);
}

void	con_paint(t_conapp *a)
{
	t_conlive		live;
	const t_conrow	*r;
	uint32_t		hist;
	uint32_t		first;
	uint32_t		i;

	gfx_fill(&a->ui.win.surface, rect_make(0, 0, a->ui.win.surface.w,
			a->ui.win.surface.h), BG);
	hist = history_rows(&a->con);
	first = 0;
	if (hist + 1 > a->rows_vis + a->scroll)
		first = hist + 1 - a->rows_vis - a->scroll;
	i = first;
	while (i < hist && i - first < a->rows_vis)
	{
		r = scr_row(&a->con.screen, i);
		draw_cells(a, CON_PAD_Y + (int32_t)(i - first) * CON_LINE_H + 2,
			r->text, r->color, r->bytes);
		i++;
	}
	if (a->scroll == 0 && hist - first < a->rows_vis)
	{
		con_live_row(&a->con, a->con.screen.cols, &live);
		i = CON_PAD_Y + (hist - first) * CON_LINE_H + 2;
		draw_cells(a, (int32_t)i, live.text, live.color, live.bytes);
		if (a->blink)
			draw_cursor(a, (int32_t)i, &live);
	}
	draw_status(a);
	wmc_present(&a->ui.win, NULL, 0);
}
