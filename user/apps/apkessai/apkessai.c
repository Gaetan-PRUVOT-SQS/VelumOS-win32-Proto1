#include "apkessai.h"

static const t_essai	g_steps[] = {
{ESSAI_INST, "/data/depot/ok.apk", PF_FSWRITE, 0},
{ESSAI_INST, "/data/depot/ok.apk", PF_FSWRITE, 0},
{ESSAI_INST, "/data/depot/altere.apk", PF_FSWRITE, 0},
{ESSAI_INST, "/data/depot/nonsigne.apk", PF_FSWRITE, 0},
{ESSAI_INST, "/data/depot/autrecle.apk", PF_FSWRITE, 0},
{ESSAI_INST, "/data/depot/absent.apk", PF_FSWRITE, 0},
{ESSAI_RUN, "/data/apps/com.velum.bonjour.apk", 0, 1},
{ESSAI_RUN, "/data/depot/altere.apk", 0, 1}
};

int	essai_step(uint32_t rank, const t_essai *e)
{
	const char	*argv[3];
	int64_t		h;

	argv[0] = e->prog;
	argv[1] = e->arg;
	argv[2] = NULL;
	v_logf(V_LOG_INFO, "APKESSAI etape %u %s", rank, e->arg);
	h = v_spawnv(e->prog, argv, e->flags);
	if (h < 0)
		return ((int)h);
	if (e->show)
	{
		v_sleep(ESSAI_SHOW_NS);
		v_logf(V_LOG_INFO, "APKESSAI etape %u affichee", rank);
		v_sleep(ESSAI_SHOW_NS);
		v_proc_kill((t_handle)h, 0);
	}
	if (v_wait((t_handle)h, ESSAI_WAIT_NS) != 0)
		return (E_TIMEOUT);
	v_close((t_handle)h);
	return (0);
}

int	main(void)
{
	uint32_t	i;
	int			r;

	v_log(V_LOG_INFO, "APKESSAI start");
	if (v_spawn(ESSAI_SERVER, NULL, 0, PF_DISPLAY | PF_INPUT | PF_LISTEN) < 0)
	{
		v_log(V_LOG_ERR, "APKESSAI FAIL serveur");
		return (1);
	}
	v_sleep(ESSAI_SETTLE_NS);
	i = 0;
	r = 0;
	while (r == 0 && i < sizeof(g_steps) / sizeof(g_steps[0]))
	{
		r = essai_step(i + 1, &g_steps[i]);
		i++;
	}
	if (r < 0)
		v_logf(V_LOG_ERR, "APKESSAI FAIL etape %u code %d", i, r);
	else
		v_logf(V_LOG_INFO, "APKESSAI PASS %u etapes", i);
	while (1)
		v_sleep(ESSAI_WAIT_NS);
	return (0);
}
