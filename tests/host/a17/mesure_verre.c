#ifdef VELUM_MESURE_VERRE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "ws_core.h"

#define MV_W 1280
#define MV_H 800
#define MV_REPS_DEFAULT 31
#define MV_REPS_MAX 201
#define MV_WARM 3
#define MV_FRAME_NS 16666667ull
#define MV_SEED 0x2468ace1u
#define MV_SCRATCH_PX 131072
#define MV_SIZES 5
#define MV_SCENES 4
#define MV_RECTS 5
#define MV_LAST_SCENE_WINDOWS 128

#ifndef MV_FLAGS
# define MV_FLAGS "non renseigne"
#endif

typedef struct s_mvstat
{
	uint64_t	min;
	uint64_t	q1;
	uint64_t	med;
	uint64_t	q3;
	uint64_t	max;
}	t_mvstat;

typedef struct s_mvsize
{
	int32_t	w;
	int32_t	h;
}	t_mvsize;

typedef struct s_mvkind
{
	const char	*name;
	t_glasskind	kind;
}	t_mvkind;

static const t_mvsize	g_size[MV_SIZES] = {{1280, 80}, {540, 480},
	{1280, 800}, {380, 290}, {800, 40}};

static const t_mvkind	g_kind[LUNA_GLASS_KINDS] = {
{"DOCK", LUNA_GLASS_DOCK}, {"LAUNCHER", LUNA_GLASS_LAUNCHER},
{"PANEL", LUNA_GLASS_PANEL}, {"CARD", LUNA_GLASS_CARD},
{"WINDOW", LUNA_GLASS_WINDOW}, {"CHIP", LUNA_GLASS_CHIP},
{"FRAME", LUNA_GLASS_FRAME}, {"CONSOLE", LUNA_GLASS_CONSOLE},
{"FLOAT", LUNA_GLASS_FLOAT}};

static uint32_t	g_bg[MV_W * MV_H];
static uint32_t	g_px[MV_W * MV_H];
static uint32_t	g_content_px[MV_W * MV_H];
static uint32_t	g_scratch[MV_SCRATCH_PX];
static uint64_t	g_sample[MV_REPS_MAX];
static t_wtable	g_table;
static int32_t	g_reps;

static uint64_t	now_ns(void)
{
	struct timespec	ts;

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
	return ((uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec);
}

static int	cmp_u64(const void *a, const void *b)
{
	uint64_t	x;
	uint64_t	y;

	x = *(const uint64_t *)a;
	y = *(const uint64_t *)b;
	return ((x > y) - (x < y));
}

static t_mvstat	stat_make(void)
{
	t_mvstat	s;

	qsort(g_sample, (size_t)g_reps, sizeof(uint64_t), cmp_u64);
	s.min = g_sample[0];
	s.q1 = g_sample[g_reps / 4];
	s.med = g_sample[g_reps / 2];
	s.q3 = g_sample[(g_reps * 3) / 4];
	s.max = g_sample[g_reps - 1];
	return (s);
}

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
	int32_t		x;
	int32_t		y;
	uint32_t	rgb;

	st = MV_SEED;
	y = -1;
	while (++y < MV_H)
	{
		x = -1;
		while (++x < MV_W)
		{
			rgb = ((uint32_t)(x * 255 / MV_W) << 16)
				| ((uint32_t)(y * 255 / MV_H) << 8)
				| (uint32_t)((x + y) * 255 / (MV_W + MV_H));
			if (((x / 96) + (y / 64)) % 3 == 0)
				rgb ^= 0x00404040u;
			g_bg[y * MV_W + x] = 0xff000000u
				| (rgb ^ (next_rand(&st) & 0x00070707u));
		}
	}
	memcpy(g_content_px, g_bg, sizeof(g_content_px));
}

static void	surface_make(t_surface *s)
{
	s->px = g_px;
	s->w = MV_W;
	s->h = MV_H;
	s->stride = MV_W;
	s->clip = rect_make(0, 0, MV_W, MV_H);
}

static uint64_t	surface_hash(t_rect r)
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
			h ^= g_px[y * MV_W + x];
			h *= 0x100000001b3ull;
		}
	}
	return (h);
}

