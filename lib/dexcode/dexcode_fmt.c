#include "dexcode_int.h"

static const t_dfmtinfo	g_dfmts[DF_COUNT] = {{0, 0, 0, 0, 0, 0, 0, 0},
{1, 0, 0, 0, 0, 0, 0, 0},
{1, DS_A4, DS_B4, 0, 0, 0, 0, 3},
{1, DS_A4, 0, 0, DL_B4, 0, 0, 1},
{1, DS_AA, 0, 0, 0, 0, 0, 1},
{1, 0, 0, 0, DL_AA, 0, 0, 0},
{2, 0, 0, 0, DL_W1, 0, 0, 0},
{2, DS_AA, DS_W1, 0, 0, 0, 0, 3},
{2, DS_AA, 0, 0, DL_W1, 0, 0, 1},
{2, DS_AA, 0, 0, DL_W1, 0, 0, 1},
{2, DS_AA, 0, 0, DL_W1, 0, 0, 1},
{2, DS_AA, 0, 0, 0, DS_W1, 0, 1},
{2, DS_AA, DS_W1L, DS_W1H, 0, 0, 0, 7},
{2, DS_AA, DS_W1L, 0, DL_W1H, 0, 0, 3},
{2, DS_A4, DS_B4, 0, DL_W1, 0, 0, 3},
{2, DS_A4, DS_B4, 0, DL_W1, 0, 0, 3},
{2, DS_A4, DS_B4, 0, 0, DS_W1, 0, 3},
{3, 0, 0, 0, DL_32, 0, 0, 0},
{3, DS_W1, DS_W2, 0, 0, 0, 0, 3},
{3, DS_AA, 0, 0, DL_32, 0, 0, 1},
{3, DS_AA, 0, 0, DL_32, 0, 0, 1},
{3, DS_AA, 0, 0, 0, DS_L32, 0, 1},
{3, DS_B4, 0, 0, 0, DS_W1, DS_B4, DR_LIST},
{3, DS_AA, 0, DS_W2, 0, DS_W1, DS_AA, DR_RANGE},
{4, DS_B4, 0, 0, 0, DS_W1, DS_B4, DR_LIST},
{4, DS_AA, 0, DS_W2, 0, DS_W1, DS_AA, DR_RANGE},
{5, DS_AA, 0, 0, DL_64, 0, 0, 1}
};

const t_dfmtinfo	*dexcode_fmtinfo(uint8_t fmt)
{
	if (fmt >= DF_COUNT)
		return (&g_dfmts[DF_NONE]);
	return (&g_dfmts[fmt]);
}

const char	*dexcode_name(uint8_t op)
{
	return (g_dops[op].name);
}

uint8_t	dexcode_format(uint8_t op)
{
	return (g_dops[op].fmt);
}

uint8_t	dexcode_index_kind(uint8_t op)
{
	return (g_dops[op].kind);
}

uint16_t	dexcode_flags(uint8_t op)
{
	return (g_dops[op].flags);
}
