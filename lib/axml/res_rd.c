#include "axml_int.h"

uint32_t	res_rd16(const uint8_t *p)
{
	return ((uint32_t)p[0] | ((uint32_t)p[1] << 8));
}

uint32_t	res_rd32(const uint8_t *p)
{
	return (res_rd16(p) | (res_rd16(p + 2) << 16));
}

int	res_chunk(t_span s, size_t off, t_reschunk *c)
{
	if (!s.p || off > s.len || s.len - off < 8 || off > RES_SIZE_MAX)
		return (E_INVAL);
	c->off = (uint32_t)off;
	c->type = res_rd16(s.p + off);
	c->hsize = res_rd16(s.p + off + 2);
	c->size = res_rd32(s.p + off + 4);
	if (c->hsize < 8 || c->hsize > c->size || c->size > s.len - off)
		return (E_INVAL);
	if (c->size > RES_SIZE_MAX)
		return (E_INVAL);
	return (E_OK);
}

int	res_next(t_span s, uint32_t *pos, t_reschunk *c)
{
	if (*pos >= s.len)
		return (0);
	if (res_chunk(s, *pos, c) < 0)
		return (E_INVAL);
	*pos += c->size;
	return (1);
}
