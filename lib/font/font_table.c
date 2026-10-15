#include "font_int.h"

static const t_font *const	g_font_table[FONT_IDS] = {
[FONT_UI] = &g_font_ui,
[FONT_UI_BOLD] = &g_font_ui_bold,
[FONT_TITLE] = &g_font_title,
[FONT_MONO] = &g_font_mono,
[FONT_INS_11] = &g_font_ins_11,
[FONT_INS_12] = &g_font_ins_12,
[FONT_INS_SEMI_12] = &g_font_ins_semi_12,
[FONT_INS_13] = &g_font_ins_13,
[FONT_INS_SEMI_13] = &g_font_ins_semi_13,
[FONT_INS_BOLD_13] = &g_font_ins_bold_13,
[FONT_INS_14] = &g_font_ins_14,
[FONT_INS_SEMI_14] = &g_font_ins_semi_14,
[FONT_INS_BOLD_14] = &g_font_ins_bold_14,
[FONT_INS_15] = &g_font_ins_15,
[FONT_INS_22] = &g_font_ins_22,
[FONT_INS_SEMI_22] = &g_font_ins_semi_22,
[FONT_INS_BOLD_24] = &g_font_ins_bold_24,
[FONT_INS_BOLD_26] = &g_font_ins_bold_26,
[FONT_INS_BOLD_28] = &g_font_ins_bold_28,
[FONT_INS_SEMI_120] = &g_font_ins_semi_120,
[FONT_JBM_14] = &g_font_jbm_14,
};

const t_font	*font_get(t_fontid id)
{
	if ((int)id < 0 || id >= FONT_IDS)
		return (NULL);
	return (g_font_table[id]);
}
