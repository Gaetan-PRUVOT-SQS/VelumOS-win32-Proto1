#include "harness.h"
#include "velum/libk.h"
#include "render.h"
#include "sh_desk.h"

#define SN 120

static uint32_t	g_px[SN * SN];
static uint32_t	g_wh[SN * SN];
static uint8_t	g_w[SN * SN];

static int32_t	centroid16(const uint8_t *w)
{
	int64_t	sum;
	int64_t	wy;
	int32_t	i;

	sum = 0;
	wy = 0;
	i = 0;
	while (i < SN * SN)
	{
		sum += w[i];
		wy += (int64_t)w[i] * (i / SN);
		i++;
	}
	if (sum == 0)
		return (-1);
	return ((int32_t)(wy * 16 / sum));
}

static void	alpha_de_reference(uint8_t *out, t_iconid id)
{
	t_surface	s;
	int32_t		i;

	gfx_surface_init(&s, g_px, SN, SN);
	gfx_fill(&s, rect_make(0, 0, SN, SN), 0xff000000u);
	luna_icon(&s, (t_point){32, 32}, id, 56);
	gfx_surface_init(&s, g_wh, SN, SN);
	gfx_fill(&s, rect_make(0, 0, SN, SN), 0xffffffffu);
	luna_icon(&s, (t_point){32, 32}, id, 56);
	i = 0;
	while (i < SN * SN)
	{
		out[i] = (uint8_t)(255 - (((g_wh[i] >> 8) & 0xff)
					- ((g_px[i] >> 8) & 0xff)));
		i++;
	}
}

static void	ombre_decalee_de_4_px(void)
{
	t_surface	s;
	uint8_t		ref[SN * SN];
	int32_t		i;
	int32_t		ref_c;

	alpha_de_reference(ref, ICON_PROGRAM);
	ref_c = centroid16(ref);
	gfx_surface_init(&s, g_px, SN, SN);
	gfx_fill(&s, rect_make(0, 0, SN, SN), 0xffffffffu);
	sh_icon_shadow(&s, (t_point){32, 32}, ICON_PROGRAM, 56);
	i = 0;
	while (i < SN * SN)
	{
		g_w[i] = (uint8_t)(255 - ((g_px[i] >> 8) & 0xff));
		i++;
	}
	h_true(ref_c > 0 && centroid16(g_w) > 0, "icone et ombre non vides");
	h_true(centroid16(g_w) - ref_c >= 56 && centroid16(g_w) - ref_c <= 72,
		"barycentre decale de 4 px vers le bas");
	h_eq_u64("loin de l'icone: fond blanc", g_px[4 * SN + 4], 0xffffffffu);
	h_true(g_px[60 * SN + 60] != 0xffffffffu, "sous l'icone: assombri");
}

static void	ombre_bornee_et_tailles(void)
{
	t_surface	parent;
	t_surface	view;
	int32_t		i;
	int32_t		bad;

	gfx_surface_init(&parent, g_px, SN, SN);
	gfx_fill(&parent, rect_make(0, 0, SN, SN), 0xff00ff00u);
	view = gfx_surface_sub(&parent, rect_make(30, 30, 60, 60));
	sh_icon_shadow(&view, (t_point){-10, -10}, ICON_PROGRAM, 56);
	sh_icon_shadow(&view, (t_point){30, 30}, ICON_PROGRAM, 56);
	sh_icon_shadow(&view, (t_point){30, 30}, ICON_PROGRAM, 0);
	sh_icon_shadow(&view, (t_point){30, 30}, ICON_PROGRAM, -4);
	sh_icon_shadow(&view, (t_point){30, 30}, ICON_PROGRAM, SH_SHADOW_MAX + 1);
	sh_icon_shadow(NULL, (t_point){0, 0}, ICON_PROGRAM, 56);
	bad = 0;
	i = 0;
	while (i < SN * SN)
	{
		if ((i % SN < 30 || i % SN >= 90 || i / SN < 30 || i / SN >= 90)
			&& g_px[i] != 0xff00ff00u)
			bad++;
		i++;
	}
	h_eq_i64("aucune ecriture hors de la surface", bad, 0);
}

int	main(void)
{
	h_begin("a20/render-deskshadow");
	h_run("ombre: barycentre decale de 4 px", ombre_decalee_de_4_px);
	h_run("ombre: bornee, tailles invalides", ombre_bornee_et_tailles);
	return (h_end());
}
