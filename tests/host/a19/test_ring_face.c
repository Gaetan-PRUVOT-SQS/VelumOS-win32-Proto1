#include "harness.h"
#include "fake.h"

#define GLYPH_W 13
#define GLYPH_GAP 4
#define MARGIN 4

static t_rect	label_ring(void)
{
	t_rect	box;

	box = rect_make(20 + GLYPH_W + GLYPH_GAP, 20 + (24 - FAKE_FONT_H) / 2,
			4 * FAKE_CHAR_W, FAKE_FONT_H);
	return (rect_make(box.x - MARGIN, box.y - MARGIN, box.w + 2 * MARGIN,
			box.h + 2 * MARGIN));
}

static void	check_face(uint32_t type)
{
	t_fakeui	u;
	t_ctl		*c;

	fake_begin(&u, 200, 80);
	c = fake_add(&u, fake_spec(type, 7, rect_make(20, 20, 120, 24), "Case"));
	ctl_invalidate(&u.r, u.r.root);
	ctl_paint(&u.r);
	h_eq_i64("sans focus : aucun accent",
		fake_px_count(&u.s, c->rect, luna_color_accent()), 0);
	ctl_focus(&u.r, c);
	ctl_invalidate(&u.r, u.r.root);
	ctl_paint(&u.r);
	h_eq_i64("anneau accent autour du libelle, dans la marge de 4 px",
		fake_px_count(&u.s, c->rect, luna_color_accent()),
		fake_band_area(label_ring()));
	fake_done(&u);
}

static void	check_and_radio(void)
{
	check_face(CT_CHECK);
	check_face(CT_RADIO);
}

int	main(void)
{
	h_begin("a19/ring-face");
	h_run("case et radio : anneau autour du libelle", check_and_radio);
	return (h_end());
}
