#include "harness.h"
#include "velum/abi/abi_input.h"
#include "velum/abi/abi_syscall.h"
#include "velum/libk.h"
#include "render.h"

static const char	*g_argv[] = {"shell", "Utilisateur", NULL};

static void	feed(t_shell *sh, uint32_t type, const t_infospec *i)
{
	t_uimsg	m;

	msg_info(&m, type, i);
	sh_dispatch(sh, &m);
}

static void	taches_activation_et_reduction(void)
{
	t_shell		sh;
	t_infospec	a;
	t_infospec	b;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	a = (t_infospec){10, WS_DEFAULT, WSTATE_NORMAL, 0, "Bloc-notes"};
	b = (t_infospec){11, WS_DEFAULT, WSTATE_NORMAL, 1, "Bonjour"};
	feed(&sh, WMS_WIN_ADD, &a);
	feed(&sh, WMS_WIN_ADD, &b);
	h_eq_u64("deux boutons", sh.shown, 2);
	render_shot("shell_taches");
	drv_click(&sh, sh.bar.id, rect_center(sh.btn[0]));
	h_eq_u64("bouton inactif: activer", g_fwm.activated, 10);
	drv_click(&sh, sh.bar.id, rect_center(sh.btn[1]));
	h_eq_u64("bouton actif: reduire", g_fwm.state_id, 11);
	h_eq_u64("etat demande", g_fwm.state_value, WSTATE_MIN);
	b.state = WSTATE_MIN;
	b.active = 0;
	feed(&sh, WMS_WIN_UPD, &b);
	drv_click(&sh, sh.bar.id, rect_center(sh.btn[1]));
	h_eq_u64("bouton reduit: activer", g_fwm.activated, 11);
	feed(&sh, WMS_WIN_DEL, &a);
	h_eq_u64("fenetre fermee", sh.shown, 1);
	sh_shutdown(&sh);
}

static void	taches_beaucoup_de_fenetres(void)
{
	t_shell		sh;
	t_infospec	w;
	uint32_t	i;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	w = (t_infospec){0, WS_DEFAULT, WSTATE_NORMAL, 0, "Fenetre"};
	i = 0;
	while (i < 40)
	{
		w.id = 20 + i;
		feed(&sh, WMS_WIN_ADD, &w);
		i++;
	}
	h_eq_u64("quinze boutons: (736+8)/(40+8)", sh.shown, 15);
	h_true(sh.btn[14].x + sh.btn[14].w <= sh.reg.tray.x, "avant la zone");
	render_shot("shell_40_taches");
	drv_click(&sh, sh.bar.id, rect_center(sh.btn[14]));
	h_eq_u64("dernier bouton actif", g_fwm.activated, 34);
	sh_shutdown(&sh);
}

static void	taches_horloge_a_la_minute(void)
{
	t_shell		sh;
	t_uimsg		m;
	uint32_t	armed;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	h_eq_str("heure initiale", sh.clock, "14:07");
	armed = g_fos.timer_armed;
	g_fos.now += 60000000000ull;
	sh_clock_tick(&sh);
	h_eq_str("une minute plus tard", sh.clock, "14:08");
	h_true(g_fos.timer_armed > armed, "minuterie rearmee");
	g_fos.now += 53ull * 60000000000ull;
	sh_clock_tick(&sh);
	h_eq_str("passage de l'heure", sh.clock, "15:01");
	msg_key(&m, sh.bar.id, INP_KEY_DOWN, VK_F1);
	m.len = 7;
	sh_dispatch(&sh, &m);
	h_eq_i64("message tronque ignore", sh.sm.open, 0);
	sh_shutdown(&sh);
}

int	main(void)
{
	h_begin("a20/render-tasks");
	h_run("rendu taches: activer et reduire", taches_activation_et_reduction);
	h_run("rendu taches: quarante fenetres", taches_beaucoup_de_fenetres);
	h_run("rendu taches: horloge", taches_horloge_a_la_minute);
	return (h_end());
}