static uint64_t	area(t_rect r)
{
	if (r.w <= 0 || r.h <= 0)
		return (0);
	return ((uint64_t)r.w * (uint64_t)r.h);
}

static uint64_t	glass_run(t_glasskind kind, t_rect r, bool dark,
				bool reduced)
{
	t_surface	s;
	int32_t		i;
	uint64_t	t0;

	surface_make(&s);
	luna_theme_set(LUNA_THEME_LIGHT);
	if (dark)
		luna_theme_set(LUNA_THEME_DARK);
	luna_transparency_set(reduced);
	i = -MV_WARM;
	while (i < g_reps)
	{
		memcpy(g_px, g_bg, sizeof(g_px));
		t0 = now_ns();
		luna_glass(&s, r, kind);
		if (i >= 0)
			g_sample[i] = now_ns() - t0;
		i++;
	}
	luna_theme_set(LUNA_THEME_LIGHT);
	luna_transparency_set(false);
	return (surface_hash(r));
}

static uint64_t	glass_hash_exact(t_glasskind kind, t_rect r, size_t px)
{
	t_surface	s;

	surface_make(&s);
	memcpy(g_px, g_bg, sizeof(g_px));
	luna_glass_scratch(g_scratch, px);
	luna_glass(&s, r, kind);
	luna_glass_scratch(g_scratch, MV_SCRATCH_PX);
	return (surface_hash(r));
}

static const char	*yes_no(bool v)
{
	if (v)
		return ("oui");
	return ("NON");
}

static void	glass_case(const t_mvkind *k, const t_mvsize *z)
{
	t_rect		r;
	t_mvstat	lt;
	t_mvstat	dk;
	t_mvstat	fb;
	uint64_t	h_blur;
	uint64_t	h_fall;
	size_t		need;

	r = rect_make((MV_W - z->w) / 2, (MV_H - z->h) / 2, z->w, z->h);
	need = luna_glass_need(r);
	h_blur = glass_run(k->kind, r, false, false);
	lt = stat_make();
	glass_run(k->kind, r, true, false);
	dk = stat_make();
	h_fall = glass_run(k->kind, r, false, true);
	fb = stat_make();
	printf("| %-8s | %4dx%-4d | %7zu | %8zu | %9.1f | %9.1f | %6.1f | "
		"%9.1f | %6.1f | %5.1f | %-3s | %-3s |\n", k->name, z->w, z->h,
		need, need * 4, (double)lt.med / 1e3, (double)lt.min / 1e3,
		(double)(lt.q3 - lt.q1) / 1e3, (double)dk.med / 1e3,
		(double)fb.med / 1e3, (double)lt.med / (double)area(r),
		yes_no(h_blur != h_fall),
		yes_no(glass_hash_exact(k->kind, r, need) == h_blur));
}

static void	glass_matrix(void)
{
	int32_t	k;
	int32_t	z;

	printf("\n## A. luna_glass seul (us, temps CPU du thread, %d "
		"repetitions apres %d d'echauffement)\n\n", g_reps, MV_WARM);
	printf("| type | taille | need px | need octets | clair med | clair min "
		"| IQR | sombre med | repli med | ns/px | flou | need suffit |\n");
	printf("|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---|---|\n");
	k = -1;
	while (++k < LUNA_GLASS_KINDS)
	{
		z = -1;
		while (++z < MV_SIZES)
			glass_case(&g_kind[k], &g_size[z]);
	}
}

static void	scene_add(t_rect r)
{
	int	slot;

	slot = wt_alloc(&g_table, 0);
	if (slot < 0)
		return ;
	g_table.w[slot].style = WS_DEFAULT;
	g_table.w[slot].rect = r;
	g_table.w[slot].content.px = g_content_px;
	g_table.w[slot].content.w = MV_W;
	g_table.w[slot].content.h = MV_H;
	g_table.w[slot].content.stride = MV_W;
	g_table.w[slot].content.clip = rect_make(0, 0, MV_W, MV_H);
	wz_insert(&g_table, slot);
}

