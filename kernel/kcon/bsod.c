#include "../../drivers/display/display_int.h"
#include "kcon_int.h"
#include "velum/boot.h"
#include "velum/err.h"
#include "velum/libk.h"

int	bsod_setup(t_bsod *b, t_surface *s, const t_font *f)
{
	if (!s || !f)
		return (E_INVAL);
	memset(b, 0, sizeof(*b));
	b->surf = s;
	b->font = f;
	b->cell_w = font_text_width(f, "M", 1);
	b->cell_h = f->height;
	if (b->cell_w < 1 || b->cell_h < 1)
		return (E_INVAL);
	b->cols = s->w / b->cell_w;
	b->rows = s->h / b->cell_h;
	if (b->cols < BSOD_MIN_COLS || b->rows < BSOD_MIN_ROWS)
		return (E_RANGE);
	b->width = b->cols - 2 * BSOD_MARGIN;
	b->row = BSOD_MARGIN;
	return (E_OK);
}

static void	bsod_build(t_bsod *b)
{
	char	buf[96];

	strlcpy(buf, g_bsod_text.build_label, sizeof(buf));
	strlcat(buf, boot_build_id(), sizeof(buf));
	bsod_para(b, buf, 2);
}

void	bsod_paint(t_bsod *b, const char *title, const char *msg)
{
	int32_t	lines;

	bsod_title(b, title);
	b->row += 1;
	bsod_para(b, g_bsod_text.intro, 3);
	b->row += 1;
	bsod_code(b, msg);
	b->row += 1;
	bsod_para(b, g_bsod_text.msg_label, 1);
	lines = b->rows - b->row - BSOD_TAIL_ROWS;
	if (lines < 1)
		lines = 1;
	bsod_para(b, msg, lines);
	b->row += 1;
	bsod_build(b);
	b->row += 1;
	bsod_para(b, g_bsod_text.halted, 2);
}

void	kcon_blue_screen(const char *title, const char *msg)
{
	t_surface	*s;
	t_bsod		b;

	s = display_surface();
	if (!s)
		return ;
	s->clip = rect_make(0, 0, s->w, s->h);
	gfx_fill(s, rect_make(0, 0, s->w, s->h), BSOD_BLUE);
	if (!msg)
		msg = "";
	if (bsod_setup(&b, s, font_get(FONT_MONO)) == E_OK)
		bsod_paint(&b, title, msg);
}
