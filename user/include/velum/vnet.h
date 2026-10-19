#ifndef VNET_API_H
# define VNET_API_H

# include <stdint.h>
# include "velum/abi/abi_net.h"

int	v_net_info(t_netinfo *out);
int	v_net_socket(void);
int	v_net_bind(int sock, uint16_t port);
int	v_net_sendto(int sock, const void *buf, uint32_t len, const t_netaddr *to);
int	v_net_recvfrom(int sock, void *buf, uint32_t len, t_netaddr *from,
		uint64_t timeout_ns);
int	v_net_close(int sock);
int	v_net_ping(uint32_t ip, uint64_t timeout_ns);

#endif
