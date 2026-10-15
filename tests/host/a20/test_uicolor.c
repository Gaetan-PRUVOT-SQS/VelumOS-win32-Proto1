#include <stddef.h>
#include <stdint.h>
#include "harness.h"
#include "uicolor.h"

static const char *const	g_names[] = {"UI_INK", "UI_BLACK", "UI_SHADOW",
	"UI_INK_2", "UI_MUTED", "UI_DIM", "UI_DANGER", "UI_ACCENT",
	"UI_ACCENT_HI", "UI_ACCENT_TEXT", "UI_INK_D", "UI_INK_2_D",
	"UI_MUTED_D", "UI_DIM_D", "UI_DANGER_D", "UI_ACCENT_TEXT_D",
	"UI_MENU_TEXT_RIGHT", "UI_LABEL_BG", "UI_LABEL_BG_D",
	"UI_LABEL_PLATE", "UI_PROMPT_PLATE", "UI_WHITE"};
static const uint32_t		g_got[] = {UI_INK, UI_BLACK, UI_SHADOW, UI_INK_2,
	UI_MUTED, UI_DIM, UI_DANGER, UI_ACCENT, UI_ACCENT_HI, UI_ACCENT_TEXT,
	UI_INK_D, UI_INK_2_D, UI_MUTED_D, UI_DIM_D, UI_DANGER_D,
	UI_ACCENT_TEXT_D, UI_MENU_TEXT_RIGHT, UI_LABEL_BG, UI_LABEL_BG_D,
	UI_LABEL_PLATE, UI_PROMPT_PLATE, UI_WHITE};
static const uint32_t		g_want[] = {0xff141824U, 0xff141824U, 0xff141824U,
	0xff2c3242U, 0xff4a5163U, 0xffaab2c3U, 0xffb3361fU, 0xff0f7d83U,
	0xff2bb3b1U, 0xff0f7d83U, 0xfff1f3f8U, 0xffc9d0dcU, 0xffaab2c3U,
	0xff5d667aU, 0xffff9a7aU, 0xff4fd1ccU, 0xff2c3242U, 0x80ffffffU,
	0x80141824U, 0xc70e111aU, 0xc70e111aU, 0xffffffffU};

static void	uicolor_table_de_la_maquette(void)
{
	size_t	i;

	i = 0;
	while (i < sizeof(g_got) / sizeof(g_got[0]))
	{
		h_eq_u64(g_names[i], g_got[i], g_want[i]);
		i++;
	}
}

static void	uicolor_fonds_d_etiquette_derives_du_theme(void)
{
	h_eq_u64("fond clair: alpha 50 %", UI_LABEL_BG >> 24, 0x80);
	h_eq_u64("fond clair: rgb du blanc", UI_LABEL_BG & 0xffffffU,
		UI_WHITE & 0xffffffU);
	h_eq_u64("fond sombre: alpha 50 %", UI_LABEL_BG_D >> 24, 0x80);
	h_eq_u64("fond sombre: rgb de VT_INK", UI_LABEL_BG_D & 0xffffffU,
		VT_INK & 0xffffffU);
	h_eq_u64("accent de texte clair = accent du theme", UI_ACCENT_TEXT,
		VT_ACCENT);
	h_eq_u64("accent de texte sombre = jeton sombre", UI_ACCENT_TEXT_D,
		VT_ACCENT_TEXT_DARK);
}

int	main(void)
{
	h_begin("a20/uicolor");
	h_run("uicolor: valeurs de la maquette (README, section 2)",
		uicolor_table_de_la_maquette);
	h_run("uicolor: derives du theme",
		uicolor_fonds_d_etiquette_derives_du_theme);
	return (h_end());
}
