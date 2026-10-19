#include "velum/err.h"
#include "velum/libk.h"
#include "wmc_int.h"

void	luna_theme_set(t_lunatheme theme) __attribute__((weak));
void	luna_transparency_set(bool reduced) __attribute__((weak));

void	wmc_theme_apply(const t_wmcmsg *m)
{
	t_wmtheme	t;

	if (m->len != sizeof(t))
		return ;
	memcpy(&t, m->buf, sizeof(t));
	if (t.dark > 1 || t.reduced > 1 || t.reserved != 0)
		return ;
	if (luna_theme_set)
		luna_theme_set(t.dark ? LUNA_THEME_DARK : LUNA_THEME_LIGHT);
	if (luna_transparency_set)
		luna_transparency_set(t.reduced != 0);
}

int	wmc_set_theme(const t_wmwin *w, uint32_t dark, uint32_t reduced)
{
	t_wmtheme	t;
	int			r;

	if (w == NULL || w->id == 0 || dark > 1 || reduced > 1)
		return (E_INVAL);
	memset(&t, 0, sizeof(t));
	wmc_hdr(&t.h, WMC_SET_THEME, sizeof(t), w->id);
	t.dark = dark;
	t.reduced = reduced;
	r = wmc_send(&t);
	if (r < 0)
		return (r);
	return (0);
}
