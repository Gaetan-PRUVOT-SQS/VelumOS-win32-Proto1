#include <string.h>
#include "velum/err.h"
#include "fake.h"
#include "harness.h"
#include "net_int.h"

#define IP_LOCAL 0x0a00020fu
#define IP_GW 0x0a000202u
#define IP_DNS 0x0a000203u

static int	g_mode;
static uint32_t	g_xid;

static void	configure(void)
{
	fake_nic_attach();
	g_net.ip = IP_LOCAL;
	g_net.mask = 0xffffff00u;
	g_net.gw = IP_GW;
	g_net.etat = NET_ETAT_PRET;
}

static void	arp_request_pour_nous(void)
{
	uint8_t		f[60];
	t_arpinfo	a;
	size_t		n;
	size_t		len;
	const uint8_t	*tx;

	configure();
	memset(f, 0, sizeof(f));
	a.op = ARP_REQUEST;
	memcpy(a.sha, g_mac_peer, 6);
	memset(a.tha, 0, 6);
	a.spa = IP_GW;
	a.tpa = IP_LOCAL;
	n = eth_build(f, (const uint8_t[6]){255, 255, 255, 255, 255, 255},
			g_mac_peer, ETH_P_ARP);
	arp_build(f + n, &a);
	fake_nic_inject(f, 60);
	net_poll();
	h_eq_i64("une reponse", fake_nic_tx_count(), 1);
	tx = fake_nic_tx(0, &len);
	h_eq_u64("trame minimale", len, 60);
	h_true(memcmp(tx, g_mac_peer, 6) == 0, "destinataire unicast");
	h_eq_u64("type ARP", rd16(tx + 12), ETH_P_ARP);
	h_eq_u64("operation reponse", rd16(tx + 20), ARP_REPLY);
	h_true(memcmp(tx + 22, g_mac_local, 6) == 0, "notre MAC");
	h_eq_u64("notre IP", rd32(tx + 28), IP_LOCAL);
	h_eq_u64("IP du demandeur", rd32(tx + 38), IP_GW);
}

static void	arp_ignore_les_autres(void)
{
	uint8_t		f[60];
	t_arpinfo	a;
	size_t		n;

	configure();
	memset(f, 0, sizeof(f));
	a.op = ARP_REQUEST;
	memcpy(a.sha, g_mac_peer, 6);
	memset(a.tha, 0, 6);
	a.spa = IP_GW;
	a.tpa = IP_LOCAL + 1;
	n = eth_build(f, (const uint8_t[6]){255, 255, 255, 255, 255, 255},
			g_mac_peer, ETH_P_ARP);
	arp_build(f + n, &a);
	fake_nic_inject(f, 60);
	net_poll();
	h_eq_i64("autre cible: silence", fake_nic_tx_count(), 0);
	a.op = ARP_REPLY;
	a.spa = IP_DNS;
	arp_build(f + n, &a);
	fake_nic_inject(f, 60);
	net_poll();
	h_true(!arp_lookup_l(IP_DNS, a.sha), "reponse non sollicitee non apprise");
}

static void	echo_repond(void)
{
	uint8_t			f[200];
	uint8_t			data[32];
	size_t			n;
	size_t			len;
	const uint8_t	*tx;
	t_icmpinfo		m;

	configure();
	memset(data, 0x5a, sizeof(data));
	n = fake_icmp_frame(f, IP_GW, IP_LOCAL, ICMP_ECHO_REQUEST, 9, data, 32);
	fake_nic_inject(f, n);
	net_poll();
	h_eq_i64("une reponse", fake_nic_tx_count(), 1);
	tx = fake_nic_tx(0, &len);
	h_true(memcmp(tx, g_mac_peer, 6) == 0, "renvoye a l'emetteur");
	h_eq_i64("analyse de la reponse", icmp_parse(tx + 34, len - 34, &m), 0);
	h_eq_u64("type reponse", m.type, ICMP_ECHO_REPLY);
	h_eq_u64("sequence reprise", m.seq, 9);
	h_eq_u64("donnees reprises", m.len, 32);
	h_true(memcmp(m.data, data, 32) == 0, "contenu repris");
}

