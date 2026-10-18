#include <string.h>
#include "harness.h"
#include "e15.h"

static t_conscreen	g_s;

static void	retour_a_la_ligne(void)
{
	scr_init(&g_s, 8);
	scr_puts(&g_s, "0123456789", CC_TEXT);
	h_eq_u64("deux lignes", scr_count(&g_s), 2);
	h_eq_str("ligne 1", scr_row(&g_s, 0)->text, "01234567");
	h_eq_str("ligne 2", scr_row(&g_s, 1)->text, "89");
}

static void	cellules_utf8(void)
{
	scr_init(&g_s, 8);
	scr_puts(&g_s, "\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9"
		"\xc3\xa9\xc3\xa9", CC_TEXT);
	h_eq_u64("8 cellules sur la premiere ligne", scr_row(&g_s, 0)->cells, 8);
	h_eq_u64("16 octets", scr_row(&g_s, 0)->bytes, 16);
	h_eq_u64("une cellule sur la suivante", scr_row(&g_s, 1)->cells, 1);
}

static void	effacement(void)
{
	scr_init(&g_s, 20);
	scr_puts(&g_s, "a\nb\nc", CC_TEXT);
	h_eq_u64("trois lignes", scr_count(&g_s), 3);
	scr_clear(&g_s);
	h_eq_u64("une ligne vide", scr_count(&g_s), 1);
	h_eq_u64("vide", scr_row(&g_s, 0)->bytes, 0);
}

static void	anneau_borne(void)
{
	uint32_t	i;
	char		num[8];

	scr_init(&g_s, 20);
	i = 0;
	while (i < CON_ROWS_MAX + 50)
	{
		num[0] = (char)('0' + i / 100);
		num[1] = (char)('0' + i / 10 % 10);
		num[2] = (char)('0' + i % 10);
		num[3] = '\0';
		scr_puts(&g_s, num, CC_TEXT);
		scr_newline(&g_s);
		i++;
	}
	h_eq_u64("borne", scr_count(&g_s), CON_ROWS_MAX);
	h_eq_str("plus ancien conserve", scr_row(&g_s, 0)->text, "051");
	h_eq_str("avant derniere", scr_row(&g_s, CON_ROWS_MAX - 2)->text, "249");
	h_true(scr_row(&g_s, CON_ROWS_MAX) == NULL, "hors borne");
}

static void	colonnes_bornees(void)
{
	scr_init(&g_s, 5);
	h_eq_u64("minimum 8", g_s.cols, 8);
	scr_set_cols(&g_s, 1000);
	h_eq_u64("maximum", g_s.cols, CON_COLS_MAX);
}

static void	octets_bornes(void)
{
	uint32_t	i;

	scr_init(&g_s, CON_COLS_MAX);
	i = 0;
	while (i < CON_ROW_BYTES * 2)
	{
		scr_puts(&g_s, "\xe2\x80\xba", CC_TEXT);
		i++;
	}
	i = 0;
	while (i < scr_count(&g_s))
	{
		h_true(scr_row(&g_s, i)->bytes < CON_ROW_BYTES, "ligne bornee");
		i++;
	}
}

int	main(void)
{
	h_begin("e15_ecran");
	h_run("retour_a_la_ligne", retour_a_la_ligne);
	h_run("cellules_utf8", cellules_utf8);
	h_run("effacement", effacement);
	h_run("anneau_borne", anneau_borne);
	h_run("colonnes_bornees", colonnes_bornees);
	h_run("octets_bornes", octets_bornes);
	return (h_end());
}
