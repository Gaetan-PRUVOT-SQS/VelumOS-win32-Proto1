#include <stdio.h>
#include <stdlib.h>
#include "display_int.h"
#include "kfix.h"

static void	hostile_message(void)
{
	char	*msg;
	int		i;

	msg = malloc(30001);
	if (!msg)
		return ;
	memset(msg, 'x', 20000);
	msg[20000] = '\0';
	fx_display(1024, 768);
	kcon_blue_screen("VelumOS", msg);
	h_true(g_ffont.calls <= 48, "un message geant tient en moins de 48 lignes");
	i = 0;
	while (i < 10000)
		memcpy(msg + 3 * i++, "ab\n", 3);
	msg[30000] = '\0';
	g_ffont.calls = 0;
	kcon_blue_screen("VelumOS", msg);
	h_true(g_ffont.calls <= 48, "10000 retours a la ligne : toujours borne");
	h_true(fake_vmm_guards_ok(), "aucune ecriture hors du mappage");
	free(msg);
}

static void	no_display(void)
{
	fake_reset();
	display_boot_init();
	h_true(!display_ready(), "pas d'affichage sans framebuffer");
	kcon_blue_screen("VelumOS", "message");
	h_eq_u64("rien dessine", g_ffont.calls, 0);
	h_eq_i64("aucune fenetre mappee", fake_vmm_live_windows(), 0);
}

static void	null_arguments(void)
{
	fx_display(1024, 768);
	kcon_blue_screen(NULL, NULL);
	h_true(g_ffont.calls > 0, "le texte fixe est dessine");
	h_true(fake_log_has("Message"), "etiquette du message");
	h_true(fake_vmm_guards_ok(), "aucune ecriture hors du mappage");
}

static void	code_line(void)
{
	t_gsurf	g;
	t_bsod	b;
	char	hex[16];

	fake_reset();
	fake_gsurf_new(&g, 1024, 768, 1024);
	h_eq_i64("montage", bsod_setup(&b, &g.s, font_get(FONT_MONO)), 0);
	bsod_code(&b, "essai");
	snprintf(hex, sizeof(hex), "%08X", bsod_hash("essai"));
	h_true(fake_log_has("Code d'arr"), "etiquette");
	h_true(fake_log_has(hex), "huit chiffres hexadecimaux");
	h_eq_i64("ligne suivante", b.row, 3);
	fake_gsurf_free(&g);
}

int	main(void)
{
	h_begin("a13/bsod_edge");
	h_run("ecran d'arret message geant", hostile_message);
	h_run("ecran d'arret sans affichage", no_display);
	h_run("ecran d'arret arguments nuls", null_arguments);
	h_run("ecran d'arret ligne de code", code_line);
	return (h_end());
}