static void	echo_refuse(void)
{
	uint8_t	f[200];
	uint8_t	data[8];
	size_t	n;

	configure();
	memset(data, 1, sizeof(data));
	n = fake_icmp_frame(f, IP_GW, IP_LOCAL + 5, ICMP_ECHO_REQUEST, 1, data, 8);
	fake_nic_inject(f, n);
	n = fake_icmp_frame(f, IP_GW, IP_LOCAL, ICMP_ECHO_REQUEST, 1, data, 8);
	f[n - 1] ^= 0xff;
	fake_nic_inject(f, n);
	n = fake_icmp_frame(f, IP_GW, IP_LOCAL, ICMP_ECHO_REQUEST, 1, data, 8);
	f[ETH_HLEN + 6] |= 0x20;
	fake_nic_inject(f, n);
	fake_nic_inject(f, 13);
	net_poll();
	h_eq_i64("aucune reponse a des paquets invalides", fake_nic_tx_count(), 0);
}

static void	udp_livre_au_socket(void)
{
	uint8_t		f[200];
	uint8_t		out[10];
	int			id;
	t_sock		*s;
	t_dgram		d;
	size_t		n;

	configure();
	id = sock_alloc_l(7, NULL);
	h_true(id >= 0, "socket cree");
	s = sock_get_l((uint32_t)id, 7);
	h_eq_i64("bind 5000", sock_bind_l(s, 5000, false), 5000);
	memcpy(out, "bonjour", 7);
	n = fake_udp_frame(f, IP_GW, IP_LOCAL, 53, 5000, out, 7);
	fake_nic_inject(f, n);
	net_poll();
	h_eq_i64("recu", sock_take_l(s, &d), 0);
	h_eq_u64("longueur", d.len, 7);
	h_eq_u64("port source", d.port, 53);
	h_eq_u64("ip source", d.ip, IP_GW);
	h_true(memcmp(d.data, "bonjour", 7) == 0, "contenu");
	h_eq_i64("file vide", sock_take_l(s, &d), E_AGAIN);
	h_true(sock_get_l((uint32_t)id, 8) == NULL, "autre proprietaire refuse");
	sock_free_l(s);
	h_true(sock_get_l((uint32_t)id, 7) == NULL, "identifiant perime");
	h_eq_i64("rien ne fuit", fake_allocs(), 0);
}

static void	udp_sans_socket_et_file_pleine(void)
{
	uint8_t	f[200];
	size_t	n;
	int		id;
	t_sock	*s;
	int		i;
	uint64_t	avant;

	configure();
	n = fake_udp_frame(f, IP_GW, IP_LOCAL, 53, 6000, (const uint8_t *)"x", 1);
	avant = g_net.drops;
	fake_nic_inject(f, n);
	net_poll();
	h_eq_u64("port ferme: abandon compte", g_net.drops, avant + 1);
	h_eq_i64("pas de reponse", fake_nic_tx_count(), 0);
	id = sock_alloc_l(1, NULL);
	s = sock_get_l((uint32_t)id, 1);
	sock_bind_l(s, 6000, false);
	i = 0;
	while (i < NET_SOCK_QUEUE + 2)
	{
		fake_nic_inject(f, n);
		i++;
	}
	net_poll();
	h_eq_u64("file bornee", s->count, NET_SOCK_QUEUE);
	sock_free_l(s);
	h_eq_i64("rien ne fuit", fake_allocs(), 0);
}

static void	sockets_regles(void)
{
	int		a;
	int		b;
	t_sock	*sa;
	t_sock	*sb;
	int		i;

	configure();
	a = sock_alloc_l(1, NULL);
	b = sock_alloc_l(1, NULL);
	sa = sock_get_l((uint32_t)a, 1);
	sb = sock_get_l((uint32_t)b, 1);
	h_eq_i64("port privilegie refuse", sock_bind_l(sa, 80, false), E_PERM);
	h_eq_i64("port privilegie admin", sock_bind_l(sa, 80, true), 80);
	h_eq_i64("port deja pris", sock_bind_l(sb, 80, true), E_BUSY);
	h_eq_i64("double bind", sock_bind_l(sa, 81, true), E_INVAL);
	i = sock_bind_l(sb, 0, false);
	h_true(i >= NET_PORT_EPHEMERAL_MIN, "port ephemere");
	sock_free_l(sa);
	sock_free_l(sb);
	i = 0;
	while (i < NET_SOCKS)
		h_true(sock_alloc_l(1, NULL) >= 0, "table remplie"), i++;
	h_eq_i64("table pleine", sock_alloc_l(1, NULL), E_NFILE);
	i = 0;
	while (i < NET_SOCKS)
		sock_free_l(&g_net.socks[i++]);
	h_eq_i64("rien ne fuit", fake_allocs(), 0);
}

