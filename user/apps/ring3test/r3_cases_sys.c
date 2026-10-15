#include "r3.h"

static void	r3_logs(t_r3 *t)
{
	char		big[R3_LOG_LEN + 8];
	char		*p;
	const char	*ctl;

	p = big;
	ctl = "r3 \n\033[2J faux";
	memset(p, 'a', sizeof(big));
	r3_check(t, "log pointeur noyau",
		r3_sys(SYS_LOG, V_LOG_INFO, R3_KPTR, 16), E_FAULT);
	r3_check(t, "log 201 octets",
		r3_sys(SYS_LOG, V_LOG_INFO, (uint64_t)p, R3_LOG_LEN + 1), E_INVAL);
	r3_check(t, "log niveau 9", r3_sys(SYS_LOG, 9, (uint64_t)p, 1), E_INVAL);
	r3_check(t, "log caractères de contrôle", r3_sys(SYS_LOG, V_LOG_INFO,
			(uint64_t)ctl, strlen(ctl)), 0);
	r3_check(t, "log longueur nulle", r3_sys(SYS_LOG, V_LOG_INFO, 0, 0), 0);
}

void	r3_suite_sys(t_r3 *t)
{
	r3_check(t, "appel inconnu 0xfe", r3_sys(0xfe, 0, 0, 0), E_NOSYS);
	r3_check(t, "appel 0xff", r3_sys(SYS_MAX, 0, 0, 0), E_NOSYS);
	r3_check(t, "numéro 64 bits géant",
		r3_sys(0x7fffffffffffull, 0, 0, 0), E_NOSYS);
	r3_logs(t);
	r3_check(t, "gettid positif", r3_sys(SYS_GETTID, 0, 0, 0) > 0, 1);
	r3_check(t, "fsbase noyau refusée",
		r3_sys(SYS_SET_FSBASE, R3_HHDM, 0, 0), E_INVAL);
	r3_check(t, "fsbase nulle", r3_sys(SYS_SET_FSBASE, 0, 0, 0), 0);
}
