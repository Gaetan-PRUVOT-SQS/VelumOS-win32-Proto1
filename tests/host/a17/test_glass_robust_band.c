#include <stdint.h>
#include "harness.h"
#include "lt.h"
#include "luna_int.h"

#define BACK 0xff3366ccu
#define FB 0xff202020u
#define SRC 0xffd0d000u
#define PAD 0xff00ff00u
#define MEM 17000
#define CW 240
#define CH 70
#define NCASE 19

static uint32_t			g_mem[MEM];

static const int32_t	g_c[NCASE][6] = {
{200, 70, 200, 100, -1, 1}, {181, 70, 182, 180, -1, 1},
{150, 70, 151, 149, -1, 1}, {150, 70, 151, 150, -1, 0},
{200, 69, 200, 100, -1, 0}, {200, 69, 200, 100, -2, 1},
{200, 70, 199, 100, -1, 0}, {200, 70, 1, 100, -1, 0},
{200, 70, 50, 100, -1, 0}, {200, 70, 0, 100, -1, 0},
{200, 70, -1, 100, -1, 0}, {200, 70, INT32_MIN, 100, -1, 0},
{0, 70, 200, 100, -1, 0}, {-5, 70, 200, 100, -1, 0},
{200, 0, 200, 100, -1, 0}, {200, -1, 200, 100, -1, 0},
{INT32_MAX, 70, 10, 100, -1, 0}, {101, 70, 101, 100, -1, 1},
{100, 70, 100, 100, -1, 0}};

static const char		*g_nom[NCASE] = {
	"stride egal a w (limite)", "stride w + 1, derniere colonne",
	"source etroite, derniere colonne", "source etroite, colonne w",
	"source courte, ligne h", "source courte, derniere ligne",
	"stride w - 1 (limite)", "stride 1 inferieur a w",
	"stride 50 inferieur a w", "stride nul", "stride -1",
	"stride INT32_MIN", "w nul", "w negatif", "h nul", "h negatif",
	"w INT32_MAX et stride 10", "source de 101 colonnes, colonne 100",
	"source de 100 colonnes, colonne 100"};

static t_surface	mksrc(const int32_t *c)
{
	t_surface	s;
	int32_t		u;
	int32_t		v;
	size_t		i;

	i = 0;
	while (i < MEM)
		g_mem[i++] = PAD;
	v = 0;
	while (c[2] > 0 && c[2] <= CW + 1 && v < c[1] && v < CH)
	{
		u = 0;
		while (u < c[0] && u < c[2])
		{
			g_mem[(size_t)v * (size_t)c[2] + (size_t)u] = SRC;
			u++;
		}
		v++;
	}
	s.px = g_mem;
	s.w = c[0];
	s.h = c[1];
	s.stride = c[2];
	s.clip = lp_rect(0, 0, c[0], c[1]);
	return (s);
}

static t_color	probe(t_lt *t, const t_surface *src, int32_t x, int32_t y)
{
	t_lunawin	w;
	t_lgeo		g;
	size_t		at;

	w = (t_lunawin){lp_rect(10, 10, CW, CH + 50), WS_CAPTION | WS_SYSMENU,
		"robuste", 0, true, false, 0, 0};
	lt_clear(t, BACK);
	luna_client_band_paint(&t->s, &w, src, FB);
	lg_make(&w, &g);
	at = (size_t)(g.client.y + g.client.h + y) * (size_t)t->s.stride;
	return (t->s.px[at + (size_t)(g.client.x + x)]);
}

static void	band_table(void)
{
	t_lt		t;
	t_surface	src;
	t_color		want;
	size_t		i;

	h_true(lt_open(&t, 280, 160) == 0, "allocation");
	i = 0;
	while (i < NCASE)
	{
		src = mksrc(g_c[i]);
		want = FB;
		if (g_c[i][5] != 0)
			want = SRC;
		h_eq_u64(g_nom[i], probe(&t, &src, g_c[i][3], g_c[i][4]), want);
		h_true(lt_guard_ok(&t), "gardes intactes");
		i++;
	}
	lt_close(&t);
}

static void	band_nulles(void)
{
	t_lt		t;
	t_surface	src;

	h_true(lt_open(&t, 280, 160) == 0, "allocation");
	h_eq_u64("source NULL", probe(&t, NULL, 100, -1), FB);
	src = mksrc(g_c[0]);
	src.px = NULL;
	h_eq_u64("pixels NULL", probe(&t, &src, 100, -1), FB);
	h_true(lt_guard_ok(&t), "gardes intactes");
	lt_close(&t);
}

int	main(void)
{
	h_begin("a17/glass_robuste_bande");
	h_run("bande du client, table de cas limites de la source", band_table);
	h_run("bande du client, sources nulles", band_nulles);
	return (h_end());
}
