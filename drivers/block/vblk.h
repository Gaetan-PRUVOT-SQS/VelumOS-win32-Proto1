#ifndef VBLK_H
# define VBLK_H

# include "blk_int.h"
# include "../virtio/virtio.h"

# define VBLK_PCI_DEVICE 0x1042
# define VBLK_PCI_LEGACY 0x1001
# define VBLK_F_SIZE_MAX 0x2ull
# define VBLK_F_RO 0x20ull
# define VBLK_F_BLK_SIZE 0x40ull
# define VBLK_F_FLUSH 0x200ull
# define VBLK_CFG_CAPACITY 0
# define VBLK_CFG_SIZE_MAX 8
# define VBLK_CFG_BLK_SIZE 20
# define VBLK_T_IN 0
# define VBLK_T_OUT 1
# define VBLK_T_FLUSH 4
# define VBLK_S_OK 0
# define VBLK_STATUS_UNSET 0xff
# define VBLK_HDR_SIZE 16
# define VBLK_STATUS_OFF 16
# define VBLK_BOUNCE_OFF 4096
# define VBLK_BOUNCE_PAGES 16
# define VBLK_DMA_PAGES 17
# define VBLK_TIMEOUT_NS 5000000000ull
# define VBLK_MAX_DISKS 26
# define VBLK_QUEUE_MIN 3

typedef struct s_vblk
{
	t_blkdev	dev;
	t_vio		vio;
	t_vq		q;
	t_mutex		lock;
	uint64_t	dma_phys;
	uint8_t		*dma;
	uint32_t	chunk;
	uint32_t	ss_factor;
	bool		dead;
}	t_vblk;

typedef struct s_vblkreq
{
	uint32_t	type;
	uint32_t	bytes;
	uint64_t	sector;
}	t_vblkreq;

int				vblk_probe(const t_pcidev *d, uint32_t idx);
int				vblk_geometry(t_vblk *v);
int				vblk_submit(t_vblk *v, const t_vblkreq *r);
int				vblk_kill(t_vblk *v, const char *why);
const t_blkops	*vblk_ops(void);
void			vblk_intx_off(const t_pcidev *d);

#endif
