#include "harness.h"
#include "velum/err.h"
#include "velum/settings.h"
#include "fake_theme_boot.h"
#include "render.h"

static const char	*g_argv[] = {"shell", "Utilisateur", NULL};

static uint32_t	hour_of(uint64_t wall)
{
	return ((uint32_t)((wall / NS_PER_SEC / 3600ull) % 24ull));
}

static uint32_t	dark_of(uint32_t theme, uint32_t hour)
{
	if (theme == SET_THEME_DARK)
		return (1);
	if (theme == SET_THEME_LIGHT)
		return (0);
	if (hour >= SET_NIGHT_FROM || hour < SET_NIGHT_TO)
		return (1);
	return (0);
}

static int	boot_with(t_shell *sh, uint32_t theme, uint32_t reduced, int rc,
	uint64_t extra)
{
	t_settings	cfg;

	theme_boot_reset();
	render_reset(A20_FIXTURE);
	g_fos.now = FOS_T0 + extra;
	cfg.theme = theme;
	cfg.reduced = reduced;
	cfg.layout = SET_LAYOUT_AZERTY;
	cfg.mode = 1;
	theme_boot_arm(rc < 0 ? NULL : &cfg, rc);
	return (sh_boot(sh, 2, (char **)g_argv));
}

static void	expect_sent(uint32_t theme, uint32_t reduced, uint64_t extra)
{
	t_shell		sh;
	uint64_t	wall;
	uint32_t	hour;
	uint32_t	dark;

	wall = WALL_NS + extra;
	hour = hour_of(wall);
	dark = dark_of(theme, hour);
	h_eq_i64("demarrage", boot_with(&sh, theme, reduced, 4, extra), 0);
	h_eq_i64("fichier ouvert", theme_boot_loads(), 1);
	h_eq_str("chemin des reglages", theme_boot_path(), SET_PATH);
	h_eq_i64("message parti", theme_boot_sends(), 1);
	h_eq_u64("dark envoye", theme_boot_dark(), dark);
	h_eq_u64("reduced envoye", theme_boot_reduced(), reduced);
	h_true(sh.desk.id != 0, "bureau cree");
	h_eq_u64("message porte par le bureau", theme_boot_window(), sh.desk.id);
	sh_shutdown(&sh);
}

static void	cas_sombre(void)
{
	expect_sent(SET_THEME_DARK, 1, 0);
}

static void	cas_clair_reduit(void)
{
	expect_sent(SET_THEME_LIGHT, 1, 0);
}

static void	cas_auto_jour(void)
{
	h_eq_u64("14 h, pas la nuit", hour_of(WALL_NS), 14);
	expect_sent(SET_THEME_AUTO, 0, 0);
}

static void	cas_auto_nuit(void)
{
	uint64_t	extra;

	extra = 6ull * 3600ull * NS_PER_SEC;
	h_eq_u64("20 h", hour_of(WALL_NS + extra), 20);
	expect_sent(SET_THEME_AUTO, 1, extra);
}

static void	cas_refuse(int rc)
{
	t_shell	sh;

	h_eq_i64("demarrage", boot_with(&sh, SET_THEME_DARK, 1, rc, 0), 0);
	h_eq_i64("tentative de lecture", theme_boot_loads(), 1);
	h_eq_str("chemin tente", theme_boot_path(), SET_PATH);
	h_eq_i64("aucun message", theme_boot_sends(), 0);
	sh_shutdown(&sh);
}

static void	cas_absent(void)
{
	cas_refuse(E_NOENT);
}

static void	cas_illisible(void)
{
	cas_refuse(E_IO);
}

static void	cas_trop_gros(void)
{
	cas_refuse(E_RANGE);
}

int	main(void)
{
	h_begin("a20/theme-boot");
	h_run("theme boot: sombre et reduit", cas_sombre);
	h_run("theme boot: clair et reduit", cas_clair_reduit);
	h_run("theme boot: auto de jour", cas_auto_jour);
	h_run("theme boot: auto de nuit", cas_auto_nuit);
	h_run("theme boot: fichier absent", cas_absent);
	h_run("theme boot: fichier illisible", cas_illisible);
	h_run("theme boot: fichier hors limite", cas_trop_gros);
	return (h_end());
}
