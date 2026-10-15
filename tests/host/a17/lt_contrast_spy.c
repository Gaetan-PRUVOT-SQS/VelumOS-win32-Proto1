#include "harness.h"
#include "lt_contrast.h"
#include "lt_geo.h"

void	lk_begin(t_lt *t, t_color back)
{
	fk_spy()->n = 0;
	fk_spy()->mute = true;
	lt_clear(t, back);
}

double	lk_spied(const t_surface *s)
{
	const t_fkspy	*spy;
	t_rect			box;
	double			worst;
	double			cur;
	int32_t			i;

	spy = fk_spy();
	if (spy->n == 0)
		return (0.0);
	worst = 21.0;
	i = 0;
	while (i < spy->n)
	{
		box = spy->rec[i].box;
		box.y -= LK_SLACK;
		box.h += 2 * LK_SLACK;
		cur = lk_worst(s, box, spy->rec[i].c);
		if (cur < worst)
			worst = cur;
		i++;
	}
	return (worst);
}

double	lk_title(t_lt *t, uint32_t style, bool active)
{
	t_lunawin	w;

	w = lt_win(lp_rect(0, 0, 300, 110), style, false);
	w.active = active;
	w.title = "Titre";
	lk_begin(t, LC_BLACK);
	luna_window_frame(&t->s, &w);
	return (lk_spied(&t->s));
}

void	lk_check(const char *what, double ratio)
{
	printf("  contraste %-34s %5.2f\n", what, ratio);
	h_true(ratio >= LK_AA, what);
}
