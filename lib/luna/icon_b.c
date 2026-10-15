#include "luna_int.h"

static const t_lop		g_recycle[] = {
{LOP_POLY, 4, {14, 20, 50, 20, 46, 58, 18, 58}, 0xffd5dce8, 0xff8794aa},
{LOP_RRECT, 0, {10, 12, 54, 21, 2}, 0xffbcc6d8, 0xff8794aa},
{LOP_RRECT, 0, {25, 7, 39, 13, 2}, 0xff9aa6bc, 0xff7c899f},
{LOP_RECT, 0, {23, 26, 25, 52}, 0x907f8ca3, 0x907f8ca3},
{LOP_RECT, 0, {31, 26, 33, 52}, 0x907f8ca3, 0x907f8ca3},
{LOP_RECT, 0, {39, 26, 41, 52}, 0x907f8ca3, 0x907f8ca3},
{LOP_LINE, 1, {32, 28, 41, 44, 6}, 0xff2fb344, 0},
{LOP_LINE, 1, {41, 44, 23, 44, 6}, 0xff2fb344, 0},
{LOP_LINE, 1, {23, 44, 32, 28, 6}, 0xff2fb344, 0}};
static const t_lop		g_drive[] = {
{LOP_RRECT, 0, {5, 22, 59, 46, 4}, 0xffe4e8f0, 0xff9aa3b8},
{LOP_RECT, 0, {7, 24, 57, 29}, 0xe0ffffff, 0x60ffffff},
{LOP_RECT, 0, {11, 36, 37, 40}, 0xff59637a, 0xff6b758c},
{LOP_RECT, 0, {46, 36, 53, 40}, 0xff7bea6b, 0xff2fb82f}};
static const t_lop		g_settings[] = {
{LOP_RECT, 0, {27, 5, 37, 59}, 0xff8996b2, 0xff8996b2},
{LOP_RECT, 0, {5, 27, 59, 37}, 0xff8996b2, 0xff8996b2},
{LOP_LINE, 0, {14, 14, 50, 50, 20}, 0xff8996b2, 0},
{LOP_LINE, 0, {50, 14, 14, 50, 20}, 0xff8996b2, 0},
{LOP_DISC, 0, {32, 32, 20}, 0xffdde4f1, 0xff8c99b4},
{LOP_DISC, 0, {32, 32, 8}, 0xff3f4b66, 0xff5b6784}};
static const t_lop		g_run[] = {
{LOP_RRECT, 0, {4, 9, 48, 49, 4}, 0xff5b7bc9, 0xff5b7bc9},
{LOP_RRECT, 0, {5, 10, 47, 48, 3}, 0xffffffff, 0xffe6ebf8},
{LOP_RRECT, 3, {5, 10, 47, 19, 3}, 0xff3f83f4, 0xff1a5cd8},
{LOP_RECT, 0, {10, 25, 34, 27}, 0xff93a5d3, 0xff93a5d3},
{LOP_RECT, 0, {10, 31, 40, 33}, 0xff93a5d3, 0xff93a5d3},
{LOP_RECT, 0, {28, 38, 42, 46}, 0xff6fdc66, 0xff2db23a},
{LOP_POLY, 3, {40, 31, 58, 42, 40, 53}, 0xff7de673, 0xff25a834}};

static const t_liconent	g_icons_b[] = {
{ICON_RECYCLE, g_recycle, sizeof(g_recycle) / sizeof(g_recycle[0])},
{ICON_DRIVE, g_drive, sizeof(g_drive) / sizeof(g_drive[0])},
{ICON_SETTINGS, g_settings, sizeof(g_settings) / sizeof(g_settings[0])},
{ICON_RUN, g_run, sizeof(g_run) / sizeof(g_run[0])},
{ICON_NONE, NULL, 0}};

t_cicon	lp_icons_b(void)
{
	return (g_icons_b);
}
