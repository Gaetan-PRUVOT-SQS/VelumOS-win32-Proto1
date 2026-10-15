#include "in_int.h"

static const t_inop	g_in_ops[256] = {
	in_op_nop, in_op_move, in_op_move, in_op_move,
	in_op_move, in_op_move, in_op_move, in_op_move,
	in_op_move, in_op_move, in_op_move_result, in_op_move_result,
	in_op_move_result, in_op_move_exception, in_op_return, in_op_return,
	in_op_return, in_op_return, in_op_const, in_op_const,
	in_op_const, in_op_const, in_op_const, in_op_const,
	in_op_const, in_op_const, in_op_const_ref, in_op_const_ref,
	in_op_const_ref, in_op_monitor, in_op_monitor, in_op_check_cast,
	in_op_instance_of, in_op_array_length, in_op_const_ref, in_op_new_array,
	in_op_filled, in_op_filled, in_op_fill_data, in_op_monitor,
	in_op_goto, in_op_goto, in_op_goto, in_op_switch,
	in_op_switch, in_op_cmp, in_op_cmp, in_op_cmp,
	in_op_cmp, in_op_cmp, in_op_if, in_op_if,
	in_op_if, in_op_if, in_op_if, in_op_if,
	in_op_if, in_op_if, in_op_if, in_op_if,
	in_op_if, in_op_if, in_op_bad, in_op_bad,
	in_op_bad, in_op_bad, in_op_bad, in_op_bad,
	in_op_aget, in_op_aget, in_op_aget, in_op_aget,
	in_op_aget, in_op_aget, in_op_aget, in_op_aput,
	in_op_aput, in_op_aput, in_op_aput, in_op_aput,
	in_op_aput, in_op_aput, in_op_field, in_op_field,
	in_op_field, in_op_field, in_op_field, in_op_field,
	in_op_field, in_op_field, in_op_field, in_op_field,
	in_op_field, in_op_field, in_op_field, in_op_field,
	in_op_field, in_op_field, in_op_field, in_op_field,
	in_op_field, in_op_field, in_op_field, in_op_field,
	in_op_field, in_op_field, in_op_field, in_op_field,
	in_op_field, in_op_field, in_op_invoke, in_op_invoke,
	in_op_invoke, in_op_invoke, in_op_invoke, in_op_bad,
	in_op_invoke, in_op_invoke, in_op_invoke, in_op_invoke,
	in_op_invoke, in_op_bad, in_op_bad, in_op_un_int,
	in_op_un_int, in_op_un_long, in_op_un_long, in_op_un_float,
	in_op_un_double, in_op_un_int, in_op_un_int, in_op_un_int,
	in_op_un_long, in_op_un_long, in_op_un_long, in_op_un_float,
	in_op_un_float, in_op_un_float, in_op_un_double, in_op_un_double,
	in_op_un_double, in_op_un_int, in_op_un_int, in_op_un_int,
	in_op_ibin, in_op_ibin, in_op_ibin, in_op_ibin,
	in_op_ibin, in_op_ibin, in_op_ibin, in_op_ibin,
	in_op_ibin, in_op_ibin, in_op_ibin, in_op_lbin,
	in_op_lbin, in_op_lbin, in_op_lbin, in_op_lbin,
	in_op_lbin, in_op_lbin, in_op_lbin, in_op_lbin,
	in_op_lbin, in_op_lbin, in_op_fbin, in_op_fbin,
	in_op_fbin, in_op_fbin, in_op_fbin, in_op_dbin,
	in_op_dbin, in_op_dbin, in_op_dbin, in_op_dbin,
	in_op_ibin, in_op_ibin, in_op_ibin, in_op_ibin,
	in_op_ibin, in_op_ibin, in_op_ibin, in_op_ibin,
	in_op_ibin, in_op_ibin, in_op_ibin, in_op_lbin,
	in_op_lbin, in_op_lbin, in_op_lbin, in_op_lbin,
	in_op_lbin, in_op_lbin, in_op_lbin, in_op_lbin,
	in_op_lbin, in_op_lbin, in_op_fbin, in_op_fbin,
	in_op_fbin, in_op_fbin, in_op_fbin, in_op_dbin,
	in_op_dbin, in_op_dbin, in_op_dbin, in_op_dbin,
	in_op_ilit, in_op_ilit, in_op_ilit, in_op_ilit,
	in_op_ilit, in_op_ilit, in_op_ilit, in_op_ilit,
	in_op_ilit, in_op_ilit, in_op_ilit, in_op_ilit,
	in_op_ilit, in_op_ilit, in_op_ilit, in_op_ilit,
	in_op_ilit, in_op_ilit, in_op_ilit, in_op_bad,
	in_op_bad, in_op_bad, in_op_bad, in_op_bad,
	in_op_bad, in_op_bad, in_op_bad, in_op_bad,
	in_op_bad, in_op_bad, in_op_bad, in_op_bad,
	in_op_bad, in_op_bad, in_op_bad, in_op_bad,
	in_op_bad, in_op_bad, in_op_bad, in_op_bad,
	in_op_bad, in_op_bad, in_op_bad, in_op_bad,
	in_op_bad, in_op_bad, in_op_bad, in_op_bad
};

t_inop	in_op(uint8_t op)
{
	return (g_in_ops[op]);
}

int	in_op_nop(t_in *in)
{
	(void)in;
	return (0);
}

int	in_op_bad(t_in *in)
{
	return (in_throw(in, IN_VERIFY));
}
