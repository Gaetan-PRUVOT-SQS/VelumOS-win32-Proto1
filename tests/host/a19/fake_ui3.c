#include "harness.h"
#include "fake.h"

void	fake_begin(t_fakeui *u, int w, int h)
{
	fake_mem_reset(0);
	h_eq_i64("ouverture de l'interface", fake_ui_open(u, w, h), 0);
}

void	fake_done(t_fakeui *u)
{
	fake_ui_close(u);
	h_eq_i64("aucune fuite", fake_mem_live(), 0);
}

int	fake_band_area(t_rect r)
{
	int	inner_w;
	int	inner_h;

	inner_w = r.w - 4;
	inner_h = r.h - 4;
	if (inner_w < 0)
		inner_w = 0;
	if (inner_h < 0)
		inner_h = 0;
	return (r.w * r.h - inner_w * inner_h);
}
