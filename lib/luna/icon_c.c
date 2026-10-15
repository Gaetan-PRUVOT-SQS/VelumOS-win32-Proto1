#include "luna_int.h"

static const t_lop		g_search[] = {
{LOP_LINE, 1, {40, 40, 56, 56, 14}, 0xff8b5a2b, 0},
{LOP_DISC, 0, {26, 26, 12}, 0xc0f4faff, 0xc0a7d3f7},
{LOP_RING, 0, {26, 26, 19, 12}, 0xff8e9cbb, 0},
{LOP_DISC, 0, {21, 20, 3}, 0xa0ffffff, 0xa0ffffff}};
static const t_lop		g_help[] = {
{LOP_DISC, 0, {32, 32, 29}, 0xff74b4ff, 0xff1752cc},
{LOP_RING, 0, {32, 32, 29, 26}, 0xffe8f2ff, 0},
{LOP_LINE, 1, {22, 24, 27, 16, 9}, 0xffffffff, 0},
{LOP_LINE, 1, {27, 16, 35, 16, 9}, 0xffffffff, 0},
{LOP_LINE, 1, {35, 16, 41, 22, 9}, 0xffffffff, 0},
{LOP_LINE, 1, {41, 22, 39, 29, 9}, 0xffffffff, 0},
{LOP_LINE, 1, {39, 29, 32, 34, 9}, 0xffffffff, 0},
{LOP_LINE, 1, {32, 34, 32, 39, 9}, 0xffffffff, 0},
{LOP_DISC, 0, {32, 48, 4}, 0xffffffff, 0xffffffff}};
static const t_lop		g_power[] = {
{LOP_DISC, 0, {32, 32, 29}, 0xffff9a7a, 0xffc02b0c},
{LOP_RING, 0, {32, 32, 29, 27}, 0x80ffffff, 0},
{LOP_LINE, 1, {40, 23, 46, 34, 8}, 0xffffffff, 0},
{LOP_LINE, 1, {46, 34, 42, 44, 8}, 0xffffffff, 0},
{LOP_LINE, 1, {42, 44, 32, 48, 8}, 0xffffffff, 0},
{LOP_LINE, 1, {32, 48, 22, 44, 8}, 0xffffffff, 0},
{LOP_LINE, 1, {22, 44, 18, 34, 8}, 0xffffffff, 0},
{LOP_LINE, 1, {18, 34, 24, 23, 8}, 0xffffffff, 0},
{LOP_LINE, 1, {32, 16, 32, 33, 8}, 0xffffffff, 0}};
static const t_lop		g_user[] = {
{LOP_RRECT, 0, {4, 4, 60, 60, 8}, 0xff9fcbff, 0xff3f7fe6},
{LOP_RRECT, 3, {12, 38, 52, 60, 14}, 0xffffffff, 0xffd6e2f8},
{LOP_DISC, 0, {32, 24, 11}, 0xfffff1dc, 0xfff5c896}};
static const t_lop		g_program[] = {
{LOP_RRECT, 0, {4, 8, 60, 56, 5}, 0xff1b3a8f, 0xff1b3a8f},
{LOP_RRECT, 0, {5, 9, 59, 55, 4}, 0xff4d86ee, 0xff2656c8},
{LOP_RECT, 0, {8, 19, 56, 52}, 0xffffffff, 0xffeef2fb},
{LOP_RRECT, 0, {13, 24, 27, 36, 2}, 0xff6fd26c, 0xff2eaa3a},
{LOP_RECT, 0, {31, 25, 51, 28}, 0xff8fa4d6, 0xff8fa4d6},
{LOP_RECT, 0, {31, 32, 51, 35}, 0xff8fa4d6, 0xff8fa4d6},
{LOP_RECT, 0, {13, 41, 51, 44}, 0xffb3c1e4, 0xffb3c1e4}};

static const t_liconent	g_icons_c[] = {
{ICON_SEARCH, g_search, sizeof(g_search) / sizeof(g_search[0])},
{ICON_HELP, g_help, sizeof(g_help) / sizeof(g_help[0])},
{ICON_POWER, g_power, sizeof(g_power) / sizeof(g_power[0])},
{ICON_USER, g_user, sizeof(g_user) / sizeof(g_user[0])},
{ICON_PROGRAM, g_program, sizeof(g_program) / sizeof(g_program[0])},
{ICON_NONE, NULL, 0}};

t_cicon	lp_icons_c(void)
{
	return (g_icons_c);
}
