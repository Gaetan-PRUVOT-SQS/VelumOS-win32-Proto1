#include "harness.h"
#include "velum/libk.h"
#include "velum/abi/abi_syscall.h"
#include "render.h"

static const char	*g_argv[] = {"shell", "Utilisateur", NULL};

static void	lancer(t_shell *sh, const char *path, uint32_t attendu,
	const char *nom)
{
	uint32_t	n;

	n = g_fos.nspawn;
	h_eq_i64(nom, sh_launch(sh, path), 0);
	h_eq_u64("un lancement de plus", g_fos.nspawn, n + 1);
	h_eq_str("chemin transmis", g_fos.path[n], path);
	h_eq_u64("privileges transmis", g_fos.flags[n], attendu);
}

static void	privileges_des_applis_du_systeme(void)
{
	t_shell	sh;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	lancer(&sh, "/system/bin/fichiers", PF_FSWRITE, "Fichiers ecrit");
	lancer(&sh, "/system/bin/reglages", PF_FSWRITE, "Reglages ecrit");
	lancer(&sh, "/system/bin/console", 0, "Console sans privilege");
	lancer(&sh, "/system/bin/hello", 0, "Bonjour sans privilege");
	lancer(&sh, "/system/bin/apkrun", 0, "apkrun sans privilege");
	lancer(&sh, "/system/bin/fichiers2", 0, "nom voisin sans privilege");
	lancer(&sh, "/data/apps/fichiers", 0, "meme nom hors du systeme");
	sh_shutdown(&sh);
}

int	main(void)
{
	h_begin("a20/render-privileges");
	h_run("rendu shell: seuls Fichiers et Reglages recoivent l'ecriture",
		privileges_des_applis_du_systeme);
	return (h_end());
}
