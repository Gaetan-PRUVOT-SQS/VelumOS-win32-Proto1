#include "layout.h"
#include "velum/abi/abi_input.h"

static const t_key	g_common_keys[] = {
{0x76, VK_ESCAPE, 0, KF_FIXED, {0x1b, 0, 0, 0, 0}},
{0x05, VK_F1, 0, 0, {0, 0, 0, 0, 0}},
{0x06, 0x71, 0, 0, {0, 0, 0, 0, 0}},
{0x04, 0x72, 0, 0, {0, 0, 0, 0, 0}},
{0x0c, 0x73, 0, 0, {0, 0, 0, 0, 0}},
{0x03, 0x74, 0, 0, {0, 0, 0, 0, 0}},
{0x0b, 0x75, 0, 0, {0, 0, 0, 0, 0}},
{0x83, 0x76, 0, 0, {0, 0, 0, 0, 0}},
{0x0a, 0x77, 0, 0, {0, 0, 0, 0, 0}},
{0x01, 0x78, 0, 0, {0, 0, 0, 0, 0}},
{0x09, 0x79, 0, 0, {0, 0, 0, 0, 0}},
{0x78, 0x7a, 0, 0, {0, 0, 0, 0, 0}},
{0x07, VK_F12, 0, 0, {0, 0, 0, 0, 0}},
{0x66, VK_BACK, 0, KF_FIXED, {0x08, 0, 0, 0, 0}},
{0x0d, VK_TAB, 0, KF_FIXED, {0x09, 0, 0, 0, 0}},
{0x29, VK_SPACE, 0, KF_FIXED, {' ', 0, 0, 0, 0}},
{0x5a, VK_RETURN, 0, KF_FIXED, {0x0d, 0, 0, 0, 0}},
{0xe05a, VK_RETURN, 0, KF_FIXED, {0x0d, 0, 0, 0, 0}},
{0x58, VK_CAPITAL, 0, 0, {0, 0, 0, 0, 0}},
{0x77, VK_NUMLOCK, 0, 0, {0, 0, 0, 0, 0}},
{0x7e, VK_SCROLL, 0, 0, {0, 0, 0, 0, 0}},
{0x12, VK_SHIFT, 0, 0, {0, 0, 0, 0, 0}},
{0x59, VK_SHIFT, 0, 0, {0, 0, 0, 0, 0}},
{0x14, VK_CONTROL, 0, 0, {0, 0, 0, 0, 0}},
{0xe014, VK_CONTROL, 0, 0, {0, 0, 0, 0, 0}},
{0x11, VK_MENU, 0, 0, {0, 0, 0, 0, 0}},
{0xe011, VK_MENU, 0, 0, {0, 0, 0, 0, 0}},
{0xe01f, VK_LWIN, 0, 0, {0, 0, 0, 0, 0}},
{0xe027, VK_RWIN, 0, 0, {0, 0, 0, 0, 0}},
{0xe02f, VK_APPS, 0, 0, {0, 0, 0, 0, 0}},
{0x84, VK_SNAPSHOT, 0, 0, {0, 0, 0, 0, 0}},
{0xe07c, VK_SNAPSHOT, 0, 0, {0, 0, 0, 0, 0}},
{0xe114, VK_PAUSE, 0, 0, {0, 0, 0, 0, 0}},
{0xe070, VK_INSERT, 0, 0, {0, 0, 0, 0, 0}},
{0xe071, VK_DELETE, 0, 0, {0, 0, 0, 0, 0}},
{0xe06c, VK_HOME, 0, 0, {0, 0, 0, 0, 0}},
{0xe069, VK_END, 0, 0, {0, 0, 0, 0, 0}},
{0xe07d, VK_PRIOR, 0, 0, {0, 0, 0, 0, 0}},
{0xe07a, VK_NEXT, 0, 0, {0, 0, 0, 0, 0}},
{0xe075, VK_UP, 0, 0, {0, 0, 0, 0, 0}},
{0xe072, VK_DOWN, 0, 0, {0, 0, 0, 0, 0}},
{0xe06b, VK_LEFT, 0, 0, {0, 0, 0, 0, 0}},
{0xe074, VK_RIGHT, 0, 0, {0, 0, 0, 0, 0}},
{0x79, VK_ADD, 0, KF_FIXED, {'+', 0, 0, 0, 0}},
{0x7b, VK_SUBTRACT, 0, KF_FIXED, {'-', 0, 0, 0, 0}},
{0x7c, VK_MULTIPLY, 0, KF_FIXED, {'*', 0, 0, 0, 0}},
{0xe04a, VK_DIVIDE, 0, KF_FIXED, {'/', 0, 0, 0, 0}},
{0x70, VK_INSERT, 0x60, KF_PAD, {'0', 0, 0, 0, 0}},
{0x69, VK_END, 0x61, KF_PAD, {'1', 0, 0, 0, 0}},
{0x72, VK_DOWN, 0x62, KF_PAD, {'2', 0, 0, 0, 0}},
{0x7a, VK_NEXT, 0x63, KF_PAD, {'3', 0, 0, 0, 0}},
{0x6b, VK_LEFT, 0x64, KF_PAD, {'4', 0, 0, 0, 0}},
{0x73, KVK_CLEAR, 0x65, KF_PAD, {'5', 0, 0, 0, 0}},
{0x74, VK_RIGHT, 0x66, KF_PAD, {'6', 0, 0, 0, 0}},
{0x6c, VK_HOME, 0x67, KF_PAD, {'7', 0, 0, 0, 0}},
{0x75, VK_UP, 0x68, KF_PAD, {'8', 0, 0, 0, 0}},
{0x7d, VK_PRIOR, 0x69, KF_PAD, {'9', 0, 0, 0, 0}},
{0x71, VK_DELETE, VK_DECIMAL, KF_PAD, {'.', 0, 0, 0, 0}},
};

const t_key	*common_key(uint16_t code)
{
	uint32_t	i;

	i = 0;
	while (i < sizeof(g_common_keys) / sizeof(g_common_keys[0]))
	{
		if (g_common_keys[i].code == code)
			return (&g_common_keys[i]);
		i++;
	}
	return (0);
}
