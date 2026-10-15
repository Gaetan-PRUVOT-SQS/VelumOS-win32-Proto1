#ifndef TBUF_H
# define TBUF_H

# include <stddef.h>
# include <stdint.h>

# define TBUF_DIGITS_MAX 20

typedef struct s_tbuf
{
	char	*p;
	size_t	size;
	size_t	len;
	int		over;
}	t_tbuf;

void	tb_init(t_tbuf *b, char *p, size_t size);
void	tb_putc(t_tbuf *b, char c);
void	tb_str(t_tbuf *b, const char *s);
void	tb_num(t_tbuf *b, uint64_t v, int width);
int		tb_result(const t_tbuf *b);

#endif
