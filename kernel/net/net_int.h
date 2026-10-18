#ifndef NET_INT_H
# define NET_INT_H

# include "velum/net.h"
# include "velum/sync.h"
# include "net_wire.h"

# define NET_ARP_SLOTS 8
# define NET_ARP_TTL_NS 60000000000ull
# define NET_ARP_TRY_NS 200000000ull
# define NET_ARP_TRIES 3
# define NET_SOCKS 16
# define NET_SOCK_QUEUE 4
# define NET_RX_BURST 16
# define NET_PORT_EPHEMERAL_MIN 49152
# define NET_PORT_EPHEMERAL_SPAN 16384
# define NET_PING_DATA 32
# define NET_DHCP_STEP_NS 2000000000ull
# define NET_DHCP_TRIES 3

typedef struct s_arpent
{
	bool		used;
	uint32_t	ip;
	uint8_t		mac[6];
	uint64_t	expire_ns;
}	t_arpent;

typedef struct s_dgram
{
	uint32_t	ip;
	uint16_t	port;
	uint16_t	len;
	uint8_t		data[NET_UDP_MAX];
}	t_dgram;

typedef struct s_sock
{
	bool		used;
	bool		bound;
	uint32_t	gen;
	uint32_t	owner;
	uint16_t	port;
	uint8_t		head;
	uint8_t		count;
	t_dgram		*queue;
}	t_sock;

typedef struct s_net
{
	bool			ready;
	t_mutex			lock;
	const t_nicops	*nic;
	void			*nic_ctx;
	uint8_t			mac[6];
	uint32_t		etat;
	uint32_t		ip;
	uint32_t		mask;
	uint32_t		gw;
	uint32_t		dns;
	uint32_t		lease_s;
	uint16_t		ip_id;
	uint32_t		arp_wait_ip;
	bool			ping_wait;
	bool			ping_got;
	uint32_t		ping_ip;
	uint16_t		ping_seq;
	uint64_t		ping_t1;
	uint64_t		drops;
	t_arpent		arp[NET_ARP_SLOTS];
	t_sock			socks[NET_SOCKS];
	uint8_t			tx[NET_FRAME_MAX];
	uint8_t			l4[NET_FRAME_MAX];
	uint8_t			rx[NET_FRAME_MAX];
}	t_net;

typedef bool	(*t_nettest)(void *arg);
typedef bool	(*t_pidalive)(uint32_t pid);

extern t_net	g_net;

void	net_poll_l(void);
void	net_rx_l(const uint8_t *frame, size_t len);
int		net_wait(t_nettest test, void *arg, uint64_t timeout_ns);
int		net_tx_l(const uint8_t mac[6], uint32_t dst, uint8_t proto,
			const uint8_t *l4, size_t l4len);
int		net_route_l(uint32_t dst, uint8_t mac[6]);
int		net_udp_send_l(uint16_t sport, uint32_t dst, uint16_t dport,
			const uint8_t *data, size_t len);
void	arp_input_l(const t_ethinfo *e);
bool	arp_lookup_l(uint32_t ip, uint8_t mac[6]);
int		arp_resolve_l(uint32_t ip, uint8_t mac[6]);
void	icmp_input_l(const t_ethinfo *e, const t_ipinfo *ip);
int		sock_alloc_l(uint32_t owner, t_pidalive alive);
t_sock	*sock_get_l(uint32_t id, uint32_t owner);
int		sock_bind_l(t_sock *s, uint16_t port, bool privileged);
void	sock_free_l(t_sock *s);
bool	sock_deliver_l(uint16_t port, uint32_t ip, uint16_t from,
			const uint8_t *data, size_t len);
int		sock_take_l(t_sock *s, t_dgram *out);
uint32_t	sock_id(const t_sock *s, uint32_t index);

#endif
