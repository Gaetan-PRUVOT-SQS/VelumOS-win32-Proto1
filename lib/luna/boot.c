#include "luna_int.h"

static void	boot_stack(const t_surface *s, t_lboot *b, int32_t e)
{
	int32_t	mh;
	int32_t	bw;
	int32_t	bh;
	int32_t	y;

	mh = lp_max(e / 5, 8);
	bw = lp_clamp(s->w * 22 / 100, 96, 280);
	bh = lp_clamp(e / 14, 6, 8);
	y = (s->h - (e + e / 4 + mh + e / 3 + bh + 24)) / 2;
	b->emblem = lp_rect((s->w - e) / 2, y, e, e);
	y += e + e / 4;
	b->mark = lp_rect((s->w - lp_wordmark_w(mh)) / 2, y,
			lp_wordmark_w(mh), mh);
	y += mh + e / 3;
	b->bar = lp_rect((s->w - bw) / 2, y, bw, bh);
	b->text = lp_rect(0, y + bh + 10, s->w, 14);
}

static void	boot_layout(const t_surface *s, t_lboot *b)
{
	int32_t	e;
	int32_t	pad;
	int32_t	w;

	e = lp_clamp(lp_min(s->w, s->h) / 6, 32, 120);
	pad = e / 2;
	boot_stack(s, b, e);
	w = lp_max(b->bar.w, b->mark.w) + 2 * pad;
	b->card = lp_isect(lp_rect((s->w - w) / 2, b->emblem.y - pad, w,
				b->text.y + b->text.h - b->emblem.y + 2 * pad),
			lp_rect(0, 0, s->w, s->h));
}

static void	boot_text(t_surface *s, const t_lboot *b)
{
	t_ltext	t;

	t.font = FONT_UI;
	t.color = lp_pal()->muted;
	t.shadow = 0;
	t.text = "D\xc3\xa9marrage en cours";
	t.box = b->text;
	t.align = LA_CENTER;
	lp_text(s, &t);
}

void	luna_boot_draw(t_surface *s, uint32_t pct, uint32_t tick)
{
	t_lboot	b;

	if (!lp_ok(s))
		return ;
	luna_logon_bg(s, lp_rect(0, 0, s->w, s->h));
	boot_layout(s, &b);
	if (b.card.w > 0 && b.card.h > 0)
		luna_glass(s, b.card, LUNA_GLASS_CARD);
	lp_emblem(s, b.emblem);
	lp_wordmark(s, b.mark, lp_pal()->ink);
	lp_boot_bar(s, &b.bar, pct, tick);
	boot_text(s, &b);
}
