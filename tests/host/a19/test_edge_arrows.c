#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static t_rect	first_button(t_fakeui *u, t_ctl *s)
{
	ctl_invalidate(&u->r, s);
	fake_log_clear();
	ctl_paint(&u->r);
	if (fake_log_get(0)->kind == FK_BUTTON)
		return (fake_log_get(0)->r);
	return (s->rect);
}

static void	horizontal_arrows(void)
{
	t_fakeui	u;
	t_ctl		*s;
	t_rect		dec;

	fake_begin(&u, 150, 60);
	s = fake_add(&u, fake_spec(CT_SCROLL, 7, rect_make(10, 10, 100, 17),
				NULL));
	ctl_scroll_set(&u.r, s, 100, 10);
	dec = first_button(&u, s);
	h_eq_i64("barre horizontale : fleche en bouton", dec.w, 17);
	h_eq_i64("fleche vivante : encre, 1+3+5+7 pixels",
		fake_px_count(&u.s, dec, 0xff000000), 16);
	ctl_scroll_set(&u.r, s, 0, 10);
	dec = first_button(&u, s);
	h_eq_i64("fleche inactive : encre attenuee",
		fake_px_count(&u.s, dec, 0xff808080), 16);
	fake_done(&u);
}

static void	short_bar_arrows(void)
{
	t_fakeui	u;
	t_ctl		*s;
	t_rect		dec;

	fake_begin(&u, 150, 60);
	s = fake_add(&u, fake_spec(CT_SCROLL, 7, rect_make(10, 10, 17, 20),
				NULL));
	ctl_scroll_set(&u.r, s, 100, 10);
	dec = first_button(&u, s);
	h_eq_i64("barre courte : fleches de la moitie", dec.h, 10);
	h_true(dec.y + dec.h <= s->rect.y + s->rect.h, "dans la barre");
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/edge-arrows");
	h_run("fleches horizontales : encre du bouton", horizontal_arrows);
	h_run("barre plus courte que deux fleches", short_bar_arrows);
	return (h_end());
}
