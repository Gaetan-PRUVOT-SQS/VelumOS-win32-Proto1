#include "alloc_int.h"
#include "velum/vmisc.h"
#include "velum/vproc.h"

static const char	*g_fault_msg[4] = {"heap: erreur inconnue",
	"heap: double liberation", "heap: pointeur invalide",
	"heap: bloc corrompu"};

void	alloc_fault(int code)
{
	g_alloc.stats.faults++;
	if (g_alloc.fault_hook)
		g_alloc.fault_hook(code);
	else
		alloc_fault_default(code);
}

void	alloc_fault_default(int code)
{
	if (code < 0 || code > ALLOC_FAULT_CORRUPT)
		code = 0;
	v_log(V_LOG_ERR, g_fault_msg[code]);
	v_exit(ALLOC_FAULT_EXIT_CODE);
}
