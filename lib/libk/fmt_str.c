#include "fmt_int.h"
#include "velum/libk.h"

void	fmt_string(t_out *o, const t_spec *sp, const char *s)
{
	size_t	len;
	int64_t	pad;

	if (!s)
		s = "(null)";
	if (sp->prec >= 0)
		len = strnlen(s, (size_t)sp->prec);
	else
		len = strlen(s);
	pad = 0;
	if (sp->width > (int64_t)len)
		pad = sp->width - (int64_t)len;
	if (!(sp->flags & F_LEFT))
		out_fill(o, ' ', pad);
	out_write(o, s, len);
	if (sp->flags & F_LEFT)
		out_fill(o, ' ', pad);
}

void	fmt_char(t_out *o, const t_spec *sp, char c)
{
	int64_t	pad;

	pad = 0;
	if (sp->width > 1)
		pad = sp->width - 1;
	if (!(sp->flags & F_LEFT))
		out_fill(o, ' ', pad);
	out_putc(o, c);
	if (sp->flags & F_LEFT)
		out_fill(o, ' ', pad);
}
