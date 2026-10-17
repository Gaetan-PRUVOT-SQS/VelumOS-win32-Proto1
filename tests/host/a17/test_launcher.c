#include "harness.h"
#include "velum/luna_launcher.h"
#include "luna_int.h"

static const int32_t	g_panel[4] = {12, 431, 540, 289};
static const int32_t	g_search[4] = {30, 449, 504, 48};
static const int32_t	g_title[4] = {30, 515, 504, 16};
static const int32_t	g_tile0[4] = {30, 541, 121, 96};
static const int32_t	g_tile1[4] = {157, 541, 122, 96};
static const int32_t	g_tile2[4] = {285, 541, 121, 96};
static const int32_t	g_tile3[4] = {412, 541, 122, 96};
static const int32_t	g_sep[4] = {30, 655, 504, 1};
static const int32_t	g_avatar[4] = {40, 667, 34, 34};
static const int32_t	g_power[4] = {488, 666, 36, 36};
static const int32_t	g_logoff[4] = {343, 666, 135, 36};
static const int32_t	g_user[4] = {84, 666, 249, 36};

static void	eq_rect(const char *what, t_rect r, const int32_t *w)
{
	h_eq_i64(what, r.x, w[0]);
	h_eq_i64(what, r.y, w[1]);
	h_eq_i64(what, r.w, w[2]);
	h_eq_i64(what, r.h, w[3]);
}

static void	geom_panel(void)
{
	t_launcherlayout	l;
	t_rect				p;

	p = luna_launcher_place(800, 80);
	eq_rect("panneau 540x289, x=12, bas a 80 du bord", p, g_panel);
	h_eq_i64("bas du panneau", p.y + p.h, 800 - 80);
	luna_launcher_layout(p, 4, &l);
	eq_rect("panneau du modele", l.panel, g_panel);
	eq_rect("recherche 48 de haut, marges 18", l.search, g_search);
	eq_rect("titre Epinglees", l.title, g_title);
	eq_rect("tuile 0, grille 4 colonnes 1fr, espacement 6", l.tile[0],
		g_tile0);
	eq_rect("tuile 1", l.tile[1], g_tile1);
	eq_rect("tuile 2", l.tile[2], g_tile2);
	eq_rect("tuile 3, bord droit a la marge de 18", l.tile[3], g_tile3);
	eq_rect("separateur de pied", l.sep, g_sep);
}

static void	geom_foot(void)
{
	t_launcherlayout	l;

	luna_launcher_layout(lp_rect(12, 431, 540, 289), 1, &l);
	eq_rect("avatar de 34", l.avatar, g_avatar);
	eq_rect("bouton rond de 36", l.poweroff, g_power);
	eq_rect("bouton fermer la session", l.logoff, g_logoff);
	eq_rect("nom d'utilisateur", l.user, g_user);
	h_eq_i64("pied a 18 du bas", l.poweroff.y + l.poweroff.h, 720 - 18);
	h_eq_i64("bouton rond a 28 du bord droit", l.panel.x + l.panel.w
		- (l.poweroff.x + l.poweroff.w), 28);
	h_eq_i64("espacement de 10 entre les deux boutons", l.poweroff.x
		- (l.logoff.x + l.logoff.w), 10);
	h_eq_i64("pied : 10 de marge apres le filet", l.poweroff.y - l.sep.y
		- l.sep.h, 10);
	luna_launcher_layout(l.panel, 1, NULL);
}

int	main(void)
{
	h_begin("a17/lanceur_geometrie");
	h_run("panneau et rangee, valeurs du modele", geom_panel);
	h_run("pied du lanceur, valeurs du modele", geom_foot);
	return (h_end());
}
