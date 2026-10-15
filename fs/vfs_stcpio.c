#include "vfs_st.h"
#include "velum/err.h"
#include "velum/libk.h"

static void	put_hex8(uint8_t *p, uint32_t v)
{
	int	i;

	i = 7;
	while (i >= 0)
	{
		p[i] = (uint8_t)"0123456789abcdef"[v & 15];
		v >>= 4;
		i--;
	}
}

static void	put_fields(uint8_t *h, uint32_t mode, uint32_t dlen,
		uint32_t nlen)
{
	uint32_t	i;

	memcpy(h, "070701", 6);
	i = 0;
	while (i < 13)
	{
		put_hex8(h + 6 + i * 8, 0);
		i++;
	}
	put_hex8(h + 6 + 1 * 8, mode);
	put_hex8(h + 6 + 4 * 8, 1);
	put_hex8(h + 6 + 6 * 8, dlen);
	put_hex8(h + 6 + 11 * 8, nlen);
}

int	vst_cpio_add(t_vbuf *b, const char *name, uint32_t mode,
		const char *data)
{
	uint64_t	nlen;
	uint64_t	dlen;
	uint64_t	head;
	uint64_t	need;

	nlen = strlen(name) + 1;
	dlen = 0;
	if (data)
		dlen = strlen(data);
	head = (110 + nlen + 3) & ~3ull;
	need = head + ((dlen + 3) & ~3ull);
	if (b->len > b->cap || need > b->cap - b->len)
		return (E_NOSPC);
	memset(b->p + b->len, 0, need);
	put_fields(b->p + b->len, mode, (uint32_t)dlen, (uint32_t)nlen);
	memcpy(b->p + b->len + 110, name, nlen);
	if (dlen)
		memcpy(b->p + b->len + head, data, dlen);
	b->len += need;
	return (0);
}
