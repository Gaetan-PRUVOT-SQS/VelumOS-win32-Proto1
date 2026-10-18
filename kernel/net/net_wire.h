#ifndef NET_WIRE_H
# define NET_WIRE_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>

# define ETH_HLEN 14
# define ETH_P_IP 0x0800
# define ETH_P_ARP 0x0806
# define ARP_LEN 28
# define ARP_REQUEST 1
# define ARP_REPLY 2
# define IP_HLEN 20
# define IP_PROTO_ICMP 1
# define IP_PROTO_UDP 17
# define IP_TTL 64
# define ICMP_ECHO_REPLY 0
# define ICMP_ECHO_REQUEST 8
# define ICMP_HLEN 8
# define UDP_HLEN 8
# define DHCP_FIXED 240
# define DHCP_SERVER_PORT 67
# define DHCP_CLIENT_PORT 68
# define DHCP_DISCOVER 1
# define DHCP_OFFER 2
# define DHCP_REQUEST 3
# define DHCP_ACK 5
# define DHCP_NAK 6
# define DHCP_MAX 300

typedef struct s_ethinfo
{
	uint8_t			dst[6];
	uint8_t			src[6];
	uint16_t		type;
	const uint8_t	*data;
	size_t			len;
}	t_ethinfo;

typedef struct s_arpinfo
{
	uint16_t	op;
	uint8_t		sha[6];
	uint32_t	spa;
	uint8_t		tha[6];
	uint32_t	tpa;
}	t_arpinfo;

typedef struct s_ipinfo
{
	uint32_t		src;
	uint32_t		dst;
	uint8_t			proto;
	uint8_t			ttl;
	uint16_t		id;
	const uint8_t	*data;
	size_t			len;
}	t_ipinfo;

typedef struct s_icmpinfo
{
	uint8_t			type;
	uint16_t		id;
	uint16_t		seq;
	const uint8_t	*data;
	size_t			len;
}	t_icmpinfo;

typedef struct s_udpinfo
{
	uint16_t		sport;
	uint16_t		dport;
	const uint8_t	*data;
	size_t			len;
}	t_udpinfo;

typedef struct s_dhcpinfo
{
	uint8_t		type;
	uint32_t	yiaddr;
	uint32_t	mask;
	uint32_t	router;
	uint32_t	dns;
	uint32_t	server;
	uint32_t	lease;
}	t_dhcpinfo;

static inline uint16_t	rd16(const uint8_t *p)
{
	return ((uint16_t)((p[0] << 8) | p[1]));
}

static inline uint32_t	rd32(const uint8_t *p)
{
	return (((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16)
		| ((uint32_t)p[2] << 8) | p[3]);
}

static inline void	wr16(uint8_t *p, uint16_t v)
{
	p[0] = (uint8_t)(v >> 8);
	p[1] = (uint8_t)v;
}

static inline void	wr32(uint8_t *p, uint32_t v)
{
	p[0] = (uint8_t)(v >> 24);
	p[1] = (uint8_t)(v >> 16);
	p[2] = (uint8_t)(v >> 8);
	p[3] = (uint8_t)v;
}

uint32_t	net_sum(const uint8_t *p, size_t n, uint32_t acc);
uint16_t	net_fold(uint32_t acc);
uint16_t	net_cksum(const uint8_t *p, size_t n);
int			eth_parse(const uint8_t *f, size_t n, t_ethinfo *out);
size_t		eth_build(uint8_t *buf, const uint8_t dst[6], const uint8_t src[6],
				uint16_t type);
int			arp_parse(const uint8_t *p, size_t n, t_arpinfo *out);
size_t		arp_build(uint8_t *buf, const t_arpinfo *a);
int			ip_parse(const uint8_t *p, size_t n, t_ipinfo *out);
size_t		ip_build(uint8_t *buf, const t_ipinfo *h);
int			icmp_parse(const uint8_t *p, size_t n, t_icmpinfo *out);
size_t		icmp_build(uint8_t *buf, const t_icmpinfo *e);
int			udp_parse(const t_ipinfo *ip, t_udpinfo *out);
size_t		udp_build(uint8_t *buf, const t_ipinfo *ip, const t_udpinfo *u);
size_t		dhcp_build(uint8_t *buf, size_t cap, uint8_t type,
				const uint8_t mac[6], uint32_t xid, uint32_t req_ip,
				uint32_t server);
int			dhcp_parse(const uint8_t *p, size_t n, uint32_t xid,
				const uint8_t mac[6], t_dhcpinfo *out);

#endif
