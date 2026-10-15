#include "velum/libk.h"
#include "logon.h"
#include "logon_text.h"

int	logon_add_dialog(t_logon *lg)
{
	t_ctlspec	s;

	s = (t_ctlspec){CT_LABEL, LOGON_ID_DLG_TEXT, CTL_ENABLED,
		lg->lay.dlg_text, LOGON_POWER_ASK};
	if (!logon_add_ctl(lg, s, FONT_INS_14))
		return (-1);
	s = (t_ctlspec){CT_BUTTON, CTL_ID_OK, CTL_ENABLED | CTL_FOCUSABLE
		| CTL_DEFAULT, lg->lay.dlg_ok, "Éteindre"};
	if (!logon_add_ctl(lg, s, FONT_INS_SEMI_13))
		return (-1);
	s = (t_ctlspec){CT_BUTTON, CTL_ID_CANCEL, CTL_ENABLED | CTL_FOCUSABLE,
		lg->lay.dlg_cancel, "Annuler"};
	if (!logon_add_ctl(lg, s, FONT_INS_SEMI_13))
		return (-1);
	return (0);
}
