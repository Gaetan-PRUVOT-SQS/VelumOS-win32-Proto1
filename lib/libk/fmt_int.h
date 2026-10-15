#ifndef FMT_INT_H
# define FMT_INT_H

# include <stdarg.h>
# include <stddef.h>
# include <stdint.h>

# define F_LEFT 0x01
# define F_ZERO 0x02
# define F_PLUS 0x04
# define F_SPACE 0x08
# define F_ALT 0x10
# define FMT_CAP 0x80000000LL

typedef struct s_out
{
	char	*buf;
	size_t	size;
	size_t	pos;
}	t_out;

typedef struct s_spec
{
	uint32_t	flags;
	int64_t		width;
	int64_t		prec;
	int32_t		len;
	char		conv;
}	t_spec;

typedef struct s_num
{
	char		tmp[24];
	char		pfx[4];
	int32_t		nd;
	int32_t		plen;
	int64_t		zeros;
	int64_t		pad;
}	t_num;

void	out_init(t_out *o, char *buf, size_t size);
void	out_putc(t_out *o, char c);
void	out_fill(t_out *o, char c, int64_t n);
void	out_write(t_out *o, const char *s, size_t n);
void	out_term(t_out *o);
void	fmt_parse(const char **p, va_list *ap, t_spec *sp);
void	fmt_number(t_out *o, const t_spec *sp, uint64_t v, int neg);
void	fmt_string(t_out *o, const t_spec *sp, const char *s);
void	fmt_char(t_out *o, const t_spec *sp, char c);

#endif
