#include <stdint.h>
#include "harness.h"
#include "a20_test.h"
#include "utf8.h"

static const t_utfrow	g_valid[] = {
{"", 0, 1}, {"abc", 3, 1}, {"\303\251", 2, 1}, {"\342\202\254", 3, 1},
{"\360\237\230\200", 4, 1}, {"\302\200", 2, 1}, {"\337\277", 2, 1},
{"\340\240\200", 3, 1}, {"\355\237\277", 3, 1}, {"\356\200\200", 3, 1},
{"\357\277\277", 3, 1}, {"\360\220\200\200", 4, 1}, {"\364\217\277\277", 4, 1},
{"a\303\251z", 4, 1}
};

static const t_utfrow	g_invalid[] = {
{"\300\200", 2, 0}, {"\301\277", 2, 0}, {"\340\200\200", 3, 0},
{"\340\237\277", 3, 0}, {"\355\240\200", 3, 0}, {"\355\277\277", 3, 0},
{"\360\200\200\200", 4, 0}, {"\360\217\277\277", 4, 0},
{"\364\220\200\200", 4, 0}, {"\365\200\200\200", 4, 0}, {"\377", 1, 0},
{"\200", 1, 0}, {"\303", 1, 0}, {"\342\202", 2, 0}, {"\360\237\230", 3, 0},
{"\303(", 2, 0}, {"a\303", 2, 0}, {"\342\202(", 3, 0}, {"\354\200\300", 3, 0}
};

static void	utf8_sequences_valides(void)
{
	uint32_t	i;

	i = 0;
	while (i < sizeof(g_valid) / sizeof(g_valid[0]))
	{
		h_eq_i64("valide", utf8_valid(g_valid[i].s, g_valid[i].len), 1);
		i++;
	}
}

static void	utf8_sequences_invalides(void)
{
	uint32_t	i;

	i = 0;
	while (i < sizeof(g_invalid) / sizeof(g_invalid[0]))
	{
		h_eq_i64("invalide", utf8_valid(g_invalid[i].s, g_invalid[i].len), 0);
		i++;
	}
}

static void	utf8_longueur_de_sequence(void)
{
	const uint8_t	*e;

	e = (const uint8_t *)"\342\202\254";
	h_eq_u64("3 octets", utf8_seq_len(e, 3), 3);
	h_eq_u64("tronquee", utf8_seq_len(e, 2), 0);
	h_eq_u64("rien a lire", utf8_seq_len(e, 0), 0);
	h_eq_u64("ascii", utf8_seq_len((const uint8_t *)"a", 1), 1);
}

int	main(void)
{
	h_begin("a20/utf8");
	h_run("utf8: sequences valides", utf8_sequences_valides);
	h_run("utf8: sequences invalides", utf8_sequences_invalides);
	h_run("utf8: longueur de sequence", utf8_longueur_de_sequence);
	return (h_end());
}
