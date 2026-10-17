#include <stdint.h>
#include <stdio.h>
#include "harness.h"
#include "lt.h"
#include "luna_int.h"

#define BACK 0xff3366ccu
#define SCR 4096
#define NRECT 8
#define NCLIP 9

static uint32_t			g_scr[SCR];

static const int32_t	g_r[NRECT][5] = {
{INT32_MAX - 3, INT32_MAX - 3, 100, 100, 0},
{INT32_MIN, INT32_MIN, INT32_MAX, INT32_MAX, 0},
{16777217, 0, 10, 10, 0}, {-1000, -1000, 10, 10, 0},
{100, 60, 10, 10, 0}, {99, 59, 1, 1, 1},
{-5, -5, INT32_MAX, INT32_MAX, 1}, {-9, -9, 10, 10, 2}};

static const int32_t	g_k[NCLIP][5] = {
{-50, -50, 10, 10, 0}, {0, 0, 0, 0, 0}, {0, 0, -5, 10, 0},
{INT32_MAX - 1, 0, 10, 10, 0}, {INT32_MIN, INT32_MIN, 5, 5, 0},
{50, 30, INT32_MAX, INT32_MAX, 1}, {-100, -100, INT32_MAX, INT32_MAX, 1},
{0, 0, INT32_MAX, INT32_MAX, 1}, {INT32_MAX, INT32_MAX, INT32_MAX,
	INT32_MAX, 0}};

static int32_t	rect_want(int32_t i, int32_t k)
{
	if (k == LUNA_GLASS_CHIP && g_r[i][2] == INT32_MAX)
		return (0);
	return (g_r[i][4]);
}

static void	zone_check(const t_lt *t, uint64_t ref, int32_t want,
	const char *who)
{
	char	name[96];

	if (want == 0)
	{
		snprintf(name, sizeof(name), "rien de peint hors zone, %s", who);
		h_true(lt_hash(&t->s) == ref, name);
	}
	if (want == 1)
	{
		snprintf(name, sizeof(name), "partie visible peinte, %s", who);
		h_true(lt_hash(&t->s) != ref, name);
	}
	h_true(lt_guard_ok(t), "gardes intactes");
}

static void	rect_table(void)
{
	t_lt		t;
	uint64_t	ref;
	int32_t		i;
	int32_t		k;
	char		who[48];

	h_true(lt_open(&t, 100, 60) == 0, "allocation");
	luna_glass_scratch(g_scr, SCR);
	i = -1;
	while (++i < NRECT)
	{
		k = -1;
		while (++k < LUNA_GLASS_KINDS)
		{
			lt_clear(&t, BACK);
			ref = lt_hash(&t.s);
			luna_glass(&t.s, lp_rect(g_r[i][0], g_r[i][1], g_r[i][2],
					g_r[i][3]), (t_glasskind)k);
			snprintf(who, sizeof(who), "rect %d genre %d", i, k);
			zone_check(&t, ref, rect_want(i, k), who);
		}
	}
	luna_glass_scratch(NULL, 0);
	lt_close(&t);
}

static void	clip_table(void)
{
	t_lt		t;
	uint64_t	ref;
	int32_t		i;

	h_true(lt_open(&t, 100, 60) == 0, "allocation");
	luna_glass_scratch(g_scr, SCR);
	i = -1;
	while (++i < NCLIP)
	{
		lt_clear(&t, BACK);
		ref = lt_hash(&t.s);
		t.s.clip = lp_rect(g_k[i][0], g_k[i][1], g_k[i][2], g_k[i][3]);
		luna_glass(&t.s, lp_rect(10, 10, 80, 40), LUNA_GLASS_DOCK);
		zone_check(&t, ref, g_k[i][4], "clip");
	}
	luna_glass_scratch(NULL, 0);
	lt_close(&t);
}

int	main(void)
{
	h_begin("a17/glass_robuste_zone");
	h_run("rectangles hors surface, tables de limites", rect_table);
	h_run("clip hostile, tables de limites", clip_table);
	return (h_end());
}
