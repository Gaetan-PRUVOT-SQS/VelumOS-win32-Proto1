#include <stdio.h>
#include <sys/stat.h>
#include "harness.h"
#include "velum/abi/abi_input.h"
#include "velum/libk.h"
#include "render.h"

static const char	g_path3[] = A20_RENDER "/programmes_trois.txt";
static const char	g_path4[] = A20_RENDER "/programmes_quatre.txt";
static const char	g_text3[] = "Edit;/system/bin/edit;/system/etc/a.txt\n"
	"Calc;/system/bin/calc\nJeu;/system/bin/jeu\n";
static const char	g_text4[] = "Edit;/system/bin/edit;/system/etc/a.txt\n"
	"Calc;/system/bin/calc\nJeu;/system/bin/jeu\nPlus;/system/bin/plus\n";
static const char	*g_argv[] = {"shell", "Utilisateur", NULL};

static void	boot_open(t_shell *sh, const char *path, const char *text)
{
	FILE	*f;

	mkdir(A20_RENDER, 0755);
	f = fopen(path, "wb");
	h_true(f != NULL, "fichier de programmes ecrit");
	if (f)
	{
		fwrite(text, 1, strlen(text), f);
		fclose(f);
	}
	render_reset(path);
	sh_boot(sh, 2, (char **)g_argv);
	drv_key(sh, sh->bar.id, VK_LWIN);
}

static void	pleins_souris(void)
{
	t_shell	sh;
	int32_t	i;

	boot_open(&sh, g_path3, g_text3);
	h_eq_i64("quatre tuiles", sh.sml.ntiles, 4);
	i = 0;
	while (i < sh.sml.ntiles)
	{
		drv_move(&sh, sh.menu.id, rect_center(sh.sml.tile[i]));
		h_eq_i64("survol de la tuile", sh.sm.hot, i);
		i++;
	}
	drv_click(&sh, sh.menu.id, rect_center(sh.sml.tile[3]));
	h_eq_i64("menu ferme", sh.sm.open, 0);
	h_eq_u64("un lancement", g_fos.nspawn, 1);
	h_eq_str("dernier programme", g_fos.path[0], "/system/bin/jeu");
	sh_shutdown(&sh);
}

static void	pleins_clavier(void)
{
	t_shell	sh;

	boot_open(&sh, g_path3, g_text3);
	drv_type(&sh, sh.menu.id, "abc d");
	h_eq_i64("saisie dans la recherche, menu ouvert", sh.sm.open, 1);
	h_eq_i64("pas de focus", sh.sm.focus, SM_NO_ITEM);
	h_eq_u64("aucun lancement", g_fos.nspawn, 0);
	drv_key(&sh, sh.menu.id, VK_ESCAPE);
	h_eq_i64("recherche quittee, menu ouvert", sh.sm.open, 1);
	drv_key(&sh, sh.menu.id, VK_END);
	drv_key(&sh, sh.menu.id, VK_UP);
	drv_key(&sh, sh.menu.id, VK_UP);
	h_eq_i64("derniere tuile", sh.sm.focus, 3);
	drv_key(&sh, sh.menu.id, VK_DOWN);
	h_eq_i64("pied", sh.sm.focus, SM_IDX_LOGOFF);
	drv_key(&sh, sh.menu.id, VK_HOME);
	drv_key(&sh, sh.menu.id, VK_DOWN);
	drv_key(&sh, sh.menu.id, VK_RETURN);
	h_eq_str("programme", g_fos.path[0], "/system/bin/edit");
	h_eq_str("argument", g_fos.arg[0], "/system/etc/a.txt");
	sh_shutdown(&sh);
}

static void	pleins_fichier_trop_long_et_zone_vide(void)
{
	t_shell	sh;
	t_point	vide;

	boot_open(&sh, g_path4, g_text4);
	h_eq_u64("liste integree", sh.progs.count, 1);
	h_eq_i64("deux tuiles", sh.sml.ntiles, 2);
	vide = rect_center(sh.sml.tile[1]);
	vide.x += sh.sml.tile[1].w + sh.sml.tile[1].w / 2 + 10;
	drv_move(&sh, sh.menu.id, vide);
	h_eq_i64("rien sous la tuile absente", sh.sm.hot, SM_NO_ITEM);
	drv_click(&sh, sh.menu.id, vide);
	h_eq_i64("toujours ouvert", sh.sm.open, 1);
	h_eq_u64("aucun lancement", g_fos.nspawn, 0);
	sh_shutdown(&sh);
}

int	main(void)
{
	h_begin("a20/render-menu-pleins");
	h_run("rendu menu: quatre tuiles a la souris", pleins_souris);
	h_run("rendu menu: quatre tuiles au clavier", pleins_clavier);
	h_run("rendu menu: fichier trop long et zone vide",
		pleins_fichier_trop_long_et_zone_vide);
	return (h_end());
}
