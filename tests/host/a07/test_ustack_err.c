#include <stdlib.h>
#include "a07_fake.h"
#include "harness.h"
#include "velum/err.h"

static void	ust_bad_inputs(void)
{
	t_ustack	st;
	t_ustart	in;

	ust_input(&in, "a\0b\0", 4, 3);
	h_eq_i64("nargs plus grand que le bloc", ust_build(&st, 0x1000, &in),
		E_INVAL);
	free(st.buf);
	ust_input(&in, NULL, 0, PROC_NARGS_MAX + 1);
	h_eq_i64("257 arguments", ust_build(&st, 0x1000, &in), E_INVAL);
	free(st.buf);
}

static void	ust_errors(void)
{
	t_ustack	st;
	t_ustart	in;

	ust_input(&in, NULL, 0, 0);
	st.cap = 0x1000;
	st.top = UST_TOP + 8;
	st.buf = malloc(st.cap);
	h_eq_i64("haut non aligne", ustack_build(&st, &in), E_INVAL);
	st.top = 0x800;
	h_eq_i64("capacite > haut", ustack_build(&st, &in), E_INVAL);
	st.top = UST_TOP;
	st.sp = st.top;
	h_eq_i64("ecriture au-dessus du haut", ustack_put(&st, st.top, 1), E_INVAL);
	h_eq_i64("ecriture non alignee", ustack_put(&st, st.top - 12, 1), E_INVAL);
	h_eq_i64("ecriture sous la zone", ustack_put(&st, st.top - 0x1008, 1),
		E_INVAL);
	h_eq_i64("empilement trop gros", ustack_push(&st, g_ust_rnd, 0x1001),
		E_NOMEM);
	free(st.buf);
}

int	main(void)
{
	h_begin("a07/pile_de_depart_erreurs");
	h_run("entrees incoherentes", ust_bad_inputs);
	h_run("bornes de la zone", ust_errors);
	return (h_end());
}
