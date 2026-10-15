#include <stdlib.h>
#include "fake.h"

t_fakeinsp	g_insp;
t_fakemem	g_mem;

int	fake_inspect(t_span apk, t_pminfo *out)
{
	(void)apk;
	g_insp.calls++;
	if (g_insp.err < 0)
		return (g_insp.err);
	*out = g_insp.info;
	return (0);
}

void	fake_app(const char *pkg, uint32_t version, uint8_t signer)
{
	memset(&g_insp.info, 0, sizeof(g_insp.info));
	strlcpy(g_insp.info.package, pkg, PM_PKG_MAX);
	strlcpy(g_insp.info.label, "Appli", PM_LABEL_MAX);
	strlcpy(g_insp.info.activity, "Lx/y/Main;", PM_ACT_MAX);
	memset(g_insp.info.cert, signer, PM_CERT_LEN);
	g_insp.info.version_code = version;
}

void	fake_mem_reset(int fail_at)
{
	g_mem.calls = 0;
	g_mem.fail_at = fail_at;
}

void	*pm_alloc(size_t size)
{
	g_mem.calls++;
	if (g_mem.fail_at > 0 && g_mem.calls == g_mem.fail_at)
		return (NULL);
	g_mem.live++;
	return (malloc(size));
}

void	pm_free(void *ptr)
{
	if (!ptr)
		return ;
	g_mem.live--;
	free(ptr);
}