static void	scene_build(int32_t scene)
{
	uint32_t	st;
	int32_t		i;

	wt_init(&g_table, rect_make(0, 0, MV_W, MV_H));
	g_table.work = g_table.screen;
	st = MV_SEED;
	if (scene == 0)
		scene_add(rect_make(320, 200, 640, 400));
	i = -1;
	while (scene == 1 && ++i < 3)
		scene_add(rect_make(100 + 200 * i, 60 + 100 * i, 640, 400));
	i = -1;
	while (scene == 2 && ++i < 8)
		scene_add(rect_make(40 + 70 * i, 30 + 50 * i, 560, 360));
	i = -1;
	while (scene == 3 && ++i < MV_LAST_SCENE_WINDOWS)
	{
		scene_add(rect_make((int32_t)(next_rand(&st) % 1100),
				(int32_t)(next_rand(&st) % 700),
				112 + (int32_t)(next_rand(&st) % 300),
				90 + (int32_t)(next_rand(&st) % 200)));
	}
}

static t_rect	top_rect(void)
{
	return (g_table.w[g_table.z[g_table.nz - 1]].rect);
}

static t_rect	dirty_rect(int32_t which)
{
	t_rect	top;

	top = top_rect();
	if (which == 0)
		return (rect_make(8, 8, WS_CURSOR_W, WS_CURSOR_H));
	if (which == 1)
		return (rect_make(top.x + top.w / 2, top.y + top.h / 2,
				WS_CURSOR_W, WS_CURSOR_H));
	if (which == 2)
		return (rect_make(top.x + top.w / 2, top.y + 4,
				WS_CURSOR_W, WS_CURSOR_H));
	if (which == 3)
		return (top);
	return (g_table.screen);
}

static const char	*g_rect_name[MV_RECTS] = {"curseur bureau",
	"curseur client", "curseur legende", "fenetre du dessus",
	"plein ecran"};

static t_mvstat	extend_run(t_rect d, t_rect *out)
{
	int32_t	i;
	uint64_t	t0;

	i = -MV_WARM;
	while (i < g_reps)
	{
		t0 = now_ns();
		*out = wgl_extend(&g_table, d);
		if (i >= 0)
			g_sample[i] = now_ns() - t0;
		i++;
	}
	return (stat_make());
}

static t_mvstat	compose_run(t_rect d)
{
	t_surface	s;
	t_wcursor	c;
	int32_t		i;
	uint64_t	t0;

	surface_make(&s);
	c.pos = (t_point){0, 0};
	c.shape = 0;
	c.visible = false;
	i = -MV_WARM;
	while (i < g_reps)
	{
		memcpy(g_px, g_bg, sizeof(g_px));
		t0 = now_ns();
		wc_compose(&g_table, &s, c, d);
		if (i >= 0)
			g_sample[i] = now_ns() - t0;
		i++;
	}
	return (stat_make());
}

static void	dirty_case(int32_t scene, int32_t which)
{
	t_rect		d;
	t_rect		ext;
	t_mvstat	ex;
	t_mvstat	co;
	t_mvstat	ce;

	d = dirty_rect(which);
	ex = extend_run(d, &ext);
	co = compose_run(d);
	ce = compose_run(ext);
	printf("| %5d | %-18s | %8llu | %8llu | %5.1f | %7.2f | %9.1f | "
		"%9.1f | %9.1f | %5.1f |\n", g_table.nz, g_rect_name[which],
		(unsigned long long)area(d), (unsigned long long)area(ext),
		(double)area(ext) / (double)(area(d) + (area(d) == 0)),
		(double)ex.med / 1e3, (double)co.med / 1e3, (double)ce.med / 1e3,
		(double)ce.q3 / 1e3 - (double)ce.q1 / 1e3,
		(double)ce.med * 100.0 / (double)MV_FRAME_NS);
	(void)scene;
}

