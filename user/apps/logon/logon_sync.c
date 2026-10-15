#include "velum/libk.h"
#include "../common/platform.h"
#include "logon.h"
#include "logon_text.h"

static void	flag(t_logon *lg, uint32_t id, uint32_t bit, bool on)
{
	t_ctl	*c;

	c = ctl_find(&lg->ui.root, id);
	if (c)
		ctl_set_flag(&lg->ui.root, c, bit, on);
}

static void	sync_tiles(t_logon *lg)
{
	uint32_t	i;
	int			main_view;
	int			locked;

	main_view = lg->flow.mode != LM_SHUTDOWN;
	locked = lg->flow.mode == LM_PASSWORD;
	i = 0;
	while (i < lg->lay.count)
	{
		flag(lg, LOGON_ID_TILE + i, CTL_VISIBLE, main_view);
		flag(lg, LOGON_ID_TILE + i, CTL_ENABLED, !locked
			|| i == lg->flow.selected);
		i++;
	}
}

static void	sync_message(t_logon *lg)
{
	char	text[LOGON_MSG_MAX];
	t_ctl	*msg;

	logon_message(&lg->flow, logon_remaining(lg), text, sizeof(text));
	msg = ctl_find(&lg->ui.root, LOGON_ID_MSG);
	if (msg)
		ctl_set_text(&lg->ui.root, msg, text);
}

static uint32_t	focus_id(const t_logon *lg)
{
	if (lg->flow.mode == LM_SHUTDOWN)
		return (CTL_ID_CANCEL);
	if (lg->flow.mode == LM_PASSWORD)
		return (LOGON_ID_PW);
	if (lg->flow.selected < lg->lay.count)
		return (LOGON_ID_TILE + lg->flow.selected);
	return (LOGON_ID_TILE);
}

void	logon_sync(t_logon *lg)
{
	int	main_view;
	int	pw;

	main_view = lg->flow.mode != LM_SHUTDOWN;
	pw = lg->flow.mode == LM_PASSWORD;
	sync_tiles(lg);
	flag(lg, LOGON_ID_PW, CTL_VISIBLE, pw);
	flag(lg, LOGON_ID_GO, CTL_VISIBLE, pw);
	flag(lg, LOGON_ID_MSG, CTL_VISIBLE, main_view);
	flag(lg, LOGON_ID_SHUT, CTL_VISIBLE, main_view);
	flag(lg, LOGON_ID_DLG_TEXT, CTL_VISIBLE, !main_view);
	flag(lg, CTL_ID_OK, CTL_VISIBLE, !main_view);
	flag(lg, CTL_ID_CANCEL, CTL_VISIBLE, !main_view);
	sync_message(lg);
	ctl_focus(&lg->ui.root, ctl_find(&lg->ui.root, focus_id(lg)));
	uiwin_flush(&lg->ui);
}
