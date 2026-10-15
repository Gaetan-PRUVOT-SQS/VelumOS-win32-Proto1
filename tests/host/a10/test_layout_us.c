#include "harness.h"
#include "th.h"

static const uint16_t	g_pos[TH_KEYS] = {
	0x0e, 0x16, 0x1e, 0x26, 0x25, 0x2e, 0x36, 0x3d, 0x3e, 0x46, 0x45, 0x4e,
	0x55, 0x15, 0x1d, 0x24, 0x2d, 0x2c, 0x35, 0x3c, 0x43, 0x44, 0x4d, 0x54,
	0x5b, 0x1c, 0x1b, 0x23, 0x2b, 0x34, 0x33, 0x3b, 0x42, 0x4b, 0x4c, 0x52,
	0x5d, 0x61, 0x1a, 0x22, 0x21, 0x2a, 0x32, 0x31, 0x3a, 0x41, 0x49, 0x4a};
static const uint8_t	g_vk[TH_KEYS] = {
	0xc0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 0xbd, 0xbb,
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 0xdb, 0xdd,
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 0xba, 0xde, 0xdc,
	0xe2, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', 0xbc, 0xbe, 0xbf};
static const uint32_t	g_normal[TH_KEYS] = {
	'`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '\\',
	'\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/'};
static const uint32_t	g_shift[TH_KEYS] = {
	'~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}',
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '|',
	'|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?'};
static const uint32_t	g_caps[TH_KEYS] = {
	'`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']',
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '\\',
	'\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/'};
static const uint32_t	g_caps_shift[TH_KEYS] = {
	'~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}',
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '"', '|',
	'|', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?'};
static const uint32_t	g_none[TH_KEYS];

static const t_colcase	g_cols[] = {
{"us normal", g_normal, 0, 0},
{"us Maj", g_shift, KM_LSHIFT, 0},
{"us Maj droite", g_shift, KM_RSHIFT, 0},
{"us AltDroit est Alt", g_none, KM_RALT, 0},
{"us Ctrl+Alt sans AltGr", g_none, KM_LCTRL | KM_LALT, 0},
{"us Maj+AltDroit", g_none, KM_LSHIFT | KM_RALT, 0},
{"us Verr Maj", g_caps, 0, INPUT_LED_CAPS},
{"us Verr Maj + Maj", g_caps_shift, KM_LSHIFT, INPUT_LED_CAPS},
{"us Verr Num sans effet", g_normal, 0, INPUT_LED_NUM},
};

static void	check_states(void)
{
	uint32_t	i;

	i = 0;
	while (i < sizeof(g_cols) / sizeof(g_cols[0]))
	{
		th_check_col(&g_layout_us, g_pos, &g_cols[i]);
		i++;
	}
}

static void	check_vk(void)
{
	g_h.name = "us codes virtuels";
	th_check_vk(&g_layout_us, g_pos, g_vk);
}

int	main(void)
{
	h_begin("a10/layout_us");
	h_run("us : 48 touches x 9 etats", check_states);
	h_run("us : codes virtuels", check_vk);
	return (h_end());
}
