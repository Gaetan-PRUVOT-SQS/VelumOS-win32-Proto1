#include <stdint.h>
#include "fake_f4.h"
#include "harness.h"
#include "velum/err.h"

static const uint64_t	g_argc_ok[4] = {0, 1, 255, 256};
static const uint64_t	g_argc_bad[7] = {257, 1000, 0xffffffffull,
	0x100000000ull, 0x8000000000000000ull, UINT64_MAX - 1, UINT64_MAX};

static void	case_invalid_args(void)
{
	t_f4_stack	s;
	t_startinfo	info;
	size_t		k;

	f4_reset(&s);
	f4_head(&s, 0, 0);
	f4_aux_full(&s);
	h_eq_i64("sp NULL", start_parse(NULL, 8, &info), E_INVAL);
	h_eq_i64("out NULL", start_parse(s.w, s.n, NULL), E_INVAL);
	k = 0;
	while (k < 3)
		h_eq_i64("fenetre sous 3 mots", f4_parse(&s, k++, &info), E_INVAL);
	h_eq_i64("fenetre de 3 mots acceptee", f4_parse(&s, 3, &info),
		START_TRUNCATED);
}

static void	case_argc_ok(void)
{
	t_f4_stack	s;
	t_startinfo	info;
	size_t		i;

	i = 0;
	while (i < 4)
	{
		f4_reset(&s);
		f4_head(&s, g_argc_ok[i], 0);
		f4_aux_full(&s);
		h_eq_i64("argc valide : code", f4_parse(&s, s.n, &info), START_OK);
		h_eq_u64("argc valide : valeur", info.argc, g_argc_ok[i]);
		h_eq_u64("argc valide : envc", info.envc, 0);
		i++;
	}
}

static void	case_argc_bad(void)
{
	t_f4_stack	s;
	t_startinfo	info;
	size_t		i;

	i = 0;
	while (i < 7)
	{
		f4_reset(&s);
		f4_push(&s, g_argc_bad[i]);
		f4_pairs(&s, 20, 0x1234);
		h_eq_i64("argc hors borne : code", f4_parse(&s, s.n, &info), E_RANGE);
		i++;
	}
}

static void	case_argv_defects(void)
{
	t_f4_stack	s;
	t_startinfo	info;

	f4_reset(&s);
	f4_head(&s, 3, 0);
	f4_aux_full(&s);
	s.w[2] = 0;
	h_eq_i64("argv[1] nul", f4_parse(&s, s.n, &info), E_PROTO);
	f4_reset(&s);
	f4_head(&s, 3, 0);
	f4_aux_full(&s);
	s.w[4] = 5;
	h_eq_i64("argv non termine", f4_parse(&s, s.n, &info), E_PROTO);
	f4_reset(&s);
	f4_head(&s, 5, 0);
	f4_aux_full(&s);
	h_eq_i64("fenetre trop courte pour argv", f4_parse(&s, 6, &info),
		E_PROTO);
	h_eq_i64("juste la place de argv sans env", f4_parse(&s, 7, &info),
		E_PROTO);
}

int	main(void)
{
	h_begin("a14/start_parse_cases");
	h_run("start_parse/partition : arguments invalides", case_invalid_args);
	h_run("start_parse/valeur limite : argc valide", case_argc_ok);
	h_run("start_parse/valeur limite : argc hors borne", case_argc_bad);
	h_run("start_parse/table de decision : defauts d'argv",
		case_argv_defects);
	return (h_end());
}
