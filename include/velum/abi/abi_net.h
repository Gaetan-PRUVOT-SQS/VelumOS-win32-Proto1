#ifndef ABI_NET_H
# define ABI_NET_H

# include <stdint.h>

# define SYS_NET_INFO 0x90
# define SYS_NET_SOCKET 0x91
# define SYS_NET_BIND 0x92
# define SYS_NET_SENDTO 0x93
# define SYS_NET_RECVFROM 0x94
# define SYS_NET_CLOSE 0x95
# define SYS_NET_PING 0x96

# define PF_NET 0x80
# define NET_UDP_MAX 1472
# define NET_PORT_PRIVILEGED 1024

typedef struct s_netaddr
{
	uint32_t	ip;
	uint16_t	port;
	uint16_t	reserve;
}	t_netaddr;

typedef struct s_netinfo
{
	uint32_t	etat;
	uint32_t	ip;
	uint32_t	masque;
	uint32_t	passerelle;
	uint32_t	dns;
	uint32_t	bail_s;
	uint8_t		mac[6];
	uint8_t		reserve[2];
}	t_netinfo;

#endif
