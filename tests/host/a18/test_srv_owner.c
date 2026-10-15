#include <string.h>
#include "harness.h"
#include "help.h"

static t_wsrv	g_s;

static void	foreign_ids(void)
{
	t_handle	a;
	t_handle	b;
	uint32_t	id;
	t_wmarg		m;

	hs_start(&g_s, 320, 240);
	a = hr_client(&g_s);
	b = hr_client(&g_s);
	id = hr_create(&g_s, a, rect_make(10, 10, 150, 100), WS_DEFAULT);
	hr_arg(&m, WMC_SET_STATE, id, WSTATE_MIN);
	hr_send(&g_s, b, &m);
	h_eq_i64("etranger deconnecte", hr_recv(b, &m, NULL), E_PIPE);
	h_eq_i64("fenetre intacte", g_s.t.w[wt_find(&g_s.t, id)].state,
		WSTATE_NORMAL);
	hr_arg(&m, WMC_DESTROY, id, 0);
	m.h.size = sizeof(t_wmhdr);
	hr_send(&g_s, a, &m);
	h_eq_i64("detruite par son proprietaire", g_s.t.nused, 0);
	hr_drain(a, 0, NULL);
	hr_send(&g_s, a, &m);
	h_eq_i64("id recycle refuse", hr_recv(a, &m, NULL), E_PIPE);
	h_eq_i64("deux violations", g_s.violations, 2);
	fk_close(a);
	fk_close(b);
	hs_stop(&g_s);
}

static void	quota_per_client(void)
{
	t_handle	a;
	int			i;

	hs_start(&g_s, 320, 240);
	a = hr_client(&g_s);
	i = 0;
	while (i++ < WM_WIN_PER_CLIENT)
		hr_create(&g_s, a, rect_make(0, 0, 112, 34), WS_DEFAULT);
	h_eq_i64("64 fenetres", g_s.t.nused, WM_WIN_PER_CLIENT);
	h_eq_i64("65e refusee", hr_create(&g_s, a, rect_make(0, 0, 112, 34),
			WS_DEFAULT), 0);
	h_eq_i64("client garde", g_s.violations, 0);
	fk_close(a);
	hs_stop(&g_s);
}

static void	quota_total(void)
{
	t_handle	c[3];
	int			i;

	hs_start(&g_s, 320, 240);
	i = 0;
	while (i < 3)
		c[i++] = hr_client(&g_s);
	i = 0;
	while (i < WS_WIN_MAX)
	{
		hr_create(&g_s, c[i / WM_WIN_PER_CLIENT], rect_make(0, 0, 112, 34),
			WS_DEFAULT);
		i++;
	}
	h_eq_i64("128 au total", g_s.t.nused, WS_WIN_MAX);
	hr_create(&g_s, c[2], rect_make(0, 0, 112, 34), WS_DEFAULT);
	h_true(g_hr.status == E_NOSPC && g_hr.seq == 7, "erreur E_NOSPC et seq");
	h_eq_i64("invariants", wz_check(&g_s.t), 0);
	hs_stop(&g_s);
	h_eq_i64("tout libere", fk_live(), 0);
}

int	main(void)
{
	h_begin("a18/srv_owner");
	h_run("identifiants etrangers", foreign_ids);
	h_run("quota par client", quota_per_client);
	h_run("quota total", quota_total);
	return (h_end());
}
