#include "velum/libk.h"
#include "console.h"

static void	live_push(t_conlive *o, const char *text, uint8_t color,
				uint32_t n)
{
	uint32_t	i;

	i = 0;
	while (i < n && o->bytes + 1 < CON_LIVE_BYTES)
	{
		o->text[o->bytes] = text[i];
		o->color[o->bytes] = color;
		o->bytes++;
		i++;
	}
	o->text[o->bytes] = '\0';
}

static void	live_skip(t_conlive *o, uint32_t cells)
{
	uint32_t	i;

	i = 0;
	while (cells > 0 && i < o->bytes)
	{
		i++;
		while (i < o->bytes && ((uint8_t)o->text[i] & 0xc0) == 0x80)
			i++;
		cells--;
	}
	memmove(o->text, o->text + i, o->bytes - i + 1);
	memmove(o->color, o->color + i, o->bytes - i);
	o->bytes -= i;
}

void	con_live_row(const t_console *c, uint32_t cols, t_conlive *out)
{
	t_conseg	seg[CON_SEGS];
	int			i;
	uint32_t	prompt_cells;

	memset(out, 0, sizeof(*out));
	i = 0;
	con_prompt_segs(c, seg);
	while (i < CON_SEGS)
	{
		live_push(out, seg[i].text, seg[i].color,
			(uint32_t)strlen(seg[i].text));
		i++;
	}
	prompt_cells = line_cells(out->text, out->bytes);
	live_push(out, c->line.buf, CC_TEXT, c->line.len);
	out->cursor = prompt_cells + line_cells(c->line.buf, c->line.cur);
	if (cols > 0 && out->cursor >= cols)
	{
		live_skip(out, out->cursor - cols + 1);
		out->cursor = cols - 1;
	}
	out->cells = line_cells(out->text, out->bytes);
}
