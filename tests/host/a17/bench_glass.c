#ifdef VELUM_BENCH_GLASS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "velum/luna.h"
#include "luna_int.h"

#define BG_W 1024
#define BG_H 768
#define BG_CASES 10
#define BG_PHASES 5
#define BG_PASSES_MAX 99
#define BG_PASSES_DEFAULT 9
#define BG_REPS 4
#define BG_SEED 0x2468ace1u

enum
{
	PH_TOTAL,
	PH_DOWN,
	PH_BLUR,
	PH_RIM,
	PH_PIXELS
};

typedef struct s_bgcase
{
	const char	*name;
	t_glasskind	kind;
	t_rect		r;
	bool		dark;
}	t_bgcase;

static const t_bgcase	g_case[BG_CASES] = {
{"dock 1024x80 clair", LUNA_GLASS_DOCK, {0, 688, 1024, 80}, false},
{"dock 1024x80 sombre", LUNA_GLASS_DOCK, {0, 688, 1024, 80}, true},
{"lanceur 540x540 clair", LUNA_GLASS_LAUNCHER, {16, 120, 540, 540}, false},
{"lanceur 540x540 sombre", LUNA_GLASS_LAUNCHER, {16, 120, 540, 540}, true},
{"carte 380x290 clair", LUNA_GLASS_CARD, {322, 240, 380, 290}, false},
{"carte 380x290 sombre", LUNA_GLASS_CARD, {322, 240, 380, 290}, true},
{"fenetre 800x40 clair", LUNA_GLASS_WINDOW, {100, 60, 800, 40}, false},
{"fenetre 800x40 sombre", LUNA_GLASS_WINDOW, {100, 60, 800, 40}, true},
{"puce 180x44 clair", LUNA_GLASS_CHIP, {40, 40, 180, 44}, false},
{"puce 180x44 sombre", LUNA_GLASS_CHIP, {40, 40, 180, 44}, true}};

static uint32_t	g_bg[BG_W * BG_H];
static uint32_t	g_px[BG_W * BG_H];
static uint32_t	g_scr[2 * BG_W * BG_H / 16 + 64];

static uint32_t	next_rand(uint32_t *st)
{
	*st ^= *st << 13;
	*st ^= *st >> 17;
	*st ^= *st << 5;
	return (*st);
}

static void	bg_fill(void)
{
	uint32_t	st;
	uint32_t	n;
	int32_t		x;
	int32_t		y;
	uint32_t	rgb;

	st = BG_SEED;
	y = -1;
	while (++y < BG_H)
	{
		x = -1;
		while (++x < BG_W)
		{
			n = next_rand(&st);
			rgb = ((uint32_t)(x * 255 / BG_W) << 16)
				| ((uint32_t)(y * 255 / BG_H) << 8)
				| (uint32_t)((x + y) * 255 / (BG_W + BG_H));
			if (((x / 96) + (y / 64)) % 3 == 0)
				rgb ^= 0x00404040u;
			g_bg[y * BG_W + x] = 0xff000000u | (rgb ^ (n & 0x00070707u));
		}
	}
}

static double	cpu_ms(void)
{
	struct timespec	ts;

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
	return ((double)ts.tv_sec * 1e3 + (double)ts.tv_nsec / 1e6);
}

static uint64_t	surface_hash(const t_surface *s, t_rect r)
{
	uint64_t	h;
	int32_t		x;
	int32_t		y;

	h = 0xcbf29ce484222325ull;
	y = r.y - 1;
	while (++y < r.y + r.h)
	{
		x = r.x - 1;
		while (++x < r.x + r.w)
		{
			h ^= s->px[(size_t)y * (size_t)s->stride + (size_t)x];
			h *= 0x100000001b3ull;
		}
	}
	return (h);
}

static void	surface_make(t_surface *s)
{
	s->px = g_px;
	s->w = BG_W;
	s->h = BG_H;
	s->stride = BG_W;
	s->clip = lp_rect(0, 0, BG_W, BG_H);
}

static void	phase_blur(t_surface *s, const t_bgcase *c, double *down,
				double *blur)
{
	t_lgb		o;
	t_lglass	g;
	double		t0;

	lp_glass_make(&g, c->r, c->kind);
	memcpy(g_px, g_bg, sizeof(g_px));
	lp_gscratch((size_t)((c->r.w + 3) / 4) * (size_t)((c->r.h + 3) / 4),
		&o.a, &o.b);
	t0 = cpu_ms();
	lp_gb_down(s, c->r, &o);
	*down = cpu_ms() - t0;
	t0 = cpu_ms();
	lp_gb_blur(&o, lp_max(g.blur / LX_SCALE, 1));
	*blur = cpu_ms() - t0;
}

static double	phase_rim(t_surface *s, const t_bgcase *c)
{
	t_lglass	g;
	double		t0;

	lp_glass_make(&g, c->r, c->kind);
	t0 = cpu_ms();
	lp_glass_rim(s, &g);
	return (cpu_ms() - t0);
}

