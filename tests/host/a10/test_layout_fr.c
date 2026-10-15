#include "harness.h"
#include "th.h"

static const uint16_t	g_pos[TH_KEYS] = {
	0x0e, 0x16, 0x1e, 0x26, 0x25, 0x2e, 0x36, 0x3d, 0x3e, 0x46, 0x45, 0x4e,
	0x55, 0x15, 0x1d, 0x24, 0x2d, 0x2c, 0x35, 0x3c, 0x43, 0x44, 0x4d, 0x54,
	0x5b, 0x1c, 0x1b, 0x23, 0x2b, 0x34, 0x33, 0x3b, 0x42, 0x4b, 0x4c, 0x52,
	0x5d, 0x61, 0x1a, 0x22, 0x21, 0x2a, 0x32, 0x31, 0x3a, 0x41, 0x49, 0x4a};
static const uint8_t	g_vk[TH_KEYS] = {
	0xde, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 0xdb, 0xbb,
	'A', 'Z', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 0xdd, 0xba,
	'Q', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 0xc0, 0xdc,
	0xe2, 'W', 'X', 'C', 'V', 'B', 'N', 0xbc, 0xbe, 0xbf, 0xdf};
static const uint32_t	g_normal[TH_KEYS] = {
	0xb2, '&', 0xe9, '"', '\'', '(', '-', 0xe8, '_', 0xe7, 0xe0, ')', '=',
	'a', 'z', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', DEAD_CIRC, '$',
	'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', 0xf9, '*',
	'<', 'w', 'x', 'c', 'v', 'b', 'n', ',', ';', ':', '!'};
static const uint32_t	g_shift[TH_KEYS] = {
	0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 0xb0, '+',
	'A', 'Z', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', DEAD_TREMA, 0xa3,
	'Q', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M', '%', 0xb5,
	'>', 'W', 'X', 'C', 'V', 'B', 'N', '?', '.', '/', 0xa7};
static const uint32_t	g_altgr[TH_KEYS] = {
	0, 0, DEAD_TILDE, '#', '{', '[', '|', DEAD_GRAVE, '\\', '^', '@', ']', '}',
	0, 0, 0x20ac, 0, 0, 0, 0, 0, 0, 0, 0, 0xa4,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const uint32_t	g_caps[TH_KEYS] = {
	0xb2, '&', 0xc9, '"', '\'', '(', '-', 0xc8, '_', 0xc7, 0xc0, ')', '=',
	'A', 'Z', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', DEAD_CIRC, '$',
	'Q', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 0xd9, '*',
	'<', 'W', 'X', 'C', 'V', 'B', 'N', ',', ';', ':', '!'};
static const uint32_t	g_caps_shift[TH_KEYS] = {
	0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 0xb0, '+',
	'a', 'z', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', DEAD_TREMA, 0xa3,
	'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', '%', 0xb5,
	'>', 'w', 'x', 'c', 'v', 'b', 'n', '?', '.', '/', 0xa7};
static const uint32_t	g_none[TH_KEYS];

static const t_colcase	g_cols[] = {
{"fr normal", g_normal, 0, 0},
{"fr Maj", g_shift, KM_LSHIFT, 0},
{"fr Maj droite", g_shift, KM_RSHIFT, 0},
{"fr AltGr", g_altgr, KM_RALT, 0},
{"fr Ctrl+Alt = AltGr", g_altgr, KM_LCTRL | KM_LALT, 0},
{"fr Maj+AltGr", g_none, KM_LSHIFT | KM_RALT, 0},
{"fr Verr Maj", g_caps, 0, INPUT_LED_CAPS},
{"fr Verr Maj + Maj", g_caps_shift, KM_LSHIFT, INPUT_LED_CAPS},
{"fr Verr Maj + AltGr", g_altgr, KM_RALT, INPUT_LED_CAPS},
{"fr Verr Num sans effet", g_normal, 0, INPUT_LED_NUM},
};

static void	check_states(void)
{
	uint32_t	i;

	i = 0;
	while (i < sizeof(g_cols) / sizeof(g_cols[0]))
	{
		th_check_col(&g_layout_fr, g_pos, &g_cols[i]);
		i++;
	}
}

static void	check_vk(void)
{
	g_h.name = "fr codes virtuels";
	th_check_vk(&g_layout_fr, g_pos, g_vk);
}

int	main(void)
{
	h_begin("a10/layout_fr");
	h_run("fr : 48 touches x 10 etats", check_states);
	h_run("fr : codes virtuels", check_vk);
	return (h_end());
}
