#include "fmt_int.h"
#include "velum/err.h"
#include "velum/libk.h"

static uint64_t	fetch_unsigned(const t_spec *sp, va_list *ap)
{
	if (sp->len >= 1)
		return (va_arg(*ap, uint64_t));
	if (sp->len == -1)
		return ((uint16_t)va_arg(*ap, unsigned int));
	if (sp->len == -2)
		return ((uint8_t)va_arg(*ap, unsigned int));
	return (va_arg(*ap, unsigned int));
}

static void	conv_signed(t_out *o, const t_spec *sp, va_list *ap)
{
	int64_t	v;

	if (sp->len >= 1)
		v = va_arg(*ap, int64_t);
	else if (sp->len == -1)
		v = (int16_t)va_arg(*ap, int);
	else if (sp->len == -2)
		v = (int8_t)va_arg(*ap, int);
	else
		v = va_arg(*ap, int);
	if (v < 0)
		fmt_number(o, sp, 0 - (uint64_t)v, 1);
	else
		fmt_number(o, sp, (uint64_t)v, 0);
}

static void	fmt_conv(t_out *o, t_spec *sp, va_list *ap)
{
	if (sp->conv == 'd' || sp->conv == 'i')
		conv_signed(o, sp, ap);
	else if (sp->conv == 'u' || sp->conv == 'x' || sp->conv == 'X'
		|| sp->conv == 'o')
		fmt_number(o, sp, fetch_unsigned(sp, ap), 0);
	else if (sp->conv == 'p')
	{
		sp->conv = 'x';
		sp->flags |= F_ALT;
		fmt_number(o, sp, (uint64_t)va_arg(*ap, void *), 0);
	}
	else if (sp->conv == 's')
		fmt_string(o, sp, va_arg(*ap, const char *));
	else if (sp->conv == 'c')
		fmt_char(o, sp, (char)va_arg(*ap, int));
	else
	{
		out_putc(o, '%');
		if (sp->conv && sp->conv != '%')
			out_putc(o, sp->conv);
	}
}

int	kvsnprintf(char *buf, size_t size, const char *fmt, va_list ap)
{
	t_out	o;
	t_spec	sp;
	va_list	cp;

	va_copy(cp, ap);
	out_init(&o, buf, size);
	while (*fmt)
	{
		if (*fmt != '%')
			out_putc(&o, *fmt++);
		else
		{
			fmt++;
			fmt_parse(&fmt, &cp, &sp);
			fmt_conv(&o, &sp, &cp);
		}
	}
	va_end(cp);
	out_term(&o);
	if (o.pos > INT32_MAX)
		return (E_OVERFLOW);
	return ((int)o.pos);
}

int	ksnprintf(char *buf, size_t size, const char *fmt, ...)
{
	va_list	ap;
	int		n;

	va_start(ap, fmt);
	n = kvsnprintf(buf, size, fmt, ap);
	va_end(ap);
	return (n);
}
