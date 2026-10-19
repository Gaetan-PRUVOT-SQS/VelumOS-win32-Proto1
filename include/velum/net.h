#ifndef NET_H
# define NET_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "abi/abi_net.h"

# define NET_FRAME_MAX 1514
# define NET_FRAME_MIN 60
# define NET_IP_BROADCAST 0xffffffffu
# define NET_ETAT_ABSENT 0
# define NET_ETAT_SANS_ADRESSE 1
# define NET_ETAT_PRET 2

typedef struct s_nicops
{
	int	(*send)(void *ctx, const uint8_t *frame, size_t len);
	int	(*recv)(void *ctx, uint8_t *frame, size_t cap);
}	t_nicops;

int		net_attach(const t_nicops *ops, void *ctx, const uint8_t mac[6]);
void	net_detach(void);
int		net_boot_init(void);
int		vnet_boot(void);
int		net_dhcp_run(void);
int		net_ping(uint32_t ip, uint64_t timeout_ns, uint64_t *rtt_ns);
void	net_info(t_netinfo *out);
void	net_poll(void);

#endif
