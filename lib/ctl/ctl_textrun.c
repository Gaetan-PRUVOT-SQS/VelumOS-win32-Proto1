#include "ctl_int.h"

bool	ctl_run_init(t_textrun *run, const t_ctltext *tx)
{
	run->font = tx->font;
	if (!run->font)
		run->font = ctl_font(tx->flags);
	if (!run->font || !tx->text)
		return (false);
	run->mn = -1;
	if (tx->mnemonic)
		run->len = ctl_text_strip(tx->text, run->buf, &run->mn);
	else
	{
		ctl_copy_text(run->buf, tx->text);
		run->len = (int32_t)strlen(run->buf);
	}
	run->cut = run->len;
	run->ellipsis = false;
	return (true);
}

void	ctl_run_fit(t_textrun *run, const t_ctltext *tx)
{
	int32_t	ell;
	int32_t	room;

	run->width = font_text_width(run->font, run->buf, run->len);
	if (run->width <= tx->box.w)
		return ;
	ell = 0;
	room = tx->box.w;
	run->ellipsis = (tx->flags & CTL_ELLIPSIS) != 0;
	if (run->ellipsis)
	{
		ell = font_text_width(run->font, "\xe2\x80\xa6", 3);
		if (ell > room)
		{
			run->ellipsis = false;
			ell = 0;
		}
	}
	room -= ell;
	if (room < 0)
		room = 0;
	run->cut = font_fit(run->font, run->buf, room);
	if (run->cut > run->len)
		run->cut = run->len;
	run->width = font_text_width(run->font, run->buf, run->cut) + ell;
}

void	ctl_run_place(t_textrun *run, const t_ctltext *tx)
{
	run->at.x = tx->box.x;
	if (tx->flags & CTL_ALIGN_CENTER)
		run->at.x += (tx->box.w - run->width) / 2;
	else if (tx->flags & CTL_ALIGN_RIGHT)
		run->at.x += tx->box.w - run->width;
	if (run->at.x < tx->box.x)
		run->at.x = tx->box.x;
	run->at.y = tx->box.y + (tx->box.h - run->font->height) / 2;
}

int32_t	ctl_text_width(const t_ctltext *tx)
{
	t_textrun	run;

	if (!ctl_run_init(&run, tx))
		return (0);
	return (font_text_width(run.font, run.buf, run.len));
}