static void	dirty_matrix(void)
{
	int32_t	scene;
	int32_t	which;

	printf("\n## C. rectangle sale etendu aux bandes de verre (wgl_extend "
		"puis wc_compose, tout en us)\n\n");
	printf("| fenetres | rectangle sale | px sale | px etendu | x | "
		"extend us | compose brut | compose etendu | IQR etendu | "
		"%% image 60 Hz |\n");
	printf("|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|\n");
	luna_glass_scratch(g_scratch, MV_SCRATCH_PX);
	scene = -1;
	while (++scene < MV_SCENES)
	{
		scene_build(scene);
		which = -1;
		while (++which < MV_RECTS)
			dirty_case(scene, which);
	}
}

static void	need_table(void)
{
	t_rect	r;
	int32_t	z;

	printf("\n## B. tampon de travail (luna_glass_need, en pixels de 4 "
		"octets)\n\n");
	printf("| taille | px | octets |\n|---|---:|---:|\n");
	z = -1;
	while (++z < MV_SIZES)
	{
		r = rect_make(0, 0, g_size[z].w, g_size[z].h);
		printf("| %dx%d | %zu | %zu |\n", g_size[z].w, g_size[z].h,
			luna_glass_need(r), luna_glass_need(r) * 4);
	}
	r = rect_make(0, 0, MV_W, 80);
	printf("| tampon serveur (srv_glass : need(largeur x 80)) 1280 | %zu | "
		"%zu |\n", luna_glass_need(r), luna_glass_need(r) * 4);
}

static void	copy_table(void)
{
	t_surface	dst;
	t_surface	src;
	t_blit		b;
	t_mvstat	st;
	int32_t		z;
	int32_t		i;
	uint64_t	t0;

	printf("\n## D. copie d'un cache de verre (gfx_blit, us)\n\n");
	printf("| taille | octets | med | min | IQR |\n|---|---:|---:|---:|---:|\n");
	surface_make(&dst);
	src = dst;
	src.px = g_content_px;
	z = -1;
	while (++z < MV_SIZES)
	{
		b.dst = &dst;
		b.src = &src;
		b.sr = rect_make(0, 0, g_size[z].w, g_size[z].h);
		b.dr = rect_make((MV_W - g_size[z].w) / 2,
				(MV_H - g_size[z].h) / 2, g_size[z].w, g_size[z].h);
		i = -MV_WARM;
		while (i < g_reps)
		{
			t0 = now_ns();
			gfx_blit(&b);
			if (i >= 0)
				g_sample[i] = now_ns() - t0;
			i++;
		}
		st = stat_make();
		printf("| %dx%d | %d | %.1f | %.1f | %.1f |\n", g_size[z].w,
			g_size[z].h, g_size[z].w * g_size[z].h * 4, (double)st.med / 1e3,
			(double)st.min / 1e3, (double)(st.q3 - st.q1) / 1e3);
	}
}

static void	machine_print(void)
{
	FILE	*f;
	char	line[160];

	printf("# Mesure du verre : sortie brute\n\n");
	f = fopen("/proc/cpuinfo", "r");
	while (f != NULL && fgets(line, sizeof(line), f) != NULL)
	{
		if (strncmp(line, "model name", 10) == 0)
		{
			printf("cpu :%s", strchr(line, ':') + 1);
			break ;
		}
	}
	if (f != NULL)
		fclose(f);
	printf("coeurs en ligne : %ld\n", sysconf(_SC_NPROCESSORS_ONLN));
	f = fopen("/proc/loadavg", "r");
	if (f != NULL && fgets(line, sizeof(line), f) != NULL)
		printf("charge au depart : %s", line);
	if (f != NULL)
		fclose(f);
	printf("compilateur : %s\nflags : %s\nsurface %dx%d, fond "
		"deterministe (graine 0x%08x)\n", __VERSION__, MV_FLAGS, MV_W,
		MV_H, MV_SEED);
}

int	main(int argc, char **argv)
{
	g_reps = MV_REPS_DEFAULT;
	if (argc > 1)
		g_reps = atoi(argv[1]);
	if (g_reps < 5 || g_reps > MV_REPS_MAX)
		g_reps = MV_REPS_DEFAULT;
	bg_fill();
	luna_glass_scratch(g_scratch, MV_SCRATCH_PX);
	machine_print();
	glass_matrix();
	need_table();
	copy_table();
	dirty_matrix();
	return (0);
}

#endif
