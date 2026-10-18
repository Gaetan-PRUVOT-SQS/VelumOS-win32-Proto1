#include "velum/libk.h"
#include "console.h"

static t_conrow	*row_at(t_conscreen *s, uint32_t i)
{
	return (&s->rows[(s->first + i) % CON_ROWS_MAX]);
}

void	scr_clear(t_conscreen *s)
{
	s->first = 0;
	s->count = 1;
	memset(&s->rows[0], 0, sizeof(s->rows[0]));
}

void	scr_init(t_conscreen *s, uint32_t cols)
{
	memset(s, 0, sizeof(*s));
	s->cols = cols;
	scr_clear(s);
	scr_set_cols(s, cols);
}

void	scr_set_cols(t_conscreen *s, uint32_t cols)
{
	if (cols < 8)
		cols = 8;
	if (cols > CON_COLS_MAX)
		cols = CON_COLS_MAX;
	s->cols = cols;
}

void	scr_newline(t_conscreen *s)
{
	t_conrow	*r;

	if (s->count == CON_ROWS_MAX)
	{
		s->first = (s->first + 1) % CON_ROWS_MAX;
		s->count--;
	}
	s->count++;
	r = row_at(s, s->count - 1);
	memset(r, 0, sizeof(*r));
}

static uint32_t	seq_len(uint8_t lead)
{
	if (lead >= 0xf0)
		return (4);
	if (lead >= 0xe0)
		return (3);
	if (lead >= 0xc0)
		return (2);
	return (1);
}

void	scr_putc(t_conscreen *s, char c, uint8_t color)
{
	t_conrow	*r;
	uint8_t		b;

	if (c == '\n')
	{
		scr_newline(s);
		return ;
	}
	b = (uint8_t)c;
	r = row_at(s, s->count - 1);
	if ((b & 0xc0) != 0x80
		&& (r->cells >= s->cols || r->bytes + seq_len(b) >= CON_ROW_BYTES))
	{
		scr_newline(s);
		r = row_at(s, s->count - 1);
	}
	if (r->bytes + 1 >= CON_ROW_BYTES)
		return ;
	if ((b & 0xc0) != 0x80)
		r->cells++;
	r->text[r->bytes] = c;
	r->color[r->bytes] = color;
	r->bytes++;
	r->text[r->bytes] = '\0';
}

void	scr_puts(t_conscreen *s, const char *text, uint8_t color)
{
	while (*text)
	{
		scr_putc(s, *text, color);
		text++;
	}
}

uint32_t	scr_count(const t_conscreen *s)
{
	return (s->count);
}

const t_conrow	*scr_row(const t_conscreen *s, uint32_t i)
{
	if (i >= s->count)
		return (NULL);
	return (&s->rows[(s->first + i) % CON_ROWS_MAX]);
}

void	scr_pad(t_conscreen *s, uint32_t from_cell, uint32_t to_cell)
{
	while (from_cell < to_cell)
	{
		scr_putc(s, ' ', CC_TEXT);
		from_cell++;
	}
}
