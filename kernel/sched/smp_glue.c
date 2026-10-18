#include "sched_int.h"

#if defined(__has_include)
# if __has_include("velum/smp.h")
#  include "velum/smp.h"
#  define SCHED_AVEC_SMP 1
# endif
#endif

void	sched_smp_kick(uint32_t cpu)
{
#ifdef SCHED_AVEC_SMP
	smp_send_ipi(cpu, VEC_RESCHED);
#else
	(void)cpu;
#endif
}
