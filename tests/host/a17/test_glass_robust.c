#include <stdint.h>
#include "velum/theme.h"
#include "harness.h"
#include "lt.h"
#include "luna_int.h"

#define BACK 0xff3366ccu
#define SENT 0xdeadbeefu
#define SCR 4096
#define NSURF 10
#define NNEED 8

static uint32_t			g_scr[SCR];

static const int32_t	g_s[NSURF][3] = {
{100, 60, 99}, {100, 60, 0}, {100, 60, -1}, {100, 60, INT32_MIN},
{0, 60, 116}, {100, 0, 116}, {-1, 60, 116}, {100, -1, 116},
{INT32_MAX, 60, 116}, {100, INT32_MAX, 116}};

static const char		*g_sn[NSURF] = {
	"stride w - 1", "stride nul", "stride -1", "stride INT32_MIN",
	"w nul", "h nul", "w negatif", "h negatif", "w INT32_MAX",
	"h INT32_MAX"};

static const int64_t	g_n[NNEED][3] = {
{1, 1, 2}, {4, 4, 2}, {5, 5, 8}, {0, 5, 0}, {-3, 5, 0}, {100, 60, 750},
{16777216, 16777216, 35184372088832LL},
{INT32_MAX, INT32_MAX, 35184372088832LL}};

static void	hostile_calls(t_surface *s)
{
	t_lunawin	w;
	t_surface	src;
	int32_t		k;

	w = (t_lunawin){lp_rect(5, 5, 90, 50), WS_CAPTION | WS_SYSMENU,
		"robuste", 0, true, false, 0, 0};
	src = *s;
	k = 0;
	while (k < LUNA_GLASS_KINDS)
	{
		luna_glass(s, lp_rect(10, 10, 60, 30), (t_glasskind)k);
		k++;
	}
	luna_client_band_paint(s, &w, &src, 0xff202020u);
	luna_window_frame(s, &w);
}

static void	surface_hostile(void)
{
	t_lt		t;
	t_surface	orig;
	uint64_t	ref;
	size_t		i;

	h_true(lt_open(&t, 100, 60) == 0, "allocation");
	luna_glass_scratch(g_scr, SCR);
	i = 0;
	while (i < NSURF)
	{
		lt_clear(&t, BACK);
		ref = lt_hash(&t.s);
		orig = t.s;
		t.s.w = g_s[i][0];
		t.s.h = g_s[i][1];
		t.s.stride = g_s[i][2];
		hostile_calls(&t.s);
		t.s = orig;
		h_true(lt_hash(&t.s) == ref, g_sn[i]);
		h_true(lt_guard_ok(&t), "gardes intactes");
		i++;
	}
	luna_glass_scratch(NULL, 0);
	lt_close(&t);
}

static void	need_table(void)
{
	size_t	i;
	t_rect	r;

	i = 0;
	while (i < NNEED)
	{
		r = lp_rect(0, 0, (int32_t)g_n[i][0], (int32_t)g_n[i][1]);
		h_eq_u64("tampon requis, partitions et limites",
			luna_glass_need(r), (uint64_t)g_n[i][2]);
		i++;
	}
	h_eq_u64("tampon requis, x hors limites",
		luna_glass_need(lp_rect(16777217, 0, 10, 10)), 0);
	h_eq_u64("tampon requis, y INT32_MIN",
		luna_glass_need(lp_rect(0, INT32_MIN, 10, 10)), 0);
}

static void	scratch_limites(void)
{
	t_lt	t;
	size_t	need;
	size_t	i;

	need = luna_glass_need(lp_rect(20, 10, 60, 30));
	h_true(lt_open(&t, 100, 60) == 0, "allocation");
	luna_theme_set(LUNA_THEME_LIGHT);
	luna_transparency_set(false);
	i = 0;
	while (i < SCR)
		g_scr[i++] = SENT;
	lt_clear(&t, BACK);
	luna_glass_scratch(g_scr, need - 1);
	luna_glass(&t.s, lp_rect(20, 10, 60, 30), LUNA_GLASS_DOCK);
	h_eq_u64("tampon trop court d'un mot, intact", g_scr[0], SENT);
	h_eq_u64("tampon trop court, repli opaque",
		t.s.px[35 * (size_t)t.s.stride + 50], VT_OPAQUE_PANEL);
	lt_clear(&t, BACK);
	luna_glass_scratch(g_scr, need);
	luna_glass(&t.s, lp_rect(20, 10, 60, 30), LUNA_GLASS_DOCK);
	h_true(g_scr[0] != SENT, "tampon exact utilise");
	h_eq_u64("aucun mot ecrit apres le tampon", g_scr[need], SENT);
	h_true(lt_guard_ok(&t), "gardes intactes");
	luna_glass_scratch(NULL, 0);
	lt_close(&t);
}

int	main(void)
{
	h_begin("a17/glass_robuste");
	h_run("surfaces hostiles, partitions de stride et de taille",
		surface_hostile);
	h_run("tampon requis, valeurs limites", need_table);
	h_run("tampon de travail trop petit, valeur limite", scratch_limites);
	return (h_end());
}
