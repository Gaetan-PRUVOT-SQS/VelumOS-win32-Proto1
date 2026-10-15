#include "harness.h"
#include "fake.h"
#include "ctl_int.h"
#include <stdio.h>
#include <stdlib.h>
#include "velum/luna.h"

#define MAQ_RADIO 14
#define MAQ_FOND 0xffece9d8u
#define MAQ_CERCLE_X 10
#define MAQ_CERCLE_Y 13
#define MAQ_CENTRE_X 17
#define MAQ_CENTRE_Y 20
#define MAQ_BORD_CLAIR 0x8a93a8u
#define MAQ_BORD_SOMBRE 0x5d667au
#define MAQ_ACCENT_CLAIR 0x0f7d83u
#define MAQ_ACCENT_SOMBRE 0x2bb3b1u
#define MAQ_OPAQUE_CLAIR 0xfffbfbfcu
#define MAQ_OPAQUE_SOMBRE 0xff141824u

static uint32_t	px(const t_fakeui *u, int x, int y)
{
	return (u->s.px[y * u->s.stride + x]);
}

static t_ctl	*paint_radio(t_fakeui *u, bool on, bool enabled)
{
	t_ctl	*c;

	fake_begin(u, 120, 40);
	c = fake_add(u, fake_spec(CT_RADIO, 1, rect_make(10, 10, 100, 20), ""));
	ctl_set_flag(&u->r, c, CTL_CHECKED, on);
	ctl_set_flag(&u->r, c, CTL_ENABLED, enabled);
	ctl_invalidate(&u->r, c);
	ctl_paint(&u->r);
	return (c);
}

static void	reset_theme(void)
{
	luna_theme_set(LUNA_THEME_LIGHT);
	luna_transparency_set(false);
}

static void	emprise_14(void)
{
	t_fakeui	u;
	int			x;
	int			y;
	int			minx;
	int			maxx;
	int			miny;
	int			maxy;

	reset_theme();
	paint_radio(&u, true, true);
	minx = 1000;
	maxx = -1;
	miny = 1000;
	maxy = -1;
	y = 0;
	while (y < 40)
	{
		x = 0;
		while (x < 120)
		{
			if (px(&u, x, y) != MAQ_FOND)
			{
				minx = x < minx ? x : minx;
				maxx = x > maxx ? x : maxx;
				miny = y < miny ? y : miny;
				maxy = y > maxy ? y : maxy;
			}
			x++;
		}
		y++;
	}
	h_eq_i64("radio : largeur 14", maxx - minx + 1, MAQ_RADIO);
	h_eq_i64("radio : hauteur 14", maxy - miny + 1, MAQ_RADIO);
	h_eq_i64("radio : bord gauche", minx, MAQ_CERCLE_X);
	h_eq_i64("radio : bord haut centre verticalement", miny, MAQ_CERCLE_Y);
	h_eq_i64("radio : coin hors du cercle", px(&u, MAQ_CERCLE_X, MAQ_CERCLE_Y),
		MAQ_FOND);
	fake_done(&u);
}

static void	bord_decoche_clair(void)
{
	t_fakeui	u;
	uint32_t	p;

	reset_theme();
	paint_radio(&u, false, true);
	p = px(&u, MAQ_CERCLE_X, MAQ_CENTRE_Y - 1);
	h_eq_i64("decochee : couleur du bord #8A93A8", p & 0xffffff,
		MAQ_BORD_CLAIR);
	h_true((p >> 24) >= 0xe0, "decochee : premier pixel de bord plein");
	p = px(&u, MAQ_CERCLE_X + 1, MAQ_CENTRE_Y - 1);
	h_true((p >> 24) >= 0x60 && (p >> 24) <= 0xa0,
		"decochee : bord de 1,5 px, second pixel a moitie");
	h_eq_i64("decochee : centre transparent",
		px(&u, MAQ_CENTRE_X, MAQ_CENTRE_Y), MAQ_FOND);
	fake_done(&u);
}

static void	bord_coche_clair(void)
{
	t_fakeui	u;
	int			i;

	reset_theme();
	paint_radio(&u, true, true);
	i = 0;
	while (i < 3)
	{
		h_eq_i64("cochee : bord de 4 px, couleur accent",
			px(&u, MAQ_CERCLE_X + 1 + i, MAQ_CENTRE_Y - 1),
			(0xff000000u | MAQ_ACCENT_CLAIR));
		i++;
	}
	h_eq_i64("cochee : quatrieme pixel de bord",
		px(&u, MAQ_CERCLE_X + 3, MAQ_CENTRE_Y - 1) & 0xffffff,
		MAQ_ACCENT_CLAIR);
	h_eq_i64("cochee : centre de 6 px transparent",
		px(&u, MAQ_CENTRE_X, MAQ_CENTRE_Y), MAQ_FOND);
	h_eq_i64("cochee : centre transparent des le 6e pixel",
		px(&u, MAQ_CERCLE_X + 5, MAQ_CENTRE_Y - 1), MAQ_FOND);
	fake_done(&u);
}

