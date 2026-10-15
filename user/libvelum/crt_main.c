#include "alloc_int.h"
#include "crt_int.h"
#include "stdlib.h"
#include "string.h"
#include "velum/vheap.h"
#include "velum/vmisc.h"
#include "velum/vproc.h"
#include "velum/vstart.h"
#include "velum/vtime.h"
#include "velum/vtls.h"

static t_vtcb	g_main_tcb;

static int	crt_tls_init(void)
{
	t_vtcb	*tcb;

	tcb = &g_main_tcb;
	tcb->self = tcb;
	tcb->thread = NULL;
	if (v_set_fsbase((uint64_t)(uintptr_t)tcb) < 0)
		return (-1);
	tcb->tid = (uint32_t)v_gettid();
	return (0);
}

static uint64_t	crt_guard_fallback(const void *sp)
{
	uint64_t	g;

	g = (uint64_t)v_time_mono();
	g = g * 0x9e3779b97f4a7c15ull;
	g ^= (uint64_t)(uintptr_t)sp;
	return (g);
}

static uint64_t	crt_guard_value(const t_startinfo *info, const void *sp)
{
	uint64_t	g;

	g = 0;
	if (info->random)
		memcpy(&g, info->random, sizeof(g));
	else if (v_getrandom(&g, CRT_GUARD_LEN, 0) != CRT_GUARD_LEN)
		g = crt_guard_fallback(sp);
	g &= ~0xffull;
	if (!g)
		g = 0x595e9fbd94fda700ull;
	return (g);
}

static void	crt_prepare(const t_startinfo *info)
{
	uint64_t	seed;

	if (info->abi && info->abi != VELUM_ABI_VERSION)
		crt_die("crt: version d'ABI incompatible", CRT_EXIT_ABI);
	seed = 0;
	if (info->random)
		memcpy(&seed, info->random + sizeof(seed), sizeof(seed));
	alloc_seed(seed | 1);
	relro_lock(v_relro_start(), v_relro_end());
}

_Noreturn void	__velum_start(void *sp)
{
	t_startinfo	info;

	if (crt_tls_init() < 0)
		crt_die("crt: SET_FSBASE refuse", CRT_EXIT_FAIL);
	if (start_parse(sp, START_WORDS_MAX, &info) < 0)
		crt_die("crt: pile de depart invalide", CRT_EXIT_FAIL);
	v_guard_set(crt_guard_value(&info, sp));
	crt_prepare(&info);
	exit(main((int)info.argc, info.argv));
}
