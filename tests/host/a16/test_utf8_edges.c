#include "a16_test.h"

static const char		*g_samples[] = {
	"\xC3\xA9",
	"\xE2\x82\xAC",
	"\xF0\x9F\x98\x80",
	"\xED\x9F\xBF",
	"\xF4\x8F\xBF\xBF",
	"\xE0\xA0\x80",
	"\xF0\x90\x80\x80",
};

static const uint32_t	g_example[] = {
	0x61, UTF8_REPLACEMENT, UTF8_REPLACEMENT, UTF8_REPLACEMENT, 0x62,
	UTF8_REPLACEMENT, 0x63, UTF8_REPLACEMENT, UTF8_REPLACEMENT, 0x64,
};

static const char		*g_table_bytes[] = {
	"\xC0\xAF", "\xE0\x80\xBF", "\xF0\x81\x82\x41",
	"\xED\xA0\x80", "\xED\xBF\xBF", "\xED\xAF\x41",
	"\xF4\x91\x92\x93\xFF\x41\x80\xBF\x42",
	"\xE1\x80\xE2\xF0\x91\x92\xF1\xBF\x41",
};

static const size_t		g_table_len[] = {2, 3, 4, 3, 3, 3, 9, 9};

static const char		*g_table_out[] = {
	"FF", "FFF", "FFFA", "FFF", "FFF", "FFA", "FFFFFAFFB", "FFFFA",
};

static void	edge_truncated_prefixes(void)
{
	size_t		i;
	size_t		k;
	size_t		used;
	uint8_t		seq[5];

	i = 0;
	while (i < sizeof(g_samples) / sizeof(g_samples[0]))
	{
		k = 1;
		while (k < strlen(g_samples[i]))
		{
			memcpy(seq, g_samples[i], k);
			h_eq_u64("TRONQUE-fin de donnees",
				decode_poisoned(seq, k, &used), UTF8_REPLACEMENT);
			h_eq_u64("TRONQUE-un seul U+FFFD pour le prefixe", used, k);
			seq[k] = 'A';
			h_eq_u64("TRONQUE-suivi d'ascii", decode_poisoned(seq, k + 1,
					&used), UTF8_REPLACEMENT);
			h_eq_u64("TRONQUE-sous-partie maximale", used, k);
			k++;
		}
		i++;
	}
}

static void	edge_maximal_subpart_example(void)
{
	uint32_t	out[16];
	size_t		count;
	size_t		i;

	count = decode_n("\x61\xF1\x80\x80\xE1\x80\xC2\x62\x80\x63\x80\xBF\x64",
			13, out, 16);
	h_eq_u64("MAXSUB-exemple W3C : nombre", count, 10);
	i = 0;
	while (i < count && i < 10)
	{
		h_eq_u64("MAXSUB-exemple W3C : valeur", out[i], g_example[i]);
		i++;
	}
	h_eq_u64("MAXSUB-E0 80", decode_n("\xE0\x80", 2, out, 16), 2);
	h_eq_u64("MAXSUB-F0 9F 98 puis 41", decode_n("\xF0\x9F\x98\x41", 4, out,
			16), 2);
	h_eq_u64("MAXSUB-valeur apres sous-partie", out[1], 0x41);
}

static void	edge_unicode_tables_3_8_to_3_11(void)
{
	size_t		i;
	size_t		k;
	size_t		count;
	uint32_t	out[16];
	char		want;

	i = 0;
	while (i < sizeof(g_table_len) / sizeof(g_table_len[0]))
	{
		count = decode_n(g_table_bytes[i], g_table_len[i], out, 16);
		h_eq_u64("TABLES-nombre de resultats", count,
			strlen(g_table_out[i]));
		k = 0;
		while (k < count && k < strlen(g_table_out[i]))
		{
			want = g_table_out[i][k];
			h_eq_u64("TABLES-U+FFFD, 0041 ou 0042 selon la table Unicode",
				out[k], UTF8_REPLACEMENT * (want == 'F') + 0x41 * (want == 'A')
				+ 0x42 * (want == 'B'));
			k++;
		}
		i++;
	}
}

static void	edge_end_of_input_and_null(void)
{
	const char	*cur;
	const char	text[2] = {'A', 'B'};

	cur = text;
	h_eq_u64("FIN-pointeur egal a end", font_utf8_next(&cur, text), 0);
	h_true(cur == text, "FIN-pas d'avance sur end");
	cur = text + 1;
	h_eq_u64("FIN-pointeur apres end", font_utf8_next(&cur, text), 0);
	h_true(cur == text + 1, "FIN-pas d'avance apres end");
	h_eq_u64("FIN-s nul", font_utf8_next(NULL, text), 0);
	cur = NULL;
	h_eq_u64("FIN-*s nul", font_utf8_next(&cur, text), 0);
	cur = "\0A";
	h_eq_u64("FIN-octet nul est U+0000", font_utf8_next(&cur, cur + 2), 0);
	h_true(*cur == 'A', "FIN-octet nul avance de 1");
}

int	main(void)
{
	h_begin("a16/utf8_edges");
	h_run("troncatures a la fin", edge_truncated_prefixes);
	h_run("sous-partie maximale, exemple W3C", edge_maximal_subpart_example);
	h_run("tables 3-8 a 3-11 du standard", edge_unicode_tables_3_8_to_3_11);
	h_run("fin de donnees et pointeurs nuls", edge_end_of_input_and_null);
	return (h_end());
}
