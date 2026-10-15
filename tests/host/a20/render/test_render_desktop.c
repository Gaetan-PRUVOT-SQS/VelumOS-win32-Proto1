#include "harness.h"
#include "velum/abi/abi_input.h"
#include "velum/abi/abi_syscall.h"
#include "velum/libk.h"
#include "render.h"

static const char	*g_argv[] = {"shell", "Utilisateur", NULL};

static void	bureau_selection_et_double_clic(void)
{
	t_shell	sh;
	t_point	hello;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	hello = rect_center(sh.cells[2]);
	drv_click(&sh, sh.desk.id, hello);
	h_eq_i64("icone selectionnee", sh.selected, 2);
	g_fos.now += 200000000ull;
	drv_click(&sh, sh.desk.id, hello);
	h_eq_u64("double-clic: lancement", g_fos.nspawn, 1);
	h_eq_str("programme", g_fos.path[0], "/system/bin/hello");
	drv_click(&sh, sh.desk.id, (t_point){600, 400});
	h_eq_i64("clic dans le vide deselectionne", sh.selected, -1);
	g_fos.now += 5000000000ull;
	drv_click(&sh, sh.desk.id, rect_center(sh.cells[0]));
	drv_click(&sh, sh.desk.id, rect_center(sh.cells[0]));
	h_eq_u64("deux clics lents ne lancent rien", g_fos.nspawn, 1);
	render_shot("shell_selection");
	sh_shutdown(&sh);
}

static void	bureau_clavier_et_messages(void)
{
	t_shell	sh;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	drv_key(&sh, sh.desk.id, VK_SHIFT);
	h_eq_i64("Maj ne selectionne rien", sh.selected, -1);
	drv_key(&sh, sh.desk.id, VK_DOWN);
	drv_key(&sh, sh.desk.id, VK_DOWN);
	h_eq_i64("deux fois bas: Corbeille", sh.selected, 1);
	drv_key(&sh, sh.desk.id, VK_RETURN);
	h_true(fwm_find("Corbeille") != NULL, "message de la Corbeille");
	render_shot("shell_corbeille");
	drv_key(&sh, fwm_find("Corbeille")->id, VK_ESCAPE);
	h_eq_i64("message ferme", sh.dlg.ready, 0);
	drv_key(&sh, sh.desk.id, VK_HOME);
	drv_key(&sh, sh.desk.id, VK_RETURN);
	h_true(fwm_find("Ordinateur") != NULL, "message de l'Ordinateur");
	sh_shutdown(&sh);
}

int	main(void)
{
	h_begin("a20/render-desktop");
	h_run("rendu bureau: selection et double-clic",
		bureau_selection_et_double_clic);
	h_run("rendu bureau: clavier", bureau_clavier_et_messages);
	return (h_end());
}
