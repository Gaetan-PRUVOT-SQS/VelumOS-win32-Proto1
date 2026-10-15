#include <stdint.h>
#include "harness.h"
#include "lt_fuzz.h"

#define BACK 0xff3366ccu
#define SENT 0xdeadbeefu
#define SCR 4096
#define PAD 64
#define NITER 3000

static uint32_t			g_scr[SCR + PAD];
static uint64_t			g_seed = 0x9e3779b97f4a7c15ULL;

static size_t	arm(const int32_t *c, t_lt *t)
{
	size_t	need;
	size_t	n;
	size_t	i;

	need = luna_glass_need(lp_rect(c[0], c[1], c[2], c[3]));
	n = SCR;
	if ((c[9] >> 1) % 5 == 0)
		n = 0;
	if ((c[9] >> 1) % 5 == 1 && need > 0)
		n = need - 1;
	if ((c[9] >> 1) % 5 == 2 && need <= SCR)
		n = need;
	i = 0;
	while (i < SCR + PAD)
		g_scr[i++] = SENT;
	luna_glass_scratch(g_scr, n);
	if ((c[9] >> 1) % 5 == 3)
		luna_glass_scratch(NULL, 0);
	luna_transparency_set((c[9] & 1) != 0);
	luna_theme_set((t_lunatheme)((c[9] >> 5) & 1));
	lt_clear(t, BACK);
	t->s.clip = lp_rect(c[4], c[5], c[6], c[7]);
	return (n);
}

static void	one_case(t_lt *t, const int32_t *c)
{
	size_t	n;
	size_t	i;
	t_rect	zone;
	bool	scratch_ok;

	n = arm(c, t);
	luna_glass(&t->s, lp_rect(c[0], c[1], c[2], c[3]), (t_glasskind)c[8]);
	zone = ltf_zone(lp_rect(c[0], c[1], c[2], c[3]), t->s.clip, t->w, t->h);
	if (c[8] < 0 || c[8] >= LUNA_GLASS_KINDS)
		zone = lp_rect(0, 0, 0, 0);
	if ((c[9] >> 1) % 5 == 3)
		n = 0;
	scratch_ok = true;
	i = n;
	while (i < SCR + PAD)
		scratch_ok = scratch_ok && g_scr[i++] == SENT;
	h_true(scratch_ok, "tampon : aucun mot ecrit au dela de la taille donnee");
	t->s.clip = lp_rect(0, 0, t->w, t->h);
	h_true(ltf_surface_ok(t, zone, BACK),
		"verre : rien hors de la zone visible, alpha plein dedans");
}

static void	fuzz(void)
{
	t_lt		t;
	int32_t		c[10];
	int32_t		i;

	h_true(lt_open(&t, 100, 60) == 0, "allocation");
	i = -1;
	while (++i < NITER)
	{
		ltf_case(&g_seed, c);
		one_case(&t, c);
	}
	luna_glass_scratch(NULL, 0);
	luna_transparency_set(false);
	lt_close(&t);
}

int	main(void)
{
	h_begin("a17/glass_fuzz");
	h_run("verre, fuzz graine fixe, modele d'intersection 64 bits", fuzz);
	return (h_end());
}
