#include "velum/err.h"
#include "velum/libk.h"
#include "net_wire.h"

#define DHCP_MAGIC 0x63825363u
#define OPT_PAD 0
#define OPT_MASK 1
#define OPT_ROUTER 3
#define OPT_DNS 6
#define OPT_REQ_IP 50
#define OPT_LEASE 51
#define OPT_TYPE 53
#define OPT_SERVER 54
#define OPT_PARAMS 55
#define OPT_END 255

static size_t	opt_put(uint8_t *o, uint8_t code, const uint8_t *v, uint8_t n)
{
	o[0] = code;
	o[1] = n;
	memcpy(o + 2, v, n);
	return ((size_t)n + 2);
}

static size_t	opts_build(uint8_t *o, uint8_t type, uint32_t req_ip,
		uint32_t server)
{
	static const uint8_t	params[] = {OPT_MASK, OPT_ROUTER, OPT_DNS,
		OPT_LEASE};
	uint8_t					v[4];
	size_t					n;

	v[0] = type;
	n = opt_put(o, OPT_TYPE, v, 1);
	if (req_ip)
	{
		wr32(v, req_ip);
		n += opt_put(o + n, OPT_REQ_IP, v, 4);
	}
	if (server)
	{
		wr32(v, server);
		n += opt_put(o + n, OPT_SERVER, v, 4);
	}
	n += opt_put(o + n, OPT_PARAMS, params, sizeof(params));
	o[n++] = OPT_END;
	return (n);
}

size_t	dhcp_build(uint8_t *buf, size_t cap, uint8_t type,
		const uint8_t mac[6], uint32_t xid, uint32_t req_ip, uint32_t server)
{
	if (cap < DHCP_MAX)
		return (0);
	memset(buf, 0, DHCP_MAX);
	buf[0] = 1;
	buf[1] = 1;
	buf[2] = 6;
	wr32(buf + 4, xid);
	wr16(buf + 10, 0x8000);
	memcpy(buf + 28, mac, 6);
	wr32(buf + 236, DHCP_MAGIC);
	return (DHCP_FIXED + opts_build(buf + DHCP_FIXED, type, req_ip, server));
}

static int	opt_store(t_dhcpinfo *out, uint8_t code, const uint8_t *v,
		uint8_t n)
{
	if (code == OPT_TYPE && n == 1)
		out->type = v[0];
	else if (code == OPT_MASK && n == 4)
		out->mask = rd32(v);
	else if (code == OPT_ROUTER && n >= 4 && n % 4 == 0)
		out->router = rd32(v);
	else if (code == OPT_DNS && n >= 4 && n % 4 == 0)
		out->dns = rd32(v);
	else if (code == OPT_SERVER && n == 4)
		out->server = rd32(v);
	else if (code == OPT_LEASE && n == 4)
		out->lease = rd32(v);
	return (0);
}

static int	opts_parse(const uint8_t *o, size_t n, t_dhcpinfo *out)
{
	size_t	i;

	i = 0;
	while (i < n && o[i] != OPT_END)
	{
		if (o[i] == OPT_PAD)
		{
			i++;
			continue ;
		}
		if (i + 1 >= n || i + 2 + o[i + 1] > n)
			return (E_INVAL);
		opt_store(out, o[i], o + i + 2, o[i + 1]);
		i += 2 + (size_t)o[i + 1];
	}
	if (i >= n)
		return (E_INVAL);
	return (0);
}

int	dhcp_parse(const uint8_t *p, size_t n, uint32_t xid,
		const uint8_t mac[6], t_dhcpinfo *out)
{
	if (!p || !mac || !out || n < DHCP_FIXED)
		return (E_INVAL);
	if (p[0] != 2 || p[1] != 1 || p[2] != 6 || rd32(p + 4) != xid)
		return (E_INVAL);
	if (memcmp(p + 28, mac, 6) != 0 || rd32(p + 236) != DHCP_MAGIC)
		return (E_INVAL);
	memset(out, 0, sizeof(*out));
	out->yiaddr = rd32(p + 16);
	if (opts_parse(p + DHCP_FIXED, n - DHCP_FIXED, out) < 0)
		return (E_INVAL);
	if (out->type == 0)
		return (E_INVAL);
	return (0);
}
