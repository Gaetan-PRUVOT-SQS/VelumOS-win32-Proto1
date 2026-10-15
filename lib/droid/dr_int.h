#ifndef DR_INT_H
# define DR_INT_H

# include <stdlib.h>
# include "velum/libk.h"
# include "velum/err.h"
# include "velum/apk/droid.h"

# define DR_SB_CAP 1024
# define DR_PU 1
# define DR_ST 9
# define DR_OBJ "Ljava/lang/Object;"
# define DR_STR "Ljava/lang/String;"
# define DR_CSQ "Ljava/lang/CharSequence;"
# define DR_SB "Ljava/lang/StringBuilder;"
# define DR_INT "Ljava/lang/Integer;"
# define DR_MATH "Ljava/lang/Math;"
# define DR_SYS "Ljava/lang/System;"
# define DR_THR "Ljava/lang/Throwable;"
# define DR_EXC "Ljava/lang/Exception;"
# define DR_RTE "Ljava/lang/RuntimeException;"
# define DR_ERR "Ljava/lang/Error;"
# define DR_NPE "Ljava/lang/NullPointerException;"
# define DR_CCE "Ljava/lang/ClassCastException;"
# define DR_IAE "Ljava/lang/IllegalArgumentException;"
# define DR_ISE "Ljava/lang/IllegalStateException;"
# define DR_IOOBE "Ljava/lang/IndexOutOfBoundsException;"
# define DR_AIOOBE "Ljava/lang/ArrayIndexOutOfBoundsException;"
# define DR_SIOOBE "Ljava/lang/StringIndexOutOfBoundsException;"
# define DR_ASE "Ljava/lang/ArrayStoreException;"
# define DR_NFE "Ljava/lang/NumberFormatException;"
# define DR_OOME "Ljava/lang/OutOfMemoryError;"
# define DR_NSME "Ljava/lang/NoSuchMethodError;"
# define DR_CTX "Landroid/content/Context;"
# define DR_BUNDLE "Landroid/os/Bundle;"
# define DR_ACT "Landroid/app/Activity;"
# define DR_LOG "Landroid/util/Log;"
# define DR_VIEW "Landroid/view/View;"
# define DR_OCL "Landroid/view/View$OnClickListener;"
# define DR_VG "Landroid/view/ViewGroup;"
# define DR_TV "Landroid/widget/TextView;"
# define DR_BTN "Landroid/widget/Button;"
# define DR_LL "Landroid/widget/LinearLayout;"
# define DR_LOGSIG "(Ljava/lang/String;Ljava/lang/String;)I"
# define DR_CTXSIG "(Landroid/content/Context;)V"

typedef struct s_drsb
{
	uint32_t	len;
	uint16_t	buf[DR_SB_CAP];
}	t_drsb;

uint32_t	dr_classes(const t_dbuiltin **tab);
uint32_t	dr_tab_lang(const t_dnative **tab);
uint32_t	dr_tab_android(const t_dnative **tab);
t_droid		*dr_host(t_dvm *vm);
int			dr_npe(t_dvm *vm);
t_dclass	*dr_class(t_dvm *vm, const char *desc);
int			dr_is(t_dvm *vm, t_dref ref, const char *desc);
size_t		dr_dotted(const char *desc, char *out, size_t cap);
size_t		dr_utf8(t_dstr16 s, char *out, size_t cap);
uint32_t	dr_dec16(int64_t v, uint16_t *out);
int			dr_new16(t_dvm *vm, t_dstr16 s, uint64_t *ret);
int			dr_new8(t_dvm *vm, const char *s, uint64_t *ret);
int			dr_str(t_dvm *vm, t_dref ref, t_dstr16 *out);
int			dr_text_of(t_dvm *vm, t_dref ref, t_dref *str);
int			dr_cstr(t_dvm *vm, t_dref ref, char *out);
int			dr_fail(t_droid *d, int rc);
int			dr_missing(t_droid *d, const t_dclass *c, const t_dname *nm);
void		dr_release(t_droid *d);
t_droidview	*dr_view_of(t_dvm *vm, t_dref ref);
int			dr_view_get(t_dvm *vm, t_dref ref, t_droidview **out);
int			dr_sb_put(t_dvm *vm, t_dref ref, t_dstr16 s);
int			dr_sb_ascii(t_dvm *vm, t_dref ref, const char *txt);
int			dr_nop(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_obj_tostring(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_obj_equals(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_obj_hash(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_str_length(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_str_charat(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_str_equals(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_str_hash(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_str_self(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_str_concat(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_int_tostring(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_int_parse(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_math_abs(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_math_min(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_math_max(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_sys_arraycopy(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_sys_millis(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_thr_init_msg(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_thr_message(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_sb_append_obj(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_sb_append_i(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_sb_append_j(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_sb_append_c(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_sb_append_z(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_sb_tostring(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_sb_length(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_act_content(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_act_title(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_act_finish(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_log_d(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_log_i(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_log_w(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_log_e(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_view_init(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_view_listen(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_ll_orient(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_tv_settext(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_tv_gettext(t_dvm *vm, const uint32_t *args, uint64_t *ret);
int			dr_vg_addview(t_dvm *vm, const uint32_t *args, uint64_t *ret);

#endif
