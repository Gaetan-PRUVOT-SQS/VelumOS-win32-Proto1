#include <string.h>
#include "harness.h"
#include "time_int.h"
#include "velum/err.h"

static void	sys_priv(void)
{
	t_process	p;

	memset(&p, 0, sizeof(p));
	h_eq_i64("processus inconnu", a05_check_priv(NULL, PF_ADMIN), E_PERM);
	h_eq_i64("inconnu sans exigence", a05_check_priv(NULL, 0), E_PERM);
	h_eq_i64("sans bit", a05_check_priv(&p, PF_ADMIN), E_PERM);
	p.flags = PF_POWER;
	h_eq_i64("autre bit", a05_check_priv(&p, PF_ADMIN), E_PERM);
	h_eq_i64("bit exige", a05_check_priv(&p, PF_POWER), E_OK);
	h_eq_i64("deux bits dont un absent", a05_check_priv(&p,
			PF_POWER | PF_ADMIN), E_PERM);
	p.flags = PF_ALL;
	h_eq_i64("tous les bits", a05_check_priv(&p, PF_POWER | PF_ADMIN), E_OK);
	h_eq_i64("sans exigence", a05_check_priv(&p, 0), E_OK);
}

static void	sys_power_ops(void)
{
	h_eq_i64("extinction", a05_check_power(POWER_OFF), E_OK);
	h_eq_i64("redemarrage", a05_check_power(POWER_REBOOT), E_OK);
	h_eq_i64("operation 2", a05_check_power(2), E_INVAL);
	h_eq_i64("operation maximale", a05_check_power(UINT64_MAX), E_INVAL);
}

int	main(void)
{
	h_begin("a05/sys");
	h_run("privileges des appels", sys_priv);
	h_run("operations d'energie", sys_power_ops);
	return (h_end());
}
