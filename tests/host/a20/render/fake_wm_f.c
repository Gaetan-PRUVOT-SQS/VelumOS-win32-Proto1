#include "velum/libk.h"
#include "render.h"

void	msg_info(t_uimsg *m, uint32_t type, const t_infospec *i)
{
	t_wminfo	w;

	memset(&w, 0, sizeof(w));
	w.h.magic = WM_MAGIC;
	w.h.type = type;
	w.h.size = sizeof(w);
	w.h.window = i->id;
	w.style = i->style;
	w.state = i->state;
	w.active = i->active;
	strlcpy(w.title, i->title, sizeof(w.title));
	memset(m, 0, sizeof(*m));
	memcpy(m->words, &w, sizeof(w));
	m->len = (int)(sizeof(w));
}

void	msg_key(t_uimsg *m, uint32_t win, uint32_t type, uint32_t code)
{
	t_wmkey	k;

	memset(&k, 0, sizeof(k));
	k.h.magic = WM_MAGIC;
	k.h.type = WMS_KEY;
	k.h.size = sizeof(k);
	k.h.window = win;
	k.ev.type = type;
	k.ev.code = code;
	memset(m, 0, sizeof(*m));
	memcpy(m->words, &k, sizeof(k));
	m->len = (int)(sizeof(k));
}
