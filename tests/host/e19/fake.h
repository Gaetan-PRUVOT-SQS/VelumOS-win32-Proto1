#ifndef E19_FAKE_H
# define E19_FAKE_H

# include <stddef.h>
# include <stdint.h>
# include "velum/net.h"

# define FAKE_FRAMES 64

typedef void	(*t_peerfn)(const uint8_t *frame, size_t len);

extern const uint8_t	g_mac_local[6];
extern const uint8_t	g_mac_peer[6];

void		fake_reset(void);
void		fake_fail_alloc(int after);
int			fake_allocs(void);
uint64_t	fake_now(void);
void		fake_nic_set_peer(t_peerfn fn);
void		fake_nic_inject(const uint8_t *frame, size_t len);
int			fake_nic_tx_count(void);
const uint8_t	*fake_nic_tx(int i, size_t *len);
void		fake_nic_attach(void);
size_t		fake_udp_frame(uint8_t *buf, uint32_t src, uint32_t dst,
				uint16_t sport, uint16_t dport, const uint8_t *data, size_t n);
size_t		fake_icmp_frame(uint8_t *buf, uint32_t src, uint32_t dst,
				uint8_t type, uint16_t seq, const uint8_t *data, size_t n);

#endif
