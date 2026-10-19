#include <string.h>
#include "harness.h"
#include "net_wire.h"

#define IP_LOCAL 0x0a00020fu
#define IP_GW 0x0a000202u

static void	cksum_rfc1071(void)
{
	static const uint8_t	ex[8] = {0x00, 0x01, 0xf2, 0x03, 0xf4, 0xf5, 0xf6,
		0xf7};

	h_eq_u64("somme repliee RFC 1071", (uint16_t)~net_fold(net_sum(ex, 8, 0)),
		0xddf2);
	h_eq_u64("complement RFC 1071", net_cksum(ex, 8), 0x220d);
	h_eq_u64("longueur impaire", net_cksum(ex, 7),
		0x2304);
}

static void	ip_en_tete_connu(void)
{
	uint8_t		buf[115];
	t_ipinfo	h;
	t_ipinfo	back;

	memset(buf, 0, sizeof(buf));
	h.src = 0xc0a80001u;
	h.dst = 0xc0a800c7u;
	h.proto = 17;
	h.ttl = 0x40;
	h.id = 0;
	h.len = 95;
	h.data = NULL;
	h_eq_u64("longueur d'en-tete", ip_build(buf, &h), 20);
	h_eq_u64("somme du liseré IPv4", rd16(buf + 10), 0xb861);
	h_eq_i64("analyse", ip_parse(buf, sizeof(buf), &back), 0);
	h_eq_u64("source", back.src, 0xc0a80001u);
	h_eq_u64("longueur utile", back.len, 95);
}

static void	udp_somme_connue(void)
{
	uint8_t		buf[11];
	t_ipinfo	ip;
	t_udpinfo	u;
	t_udpinfo	back;

	ip.src = IP_LOCAL;
	ip.dst = IP_GW;
	ip.proto = 17;
	ip.len = 11;
	u.sport = 1234;
	u.dport = 5678;
	u.data = (const uint8_t *)"abc";
	u.len = 3;
	h_eq_u64("longueur UDP", udp_build(buf, &ip, &u), 11);
	h_eq_u64("somme UDP avec pseudo-en-tete", rd16(buf + 6), 0x0865);
	ip.data = buf;
	h_eq_i64("analyse", udp_parse(&ip, &back), 0);
	h_eq_u64("port destination", back.dport, 5678);
	buf[8] ^= 1;
	h_true(udp_parse(&ip, &back) < 0, "donnee alteree refusee");
}

static void	icmp_somme_connue(void)
{
	uint8_t		buf[12];
	t_icmpinfo	m;
	t_icmpinfo	back;

	m.type = ICMP_ECHO_REQUEST;
	m.id = 0x5656;
	m.seq = 1;
	m.data = (const uint8_t *)"\0\1\2\3";
	m.len = 4;
	h_eq_u64("longueur ICMP", icmp_build(buf, &m), 12);
	h_eq_u64("somme ICMP", rd16(buf + 2), 0x9fa4);
	h_eq_i64("analyse", icmp_parse(buf, 12, &back), 0);
	h_eq_u64("sequence", back.seq, 1);
}

static void	ip_hostile(void)
{
	uint8_t		buf[60];
	t_ipinfo	h;
	t_ipinfo	out;

	memset(buf, 0, sizeof(buf));
	h.src = IP_LOCAL;
	h.dst = IP_GW;
	h.proto = 17;
	h.ttl = 64;
	h.id = 7;
	h.len = 40;
	ip_build(buf, &h);
	h_eq_i64("valide", ip_parse(buf, 60, &out), 0);
	h_true(ip_parse(buf, 19, &out) < 0, "tronque sous 20 octets");
	h_true(ip_parse(buf, 59, &out) < 0, "longueur declaree > recue");
	buf[0] = 0x44;
	h_true(ip_parse(buf, 60, &out) < 0, "IHL de 4");
	buf[0] = 0x65;
	h_true(ip_parse(buf, 60, &out) < 0, "version 6");
	buf[0] = 0x45;
	buf[15] ^= 0x80;
	h_true(ip_parse(buf, 60, &out) < 0, "somme fausse");
	ip_build(buf, &h);
	buf[6] |= 0x20;
	wr16(buf + 10, 0);
	wr16(buf + 10, net_cksum(buf, 20));
	h_eq_i64("fragment refuse", ip_parse(buf, 60, &out), -95);
	ip_build(buf, &h);
	buf[0] = 0x4f;
	h_true(ip_parse(buf, 60, &out) < 0, "IHL 60 > total");
}

