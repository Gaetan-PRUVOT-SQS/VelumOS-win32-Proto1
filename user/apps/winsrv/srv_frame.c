#include "ws_srv.h"

void	ws_mark(t_wsrv *s, t_rect r)
{
	wdy_add(&s->dirty, r);
}

void	ws_mark_win(t_wsrv *s, int slot)
{
	if (slot >= 0 && slot < WS_WIN_MAX && s->t.w[slot].id != 0)
		wdy_add(&s->dirty, wsh_extent(&s->t, slot));
}

void	ws_cursor_set(t_wsrv *s, t_point pos, uint32_t shape)
{
	if (pos.x == s->cur.pos.x && pos.y == s->cur.pos.y
		&& shape == s->cur.shape)
		return ;
	ws_mark(s, wcur_rect(s->cur));
	s->cur.pos = pos;
	s->cur.shape = shape;
	ws_mark(s, wcur_rect(s->cur));
}

void	ws_frame(t_wsrv *s)
{
	uint32_t	i;
	t_rect		r;
	t_blit		b;

	i = 0;
	while (s->back.px != NULL && i < s->dirty.n)
	{
		r = wgl_extend(&s->t, s->dirty.r[i]);
		wc_compose(&s->t, &s->back, s->cur, r);
		if (s->fb.px != NULL)
		{
			b.dst = &s->fb;
			b.src = &s->back;
			b.sr = r;
			b.dr = r;
			gfx_blit(&b);
		}
		i++;
	}
	s->dirty.n = 0;
	s->frames++;
	s->next_frame = s->now + WS_FRAME_NS;
}
