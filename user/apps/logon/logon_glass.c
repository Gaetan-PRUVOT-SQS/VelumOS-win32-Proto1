#include "velum/err.h"
#include "velum/luna.h"
#include "logon_glass.h"

static uint32_t	g_pool[LOGON_GLASS_PX];

int	logon_glass_attach(uint32_t *buf, size_t cap_px, t_rect card)
{
	if (buf == NULL || cap_px < luna_glass_need(card))
	{
		luna_glass_scratch(NULL, 0);
		return (E_NOMEM);
	}
	luna_glass_scratch(buf, cap_px);
	return (0);
}

int	logon_glass_open(t_rect card)
{
	return (logon_glass_attach(g_pool, LOGON_GLASS_PX, card));
}

void	logon_glass_close(void)
{
	luna_glass_scratch(NULL, 0);
}