static void	arp_hostile(void)
{
	uint8_t		buf[ARP_LEN];
	t_arpinfo	a;

	a.op = ARP_REQUEST;
	memset(a.sha, 1, 6);
	memset(a.tha, 0, 6);
	a.spa = IP_GW;
	a.tpa = IP_LOCAL;
	arp_build(buf, &a);
	h_eq_i64("valide", arp_parse(buf, ARP_LEN, &a), 0);
	h_true(arp_parse(buf, ARP_LEN - 1, &a) < 0, "tronque");
	buf[4] = 8;
	h_true(arp_parse(buf, ARP_LEN, &a) < 0, "hlen 8");
	buf[4] = 6;
	buf[7] = 9;
	h_true(arp_parse(buf, ARP_LEN, &a) < 0, "operation 9");
}

static void	udp_hostile(void)
{
	uint8_t		buf[16];
	t_ipinfo	ip;
	t_udpinfo	u;

	memset(buf, 0, sizeof(buf));
	ip.proto = 17;
	ip.data = buf;
	ip.len = 7;
	h_true(udp_parse(&ip, &u) < 0, "moins de 8 octets");
	ip.len = 16;
	wr16(buf + 4, 7);
	h_true(udp_parse(&ip, &u) < 0, "longueur UDP < 8");
	wr16(buf + 4, 17);
	h_true(udp_parse(&ip, &u) < 0, "longueur UDP > IP");
	wr16(buf + 4, 16);
	h_eq_i64("somme nulle acceptee", udp_parse(&ip, &u), 0);
}

static size_t	dhcp_reply(uint8_t *b, uint32_t xid, const uint8_t *mac)
{
	static const uint8_t	opts[] = {53, 1, 2, 1, 4, 255, 255, 255, 0, 3, 4, 10,
		0, 2, 2, 6, 4, 10, 0, 2, 3, 51, 4, 0, 0, 14, 16, 54, 4, 10, 0, 2, 2,
		255};

	memset(b, 0, 300);
	b[0] = 2;
	b[1] = 1;
	b[2] = 6;
	wr32(b + 4, xid);
	wr32(b + 16, IP_LOCAL);
	memcpy(b + 28, mac, 6);
	wr32(b + 236, 0x63825363u);
	memcpy(b + 240, opts, sizeof(opts));
	return (240 + sizeof(opts));
}

static void	dhcp_valide_et_hostile(void)
{
	static const uint8_t	mac[6] = {1, 2, 3, 4, 5, 6};
	uint8_t					b[300];
	t_dhcpinfo				d;
	size_t					n;

	n = dhcp_reply(b, 0xabcdef01u, mac);
	h_eq_i64("offre valide", dhcp_parse(b, n, 0xabcdef01u, mac, &d), 0);
	h_eq_u64("type", d.type, 2);
	h_eq_u64("adresse", d.yiaddr, IP_LOCAL);
	h_eq_u64("masque", d.mask, 0xffffff00u);
	h_eq_u64("routeur", d.router, IP_GW);
	h_eq_u64("dns", d.dns, 0x0a000203u);
	h_eq_u64("bail", d.lease, 3600);
	h_eq_u64("serveur", d.server, IP_GW);
	h_true(dhcp_parse(b, n, 0x11111111u, mac, &d) < 0, "mauvais xid");
	h_true(dhcp_parse(b, 239, 0xabcdef01u, mac, &d) < 0, "sous 240 octets");
	b[236] ^= 1;
	h_true(dhcp_parse(b, n, 0xabcdef01u, mac, &d) < 0, "cookie faux");
	b[236] ^= 1;
	b[244] = 200;
	h_true(dhcp_parse(b, n, 0xabcdef01u, mac, &d) < 0, "option debordante");
	b[244] = 4;
	h_true(dhcp_parse(b, 242, 0xabcdef01u, mac, &d) < 0, "sans fin d'options");
}

