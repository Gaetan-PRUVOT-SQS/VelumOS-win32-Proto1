#include <stdio.h>
#include <stdlib.h>
#include "kfix.h"

#define BULK 100000

static void	hundred_thousand(void)
{
	t_kfix	f;
	char	*buf;
	int		bad;

	buf = malloc(BULK);
	if (!buf || !kfix_open(&f, 80, 64, 80))
		return ;
	kfix_stream(buf, BULK);
	bad = kfix_chunked(&f, buf, BULK);
	h_eq_i64("invariants de curseur sur 100000 caracteres", bad, 0);
	h_true(g_ffont.glyphs > 50000, "des glyphes ont ete dessines");
	kfix_close(&f);
	free(buf);
}

static void	chunk_metamorphic(void)
{
	t_kfix	a;
	t_kfix	b;
	char	*buf;
	int		bad;

	buf = malloc(BULK);
	if (!buf || !kfix_open(&a, 80, 64, 96))
		return ;
	if (!kfix_open(&b, 80, 64, 96))
		return ;
	kfix_stream(buf, BULK);
	kfix_feed_n(&a, buf, BULK);
	bad = kfix_chunked(&b, buf, BULK);
	h_eq_i64("invariants pendant les morceaux", bad, 0);
	h_true(kfix_same(&a, &b), "un bloc = plusieurs morceaux");
	h_true(a.k.col == b.k.col && a.k.row == b.k.row, "meme curseur");
	kfix_close(&a);
	kfix_close(&b);
	free(buf);
}

static void	random_bytes(void)
{
	t_kfix		f;
	uint64_t	seed;
	size_t		i;
	int			bad;

	seed = 20261005;
	printf("a13/kcon_bulk : graine %llu\n", (unsigned long long)seed);
	if (!kfix_open(&f, 80, 64, 80))
		return ;
	i = 0;
	bad = 0;
	while (i < 20000)
	{
		bad += kfix_fuzz_once(&f, &seed);
		i++;
	}
	h_eq_i64("invariants sur 20000 ecritures aleatoires", bad, 0);
	kfix_close(&f);
}

int	main(void)
{
	h_begin("a13/kcon_bulk");
	h_run("bulk 100000 caracteres", hundred_thousand);
	h_run("bulk un bloc = morceaux", chunk_metamorphic);
	h_run("bulk octets aleatoires", random_bytes);
	return (h_end());
}
