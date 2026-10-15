#include <stdio.h>
#include "velum/abi/abi_input.h"
#include "velum/libk.h"
#include "render.h"

static void	write_row(FILE *f, const t_surface *s, int32_t y)
{
	int32_t		x;
	uint32_t	c;
	uint8_t		rgb[3];

	x = 0;
	while (x < s->w)
	{
		c = s->px[(size_t)y * (size_t)s->stride + (size_t)x];
		rgb[0] = (uint8_t)(c >> 16);
		rgb[1] = (uint8_t)(c >> 8);
		rgb[2] = (uint8_t)c;
		fwrite(rgb, 1, 3, f);
		x++;
	}
}

int	ppm_write(const t_surface *s, const char *path)
{
	FILE	*f;
	int32_t	y;

	f = fopen(path, "wb");
	if (!f)
		return (-1);
	fprintf(f, "P6\n%d %d\n255\n", s->w, s->h);
	y = 0;
	while (y < s->h)
	{
		write_row(f, s, y);
		y++;
	}
	return (fclose(f));
}

int	px_near(const t_surface *s, t_point p, uint32_t rgb, int tol)
{
	uint32_t	c;
	int			dr;
	int			dg;
	int			db;

	if (p.x < 0 || p.y < 0 || p.x >= s->w || p.y >= s->h)
		return (0);
	c = s->px[(size_t)p.y * (size_t)s->stride + (size_t)p.x];
	dr = (int)((c >> 16) & 0xff) - (int)((rgb >> 16) & 0xff);
	dg = (int)((c >> 8) & 0xff) - (int)((rgb >> 8) & 0xff);
	db = (int)(c & 0xff) - (int)(rgb & 0xff);
	return (dr >= -tol && dr <= tol && dg >= -tol && dg <= tol && db >= -tol
		&& db <= tol);
}

void	msg_mouse(t_uimsg *m, uint32_t win, uint32_t type, t_point p)
{
	t_wmmouse	ev;

	memset(&ev, 0, sizeof(ev));
	ev.h.magic = WM_MAGIC;
	ev.h.type = WMS_MOUSE;
	ev.h.size = sizeof(ev);
	ev.h.window = win;
	ev.type = type;
	ev.x = p.x;
	ev.y = p.y;
	ev.hit = HT_CLIENT;
	memset(m, 0, sizeof(*m));
	memcpy(m->words, &ev, sizeof(ev));
	m->len = (int)(sizeof(ev));
}

void	msg_button(t_uimsg *m, uint32_t win, uint32_t type, t_point p)
{
	t_wmmouse	ev;

	msg_mouse(m, win, type, p);
	memcpy(&ev, m->words, sizeof(ev));
	if (type == INP_MOUSE_DOWN)
		ev.buttons = 1u << BTN_LEFT;
	memcpy(m->words, &ev, sizeof(ev));
}