static void	echec_allocation(void)
{
	configure();
	fake_fail_alloc(0);
	h_eq_i64("allocation refusee", sock_alloc_l(1, NULL), E_NOMEM);
	fake_fail_alloc(-1);
	h_true(!g_net.socks[0].used, "emplacement non reserve");
	h_eq_i64("rien ne fuit", fake_allocs(), 0);
}

static bool	pid_vivant(uint32_t pid)
{
	return (pid != 99);
}

static void	reclamation_des_morts(void)
{
	int	i;

	configure();
	i = 0;
	while (i < NET_SOCKS)
	{
		sock_alloc_l(i == 3 ? 99 : 5, NULL);
		i++;
	}
	h_true(sock_alloc_l(6, pid_vivant) >= 0, "place recuperee");
	h_eq_u64("socket du mort recupere", g_net.socks[3].owner, 6);
	h_eq_u64("socket vivant conserve", g_net.socks[0].owner, 5);
	i = 0;
	while (i < NET_SOCKS)
		if (g_net.socks[i++].used)
			sock_free_l(&g_net.socks[i - 1]);
	h_eq_i64("rien ne fuit", fake_allocs(), 0);
}

static void	peer_dhcp(const uint8_t *f, size_t len)
{
	uint8_t	r[400];
	uint8_t	pl[300];
	size_t	n;
	size_t	k;

	if (len < 282 || rd16(f + 12) != ETH_P_IP || f[23] != IP_PROTO_UDP
		|| rd16(f + 36) != 67)
		return ;
	g_xid = rd32(f + 46);
	memset(pl, 0, sizeof(pl));
	pl[0] = 2;
	pl[1] = 1;
	pl[2] = 6;
	wr32(pl + 4, g_mode == 3 ? g_xid + 1 : g_xid);
	wr32(pl + 16, IP_LOCAL);
	memcpy(pl + 28, f + 70, 6);
	wr32(pl + 236, 0x63825363u);
	k = 240;
	pl[k++] = 53;
	pl[k++] = 1;
	pl[k++] = (f[284] == 1) ? 2 : (g_mode == 2 ? 6 : 5);
	pl[k++] = 1;
	pl[k++] = 4;
	wr32(pl + k, 0xffffff00u);
	k += 4;
	pl[k++] = 3;
	pl[k++] = 4;
	wr32(pl + k, IP_GW);
	k += 4;
	pl[k++] = 6;
	pl[k++] = 4;
	wr32(pl + k, IP_DNS);
	k += 4;
	pl[k++] = 51;
	pl[k++] = 4;
	wr32(pl + k, 7200);
	k += 4;
	pl[k++] = 54;
	pl[k++] = 4;
	wr32(pl + k, IP_GW);
	k += 4;
	pl[k++] = 255;
	n = fake_udp_frame(r, IP_GW, NET_IP_BROADCAST, 67, 68, pl, k);
	memset(r, 0xff, 6);
	fake_nic_inject(r, n);
}

static void	dhcp_complet(void)
{
	size_t		len;
	const uint8_t	*tx;

	fake_nic_attach();
	g_mode = 1;
	fake_nic_set_peer(peer_dhcp);
	h_eq_i64("bail obtenu", net_dhcp_run(), 0);
	h_eq_u64("adresse", g_net.ip, IP_LOCAL);
	h_eq_u64("masque", g_net.mask, 0xffffff00u);
	h_eq_u64("passerelle", g_net.gw, IP_GW);
	h_eq_u64("dns", g_net.dns, IP_DNS);
	h_eq_u64("duree", g_net.lease_s, 7200);
	h_eq_u64("etat pret", g_net.etat, NET_ETAT_PRET);
	h_eq_i64("DISCOVER puis REQUEST", fake_nic_tx_count(), 2);
	tx = fake_nic_tx(0, &len);
	h_true(memcmp(tx, "\xff\xff\xff\xff\xff\xff", 6) == 0, "diffusion");
	h_eq_u64("source 0.0.0.0", rd32(tx + 26), 0);
	h_eq_u64("destination 255.255.255.255", rd32(tx + 30), 0xffffffffu);
	h_eq_u64("port client", rd16(tx + 34), 68);
	h_eq_u64("socket DHCP libere", g_net.socks[0].used, 0);
	h_eq_i64("rien ne fuit", fake_allocs(), 0);
}

