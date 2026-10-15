#include <string.h>
#include "velum/velum.h"
#include "velum/luna.h"
#include "velum/wm.h"
#include "wstress.h"

uint32_t	wstress_free_kb(void)
{
	t_sysinfo	si;

	if (v_sysinfo(&si) < 0)
		return (0);
	return ((uint32_t)(si.mem_free / 1024));
}

static int	connect_patiently(void)
{
	int	tries;
	int	r;

	tries = 0;
	r = wmc_connect(NULL);
	while (r < 0 && tries < WSTRESS_RETRIES)
	{
		v_sleep(WSTRESS_RETRY_NS);
		r = wmc_connect(NULL);
		tries++;
	}
	return (r);
}

int	wstress_cycle(void)
{
	t_wmcreate	rq;
	t_wmwin		w;
	int			r;

	r = connect_patiently();
	if (r < 0)
		return (r);
	memset(&rq, 0, sizeof(rq));
	rq.rect = rect_make(100, 80, WSTRESS_WIDTH, WSTRESS_HEIGHT);
	rq.style = WS_DEFAULT;
	rq.state = WSTATE_NORMAL;
	strlcpy(rq.title, "wstress", sizeof(rq.title));
	r = wmc_create(&w, &rq);
	if (r >= 0)
		r = wmc_destroy(&w);
	wmc_disconnect();
	return (r);
}
