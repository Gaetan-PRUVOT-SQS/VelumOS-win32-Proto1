#include "velum/luna.h"
#include "velum/theme.h"
#include "../common/tbuf.h"
#include "../common/timefmt.h"
#include "../common/uitext.h"
#include "logon_paint.h"

static const char	*g_day[7] = {"dimanche", "lundi", "mardi", "mercredi",
	"jeudi", "vendredi", "samedi"};
static const char	*g_month[12] = {"janvier", "février", "mars", "avril",
	"mai", "juin", "juillet", "août", "septembre", "octobre", "novembre",
	"décembre"};

static void	civil_date(uint64_t days, uint32_t *day, uint32_t *month)
{
	uint64_t	doe;
	uint64_t	yoe;
	uint64_t	doy;
	uint64_t	mp;

	doe = (days + 719468) % 146097;
	yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
	doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
	mp = (5 * doy + 2) / 153;
	*day = (uint32_t)(doy - (153 * mp + 2) / 5 + 1);
	*month = (uint32_t)(mp + 3);
	if (mp >= 10)
		*month = (uint32_t)(mp - 9);
}

int	logon_clock_text(uint64_t wall_ns, char *clock, char *date, size_t size)
{
	t_tbuf		b;
	uint64_t	days;
	uint32_t	day;
	uint32_t	month;

	days = wall_ns / 86400000000000ull;
	fmt_clock(wall_ns, clock, size);
	civil_date(days, &day, &month);
	tb_init(&b, date, size);
	tb_str(&b, g_day[(days + 4) % 7]);
	tb_putc(&b, ' ');
	tb_num(&b, day, 1);
	tb_putc(&b, ' ');
	tb_str(&b, g_month[month - 1]);
	return (tb_result(&b));
}

static void	put_centered(t_surface *s, t_rect box, t_uitext *st,
		const char *text)
{
	st->at.x = box.x + (box.w - ui_text_width(st->font, text)) / 2;
	st->at.y = box.y + (box.h - ui_font_height(st->font)) / 2;
	ui_text(s, st, text);
}

void	logon_draw_head(t_logon *lg, uint64_t wall_ns)
{
	char		clock[LOGON_HEAD_TEXT];
	char		date[LOGON_HEAD_TEXT];
	t_uitext	st;
	int			dark;
	t_rect		box;

	if (lg->lay.clock.h <= 0)
		return ;
	logon_clock_text(wall_ns, clock, date, LOGON_HEAD_TEXT);
	dark = (luna_theme_get() == LUNA_THEME_DARK);
	st.max_w = lg->lay.clock.w;
	st.font = FONT_INS_SEMI_120;
	st.color = VT_INK;
	if (dark)
		st.color = VT_INK_D;
	box = lg->lay.clock;
	put_centered(&lg->ui.win.surface, box, &st, clock);
	st.font = FONT_INS_22;
	st.color = VT_INK_2;
	if (dark)
		st.color = VT_INK_2_D;
	put_centered(&lg->ui.win.surface, lg->lay.date, &st, date);
}
