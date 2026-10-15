#ifndef VIRTIO_H
# define VIRTIO_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "velum/pci.h"

# define VIO_VENDOR 0x1af4
# define VIO_ST_ACK 0x01
# define VIO_ST_DRIVER 0x02
# define VIO_ST_DRIVER_OK 0x04
# define VIO_ST_FEATURES_OK 0x08
# define VIO_ST_NEEDS_RESET 0x40
# define VIO_ST_FAILED 0x80
# define VIO_F_VERSION_1 0x100000000ull
# define VIO_F_ACCESS_PLATFORM 0x200000000ull
# define VIO_CAP_VNDR 0x09
# define VIO_CAP_COMMON 1
# define VIO_CAP_NOTIFY 2
# define VIO_CAP_ISR 3
# define VIO_CAP_DEVICE 4
# define VIO_CAP_TYPES 5
# define VIO_CAP_MIN_LEN 16
# define VIO_NOTIFY_CAP_LEN 20
# define VIO_CAPS_MAX 48
# define VIO_COMMON_MIN 0x38
# define VIO_CC_DFSELECT 0x00
# define VIO_CC_DF 0x04
# define VIO_CC_GFSELECT 0x08
# define VIO_CC_GF 0x0c
# define VIO_CC_MSIX 0x10
# define VIO_CC_NUMQ 0x12
# define VIO_CC_STATUS 0x14
# define VIO_CC_GENERATION 0x15
# define VIO_CC_QSELECT 0x16
# define VIO_CC_QSIZE 0x18
# define VIO_CC_QMSIX 0x1a
# define VIO_CC_QENABLE 0x1c
# define VIO_CC_QNOTIFY_OFF 0x1e
# define VIO_CC_QDESC 0x20
# define VIO_CC_QDRIVER 0x28
# define VIO_CC_QDEVICE 0x30
# define VIO_NO_VECTOR 0xffff
# define VIO_RESET_TIMEOUT_NS 1000000000ull
# define VIO_GEN_TRIES 8
# define VQ_SIZE_MAX 128
# define VQ_DESC_F_NEXT 1
# define VQ_DESC_F_WRITE 2
# define VQ_AVAIL_F_NO_INTERRUPT 1

typedef struct s_vqdesc
{
	uint64_t	addr;
	uint32_t	len;
	uint16_t	flags;
	uint16_t	next;
}	t_vqdesc;

typedef struct s_vqavail
{
	uint16_t	flags;
	uint16_t	idx;
	uint16_t	ring[];
}	t_vqavail;

typedef struct s_vqelem
{
	uint32_t	id;
	uint32_t	len;
}	t_vqelem;

typedef struct s_vqused
{
	uint16_t	flags;
	uint16_t	idx;
	t_vqelem	ring[];
}	t_vqused;

typedef struct s_vqbuf
{
	uint64_t	phys;
	uint32_t	len;
	bool		write;
}	t_vqbuf;

typedef struct s_vq
{
	volatile t_vqdesc	*desc;
	volatile t_vqavail	*avail;
	volatile t_vqused	*used;
	volatile uint8_t	*notify;
	uint64_t			phys;
	size_t				pages;
	uint16_t			size;
	uint16_t			index;
	uint16_t			free_head;
	uint16_t			nfree;
	uint16_t			inflight;
	uint16_t			avail_idx;
	uint16_t			last_used;
	uint16_t			next[VQ_SIZE_MAX];
	uint16_t			chain[VQ_SIZE_MAX];
}	t_vq;

typedef struct s_viocap
{
	bool		found;
	uint8_t		bar;
	uint32_t	offset;
	uint32_t	length;
	uint32_t	mult;
}	t_viocap;

typedef struct s_vio
{
	const t_pcidev		*pci;
	volatile uint8_t	*common;
	volatile uint8_t	*notify;
	volatile uint8_t	*isr;
	volatile uint8_t	*devcfg;
	uint32_t			notify_len;
	uint32_t			notify_mult;
	uint32_t			devcfg_len;
	uint64_t			features;
	t_viocap			caps[VIO_CAP_TYPES];
}	t_vio;

uint8_t		vio_rd8(volatile uint8_t *base, uint32_t off);
uint16_t	vio_rd16(volatile uint8_t *base, uint32_t off);
uint32_t	vio_rd32(volatile uint8_t *base, uint32_t off);
void		vio_wr8(volatile uint8_t *base, uint32_t off, uint8_t v);
void		vio_wr16(volatile uint8_t *base, uint32_t off, uint16_t v);
void		vio_wr32(volatile uint8_t *base, uint32_t off, uint32_t v);
void		vio_wr64(volatile uint8_t *base, uint32_t off, uint64_t v);
void		vio_barrier(void);
int			vio_find_caps(const t_pcidev *d, t_vio *v);
int			vio_map(t_vio *v);
int			vio_reset(t_vio *v);
void		vio_status_add(t_vio *v, uint8_t bits);
int			vio_negotiate(t_vio *v, uint64_t wanted);
int			vio_start(t_vio *v);
int			vio_cfg_read(t_vio *v, uint32_t off, void *out, uint32_t len);
int			vio_queue_setup(t_vio *v, uint16_t index, t_vq *q);
void		vio_queue_release(t_vq *q);
void		vio_kick(t_vq *q);
size_t		vq_mem_size(uint16_t qsz);
int			vq_init(t_vq *q, void *mem, uint64_t phys, uint16_t qsz);
int			vq_add(t_vq *q, const t_vqbuf *b, uint16_t n);
bool		vq_used_pending(void *ctx);
int			vq_get(t_vq *q, uint32_t *len);

#endif
