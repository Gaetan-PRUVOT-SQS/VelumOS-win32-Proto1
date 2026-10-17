#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static t_ctl	*add(t_fakeui *u, uint32_t type, t_rect r, const char *text)
{
	return (fake_add(u, fake_spec(type, 7, r, text)));
}

static const t_fakecall	*paint_last(t_fakeui *u, t_ctl *c, int kind)
{
	ctl_invalidate(&u->r, c);
	fake_log_clear();
	ctl_paint(&u->r);
	return (fake_log_last(kind));
}

static void	focus_margin(void)
{
	t_fakeui	u;
	t_ctl		*b;
	t_rect		face;
	t_rect		ring;
	int			m;

	m = LUNA_FOCUS_MARGIN;
	fake_begin(&u, 200, 100);
	b = add(&u, CT_BUTTON, rect_make(20, 20, 80, 32), "OK");
	ctl_focus(&u.r, b);
	face = paint_last(&u, b, FK_BUTTON)->r;
	h_true(fake_rect_eq(face, rect_make(20 + 4, 20 + 4, 80 - 8, 32 - 8)),
		"capsule : rectangle moins la marge de 4 px");
	ring = rect_make(face.x - m, face.y - m, face.w + 2 * m, face.h + 2 * m);
	h_true(fake_rect_eq(ring, b->rect),
		"anneau (jeu 2 + trait 2) dans le rect");
	h_true((fake_log_last(FK_BUTTON)->b & 2) != 0, "focus transmis");
	fake_done(&u);
}

static void	focus_margin_limits(void)
{
	t_fakeui	u;
	t_ctl		*b;

	fake_begin(&u, 200, 100);
	b = add(&u, CT_BUTTON, rect_make(20, 20, 8, 8), "");
	h_true(fake_rect_eq(paint_last(&u, b, FK_BUTTON)->r, b->rect),
		"8x8 : pas de place pour la marge, rect entier");
	ctl_set_rect(&u.r, b, rect_make(20, 20, 9, 9));
	h_true(fake_rect_eq(paint_last(&u, b, FK_BUTTON)->r,
			rect_make(24, 24, 1, 1)), "9x9 : capsule de 1 px");
	ctl_set_rect(&u.r, b, rect_make(20, 20, 9, 8));
	h_true(fake_rect_eq(paint_last(&u, b, FK_BUTTON)->r, b->rect),
		"9x8 : une seule dimension trop courte, rect entier");
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/glass-focus");
	h_run("marge de focus : capsule et anneau dans le rectangle",
		focus_margin);
	h_run("marge de focus : valeurs limites 8, 9", focus_margin_limits);
	return (h_end());
}
