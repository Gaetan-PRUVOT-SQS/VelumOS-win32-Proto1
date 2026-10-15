#include "fmt_int.h"

static int32_t	num_digits(uint64_t v, uint32_t base, int upper, char *tmp)
{
	const char	*dig;
	int32_t		n;

	dig = "0123456789abcdef";
	if (upper)
		dig = "0123456789ABCDEF";
	n = 0;
	while (v)
	{
		tmp[n++] = dig[v % base];
		v /= base;
	}
	return (n);
}

static void	num_prefix(t_num *n, const t_spec *sp, uint64_t v, int neg)
{
	n->plen = 0;
	if (neg)
		n->pfx[n->plen++] = '-';
	else if (sp->flags & F_PLUS && (sp->conv == 'd' || sp->conv == 'i'))
		n->pfx[n->plen++] = '+';
	else if (sp->flags & F_SPACE && (sp->conv == 'd' || sp->conv == 'i'))
		n->pfx[n->plen++] = ' ';
	if (sp->flags & F_ALT && v && (sp->conv == 'x' || sp->conv == 'X'))
	{
		n->pfx[n->plen++] = '0';
		n->pfx[n->plen++] = sp->conv;
	}
}

static void	num_layout(t_num *n, const t_spec *sp)
{
	int64_t	total;

	n->zeros = 0;
	if (sp->prec > n->nd)
		n->zeros = sp->prec - n->nd;
	if (sp->conv == 'o' && sp->flags & F_ALT && !n->zeros
		&& (!n->nd || n->tmp[n->nd - 1] != '0'))
		n->zeros = 1;
	total = n->plen + n->zeros + n->nd;
	if (sp->flags & F_ZERO && !(sp->flags & F_LEFT) && sp->prec < 0
		&& sp->width > total)
		n->zeros += sp->width - total;
	total = n->plen + n->zeros + n->nd;
	n->pad = 0;
	if (sp->width > total)
		n->pad = sp->width - total;
}

static uint32_t	num_base(char conv)
{
	if (conv == 'x' || conv == 'X')
		return (16);
	if (conv == 'o')
		return (8);
	return (10);
}

void	fmt_number(t_out *o, const t_spec *sp, uint64_t v, int neg)
{
	t_num	n;

	n.nd = num_digits(v, num_base(sp->conv), sp->conv == 'X', n.tmp);
	if (!v && sp->prec != 0)
	{
		n.tmp[0] = '0';
		n.nd = 1;
	}
	num_prefix(&n, sp, v, neg);
	num_layout(&n, sp);
	if (!(sp->flags & F_LEFT))
		out_fill(o, ' ', n.pad);
	out_write(o, n.pfx, n.plen);
	out_fill(o, '0', n.zeros);
	while (n.nd > 0)
		out_putc(o, n.tmp[--n.nd]);
	if (sp->flags & F_LEFT)
		out_fill(o, ' ', n.pad);
}
