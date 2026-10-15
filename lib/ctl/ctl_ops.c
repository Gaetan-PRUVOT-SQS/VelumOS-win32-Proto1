#include "ctl_int.h"

const t_ctlops	g_ctl_ops[CT_TYPES] = {
{0, true, false, NULL, NULL, ctl_pnl_paint, NULL, NULL, NULL},
{0, true, false, NULL, NULL, ctl_lbl_paint, NULL, NULL, ctl_lbl_activate},
{0, true, true, NULL, NULL, ctl_btn_paint, ctl_press_mouse, ctl_btn_key,
	ctl_btn_activate},
{0, true, true, NULL, NULL, ctl_chk_paint, ctl_press_mouse, ctl_chk_key,
	ctl_chk_activate},
{0, true, true, NULL, NULL, ctl_chk_paint, ctl_press_mouse, ctl_rad_key,
	ctl_rad_activate},
{0, true, false, NULL, NULL, ctl_grp_paint, NULL, NULL, ctl_lbl_activate},
{sizeof(t_edit), true, false, ctl_edt_init, NULL, ctl_edt_paint,
	ctl_edt_mouse, ctl_edt_key, NULL},
{sizeof(t_list), true, false, ctl_lst_init, ctl_lst_destroy, ctl_lst_paint,
	ctl_lst_mouse, ctl_lst_key, NULL},
{sizeof(t_scrst), true, false, ctl_scr_init, NULL, ctl_scr_paint,
	ctl_scr_mouse, ctl_scr_key, NULL},
{sizeof(t_progress), true, false, NULL, NULL, ctl_prg_paint, NULL, NULL,
	NULL},
{sizeof(t_menu), true, false, NULL, NULL, ctl_mnu_paint, ctl_mnu_mouse,
	ctl_mnu_key, NULL},
{0, false, false, NULL, NULL, NULL, NULL, NULL, NULL},
};

const t_ctlops	*ctl_ops_get(uint32_t type)
{
	if (type >= CT_TYPES)
		return (NULL);
	return (&g_ctl_ops[type]);
}
