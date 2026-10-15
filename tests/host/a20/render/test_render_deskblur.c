#include "harness.h"
#include "velum/libk.h"
#include "render.h"
#include "sh_desk.h"

#define BN 64

static uint8_t	g_a[BN * BN];
static uint8_t	g_tmp[BN * BN];

static void	flou_uniforme_et_bords(void)
{
	memset(g_a, 200, sizeof(g_a));
	sh_blur_alpha(g_a, g_tmp, BN);
	h_eq_i64("interieur inchange", g_a[32 * BN + 32], 200);
	h_eq_i64("a 13 px du bord", g_a[13 * BN + 32], 200);
	h_true(g_a[0] < 200, "coin: le vide hors surface assombrit");
	h_true(g_a[32 * BN] < 200 && g_a[32 * BN] > g_a[0], "bord entre les deux");
}

static void	flou_bloc_symetrie_et_portee(void)
{
	int32_t	y;
	int32_t	x;

	memset(g_a, 0, sizeof(g_a));
	y = 22;
	while (y < 42)
	{
		x = 22;
		while (x < 42)
			g_a[y * BN + x++] = 255;
		y++;
	}
	sh_blur_alpha(g_a, g_tmp, BN);
	h_eq_i64("miroir horizontal", g_a[31 * BN + 20], g_a[31 * BN + 43]);
	h_eq_i64("miroir vertical", g_a[20 * BN + 31], g_a[43 * BN + 31]);
	h_eq_i64("transposition", g_a[25 * BN + 18], g_a[18 * BN + 25]);
	h_eq_i64("portee: 13 px hors du bloc", g_a[31 * BN + 9], 0);
	h_true(g_a[31 * BN + 21] > g_a[31 * BN + 17], "decroit vers l'exterieur");
	h_true(g_a[31 * BN + 17] > g_a[31 * BN + 12], "decroit encore");
	h_true(g_a[31 * BN + 31] >= 236 && g_a[31 * BN + 31] <= 250,
		"coeur: 255 * (1 - 20/729)^2 = 241, moins les troncatures");
}

static void	flou_entrees_invalides(void)
{
	memset(g_a, 77, sizeof(g_a));
	sh_blur_alpha(NULL, g_tmp, BN);
	sh_blur_alpha(g_a, NULL, BN);
	sh_blur_alpha(g_a, g_tmp, 0);
	sh_blur_alpha(g_a, g_tmp, -3);
	h_eq_i64("tampon intact", g_a[5 * BN + 5], 77);
	g_a[0] = 90;
	sh_blur_alpha(g_a, g_tmp, 1);
	h_eq_i64("une seule case: tout part dans le vide", g_a[0], 0);
}

int	main(void)
{
	h_begin("a20/render-deskblur");
	h_run("flou: uniforme et bords", flou_uniforme_et_bords);
	h_run("flou: bloc, symetrie et portee", flou_bloc_symetrie_et_portee);
	h_run("flou: entrees invalides", flou_entrees_invalides);
	return (h_end());
}
