#ifndef IN_INT_H
# define IN_INT_H

# include <stddef.h>
# include <stdint.h>
# include "velum/apk/dvm.h"
# include "velum/err.h"

# define IN_HDR 4u
# define IN_INVOKE_LEN 3u
# define IN_VERIFY "Ljava/lang/VerifyError;"
# define IN_SOE "Ljava/lang/StackOverflowError;"
# define IN_NPE "Ljava/lang/NullPointerException;"
# define IN_ARITH "Ljava/lang/ArithmeticException;"
# define IN_CAST "Ljava/lang/ClassCastException;"
# define IN_ABSTRACT "Ljava/lang/AbstractMethodError;"

typedef struct s_in
{
	t_dvm			*vm;
	const t_dmethod	*m;
	uint32_t		*r;
	uint32_t		nreg;
	uint32_t		pc;
	uint32_t		next;
	uint32_t		fp;
	uint32_t		fp0;
	uint32_t		done;
	uint32_t		bad;
	t_dinsn			i;
	uint32_t		argv[5];
}	t_in;

typedef int		(*t_inop)(t_in *in);

static inline float	in_f32(uint32_t bits)
{
	float	f;

	__builtin_memcpy(&f, &bits, sizeof(f));
	return (f);
}

static inline uint32_t	in_f32b(float f)
{
	uint32_t	bits;

	__builtin_memcpy(&bits, &f, sizeof(bits));
	return (bits);
}

static inline double	in_f64(uint64_t bits)
{
	double	d;

	__builtin_memcpy(&d, &bits, sizeof(d));
	return (d);
}

static inline uint64_t	in_f64b(double d)
{
	uint64_t	bits;

	__builtin_memcpy(&bits, &d, sizeof(bits));
	return (bits);
}

uint32_t		in_get(t_in *in, uint32_t n);
void			in_set(t_in *in, uint32_t n, uint32_t v);
uint64_t		in_getw(t_in *in, uint32_t n);
void			in_setw(t_in *in, uint32_t n, uint64_t v);
int				in_throw(t_in *in, const char *desc);
int				in_push(t_in *in, const t_dmethod *m, const uint32_t *args);
void			in_pop(t_in *in);
int				in_native(t_in *in, const t_dmethod *m, const uint32_t *args);
int				in_enter(t_in *in, const t_dmethod *m, const uint32_t *args);
int				in_unwind(t_in *in);
t_inop			in_op(uint8_t op);
const uint32_t	*in_gather(t_in *in, uint32_t *n);
uint64_t		in_narrow(uint32_t kind, uint64_t v);
double			in_fmod(double x, double y);
int32_t			in_d2i(double d);
int64_t			in_d2l(double d);
int				in_iop(t_in *in, uint32_t k, uint32_t *x, uint32_t y);
int				in_lop(t_in *in, uint32_t k, uint64_t *x, uint64_t y);
int				in_op_nop(t_in *in);
int				in_op_bad(t_in *in);
int				in_op_move(t_in *in);
int				in_op_move_result(t_in *in);
int				in_op_move_exception(t_in *in);
int				in_op_return(t_in *in);
int				in_op_const(t_in *in);
int				in_op_const_ref(t_in *in);
int				in_op_monitor(t_in *in);
int				in_op_check_cast(t_in *in);
int				in_op_instance_of(t_in *in);
int				in_op_array_length(t_in *in);
int				in_op_new_array(t_in *in);
int				in_op_filled(t_in *in);
int				in_op_fill_data(t_in *in);
int				in_op_goto(t_in *in);
int				in_op_switch(t_in *in);
int				in_op_cmp(t_in *in);
int				in_op_if(t_in *in);
int				in_op_aget(t_in *in);
int				in_op_aput(t_in *in);
int				in_op_field(t_in *in);
int				in_op_invoke(t_in *in);
int				in_op_un_int(t_in *in);
int				in_op_un_long(t_in *in);
int				in_op_un_float(t_in *in);
int				in_op_un_double(t_in *in);
int				in_op_ibin(t_in *in);
int				in_op_ilit(t_in *in);
int				in_op_lbin(t_in *in);
int				in_op_fbin(t_in *in);
int				in_op_dbin(t_in *in);

#endif
