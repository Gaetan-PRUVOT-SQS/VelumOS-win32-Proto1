#include "dr_int.h"

static const t_dnative	g_dr_android[] = {
{DR_CTX, "<init>", "()V", DR_PU, dr_nop},
{DR_BUNDLE, "<init>", "()V", DR_PU, dr_nop},
{DR_ACT, "<init>", "()V", DR_PU, dr_nop},
{DR_ACT, "onCreate", "(Landroid/os/Bundle;)V", DR_PU, dr_nop},
{DR_ACT, "onStart", "()V", DR_PU, dr_nop},
{DR_ACT, "onResume", "()V", DR_PU, dr_nop},
{DR_ACT, "onPause", "()V", DR_PU, dr_nop},
{DR_ACT, "onStop", "()V", DR_PU, dr_nop},
{DR_ACT, "onDestroy", "()V", DR_PU, dr_nop},
{DR_ACT, "setContentView", "(Landroid/view/View;)V", DR_PU, dr_act_content},
{DR_ACT, "setTitle", "(Ljava/lang/CharSequence;)V", DR_PU, dr_act_title},
{DR_ACT, "finish", "()V", DR_PU, dr_act_finish},
{DR_LOG, "d", DR_LOGSIG, DR_ST, dr_log_d},
{DR_LOG, "i", DR_LOGSIG, DR_ST, dr_log_i},
{DR_LOG, "w", DR_LOGSIG, DR_ST, dr_log_w},
{DR_LOG, "e", DR_LOGSIG, DR_ST, dr_log_e},
{DR_VIEW, "<init>", DR_CTXSIG, DR_PU, dr_view_init},
{DR_VIEW, "setOnClickListener", "(Landroid/view/View$OnClickListener;)V",
	DR_PU, dr_view_listen},
{DR_VG, "<init>", DR_CTXSIG, DR_PU, dr_view_init},
{DR_VG, "addView", "(Landroid/view/View;)V", DR_PU, dr_vg_addview},
{DR_TV, "<init>", DR_CTXSIG, DR_PU, dr_view_init},
{DR_TV, "setText", "(Ljava/lang/CharSequence;)V", DR_PU, dr_tv_settext},
{DR_TV, "getText", "()Ljava/lang/CharSequence;", DR_PU, dr_tv_gettext},
{DR_BTN, "<init>", DR_CTXSIG, DR_PU, dr_view_init},
{DR_LL, "<init>", DR_CTXSIG, DR_PU, dr_view_init},
{DR_LL, "setOrientation", "(I)V", DR_PU, dr_ll_orient}
};

uint32_t	dr_tab_android(const t_dnative **tab)
{
	*tab = g_dr_android;
	return (sizeof(g_dr_android) / sizeof(g_dr_android[0]));
}