static void	dhcp_construction(void)
{
	static const uint8_t	mac[6] = {1, 2, 3, 4, 5, 6};
	uint8_t					b[DHCP_MAX];
	size_t					n;

	h_eq_u64("tampon trop court", dhcp_build(b, DHCP_MAX - 1, 1, mac, 1, 0, 0),
		0);
	n = dhcp_build(b, sizeof(b), DHCP_REQUEST, mac, 0x01020304u, IP_LOCAL,
			IP_GW);
	h_true(n > 240, "longueur");
	h_eq_u64("op", b[0], 1);
	h_eq_u64("xid", rd32(b + 4), 0x01020304u);
	h_eq_u64("drapeau broadcast", rd16(b + 10), 0x8000);
	h_eq_u64("cookie", rd32(b + 236), 0x63825363u);
	h_eq_u64("option 53", b[240], 53);
	h_eq_u64("type REQUEST", b[242], 3);
	h_eq_u64("fin", b[n - 1], 255);
}

static uint64_t	g_seed = 88172645463325252ull;

static uint8_t	rnd(void)
{
	g_seed ^= g_seed << 13;
	g_seed ^= g_seed >> 7;
	g_seed ^= g_seed << 17;
	return ((uint8_t)g_seed);
}

static void	parse_all(const uint8_t *b, size_t n)
{
	static const uint8_t	mac[6] = {1, 2, 3, 4, 5, 6};
	t_ethinfo				e;
	t_arpinfo				a;
	t_ipinfo				ip;
	t_icmpinfo				m;
	t_udpinfo				u;
	t_dhcpinfo				d;

	eth_parse(b, n, &e);
	arp_parse(b, n, &a);
	icmp_parse(b, n, &m);
	dhcp_parse(b, n, 0xabcdef01u, mac, &d);
	if (ip_parse(b, n, &ip) == 0)
		udp_parse(&ip, &u);
}

static void	fuzz_mutations(void)
{
	static const uint8_t	mac[6] = {1, 2, 3, 4, 5, 6};
	uint8_t					base[300];
	uint8_t					b[300];
	size_t					n;
	int						i;
	int						k;

	n = dhcp_reply(base, 0xabcdef01u, mac);
	i = 0;
	while (i < 20000)
	{
		memcpy(b, base, sizeof(b));
		k = rnd() % 4;
		while (k-- >= 0)
			b[rnd() % n] = rnd();
		parse_all(b, rnd() % (n + 1));
		i++;
	}
	h_true(1, "20000 mutations DHCP sans defaut");
}

static void	fuzz_aleatoire(void)
{
	uint8_t	b[200];
	int		i;
	size_t	j;

	i = 0;
	while (i < 20000)
	{
		j = 0;
		while (j < sizeof(b))
			b[j++] = rnd();
		if (i % 2)
			b[0] = 0x45;
		parse_all(b, rnd() % (sizeof(b) + 1));
		i++;
	}
	h_true(1, "20000 tampons aleatoires sans defaut");
}

int	main(void)
{
	h_begin("e19_wire");
	h_run("cksum_rfc1071", cksum_rfc1071);
	h_run("ip_en_tete_connu", ip_en_tete_connu);
	h_run("udp_somme_connue", udp_somme_connue);
	h_run("icmp_somme_connue", icmp_somme_connue);
	h_run("ip_hostile", ip_hostile);
	h_run("arp_hostile", arp_hostile);
	h_run("udp_hostile", udp_hostile);
	h_run("dhcp_valide_et_hostile", dhcp_valide_et_hostile);
	h_run("dhcp_construction", dhcp_construction);
	h_run("fuzz_mutations", fuzz_mutations);
	h_run("fuzz_aleatoire", fuzz_aleatoire);
	return (h_end());
}
