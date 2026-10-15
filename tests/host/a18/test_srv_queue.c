#include <string.h>
#include "harness.h"
#include "help.h"

static t_wsrv	g_s;

static void	flood(int moves, int keys)
{
	int	i;

	i = 0;
	while (i < moves)
	{
		hs_point(&g_s, 50 + (i % 2), 80);
		i++;
	}
	i = 0;
	while (i < keys)
	{
		hs_key(&g_s, 'A' + (uint32_t)(i % 26), 0);
		i++;
	}
}

static void	mouse_dropped_keys_kept(void)
{
	t_handle	c;
	uint32_t	id;
	uint32_t	keys;

	hs_one(&g_s, &c, &id);
	flood(100, 0);
	h_eq_i64("file noyau pleine", fk_queued(c), IPC_QUEUE_MAX);
	h_eq_i64("rien en attente cote serveur", g_s.c[0].nout, 0);
	h_true(g_s.c[0].dropped > 0, "mouvements ecartes");
	flood(0, 8);
	h_eq_i64("touches gardees en attente", g_s.c[0].nout, WS_OUT_MAX);
	h_true(ws_pending(&g_s), "envoi differe");
	keys = hr_drain(c, WMS_KEY, NULL);
	h_eq_i64("aucune touche dans la file noyau", keys, 0);
	g_fk.now += WS_FRAME_NS;
	ws_step(&g_s);
	h_eq_i64("touches livrees ensuite", hr_drain(c, WMS_KEY, NULL), 16);
	h_eq_i64("file serveur videe", g_s.c[0].nout, 0);
	h_eq_i64("client garde", g_s.violations + (g_s.c[0].flags & WCF_DEAD), 0);
	hs_stop(&g_s);
}

static void	handle_reply_overflow(void)
{
	t_handle	c;
	uint32_t	id;
	t_wmcreate	m;

	hs_one(&g_s, &c, &id);
	flood(0, 60);
	h_eq_i64("attente pleine", g_s.c[0].nout, WS_OUT_MAX);
	memset(&m, 0, sizeof(m));
	ws_hdr(&m.h, WMC_CREATE, sizeof(m), 0);
	m.rect = rect_make(0, 0, 150, 100);
	m.style = WS_DEFAULT;
	hr_send(&g_s, c, &m);
	h_eq_i64("client qui ne lit plus rejete", g_s.c[0].flags, 0);
	h_eq_i64("ses fenetres detruites", g_s.t.nused, 0);
	h_eq_i64("sections liberees", g_s.sec_total, 0);
	hs_stop(&g_s);
	h_eq_i64("rien ne fuit", fk_live(), 0);
}

int	main(void)
{
	h_begin("a18/srv_queue");
	h_run("souris ecartee, touches gardees", mouse_dropped_keys_kept);
	h_run("reponse avec handle impossible", handle_reply_overflow);
	return (h_end());
}
