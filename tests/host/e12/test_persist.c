#include <string.h>
#include "../harness.h"
#include "fake_vfile.h"
#include "velum/err.h"
#include "velum/settings.h"

static const char	g_dark[] = "theme=sombre\ntransparence=reduite\n"
	"clavier=qwerty\nresolution=1920x1080\n";
static const char	g_light[] = "theme=clair\ntransparence=normale\n"
	"clavier=azerty\nresolution=1280x720\n";
static const char	g_auto[] = "theme=auto\ntransparence=normale\n"
	"clavier=azerty\nresolution=1024x768\n";
static const char	g_tmp[] = "/data/reglages.tmp";

static int	same_file(const char *want)
{
	char	buf[SET_FILE_MAX];
	int		n;

	n = fv_slurp(SET_PATH, buf, sizeof(buf));
	if (n != (int)strlen(want))
		return (0);
	return (memcmp(buf, want, (size_t)n) == 0);
}

static void	fill(t_settings *s, uint32_t theme, uint32_t reduced,
	uint32_t layout, uint32_t mode)
{
	s->theme = theme;
	s->reduced = reduced;
	s->layout = layout;
	s->mode = mode;
}

static void	roundtrip(void)
{
	t_settings	s;
	t_settings	back;

	fv_reset();
	h_eq_i64("fichier depose", fv_plant(SET_PATH, g_dark, strlen(g_dark)), 0);
	h_eq_i64("4 cles relues", set_load(SET_PATH, &s), 4);
	h_eq_i64("sombre relu", s.theme, SET_THEME_DARK);
	h_eq_i64("transparence relue", s.reduced, 1);
	h_eq_i64("qwerty relu", s.layout, SET_LAYOUT_QWERTY);
	h_eq_i64("1920 relu", set_mode_get(s.mode).w, 1920);
	h_true(set_dark_at(&s, 12), "midi reste sombre");
	fill(&s, SET_THEME_LIGHT, 0, SET_LAYOUT_AZERTY, 1);
	h_eq_i64("ecriture clair", set_save(SET_PATH, &s), 0);
	h_eq_i64("un seul fichier", fv_nfiles(), 1);
	h_eq_i64("temporaire efface", fv_slurp(g_tmp, (char *)&back, sizeof(back)),
		E_NOENT);
	h_true(same_file(g_light), "octets clair sur le fichier");
	memset(&back, 0x5a, sizeof(back));
	h_eq_i64("rechargement", set_load(SET_PATH, &back), 4);
	h_true(memcmp(&s, &back, sizeof(s)) == 0, "clair identique apres relecture");
	h_true(!set_dark_at(&back, 23), "clair force apres relecture");
	fill(&s, SET_THEME_AUTO, 0, SET_LAYOUT_AZERTY, 0);
	h_eq_i64("ecriture auto", set_save(SET_PATH, &s), 0);
	h_true(same_file(g_auto), "octets auto sur le fichier");
	h_eq_i64("auto relu", set_load(SET_PATH, &back), 4);
	h_eq_i64("theme auto", back.theme, SET_THEME_AUTO);
	h_true(set_dark_at(&back, 6), "6 h sombre apres relecture");
	h_true(!set_dark_at(&back, 7), "7 h clair apres relecture");
	h_true(!set_dark_at(&back, 18), "18 h clair apres relecture");
	h_true(set_dark_at(&back, 19), "19 h sombre apres relecture");
	h_true(set_dark_at(&back, 0), "minuit sombre apres relecture");
}

