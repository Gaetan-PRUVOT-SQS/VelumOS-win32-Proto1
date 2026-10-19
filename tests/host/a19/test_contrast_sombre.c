#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	champ_et_liste_en_theme_sombre(void)
{
	double	r;

	luna_theme_set(LUNA_THEME_DARK);
	r = fake_contrast(ctl_color_text(true), ctl_color_back(true));
	h_true(r >= 4.5, "texte sur fond de champ et de liste, theme sombre");
	r = fake_contrast(luna_color_accent(), ctl_color_back(true));
	h_true(r >= 3.0, "anneau accent sur le fond de liste, theme sombre");
	h_true(ctl_color_back(true) != 0xffffffff, "le fond n'est plus blanc");
	luna_theme_set(LUNA_THEME_LIGHT);
	h_eq_u64("clair inchange", ctl_color_back(true), 0xffffffffu);
}

int	main(void)
{
	h_begin("a19/contrast-sombre");
	h_run("champs et listes lisibles en theme sombre",
		champ_et_liste_en_theme_sombre);
	return (h_end());
}
