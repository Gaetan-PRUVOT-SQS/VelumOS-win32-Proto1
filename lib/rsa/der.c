#include "rsa_int.h"

static int	der_len(t_span in, size_t *hdr, size_t *len)
{
	size_t	n;
	size_t	i;

	*hdr = 2;
	*len = in.p[1];
	if (in.p[1] < 0x80)
		return (0);
	n = in.p[1] & 0x7f;
	if (n == 0 || n > 4 || in.len < 2 + n || in.p[2] == 0)
		return (E_INVAL);
	*len = 0;
	i = 0;
	while (i < n)
	{
		*len = (*len << 8) | in.p[2 + i];
		i++;
	}
	if (*len < 0x80)
		return (E_INVAL);
	*hdr = 2 + n;
	return (0);
}

int	der_read(t_span in, t_der *out)
{
	size_t	hdr;
	size_t	len;

	if (!in.p || !out || in.len < 2 || (in.p[0] & 0x1f) == 0x1f)
		return (E_INVAL);
	if (der_len(in, &hdr, &len) < 0 || len > in.len - hdr)
		return (E_INVAL);
	out->tag = in.p[0];
	out->all.p = in.p;
	out->all.len = hdr + len;
	out->val.p = in.p + hdr;
	out->val.len = len;
	out->rest.p = in.p + hdr + len;
	out->rest.len = in.len - hdr - len;
	return (0);
}

int	der_take(t_span in, uint8_t tag, t_der *out)
{
	if (der_read(in, out) < 0 || out->tag != tag)
		return (E_INVAL);
	return (0);
}

int	der_only(t_span in, uint8_t tag, t_span *val)
{
	t_der	d;

	if (der_take(in, tag, &d) < 0 || d.rest.len != 0)
		return (E_INVAL);
	*val = d.val;
	return (0);
}

int	der_uint(t_span in, t_span *mag, t_span *rest)
{
	t_der	d;

	if (der_take(in, DER_INT, &d) < 0 || d.val.len == 0)
		return (E_INVAL);
	if (d.val.p[0] & 0x80)
		return (E_INVAL);
	if (d.val.len > 1 && d.val.p[0] == 0 && !(d.val.p[1] & 0x80))
		return (E_INVAL);
	*mag = d.val;
	if (d.val.len > 1 && d.val.p[0] == 0)
	{
		mag->p++;
		mag->len--;
	}
	*rest = d.rest;
	return (0);
}
