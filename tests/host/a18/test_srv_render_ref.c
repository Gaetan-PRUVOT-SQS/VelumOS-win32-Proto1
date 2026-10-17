#include "harness.h"
#include "help.h"

#define W 320
#define H 240

static t_wsrv	g_s;

static uint32_t	shown(void)
{
	return (hp_hash(&g_s.fb));
}

static void	scenario(uint32_t *out)
{
	t_handle	c;
	uint32_t	a;
	uint32_t	b;

	hs_start(&g_s, W, H);
	c = hr_client(&g_s);
	a = hs_create(&g_s, c, rect_make(10, 10, 150, 100), WS_DEFAULT);
	b = hs_create(&g_s, c, rect_make(100, 70, 150, 100), WS_DEFAULT);
	hs_settle(&g_s, c);
	g_fk.now += 20000000ull;
	ws_step(&g_s);
	out[0] = shown();
	hs_point(&g_s, 30, 20);
	hs_point(&g_s, 33, 21);
	hs_point(&g_s, 36, 22);
	hs_point(&g_s, 120, 80);
	out[1] = shown();
	hs_drag(&g_s, (t_point){160, 80}, (t_point){200, 130});
	out[2] = shown();
	hs_click(&g_s, 40, 60);
	hs_point(&g_s, 60, 30);
	out[3] = shown();
	(void)a;
	(void)b;
	hs_stop(&g_s);
}

static void	rendu_de_reference(void)
{
	uint32_t	h[4];

	scenario(h);
	h_eq_u64("apres creation", h[0], 3982986116u);
	h_eq_u64("apres survol de la legende", h[1], 134514227u);
	h_eq_u64("apres deplacement", h[2], 317331635u);
	h_eq_u64("apres activation et curseur", h[3], 485249174u);
}

int	main(void)
{
	h_begin("a18/srv_render_ref");
	h_run("rendu du framebuffer inchange par la mesure et la fusion",
		rendu_de_reference);
	return (h_end());
}