static void	absent_and_bounds(void)
{
	t_settings	s;
	char		p91[92];
	char		p92[93];
	char		big[SET_FILE_MAX + 1];

	fv_reset();
	h_eq_i64("absent", set_load(SET_PATH, &s), E_NOENT);
	h_eq_i64("defaut si absent", s.theme, SET_THEME_LIGHT);
	h_eq_i64("reglages NULL", set_load(SET_PATH, NULL), E_INVAL);
	h_eq_i64("chemin de lecture NULL", set_load(NULL, &s), E_INVAL);
	h_eq_i64("theme encore defaut", s.theme, SET_THEME_LIGHT);
	set_default(&s);
	h_eq_i64("sauve NULL", set_save(NULL, &s), E_INVAL);
	h_eq_i64("reglages de sauve NULL", set_save(SET_PATH, NULL), E_INVAL);
	h_eq_i64("rien ecrit", fv_nfiles(), 0);
	s.theme = 3;
	h_eq_i64("theme hors domaine", set_save(SET_PATH, &s), E_INVAL);
	s.theme = SET_THEME_LIGHT;
	s.reduced = 2;
	h_eq_i64("transparence hors domaine", set_save(SET_PATH, &s), E_INVAL);
	s.reduced = 0;
	s.mode = SET_MODES;
	h_eq_i64("mode hors domaine", set_save(SET_PATH, &s), E_INVAL);
	h_eq_i64("toujours rien", fv_nfiles(), 0);
	memset(p91, 'c', 91);
	p91[91] = '\0';
	memset(p92, 'c', 92);
	p92[92] = '\0';
	set_default(&s);
	h_eq_i64("chemin 92", set_save(p92, &s), E_RANGE);
	h_eq_i64("92 ne cree rien", fv_nfiles(), 0);
	h_eq_i64("chemin 91", set_save(p91, &s), 0);
	h_eq_i64("91 cree un fichier", fv_nfiles(), 1);
	memset(big, 'a', sizeof(big));
	fv_reset();
	h_eq_i64("depot max", fv_plant(SET_PATH, big, SET_FILE_MAX), 0);
	h_eq_i64("max relu", set_load(SET_PATH, &s), 0);
	h_eq_i64("max sans cle = defaut", s.theme, SET_THEME_LIGHT);
	h_eq_i64("depot max+1", fv_plant(SET_PATH, big, SET_FILE_MAX + 1), 0);
	h_eq_i64("max+1 refuse", set_load(SET_PATH, &s), E_RANGE);
	h_eq_i64("trop grand = defaut", s.theme, SET_THEME_LIGHT);
}

static void	kept(const char *what)
{
	char	scratch[8];

	h_eq_i64(what, fv_nfiles(), 1);
	h_eq_i64("pas de temporaire", fv_slurp(g_tmp, scratch, sizeof(scratch)),
		E_NOENT);
	h_true(same_file(g_dark), "ancien fichier intact");
}

static void	io_failures(void)
{
	t_settings	s;
	t_settings	back;

	fv_reset();
	h_eq_i64("depot sombre", fv_plant(SET_PATH, g_dark, strlen(g_dark)), 0);
	fill(&s, SET_THEME_LIGHT, 0, SET_LAYOUT_AZERTY, 1);
	fv_open_fail(1);
	h_eq_i64("ouverture refusee", set_save(SET_PATH, &s), E_IO);
	kept("ouverture : un fichier");
	fv_open_fail(0);
	fv_write_budget(0);
	h_eq_i64("ecriture refusee", set_save(SET_PATH, &s), E_IO);
	kept("ecriture : un fichier");
	fv_write_budget(4);
	h_eq_i64("ecriture partielle", set_save(SET_PATH, &s), E_IO);
	kept("partiel : un fichier");
	fv_write_budget(-1);
	fv_fsync_fail(1);
	h_eq_i64("fsync refuse", set_save(SET_PATH, &s), E_IO);
	kept("fsync : un fichier");
	fv_fsync_fail(0);
	fv_rename_fail(1);
	h_eq_i64("renommage refuse", set_save(SET_PATH, &s), E_IO);
	kept("renommage : un fichier");
	fv_rename_fail(0);
	fv_read_budget(0);
	memset(&back, 0, sizeof(back));
	back.theme = 9;
	h_eq_i64("lecture refusee", set_load(SET_PATH, &back), E_IO);
	h_eq_i64("pas de melange", back.theme, SET_THEME_LIGHT);
	fv_read_budget(8);
	back.theme = 9;
	h_eq_i64("lecture partielle", set_load(SET_PATH, &back), E_IO);
	h_eq_i64("coupure avant analyse", back.theme, SET_THEME_LIGHT);
	fv_read_budget(-1);
	h_eq_i64("fichier encore sombre", set_load(SET_PATH, &back), 4);
	h_eq_i64("sombre conserve", back.theme, SET_THEME_DARK);
}
int	main(void)
{
	h_begin("e12/persist");
	h_run("e12-persist-aller-retour", roundtrip);
	h_run("e12-persist-bornes", absent_and_bounds);
	h_run("e12-persist-echecs", io_failures);
	return (h_end());
}
