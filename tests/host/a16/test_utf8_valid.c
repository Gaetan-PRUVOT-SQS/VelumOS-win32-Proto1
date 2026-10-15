#include "a16_test.h"

static const t_vec	g_edges[] = {
{"\x7F", 1, 0x7F},
{"\xC2\x80", 2, 0x80},
{"\xDF\xBF", 2, 0x7FF},
{"\xE0\xA0\x80", 3, 0x800},
{"\xE0\xBF\xBF", 3, 0xFFF},
{"\xE1\x80\x80", 3, 0x1000},
{"\xEC\xBF\xBF", 3, 0xCFFF},
{"\xED\x80\x80", 3, 0xD000},
{"\xED\x9F\xBF", 3, 0xD7FF},
{"\xEE\x80\x80", 3, 0xE000},
{"\xEF\xBF\xBF", 3, 0xFFFF},
{"\xF0\x90\x80\x80", 4, 0x10000},
{"\xF0\xBF\xBF\xBF", 4, 0x3FFFF},
{"\xF1\x80\x80\x80", 4, 0x40000},
{"\xF3\xBF\xBF\xBF", 4, 0xFFFFF},
{"\xF4\x80\x80\x80", 4, 0x100000},
{"\xF4\x8F\xBF\xBF", 4, 0x10FFFF},
{"\xC3\xA9", 2, 0xE9},
{"\xE2\x82\xAC", 3, 0x20AC},
{"\xF0\x9F\x98\x80", 4, 0x1F600},
};

static void	wf_table_3_7_edges(void)
{
	size_t		i;
	size_t		used;
	uint32_t	cp;

	i = 0;
	while (i < sizeof(g_edges) / sizeof(g_edges[0]))
	{
		cp = decode_poisoned((const uint8_t *)g_edges[i].bytes,
				(size_t)g_edges[i].len, &used);
		h_eq_u64("WF-bord valeur", cp, g_edges[i].cp);
		h_eq_u64("WF-bord octets consommes", used, (size_t)g_edges[i].len);
		i++;
	}
}

static void	wf_ascii_complete(void)
{
	uint8_t		byte;
	size_t		used;
	uint32_t	cp;

	byte = 0;
	while (byte < 0x80)
	{
		cp = decode_poisoned(&byte, 1, &used);
		h_eq_u64("WF-ascii valeur", cp, byte);
		h_eq_u64("WF-ascii consomme 1", used, 1);
		byte++;
	}
}

static void	wf_every_scalar_roundtrip(void)
{
	uint32_t	cp;
	uint8_t		enc[4];
	size_t		len;
	size_t		used;

	cp = 0;
	while (cp <= 0x10FFFF)
	{
		if (cp < 0xD800 || cp > 0xDFFF)
		{
			len = ref_utf8_encode(cp, enc);
			h_true(decode_poisoned(enc, len, &used) == cp && used == len,
				"WF-aller-retour de tout scalaire");
		}
		cp++;
	}
}

int	main(void)
{
	h_begin("a16/utf8_valid");
	h_run("table 3-7 bornes de chaque ligne", wf_table_3_7_edges);
	h_run("ascii complet", wf_ascii_complete);
	h_run("aller-retour des 1112064 scalaires", wf_every_scalar_roundtrip);
	return (h_end());
}
