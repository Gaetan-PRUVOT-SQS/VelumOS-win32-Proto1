#include <stdint.h>
#include "fake_f4.h"
#include "harness.h"
#include "velum/err.h"

static void	aux_fields(void)
{
	t_f4_stack	s;
	t_startinfo	info;

	f4_reset(&s);
	f4_head(&s, 1, 0);
	f4_aux_full(&s);
	h_eq_i64("auxv complete", f4_parse(&s, s.n, &info), START_OK);
	h_eq_u64("AT_PAGESZ", info.pagesz, 4096);
	h_eq_u64("AT_RANDOM", (uint64_t)(uintptr_t)info.random, 0x7000);
	h_eq_u64("AT_ENTRY", info.entry, 0x401000);
	h_eq_u64("AT_PHDR", (uint64_t)(uintptr_t)info.phdr, 0x400040);
	h_eq_u64("AT_PHNUM", info.phnum, 6);
	h_eq_u64("AT_VELUM_ABI", info.abi, 1);
	h_eq_u64("AT_VELUM_FLAGS", info.flags, 0x3f);
}

static void	aux_edges(void)
{
	t_f4_stack	s;
	t_startinfo	info;

	f4_reset(&s);
	f4_head(&s, 0, 0);
	f4_push(&s, AT_NULL);
	h_eq_i64("AT_NULL seul", f4_parse(&s, s.n, &info), START_OK);
	h_eq_u64("aucun champ rempli", info.pagesz + info.entry + info.abi, 0);
	f4_reset(&s);
	f4_head(&s, 0, 0);
	f4_push(&s, AT_PAGESZ);
	f4_push(&s, 4096);
	h_eq_i64("sans AT_NULL", f4_parse(&s, s.n, &info), START_TRUNCATED);
	h_eq_u64("champ trouve conserve", info.pagesz, 4096);
	f4_push(&s, AT_ENTRY);
	h_eq_i64("paire incomplete", f4_parse(&s, s.n, &info), START_TRUNCATED);
	h_eq_u64("paire incomplete ignoree", info.entry, 0);
}

static void	aux_limit(void)
{
	t_f4_stack	s;
	t_startinfo	info;
	size_t		pairs;
	int			want;

	pairs = 62;
	while (pairs < 66)
	{
		f4_reset(&s);
		f4_head(&s, 0, 0);
		f4_pairs(&s, pairs, 0x1234);
		f4_push(&s, AT_NULL);
		f4_push(&s, 0);
		want = START_TRUNCATED;
		if (pairs < START_AUX_MAX)
			want = START_OK;
		h_eq_i64("paires puis AT_NULL", f4_parse(&s, s.n, &info), want);
		pairs++;
	}
}

static void	aux_unknown_duplicates(void)
{
	t_f4_stack	s;
	t_startinfo	info;

	f4_reset(&s);
	f4_head(&s, 0, 0);
	f4_pairs(&s, 5, 0x1234);
	f4_push(&s, AT_PAGESZ);
	f4_push(&s, 8192);
	f4_push(&s, AT_PAGESZ);
	f4_push(&s, 16384);
	f4_push(&s, AT_NULL);
	f4_push(&s, 0);
	h_eq_i64("types inconnus ignores", f4_parse(&s, s.n, &info), START_OK);
	h_eq_u64("doublon : le dernier gagne", info.pagesz, 16384);
	h_eq_u64("types inconnus : rien d'autre", info.entry + info.abi
		+ info.flags + info.phnum, 0);
}

int	main(void)
{
	h_begin("a14/start_parse_aux");
	h_run("start_parse/exigence : champs de l'auxv", aux_fields);
	h_run("start_parse/partition : AT_NULL, troncature", aux_edges);
	h_run("start_parse/valeur limite : 64 paires", aux_limit);
	h_run("start_parse/supposition d'erreur : inconnus, doublons",
		aux_unknown_duplicates);
	return (h_end());
}