static void	dhcp_refus_et_ignore(void)
{
	fake_nic_attach();
	g_mode = 2;
	fake_nic_set_peer(peer_dhcp);
	h_eq_i64("NAK", net_dhcp_run(), E_PROTO);
	h_eq_u64("pas d'adresse", g_net.ip, 0);
	fake_nic_attach();
	g_mode = 3;
	fake_nic_set_peer(peer_dhcp);
	h_eq_i64("mauvais xid ignore", net_dhcp_run(), E_TIMEOUT);
	h_eq_u64("pas d'adresse", g_net.ip, 0);
	h_eq_i64("rien ne fuit", fake_allocs(), 0);
}

static void	peer_ping(const uint8_t *f, size_t len)
{
	uint8_t		r[200];
	t_icmpinfo	m;
	size_t		n;
	t_arpinfo	a;

	if (rd16(f + 12) == ETH_P_ARP && rd16(f + 20) == ARP_REQUEST
		&& rd32(f + 38) == IP_GW)
	{
		memset(r, 0, 60);
		a.op = ARP_REPLY;
		memcpy(a.sha, g_mac_peer, 6);
		a.spa = IP_GW;
		memcpy(a.tha, g_mac_local, 6);
		a.tpa = IP_LOCAL;
		n = eth_build(r, g_mac_local, g_mac_peer, ETH_P_ARP);
		arp_build(r + n, &a);
		fake_nic_inject(r, 60);
	}
	else if (g_mode == 1 && rd16(f + 12) == ETH_P_IP && f[23] == 1
		&& icmp_parse(f + 34, len - 34, &m) == 0)
	{
		n = fake_icmp_frame(r, IP_GW, IP_LOCAL, ICMP_ECHO_REPLY, m.seq,
				m.data, m.len);
		fake_nic_inject(r, n);
	}
}

static void	ping_passerelle(void)
{
	uint64_t	rtt;

	configure();
	g_mode = 1;
	fake_nic_set_peer(peer_ping);
	rtt = 0;
	h_eq_i64("ping repondu", net_ping(IP_GW, 1000000000ull, &rtt), 0);
	h_true(rtt > 0 && rtt < 1000000000ull, "temps mesure");
	h_eq_i64("ARP puis echo", fake_nic_tx_count(), 2);
	h_true(!g_net.ping_wait, "etat de ping libere");
}

static void	ping_sans_reponse(void)
{
	configure();
	g_mode = 0;
	fake_nic_set_peer(peer_ping);
	h_eq_i64("delai depasse", net_ping(IP_GW, 100000000ull, NULL), E_TIMEOUT);
	h_true(!g_net.ping_wait, "etat de ping libere");
	fake_nic_attach();
	h_eq_i64("sans adresse", net_ping(IP_GW, 100000000ull, NULL), E_NODEV);
}

static void	arp_sans_voisin(void)
{
	uint8_t	mac[6];

	configure();
	h_eq_i64("pas de voisin", arp_resolve_l(IP_GW, mac), E_TIMEOUT);
	h_eq_i64("trois requetes", fake_nic_tx_count(), NET_ARP_TRIES);
	g_net.gw = 0;
	h_eq_i64("pas de route", net_route_l(0x08080808u, mac), E_NOENT);
}

int	main(void)
{
	h_begin("e19_stack");
	h_run("arp_request_pour_nous", arp_request_pour_nous);
	h_run("arp_ignore_les_autres", arp_ignore_les_autres);
	h_run("echo_repond", echo_repond);
	h_run("echo_refuse", echo_refuse);
	h_run("udp_livre_au_socket", udp_livre_au_socket);
	h_run("udp_sans_socket_et_file_pleine", udp_sans_socket_et_file_pleine);
	h_run("sockets_regles", sockets_regles);
	h_run("echec_allocation", echec_allocation);
	h_run("reclamation_des_morts", reclamation_des_morts);
	h_run("dhcp_complet", dhcp_complet);
	h_run("dhcp_refus_et_ignore", dhcp_refus_et_ignore);
	h_run("ping_passerelle", ping_passerelle);
	h_run("ping_sans_reponse", ping_sans_reponse);
	h_run("arp_sans_voisin", arp_sans_voisin);
	return (h_end());
}
