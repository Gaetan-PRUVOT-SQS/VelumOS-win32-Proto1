#include <stdlib.h>
#include <string.h>
#include "a07_fake.h"
#include "harness.h"
#include "velum/abi/abi_syscall.h"
#include "velum/err.h"
#include "velum/proc.h"

static void	ust_basic(void)
{
	t_ustack	st;
	t_ustart	in;
	t_ustparsed	p;

	ust_input(&in, NULL, 0, 0);
	h_eq_i64("pile sans argument", ust_build(&st, 0x2000, &in), 0);
	ust_parse(&st, &p);
	h_true(p.ok, "pile lisible et alignee sur 16");
	h_eq_u64("argc 1", p.argc, 1);
	h_eq_str("argv[0] = chemin", ust_str(&st, p.argv[0]), "/system/bin/init");
	h_true(ust_aux_order_ok(&p), "8 paires auxv dans l'ordre du contrat");
	h_eq_u64("AT_ENTRY", p.aux_val[3], 0x10001000);
	h_eq_u64("AT_VELUM_ABI", p.aux_val[5], VELUM_ABI_VERSION);
	h_eq_u64("AT_VELUM_FLAGS", p.aux_val[6], 0x7f);
	h_true(!memcmp(ust_str(&st, p.aux_val[4]), g_ust_rnd, USTACK_RANDOM),
		"AT_RANDOM pointe sur l'alea");
	free(st.buf);
}

static void	ust_args(void)
{
	t_ustack	st;
	t_ustart	in;
	t_ustparsed	p;
	int			n;

	ust_input(&in, "a\0bb\0", 6, 3);
	h_eq_i64("args avec chaine vide", ust_build(&st, 0x2000, &in), 0);
	ust_parse(&st, &p);
	h_true(p.ok && p.argc == 4, "argc 4");
	h_eq_str("argv[1]", ust_str(&st, p.argv[1]), "a");
	h_eq_str("argv[2]", ust_str(&st, p.argv[2]), "bb");
	h_eq_str("argv[3] vide", ust_str(&st, p.argv[3]), "");
	free(st.buf);
	n = 0;
	while (n < 6)
	{
		ust_input(&in, "x\0x\0x\0x\0x\0x\0", 2 * (uint64_t)n, n);
		h_eq_i64("argc pair et impair", ust_build(&st, 0x2000, &in), 0);
		ust_parse(&st, &p);
		h_true(p.ok && p.argc == (uint64_t)n + 1, "alignement selon argc");
		free(st.buf);
		n++;
	}
}

static void	ust_limits(void)
{
	t_ustack	st;
	t_ustart	in;
	char		*big;
	uint64_t	need;
	uint64_t	cap;

	big = args_block(PROC_NARGS_MAX, PROC_ARGS_MAX / PROC_NARGS_MAX);
	ust_input(&in, big, PROC_ARGS_MAX, PROC_NARGS_MAX);
	h_eq_i64("256 args, 32 Kio", ust_build(&st, 0xa000, &in), 0);
	need = st.top - st.sp;
	free(st.buf);
	cap = need - 40;
	while (cap < need)
	{
		h_eq_i64("capacite trop petite", ust_build(&st, cap, &in), E_NOMEM);
		free(st.buf);
		cap++;
	}
	h_eq_i64("capacite exacte", ust_build(&st, need, &in), 0);
	free(st.buf);
	free(big);
}

int	main(void)
{
	h_begin("a07/pile_de_depart");
	h_run("pile minimale", ust_basic);
	h_run("arguments", ust_args);
	h_run("limites de capacite", ust_limits);
	return (h_end());
}
