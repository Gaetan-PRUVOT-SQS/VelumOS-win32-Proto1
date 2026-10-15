#include <stdio.h>
#include "display_int.h"
#include "kfix.h"

static void	full_screen(void)
{
	char		hex[16];
	t_surface	*s;
	t_rect		all;

	h_true(fx_display(1024, 768), "affichage 1024x768 pret");
	kcon_blue_screen("VelumOS", "test message");
	s = display_surface();
	all = rect_make(0, 0, 1024, 768);
	h_true(fake_count(s, all, BSOD_BLUE) * 5 >= 1024 * 768 * 4,
		"plus de 80 % de pixels bleus");
	h_true(fake_count(s, all, BSOD_WHITE) > 0, "barre de titre blanche");
	h_true(fake_log_has("VelumOS"), "titre");
	h_true(fake_log_has("Code d'arr"), "code d'arret");
	h_true(fake_log_has("test message"), "message");
	h_true(fake_log_has("0123456789abcdef0123456789abcdef01234567"),
		"identifiant de build");
	snprintf(hex, sizeof(hex), "%08X", bsod_hash("test message"));
	h_true(fake_log_has(hex), "somme du message");
	h_true(fake_vmm_guards_ok(), "aucune ecriture hors du mappage");
}

static void	one_size(uint32_t w, uint32_t h, bool draws)
{
	t_surface	*s;

	h_true(fx_display(w, h), "affichage pret");
	kcon_blue_screen("VelumOS", "essai");
	s = display_surface();
	h_true((g_ffont.calls > 0) == draws, "texte selon la taille");
	if (!draws)
		h_eq_u64("fond entierement bleu",
			fake_count(s, rect_make(0, 0, (int32_t)w, (int32_t)h), BSOD_BLUE),
			(uint64_t)w * h);
	h_true(fake_vmm_guards_ok(), "aucune ecriture hors du mappage");
}

static void	small_screens(void)
{
	one_size(1, 1, false);
	one_size(15, 15, false);
	one_size(127, 127, false);
	one_size(128, 127, false);
	one_size(128, 128, true);
}

int	main(void)
{
	h_begin("a13/bsod_paint");
	h_run("ecran d'arret 1024x768", full_screen);
	h_run("ecran d'arret petites surfaces", small_screens);
	return (h_end());
}
