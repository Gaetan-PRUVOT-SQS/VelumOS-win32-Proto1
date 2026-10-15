#include "velum/err.h"
#include "velum/libk.h"
#include "wmc_int.h"

void	wmc_title_copy(char *dst, const char *src)
{
	size_t	n;

	memset(dst, 0, WM_TITLE_MAX);
	if (src == NULL)
		return ;
	n = strnlen(src, WM_TITLE_MAX - 1);
	if (n == WM_TITLE_MAX - 1)
	{
		while (n > 0 && ((uint8_t)src[n] & 0xc0) == 0x80)
			n--;
	}
	memcpy(dst, src, n);
}

int	wmc_simple(uint32_t type, uint32_t window, uint32_t value)
{
	t_wmarg	m;
	int		r;

	memset(&m, 0, sizeof(m));
	if (type == WMC_DESTROY || type == WMC_ACTIVATE)
		wmc_hdr(&m.h, type, sizeof(m.h), window);
	else
		wmc_hdr(&m.h, type, sizeof(m), window);
	m.value = value;
	r = wmc_send(&m);
	if (r < 0)
		return (r);
	return (0);
}

int	wmc_set_title(t_wmwin *w, const char *title)
{
	t_wmtitle	m;
	int			r;

	if (w == NULL || title == NULL)
		return (E_INVAL);
	memset(&m, 0, sizeof(m));
	wmc_hdr(&m.h, WMC_SET_TITLE, sizeof(m), w->id);
	wmc_title_copy(m.title, title);
	r = wmc_send(&m);
	if (r < 0)
		return (r);
	return (0);
}

int	wmc_set_rect(t_wmwin *w, t_rect rect)
{
	t_wmrect	m;
	int			r;

	if (w == NULL)
		return (E_INVAL);
	memset(&m, 0, sizeof(m));
	wmc_hdr(&m.h, WMC_SET_RECT, sizeof(m), w->id);
	m.rect = rect;
	r = wmc_send(&m);
	if (r < 0)
		return (r);
	return (0);
}

int	wmc_set_workarea(t_rect rect)
{
	t_wmrect	m;
	int			r;

	memset(&m, 0, sizeof(m));
	wmc_hdr(&m.h, WMC_SET_WORKAREA, sizeof(m), 0);
	m.rect = rect;
	r = wmc_send(&m);
	if (r < 0)
		return (r);
	return (0);
}
