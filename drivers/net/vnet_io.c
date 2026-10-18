#include "velum/err.h"
#include "velum/klog.h"
#include "velum/libk.h"
#include "velum/timer.h"
#include "vnet.h"

int	vnet_post_rx(t_vnet *v, uint16_t slot)
{
	t_vqbuf	b;
	int		head;

	b.phys = v->dma_phys + (uint64_t)slot * VNET_SLOT;
	b.len = VNET_SLOT;
	b.write = true;
	head = vq_add(&v->rxq, &b, 1);
	if (head < 0)
		return (head);
	v->slot_of[head] = slot;
	return (0);
}

int	vnet_fill_rx(t_vnet *v)
{
	uint16_t	i;
	int			rc;

	i = 0;
	while (i < VNET_RX_SLOTS)
	{
		rc = vnet_post_rx(v, i);
		if (rc < 0)
			return (rc);
		i++;
	}
	vio_kick(&v->rxq);
	return (0);
}

int	vnet_send(void *ctx, const uint8_t *frame, size_t len)
{
	t_vnet	*v;
	t_vqbuf	b;
	uint8_t	*tx;
	int		head;

	v = ctx;
	if (v->dead || !frame || len < 14 || len > NET_FRAME_MAX)
		return (E_INVAL);
	tx = v->dma + VNET_TX_OFF;
	memset(tx, 0, VNET_HDR_LEN);
	memcpy(tx + VNET_HDR_LEN, frame, len);
	b.phys = v->dma_phys + VNET_TX_OFF;
	b.len = (uint32_t)(VNET_HDR_LEN + len);
	b.write = false;
	head = vq_add(&v->txq, &b, 1);
	if (head < 0)
		return (head);
	vio_kick(&v->txq);
	if (wait_until(vq_used_pending, &v->txq, VNET_TX_TIMEOUT_NS) < 0)
	{
		v->dead = true;
		klog_err("net: émission sans réponse de la carte, désactivée");
		return (E_IO);
	}
	return (vq_get(&v->txq, NULL) == head ? 0 : E_IO);
}

static int	rx_copy(t_vnet *v, int head, uint32_t got, uint8_t *frame,
		size_t cap)
{
	const uint8_t	*src;
	size_t			n;

	if (head < 0 || (uint32_t)head >= v->rxq.size)
		return (E_IO);
	if (got <= VNET_HDR_LEN || got > VNET_SLOT)
		return (0);
	n = got - VNET_HDR_LEN;
	if (n > cap)
		return (0);
	src = v->dma + (size_t)v->slot_of[head] * VNET_SLOT + VNET_HDR_LEN;
	memcpy(frame, src, n);
	return ((int)n);
}

int	vnet_recv(void *ctx, uint8_t *frame, size_t cap)
{
	t_vnet		*v;
	uint32_t	got;
	int			head;
	int			n;

	v = ctx;
	if (v->dead)
		return (0);
	got = 0;
	head = vq_get(&v->rxq, &got);
	if (head == E_AGAIN)
		return (0);
	n = rx_copy(v, head, got, frame, cap);
	if (head < 0)
	{
		v->dead = true;
		return (0);
	}
	vnet_post_rx(v, v->slot_of[head]);
	vio_kick(&v->rxq);
	return (n);
}
