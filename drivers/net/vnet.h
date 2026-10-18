#ifndef VNET_H
# define VNET_H

# include "velum/net.h"
# include "../virtio/virtio.h"

# define VNET_PCI_DEVICE 0x1041
# define VNET_F_MAC 0x20ull
# define VNET_WANTED 0x20ull
# define VNET_CFG_MAC 0
# define VNET_HDR_LEN 12
# define VNET_SLOT 2048
# define VNET_RX_SLOTS 16
# define VNET_DMA_PAGES 9
# define VNET_TX_OFF (VNET_RX_SLOTS * VNET_SLOT)
# define VNET_TX_TIMEOUT_NS 100000000ull
# define VNET_QUEUE_RX 0
# define VNET_QUEUE_TX 1

typedef struct s_vnet
{
	t_vio		vio;
	t_vq		rxq;
	t_vq		txq;
	uint64_t	dma_phys;
	uint8_t		*dma;
	uint8_t		mac[6];
	uint16_t	slot_of[VQ_SIZE_MAX];
	bool		dead;
}	t_vnet;

int	vnet_post_rx(t_vnet *v, uint16_t slot);
int	vnet_fill_rx(t_vnet *v);
int	vnet_send(void *ctx, const uint8_t *frame, size_t len);
int	vnet_recv(void *ctx, uint8_t *frame, size_t cap);

#endif