static void	couleurs_sombres(void)
{
	t_fakeui	u;

	luna_theme_set(LUNA_THEME_DARK);
	luna_transparency_set(false);
	paint_radio(&u, false, true);
	h_eq_i64("sombre decochee : #5D667A",
		px(&u, MAQ_CERCLE_X, MAQ_CENTRE_Y - 1) & 0xffffff, MAQ_BORD_SOMBRE);
	fake_done(&u);
	paint_radio(&u, true, true);
	h_eq_i64("sombre cochee : #2BB3B1",
		px(&u, MAQ_CERCLE_X + 2, MAQ_CENTRE_Y - 1), (0xff000000u
			| MAQ_ACCENT_SOMBRE));
	fake_done(&u);
	reset_theme();
}

static void	repli_opaque(void)
{
	t_fakeui	u;

	luna_theme_set(LUNA_THEME_LIGHT);
	luna_transparency_set(true);
	paint_radio(&u, false, true);
	h_eq_i64("repli opaque clair : interieur plein",
		px(&u, MAQ_CENTRE_X, MAQ_CENTRE_Y), MAQ_OPAQUE_CLAIR);
	h_eq_i64("repli opaque : bord toujours #8A93A8",
		px(&u, MAQ_CERCLE_X, MAQ_CENTRE_Y - 1) & 0xffffff, MAQ_BORD_CLAIR);
	h_eq_i64("repli opaque : coin intact", px(&u, MAQ_CERCLE_X, MAQ_CERCLE_Y),
		MAQ_FOND);
	fake_done(&u);
	luna_theme_set(LUNA_THEME_DARK);
	paint_radio(&u, true, true);
	h_eq_i64("repli opaque sombre : interieur plein",
		px(&u, MAQ_CENTRE_X, MAQ_CENTRE_Y), MAQ_OPAQUE_SOMBRE);
	fake_done(&u);
	reset_theme();
}

static void	desactivee(void)
{
	t_fakeui	u;
	uint32_t	a;

	reset_theme();
	paint_radio(&u, true, false);
	a = px(&u, MAQ_CERCLE_X + 2, MAQ_CENTRE_Y - 1) >> 24;
	h_true(a >= 0x60 && a <= 0xa0, "desactivee : bord attenue a moitie");
	fake_done(&u);
}

static t_fakecall	*label_call(t_fakeui *u, uint32_t type, const char *text)
{
	t_ctl	*c;

	fake_begin(u, 160, 40);
	c = fake_add(u, fake_spec(type, 1, rect_make(10, 10, 140, 20), text));
	ctl_invalidate(&u->r, c);
	ctl_paint(&u->r);
	return (fake_log_last(FK_TEXT));
}

static void	libelle_decale(void)
{
	t_fakeui	u;
	int			check_x;
	int			radio_x;

	reset_theme();
	check_x = label_call(&u, CT_CHECK, "Gras")->r.x;
	fake_done(&u);
	radio_x = label_call(&u, CT_RADIO, "Gras")->r.x;
	fake_done(&u);
	h_eq_i64("libelle radio : un pixel de plus que la case", radio_x - check_x,
		1);
}

static void	planche(const char *name, bool dark, bool reduced)
{
	t_fakeui	u;
	t_ctl		*c;
	t_ctl		*groupe;
	const char	*dir;
	char		path[256];
	FILE		*f;
	int			i;
	int32_t		dim[2];

	dir = getenv("VELUM_SORTIE");
	if (!dir)
		return ;
	luna_theme_set(dark ? LUNA_THEME_DARK : LUNA_THEME_LIGHT);
	luna_transparency_set(reduced);
	fake_begin(&u, 120, 24);
	groupe = fake_add(&u, fake_spec(CT_PANEL, 50, rect_make(0, 0, 120, 24),
				""));
	i = 0;
	while (i < 4)
	{
		c = fake_addp(&u, i < 2 ? u.r.root : groupe, fake_spec(CT_RADIO,
					1 + i, rect_make(4 + 28 * i, 2, 20, 20), ""));
		ctl_set_flag(&u.r, c, CTL_CHECKED, (i & 1) != 0);
		ctl_set_flag(&u.r, c, CTL_ENABLED, (i & 2) == 0);
		i++;
	}
	ctl_invalidate(&u.r, u.r.root);
	ctl_paint(&u.r);
	snprintf(path, sizeof(path), "%s/%s.raw", dir, name);
	f = fopen(path, "wb");
	dim[0] = u.s.w;
	dim[1] = u.s.h;
	if (f)
	{
		fwrite(dim, sizeof(dim), 1, f);
		fwrite(u.s.px, sizeof(uint32_t), (size_t)(u.s.w * u.s.h), f);
		fclose(f);
	}
	fake_done(&u);
	reset_theme();
}

static void	planches(void)
{
	planche("radios-clair", false, false);
	planche("radios-sombre", true, false);
	planche("radios-opaque", false, true);
}

int	main(void)
{
	h_begin("a19/radio_dims");
	h_run("radio : emprise 14 x 14 centree", emprise_14);
	h_run("radio decochee : bord 1,5 px #8A93A8", bord_decoche_clair);
	h_run("radio cochee : bord 4 px accent, centre vide", bord_coche_clair);
	h_run("radio : couleurs du theme sombre", couleurs_sombres);
	h_run("radio : repli opaque (transparence reduite)", repli_opaque);
	h_run("radio desactivee : bord attenue", desactivee);
	h_run("radio : libelle apres 14 + espace", libelle_decale);
	h_run("radio : planches de rendu (VELUM_SORTIE)", planches);
	return (h_end());
}
