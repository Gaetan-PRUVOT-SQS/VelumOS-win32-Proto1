#include <string.h>
#include "init.h"
#include "velum/velum.h"

static void	init_services(t_service *sv)
{
	memset(sv, 0, sizeof(t_service) * INIT_SERVICES);
	sv[0].path = "/system/bin/winsrv";
	sv[0].flags = PF_DISPLAY | PF_INPUT | PF_LISTEN;
	sv[0].enabled = 1;
	sv[1].path = "/system/bin/logon";
	sv[1].flags = PF_SPAWN | PF_POWER;
	sv[1].enabled = 1;
}

static uint64_t	init_now(void)
{
	int64_t	t;

	t = v_time_mono();
	if (t < 0)
		return (0);
	return ((uint64_t)t);
}

static uint32_t	init_collect(const t_service *sv, t_handle *hs, uint32_t *map)
{
	uint32_t	i;
	uint32_t	n;

	n = 0;
	i = 0;
	while (i < INIT_SERVICES)
	{
		if (sv[i].enabled && sv[i].h != 0)
		{
			hs[n] = sv[i].h;
			map[n] = i;
			n++;
		}
		i++;
	}
	return (n);
}

static void	init_wait(t_service *sv, t_killtest *kt)
{
	t_handle	hs[INIT_SERVICES];
	uint32_t	map[INIT_SERVICES];
	uint32_t	n;
	int			r;

	n = init_collect(sv, hs, map);
	r = E_INVAL;
	if (n > 0)
		r = v_wait_many(hs, n, WAIT_ANY, killtest_timeout(kt));
	if (r == E_TIMEOUT)
		killtest_tick(kt, &sv[0], init_now());
	else if (r < 0 || (uint32_t)r >= n)
		v_sleep(INIT_IDLE_NS);
	else
		service_died(&sv[map[r]], init_now());
}

int	main(int argc, char **argv)
{
	t_service	sv[INIT_SERVICES];
	t_killtest	kt;
	uint32_t	i;

	init_services(sv);
	killtest_setup(&kt, argc, argv);
	v_log(V_LOG_INFO, "init: démarrage de la session système");
	i = 0;
	while (i < INIT_SERVICES)
	{
		service_start(&sv[i]);
		i++;
	}
	while (1)
		init_wait(sv, &kt);
	return (0);
}
