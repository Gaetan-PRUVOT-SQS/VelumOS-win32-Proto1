#include "velum/boot.h"
#include "velum/err.h"
#include "velum/klog.h"
#include "net_int.h"

int	net_sys_register(void);

static void	report_lease(void)
{
	klog_info("net: bail %u.%u.%u.%u masque %u.%u.%u.%u passerelle "
		"%u.%u.%u.%u dns %u.%u.%u.%u (%u s)",
		g_net.ip >> 24, (g_net.ip >> 16) & 255, (g_net.ip >> 8) & 255,
		g_net.ip & 255, g_net.mask >> 24, (g_net.mask >> 16) & 255,
		(g_net.mask >> 8) & 255, g_net.mask & 255, g_net.gw >> 24,
		(g_net.gw >> 16) & 255, (g_net.gw >> 8) & 255, g_net.gw & 255,
		g_net.dns >> 24, (g_net.dns >> 16) & 255, (g_net.dns >> 8) & 255,
		g_net.dns & 255, g_net.lease_s);
}

static void	ping_gateway(void)
{
	uint64_t	rtt;
	int			rc;

	if (g_net.gw == 0)
		return ;
	rtt = 0;
	rc = net_ping(g_net.gw, 2000000000ull, &rtt);
	if (rc == 0)
		klog_info("net: écho de la passerelle reçu en %u us",
			(unsigned)(rtt / 1000));
	else
		klog_warn("net: écho de la passerelle sans réponse (%d)", rc);
}

int	net_boot_init(void)
{
	int	rc;

	if (!boot_cmdline_has("net"))
		return (0);
	rc = vnet_boot();
	if (rc < 0)
	{
		klog_warn("net: aucune carte utilisable (%d)", rc);
		return (0);
	}
	if (net_sys_register() < 0)
		klog_warn("net: appels système incomplets");
	rc = net_dhcp_run();
	if (rc < 0)
	{
		klog_warn("net: DHCP sans bail (%d)", rc);
		return (0);
	}
	report_lease();
	ping_gateway();
	return (0);
}
