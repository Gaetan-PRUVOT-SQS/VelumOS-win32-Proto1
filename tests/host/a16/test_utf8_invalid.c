#include "a16_test.h"

static const t_vec	g_never[] = {
{"\xC0\x80", 2, UTF8_REPLACEMENT},
{"\xC1\xBF", 2, UTF8_REPLACEMENT},
{"\xE0\x80\x80", 3, UTF8_REPLACEMENT},
{"\xE0\x9F\xBF", 3, UTF8_REPLACEMENT},
{"\xED\xA0\x80", 3, UTF8_REPLACEMENT},
{"\xED\xBF\xBF", 3, UTF8_REPLACEMENT},
{"\xF0\x80\x80\x80", 4, UTF8_REPLACEMENT},
{"\xF0\x8F\xBF\xBF", 4, UTF8_REPLACEMENT},
{"\xF4\x90\x80\x80", 4, UTF8_REPLACEMENT},
{"\xF5\x80\x80\x80", 4, UTF8_REPLACEMENT},
{"\xF8\x88\x80\x80\x80", 5, UTF8_REPLACEMENT},
{"\xFF\xFE", 2, UTF8_REPLACEMENT},
};

static void	bad_lead_bytes_alone(void)
{
	int			byte;
	uint8_t		seq;
	size_t		used;

	byte = 0x80;
	while (byte <= 0xFF)
	{
		seq = (uint8_t)byte;
		if (byte <= 0xC1 || byte >= 0xF5)
		{
			h_eq_u64("BAD-octet de tete seul", decode_poisoned(&seq, 1, &used),
				UTF8_REPLACEMENT);
			h_eq_u64("BAD-octet de tete avance de 1", used, 1);
		}
		byte++;
	}
}

static void	bad_lead_second_against_model(void)
{
	uint8_t		seq[4];
	int			lead;
	int			second;
	size_t		used[2];
	uint32_t	want;

	seq[2] = 0x80;
	seq[3] = 0x80;
	lead = 0x80;
	while (lead <= 0xFF)
	{
		second = 0;
		while (second <= 0xFF)
		{
			seq[0] = (uint8_t)lead;
			seq[1] = (uint8_t)second++;
			used[1] = ref_utf8_decode(seq, 4, &want);
			h_eq_u64("BAD-balayage tete x deuxieme octet valeur",
				decode_poisoned(seq, 4, &used[0]), want);
			h_eq_u64("BAD-balayage consomme", used[0], used[1]);
		}
		lead++;
	}
}

static void	bad_never_valid_sequences(void)
{
	size_t		i;
	uint32_t	cp;
	uint8_t		enc[4];

	i = 0;
	while (i < sizeof(g_never) / sizeof(g_never[0]))
	{
		h_eq_i64("BAD-sur-longueur, substitut ou hors plage",
			count_replacements(g_never[i].bytes, (size_t)g_never[i].len),
			g_never[i].len);
		i++;
	}
	cp = 0xD800;
	while (cp <= 0xDFFF)
	{
		h_eq_i64("BAD-substitut D800-DFFF encode en 3 octets",
			count_replacements(enc, ref_utf8_encode(cp, enc)), 3);
		cp++;
	}
}

int	main(void)
{
	h_begin("a16/utf8_invalid");
	h_run("octets de tete invalides", bad_lead_bytes_alone);
	h_run("tete x deuxieme octet vs modele", bad_lead_second_against_model);
	h_run("sur-longueurs, substituts, hors plage", bad_never_valid_sequences);
	return (h_end());
}
