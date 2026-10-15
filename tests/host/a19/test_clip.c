#include <string.h>
#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	set_get_bounds(void)
{
	char	out[CTL_TEXT_MAX];
	char	big[300];

	memset(big, 'z', sizeof(big));
	ctl_clip_clear();
	h_eq_i64("vide", ctl_clip_get(out, sizeof(out)), 0);
	h_eq_i64("255 octets acceptes", ctl_clip_set(big, 255), 0);
	h_eq_i64("256 octets refuses", ctl_clip_set(big, 256), E_RANGE);
	h_eq_i64("relecture de 255", ctl_clip_get(out, sizeof(out)), 255);
	h_eq_i64("NULL avec longueur", ctl_clip_set(NULL, 3), E_INVAL);
	h_eq_i64("NULL sans longueur", ctl_clip_set(NULL, 0), 0);
	h_eq_i64("vide apres set vide", ctl_clip_get(out, sizeof(out)), 0);
	h_eq_i64("tampon NULL", ctl_clip_get(NULL, 4), E_INVAL);
	h_eq_i64("capacite 0", ctl_clip_get(out, 0), E_INVAL);
}

static void	truncated_on_boundary(void)
{
	char	out[16];

	ctl_clip_set("\xc3\xa9\xe2\x82\xac", 5);
	h_eq_i64("capacite 6 : tout", ctl_clip_get(out, 6), 5);
	h_eq_i64("capacite 5 : l'euro ne tient pas", ctl_clip_get(out, 5), 2);
	h_eq_str("seul le e acute", out, "\xc3\xa9");
	h_eq_i64("capacite 3", ctl_clip_get(out, 3), 2);
	h_eq_i64("capacite 2 : rien", ctl_clip_get(out, 2), 0);
	h_eq_i64("capacite 1", ctl_clip_get(out, 1), 0);
	h_eq_str("toujours termine", out, "");
}

static void	shared_between_edits(void)
{
	t_fakeui	u;
	t_ctl		*a;
	t_ctl		*b;

	fake_begin(&u, 200, 80);
	ctl_clip_clear();
	a = fake_edit(&u, "abc");
	b = fake_add(&u, fake_spec(CT_EDIT, 2, rect_make(5, 40, 100, 20), ""));
	fake_press(&u.r, 'A', INPM_CTRL);
	fake_press(&u.r, 'C', INPM_CTRL);
	ctl_focus(&u.r, b);
	fake_press(&u.r, 'V', INPM_CTRL);
	h_eq_str("collage dans un autre champ", b->text, "abc");
	h_eq_str("la source est intacte", a->text, "abc");
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/clip");
	h_run("ctl_clip : bornes et arguments invalides", set_get_bounds);
	h_run("lecture tronquee a une frontiere utf-8", truncated_on_boundary);
	h_run("presse-papiers partage entre deux champs", shared_between_edits);
	return (h_end());
}
