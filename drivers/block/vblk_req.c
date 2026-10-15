#include "velum/timer.h"
#include "vblk.h"

int	vblk_kill(t_vblk *v, const char *why)
{
	klog_err("block: %s: %s, appareil désactivé", v->dev.name, why);
	v->dead = true;
	if (vio_reset(&v->vio) < 0)
		klog_err("block: %s: mémoire DMA conservée", v->dev.name);
	return (E_IO);
}

static void	vblk_header(t_vblk *v, const t_vblkreq *r)
{
	uint32_t	zero;

	zero = 0;
	memcpy(v->dma, &r->type, 4);
	memcpy(v->dma + 4, &zero, 4);
	memcpy(v->dma + 8, &r->sector, 8);
	*(volatile uint8_t *)(v->dma + VBLK_STATUS_OFF) = VBLK_STATUS_UNSET;
}

static uint16_t	vblk_bufs(const t_vblk *v, const t_vblkreq *r, t_vqbuf *b)
{
	uint16_t	n;

	b[0].phys = v->dma_phys;
	b[0].len = VBLK_HDR_SIZE;
	b[0].write = false;
	n = 1;
	if (r->bytes)
	{
		b[1].phys = v->dma_phys + VBLK_BOUNCE_OFF;
		b[1].len = r->bytes;
		b[1].write = (r->type == VBLK_T_IN);
		n = 2;
	}
	b[n].phys = v->dma_phys + VBLK_STATUS_OFF;
	b[n].len = 1;
	b[n].write = true;
	return (n + 1);
}

static int	vblk_complete(t_vblk *v, const t_vblkreq *r, int head)
{
	uint32_t	len;
	uint8_t		status;

	if (wait_until(vq_used_pending, &v->q, VBLK_TIMEOUT_NS) < 0)
		return (vblk_kill(v, "délai dépassé"));
	len = 0;
	if (vq_get(&v->q, &len) != head)
		return (vblk_kill(v, "réponse incohérente"));
	if (len < 1 || (r->type == VBLK_T_IN && len < r->bytes + 1))
		return (vblk_kill(v, "longueur rendue trop courte"));
	status = *(volatile uint8_t *)(v->dma + VBLK_STATUS_OFF);
	if (status != VBLK_S_OK)
	{
		klog_warn("block: %s: requête %u secteur %llu en échec (état %u)",
			v->dev.name, r->type, (unsigned long long)r->sector, status);
		return (E_IO);
	}
	return (0);
}

int	vblk_submit(t_vblk *v, const t_vblkreq *r)
{
	t_vqbuf	b[3];
	int		head;

	if (v->dead)
		return (E_IO);
	vblk_header(v, r);
	head = vq_add(&v->q, b, vblk_bufs(v, r, b));
	if (head < 0)
		return (vblk_kill(v, "file pleine"));
	vio_kick(&v->q);
	return (vblk_complete(v, r, head));
}
