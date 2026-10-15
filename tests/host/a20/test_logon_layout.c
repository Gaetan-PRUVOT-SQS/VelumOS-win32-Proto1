#include <stdint.h>
#include "harness.h"
#include "velum/libk.h"
#include "a20_test.h"
#include "layout.h"
#include "logon_layout.h"
#include "a20_logon_check.h"

static void	layout_balayage_tailles_et_nombres(void)
{
	static const int32_t	sizes[][2] = {{640, 480}, {800, 600}, {1024, 768},
	{1280, 800}, {1280, 1024}, {1920, 1080}, {4096, 2160}};
	t_logonlayout			o;
	uint32_t				s;
	uint32_t				c;

	s = 0;
	while (s < sizeof(sizes) / sizeof(sizes[0]))
	{
		c = 0;
		while (c <= 17)
		{
			h_true(logon_layout(sizes[s][0], sizes[s][1], c, &o) >= 0, "ok");
			h_eq_i64("carte de 380", o.card.w, 380);
			check_tiles(&o, lay_rect(0, 0, sizes[s][0], sizes[s][1]));
			check_rest(&o, lay_rect(0, 0, sizes[s][0], sizes[s][1]));
			c++;
		}
		s++;
	}
}

static void	layout_nombre_de_tuiles_affichees(void)
{
	t_logonlayout	o;

	h_eq_i64("aucune", logon_layout(1024, 768, 0, &o), 0);
	h_eq_i64("une", logon_layout(1024, 768, 1, &o), 1);
	h_eq_i64("six tiennent en 1024x768", logon_layout(1024, 768, 6, &o), 6);
	h_eq_i64("plafond a six", logon_layout(1024, 768, 16, &o), 6);
	h_eq_i64("deux tiennent en 640x480", logon_layout(640, 480, 2, &o), 2);
	h_eq_i64("plafond par la hauteur", logon_layout(640, 480, 6, &o), 3);
}

static void	layout_refus_sous_le_minimum(void)
{
	t_logonlayout	o;

	h_eq_i64("largeur 639", logon_layout(639, 480, 1, &o), -34);
	h_eq_i64("hauteur 479", logon_layout(640, 479, 1, &o), -34);
	h_eq_i64("negatif", logon_layout(-5, -5, 1, &o), -34);
	h_eq_i64("zero", logon_layout(0, 0, 1, &o), -34);
	h_eq_i64("sortie nulle", logon_layout(1024, 768, 1, NULL), -34);
}

int	main(void)
{
	h_begin("a20/logon-layout");
	h_run("layout: tailles x nombres de comptes",
		layout_balayage_tailles_et_nombres);
	h_run("layout: tuiles affichees", layout_nombre_de_tuiles_affichees);
	h_run("layout: refus sous le minimum", layout_refus_sous_le_minimum);
	return (h_end());
}
