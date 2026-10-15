#include <stdlib.h>
#include <string.h>
#include "fake.h"
#include "virtio.h"

static int	fk_chain(t_fkdev *f, uint16_t head, uint16_t *idx)
{
	volatile t_vqdesc	*d;
	int					n;
	uint16_t			cur;

	d = (volatile t_vqdesc *)(uintptr_t)f->qaddr[0];
	n = 0;
	cur = head;
	while (n < 3 && cur < f->qsize)
	{
		idx[n++] = cur;
		if (!(d[cur].flags & VQ_DESC_F_NEXT))
			return (n);
		cur = d[cur].next;
	}
	return (-1);
}

static uint8_t	fk_blk(t_fkdev *f, uint32_t type, uint64_t sector,
	volatile t_vqdesc *data)
{
	uint64_t	off;
	void		*buf;

	f->requests++;
	f->last_type = type;
	f->last_sector = sector;
	f->flushes += (type == 4);
	if (type == 4)
		return (0);
	if (!data || sector > f->disk_bytes / 512)
		return (1);
	off = sector * 512;
	buf = (void *)(uintptr_t)data->addr;
	if (data->len > f->disk_bytes - off || (data->len % 512))
		return (1);
	if (type == 0 && (data->flags & VQ_DESC_F_WRITE))
		memcpy(buf, f->disk + off, data->len);
	else if (type == 1 && !(data->flags & VQ_DESC_F_WRITE)
		&& !(f->features & 0x20))
		memcpy(f->disk + off, buf, data->len);
	else
		return (1);
	return (0);
}

static uint32_t	fk_exec(t_fkdev *f, uint16_t head)
{
	volatile t_vqdesc	*d;
	uint16_t			idx[3];
	int					n;
	uint8_t				st;
	uint32_t			type;

	d = (volatile t_vqdesc *)(uintptr_t)f->qaddr[0];
	n = fk_chain(f, head, idx);
	if (n < 2 || (d[idx[0]].flags & VQ_DESC_F_WRITE) || d[idx[0]].len != 16
		|| !(d[idx[n - 1]].flags & VQ_DESC_F_WRITE) || d[idx[n - 1]].len < 1)
		abort();
	type = *(uint32_t *)(uintptr_t)d[idx[0]].addr;
	if (n == 3)
		st = fk_blk(f, type, *(uint64_t *)(uintptr_t)(d[idx[0]].addr + 8),
				&d[idx[1]]);
	else
		st = fk_blk(f, type, *(uint64_t *)(uintptr_t)(d[idx[0]].addr + 8),
				NULL);
	if (f->modes & FK_IOERR)
		st = 1;
	if (!(f->modes & FK_NO_STATUS))
		*(uint8_t *)(uintptr_t)d[idx[n - 1]].addr = st;
	if (n == 3 && type == 0)
		return (d[idx[1]].len + 1);
	return (1);
}

static void	fk_used_push(t_fkdev *f, uint16_t head, uint32_t len)
{
	volatile t_vqused	*u;
	uint16_t			slot;

	u = (volatile t_vqused *)(uintptr_t)f->qaddr[2];
	slot = u->idx & (f->qsize - 1);
	u->ring[slot].id = head;
	if (f->modes & FK_BAD_ID)
		u->ring[slot].id = 9999;
	u->ring[slot].len = len;
	if (f->modes & FK_SHORT_LEN)
		u->ring[slot].len = 0;
	__atomic_thread_fence(__ATOMIC_SEQ_CST);
	if (f->modes & FK_CRAZY_IDX)
		u->idx += 1000;
	else
		u->idx++;
}

void	fk_process(t_fkdev *f)
{
	volatile t_vqavail	*av;
	uint16_t			head;

	if ((f->modes & FK_HANG) || !f->qenable
		|| !(f->status & VIO_ST_DRIVER_OK))
		return ;
	if (f->qsize == 0 || f->qsize > f->qmax || (f->qsize & (f->qsize - 1)))
		abort();
	av = (volatile t_vqavail *)(uintptr_t)f->qaddr[1];
	while (f->last_avail != av->idx)
	{
		head = av->ring[f->last_avail & (f->qsize - 1)];
		fk_used_push(f, head, fk_exec(f, head));
		f->last_avail++;
	}
}