static double	phase_total(t_surface *s, const t_bgcase *c)
{
	double	t0;

	memcpy(g_px, g_bg, sizeof(g_px));
	t0 = cpu_ms();
	luna_glass(s, c->r, c->kind);
	return (cpu_ms() - t0);
}

static void	one_pass(t_surface *s, const t_bgcase *c, double *out)
{
	double	down;
	double	blur;
	int32_t	k;

	memset(out, 0, sizeof(double) * BG_PHASES);
	k = -1;
	while (++k < BG_REPS)
	{
		out[PH_TOTAL] += phase_total(s, c);
		phase_blur(s, c, &down, &blur);
		out[PH_DOWN] += down;
		out[PH_BLUR] += blur;
		out[PH_RIM] += phase_rim(s, c);
	}
	k = -1;
	while (++k < BG_PHASES)
		out[k] /= BG_REPS;
	out[PH_PIXELS] = out[PH_TOTAL] - out[PH_DOWN] - out[PH_BLUR]
		- out[PH_RIM];
}

static int	cmp_double(const void *a, const void *b)
{
	double	x;
	double	y;

	x = *(const double *)a;
	y = *(const double *)b;
	return ((x > y) - (x < y));
}

static double	median(double *v, int32_t n)
{
	qsort(v, (size_t)n, sizeof(double), cmp_double);
	return (v[n / 2]);
}

static void	run_case(t_surface *s, const t_bgcase *c, int32_t passes)
{
	double		val[BG_PHASES][BG_PASSES_MAX];
	double		one[BG_PHASES];
	double		med[BG_PHASES];
	double		mn;
	int32_t		i;
	int32_t		k;

	luna_theme_set(LUNA_THEME_LIGHT);
	if (c->dark)
		luna_theme_set(LUNA_THEME_DARK);
	one_pass(s, c, one);
	i = -1;
	while (++i < passes)
	{
		one_pass(s, c, one);
		k = -1;
		while (++k < BG_PHASES)
			val[k][i] = one[k];
	}
	mn = val[PH_TOTAL][0];
	i = -1;
	while (++i < passes)
		if (val[PH_TOTAL][i] < mn)
			mn = val[PH_TOTAL][i];
	k = -1;
	while (++k < BG_PHASES)
		med[k] = median(val[k], passes);
	phase_total(s, c);
	printf("%-24s total %8.3f (min %8.3f) | reduc %7.3f flou %7.3f "
		"pixels %7.3f rim %6.3f | %6.1f ns/px | %016llx\n", c->name,
		med[PH_TOTAL], mn, med[PH_DOWN], med[PH_BLUR], med[PH_PIXELS],
		med[PH_RIM], med[PH_TOTAL] * 1e6 / ((double)c->r.w * c->r.h),
		(unsigned long long)surface_hash(s, c->r));
}

static void	load_print(const char *when)
{
	FILE	*f;
	char	buf[96];

	f = fopen("/proc/loadavg", "r");
	if (f == NULL)
		return ;
	if (fgets(buf, sizeof(buf), f) != NULL)
	{
		buf[strcspn(buf, "\n")] = '\0';
		printf("charge %s : %s\n", when, buf);
	}
	fclose(f);
}

static void	solo_case(t_surface *s, const t_bgcase *c, int32_t calls)
{
	int32_t	i;

	luna_theme_set(LUNA_THEME_LIGHT);
	if (c->dark)
		luna_theme_set(LUNA_THEME_DARK);
	i = -1;
	while (++i < calls)
		phase_total(s, c);
	printf("%s : %d appels, empreinte %016llx\n", c->name, calls,
		(unsigned long long)surface_hash(s, c->r));
}

int	main(int argc, char **argv)
{
	t_surface	s;
	int32_t		passes;
	int32_t		only;
	int32_t		i;

	passes = BG_PASSES_DEFAULT;
	only = -1;
	if (argc > 1)
		passes = atoi(argv[1]);
	if (argc > 2)
		only = atoi(argv[2]);
	if (passes < 1 || passes > BG_PASSES_MAX)
		passes = BG_PASSES_DEFAULT;
	bg_fill();
	surface_make(&s);
	luna_transparency_set(false);
	luna_glass_scratch(g_scr, sizeof(g_scr) / sizeof(g_scr[0]));
	if (only >= 0 && only < BG_CASES)
	{
		solo_case(&s, &g_case[only], passes);
		return (0);
	}
	printf("bench_glass : %d passes de %d appels, temps CPU du thread (ms), "
		"mediane\n", passes, BG_REPS);
	load_print("avant");
	i = -1;
	while (++i < BG_CASES)
		run_case(&s, &g_case[i], passes);
	load_print("apres");
	return (0);
}

#endif
