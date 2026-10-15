#include "luna_int.h"

static const t_lop		g_logo[] = {
{LOP_DISC, 0, {32, 32, 31}, 0xff9ed4ff, 0xff0b3bb0},
{LOP_DISC, 0, {32, 32, 28}, 0xff4c9bff, 0xff0b4bd0},
{LOP_DISC, 0, {32, 20, 16}, 0x50ffffff, 0x08ffffff},
{LOP_LINE, 1, {46, 18, 32, 43, 22}, 0xffbfe6ff, 0},
{LOP_LINE, 1, {18, 18, 32, 43, 22}, 0xffffffff, 0}};
static const t_lop		g_computer[] = {
{LOP_RRECT, 0, {6, 6, 58, 46, 5}, 0xffedeadf, 0xffb9b5a3},
{LOP_RRECT, 0, {9, 9, 55, 40, 2}, 0xff6e6a5c, 0xff8c8878},
{LOP_RRECT, 0, {11, 11, 53, 38, 1}, 0xff5eb0ff, 0xff1c5ed6},
{LOP_POLY, 3, {11, 11, 40, 11, 11, 30}, 0x50ffffff, 0x10ffffff},
{LOP_RECT, 0, {26, 46, 38, 51}, 0xffb8b4a2, 0xff8f8b79},
{LOP_RRECT, 0, {14, 51, 50, 57, 3}, 0xffdad6c8, 0xffa39f8d}};
static const t_lop		g_folder[] = {
{LOP_RRECT, 3, {5, 12, 27, 24, 3}, 0xffe2a83a, 0xffcc8f22},
{LOP_RRECT, 0, {5, 18, 59, 52, 3}, 0xffe8b546, 0xffd69a28},
{LOP_RECT, 0, {10, 16, 54, 30}, 0xfff6f8ff, 0xffdde3f5},
{LOP_RRECT, 0, {5, 26, 59, 53, 3}, 0xffffe488, 0xfff0b93a},
{LOP_RECT, 0, {7, 27, 57, 29}, 0x90ffffff, 0x30ffffff}};
static const t_lop		g_document[] = {
{LOP_POLY, 5, {13, 4, 41, 4, 53, 16, 53, 60, 13, 60}, 0xff7a88a8, 0xff5a6a90},
{LOP_POLY, 5, {14, 5, 40, 5, 52, 17, 52, 59, 14, 59}, 0xffffffff, 0xffe4eaf8},
{LOP_POLY, 3, {40, 5, 40, 17, 52, 17}, 0xffc8d3ec, 0xff9eafd6},
{LOP_RECT, 0, {19, 12, 33, 16}, 0xff5a86e0, 0xff5a86e0},
{LOP_RECT, 0, {19, 24, 47, 26}, 0xff8fa4d6, 0xff8fa4d6},
{LOP_RECT, 0, {19, 31, 47, 33}, 0xff8fa4d6, 0xff8fa4d6},
{LOP_RECT, 0, {19, 38, 47, 40}, 0xff8fa4d6, 0xff8fa4d6},
{LOP_RECT, 0, {19, 45, 39, 47}, 0xffb3c1e4, 0xffb3c1e4}};

static const t_liconent	g_icons_a[] = {
{ICON_LOGO, g_logo, sizeof(g_logo) / sizeof(g_logo[0])},
{ICON_COMPUTER, g_computer, sizeof(g_computer) / sizeof(g_computer[0])},
{ICON_FOLDER, g_folder, sizeof(g_folder) / sizeof(g_folder[0])},
{ICON_DOCUMENT, g_document, sizeof(g_document) / sizeof(g_document[0])},
{ICON_NONE, NULL, 0}};

t_cicon	lp_icons_a(void)
{
	return (g_icons_a);
}
