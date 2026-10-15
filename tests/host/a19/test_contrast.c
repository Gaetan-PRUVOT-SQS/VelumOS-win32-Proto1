#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	calculator(void)
{
	double	r;

	r = fake_contrast(0xff000000, 0xffffffff);
	h_true(r > 20.99 && r < 21.01, "noir sur blanc : 21:1");
	r = fake_contrast(0xff808080, 0xff808080);
	h_true(r > 0.999 && r < 1.001, "identiques : 1:1");
	r = fake_contrast(0xff767676, 0xffffffff);
	h_true(r > 4.5 && r < 4.6, "#767676 sur blanc : 4,54:1 (valeur publiee)");
	r = fake_contrast(0xffffffff, 0xff767676);
	h_true(r > 4.5 && r < 4.6, "symetrique");
	r = fake_contrast(0xff777777, 0xffffffff);
	h_true(r > 4.4 && r < 4.5, "#777777 sur blanc : 4,48:1 (sous le seuil)");
}

static void	text_colors_aa(void)
{
	double	r;

	r = fake_contrast(ctl_color_text(true), luna_color_window());
	h_true(r >= 4.5, "texte sur fond de fenetre (WCAG 1.4.3)");
	r = fake_contrast(ctl_color_text(true), ctl_color_back(true));
	h_true(r >= 4.5, "texte sur fond de champ et de liste");
	r = fake_contrast(ctl_color_selected_text(), luna_color_selection());
	h_true(r >= 4.5, "texte blanc sur selection");
	r = fake_contrast(luna_color_text(), fake_menu_hot_bg());
	h_true(r >= 4.5, "encre de menu sur survol accent 12 %");
	r = fake_contrast(ctl_color_text(true), ctl_color_track());
	h_true(r >= 4.5, "fleches d'ascenseur sur la piste");
}

static void	disabled_and_graphics(void)
{
	double	r;

	r = fake_contrast(ctl_color_text(false), luna_color_window());
	h_true(r >= 3.0, "texte desactive lisible : >= 3:1 (exempte de 1.4.3)");
	r = fake_contrast(ctl_color_text(false), ctl_color_back(false));
	h_true(r >= 3.0, "champ desactive");
	r = fake_contrast(luna_color_accent(), luna_color_window());
	h_true(r >= 3.0, "anneau accent sur le fond (WCAG 1.4.11)");
	r = fake_contrast(luna_color_accent(), ctl_color_back(true));
	h_true(r >= 3.0, "anneau accent sur le fond de liste");
	r = fake_contrast(ctl_color_selected_text(), luna_color_selection());
	h_true(r >= 3.0, "anneau sur une ligne selectionnee");
	r = fake_contrast(ctl_color_text(true), ctl_color_text(false));
	h_true(r > 1.0, "actif et desactif se distinguent");
}

int	main(void)
{
	h_begin("a19/contrast");
	h_run("calcul du rapport de luminance (valeurs publiees)", calculator);
	h_run("texte actif : AA (4,5:1)", text_colors_aa);
	h_run("texte desactive et graphiques : 3:1", disabled_and_graphics);
	return (h_end());
}
