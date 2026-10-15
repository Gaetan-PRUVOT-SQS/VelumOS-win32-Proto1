#include "kfix.h"
#include "velum/err.h"

bool	kfix_open(t_kfix *f, int32_t w, int32_t h, int32_t stride)
{
	bool	ok;

	fake_reset();
	ok = fake_gsurf_new(&f->g, w, h, stride);
	if (ok)
		ok = (kcon_setup(&f->k, &f->g.s, font_get(FONT_MONO)) == E_OK);
	h_true(ok, "montage de la console ouvert");
	return (ok);
}

void	kfix_close(t_kfix *f)
{
	h_true(fake_gsurf_ok(&f->g), "zones de garde de la surface intactes");
	fake_gsurf_free(&f->g);
}

void	kfix_feed(t_kfix *f, const char *s)
{
	kcon_feed(&f->k, s, strlen(s));
}

const t_fdraw	*kfix_glyph(uint32_t back)
{
	return (&g_ffont.log[(g_ffont.glyphs - 1 - back) % FAKE_LOG]);
}

uint32_t	kfix_cell_fg(const t_kfix *f, int32_t col, int32_t row)
{
	t_rect	r;

	r = rect_make(col * f->k.cell_w, row * f->k.cell_h, f->k.cell_w,
			f->k.cell_h);
	return (fake_count(&f->g.s, r, f->k.fg));
}
