#include "crt_int.h"
#include "velum/vmisc.h"
#include "velum/vproc.h"

_Noreturn void	crt_die(const char *msg, int code)
{
	v_log(V_LOG_ERR, msg);
	v_exit(code);
}

_Noreturn void	__stack_chk_fail(void)
{
	crt_die("stack smashing detected", CRT_EXIT_SMASH);
}
