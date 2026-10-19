use core::ffi::CStr;

use crate::abi::{DR_PU, DR_ST, t_dbuiltin, t_dnative, t_dnativefn, t_drsb};
use crate::acopy::dr_sys_arraycopy;
use crate::act::{dr_act_content, dr_act_finish, dr_act_title};
use crate::log::{dr_log_d, dr_log_e, dr_log_i, dr_log_w};
use crate::math::{dr_math_abs, dr_math_max, dr_math_min, dr_sys_millis};
use crate::object::{dr_nop, dr_obj_equals, dr_obj_hash, dr_obj_tostring};
use crate::sb::{
    dr_sb_append_c, dr_sb_append_i, dr_sb_append_j, dr_sb_append_obj, dr_sb_append_z, dr_sb_length,
    dr_sb_tostring,
};
use crate::string::{
    dr_int_parse, dr_int_tostring, dr_str_charat, dr_str_concat, dr_str_equals, dr_str_hash,
    dr_str_length, dr_str_self,
};
use crate::thr::{dr_thr_init_msg, dr_thr_message};
use crate::util::{
    ACT, AIOOBE, ASE, BTN, BUNDLE, CCE, CSQ, CTX, CTXSIG, ERR, EXC, IAE, INT, IOOBE, ISE, LL, LOG,
    LOGSIG, MATH, NFE, NPE, NSME, OBJ, OCL, OOME, RTE, SB, SIOOBE, STR, SYS, THR, TV, VG, VIEW,
};
use crate::view::{dr_view_init, dr_view_listen};
use crate::widget::{dr_ll_orient, dr_tv_gettext, dr_tv_settext, dr_vg_addview};

const fn sup(s: &'static CStr) -> *const core::ffi::c_char {
    s.as_ptr()
}

const fn none() -> *const core::ffi::c_char {
    core::ptr::null()
}

const fn builtin(
    desc: &'static CStr,
    parent: *const core::ffi::c_char,
    iface: *const core::ffi::c_char,
    access: u32,
    payload: u32,
) -> t_dbuiltin {
    t_dbuiltin {
        desc: desc.as_ptr(),
        super_desc: parent,
        iface_desc: iface,
        access,
        payload_bytes: payload,
    }
}

const fn native(
    cls: &'static CStr,
    name: &'static CStr,
    sig: &'static CStr,
    access: u32,
    func: t_dnativefn,
) -> t_dnative {
    t_dnative {
        cls: cls.as_ptr(),
        name: name.as_ptr(),
        sig: sig.as_ptr(),
        access,
        r#fn: Some(func),
    }
}

const SB_PAY: u32 = core::mem::size_of::<t_drsb>() as u32;

static G_CLASSES: [t_dbuiltin; 35] = [
    builtin(EXC, sup(THR), none(), 1, 0),
    builtin(RTE, sup(EXC), none(), 1, 0),
    builtin(c"Ljava/lang/ArithmeticException;", sup(RTE), none(), 1, 0),
    builtin(NPE, sup(RTE), none(), 1, 0),
    builtin(IOOBE, sup(RTE), none(), 1, 0),
    builtin(AIOOBE, sup(IOOBE), none(), 1, 0),
    builtin(SIOOBE, sup(IOOBE), none(), 1, 0),
    builtin(
        c"Ljava/lang/NegativeArraySizeException;",
        sup(RTE),
        none(),
        1,
        0,
    ),
    builtin(CCE, sup(RTE), none(), 1, 0),
    builtin(IAE, sup(RTE), none(), 1, 0),
    builtin(NFE, sup(IAE), none(), 1, 0),
    builtin(ISE, sup(RTE), none(), 1, 0),
    builtin(ASE, sup(RTE), none(), 1, 0),
    builtin(ERR, sup(THR), none(), 1, 0),
    builtin(c"Ljava/lang/StackOverflowError;", sup(ERR), none(), 1, 0),
    builtin(OOME, sup(ERR), none(), 1, 0),
    builtin(NSME, sup(ERR), none(), 1, 0),
    builtin(c"Ljava/lang/NoClassDefFoundError;", sup(ERR), none(), 1, 0),
    builtin(c"Ljava/lang/VerifyError;", sup(ERR), none(), 1, 0),
    builtin(
        c"Ljava/lang/IncompatibleClassChangeError;",
        sup(ERR),
        none(),
        1,
        0,
    ),
    builtin(c"Ljava/lang/AbstractMethodError;", sup(ERR), none(), 1, 0),
    builtin(SB, sup(OBJ), sup(CSQ), 0x11, SB_PAY),
    builtin(INT, sup(OBJ), none(), 0x11, 0),
    builtin(MATH, sup(OBJ), none(), 0x11, 0),
    builtin(SYS, sup(OBJ), none(), 0x11, 0),
    builtin(CTX, sup(OBJ), none(), 1, 0),
    builtin(BUNDLE, sup(OBJ), none(), 0x11, 0),
    builtin(ACT, sup(CTX), none(), 1, 0),
    builtin(LOG, sup(OBJ), none(), 0x11, 0),
    builtin(OCL, none(), none(), 0x601, 0),
    builtin(VIEW, sup(OBJ), none(), 1, 8),
    builtin(VG, sup(VIEW), none(), 1, 0),
    builtin(TV, sup(VIEW), none(), 1, 0),
    builtin(BTN, sup(TV), none(), 1, 0),
    builtin(LL, sup(VG), none(), 1, 0),
];

static G_LANG: [t_dnative; 31] = [
    native(OBJ, c"<init>", c"()V", DR_PU, dr_nop),
    native(
        OBJ,
        c"toString",
        c"()Ljava/lang/String;",
        DR_PU,
        dr_obj_tostring,
    ),
    native(
        OBJ,
        c"equals",
        c"(Ljava/lang/Object;)Z",
        DR_PU,
        dr_obj_equals,
    ),
    native(OBJ, c"hashCode", c"()I", DR_PU, dr_obj_hash),
    native(STR, c"length", c"()I", DR_PU, dr_str_length),
    native(STR, c"charAt", c"(I)C", DR_PU, dr_str_charat),
    native(
        STR,
        c"equals",
        c"(Ljava/lang/Object;)Z",
        DR_PU,
        dr_str_equals,
    ),
    native(STR, c"hashCode", c"()I", DR_PU, dr_str_hash),
    native(
        STR,
        c"toString",
        c"()Ljava/lang/String;",
        DR_PU,
        dr_str_self,
    ),
    native(
        STR,
        c"valueOf",
        c"(I)Ljava/lang/String;",
        DR_ST,
        dr_int_tostring,
    ),
    native(
        STR,
        c"concat",
        c"(Ljava/lang/String;)Ljava/lang/String;",
        DR_PU,
        dr_str_concat,
    ),
    native(
        INT,
        c"toString",
        c"(I)Ljava/lang/String;",
        DR_ST,
        dr_int_tostring,
    ),
    native(
        INT,
        c"parseInt",
        c"(Ljava/lang/String;)I",
        DR_ST,
        dr_int_parse,
    ),
    native(MATH, c"abs", c"(I)I", DR_ST, dr_math_abs),
    native(MATH, c"min", c"(II)I", DR_ST, dr_math_min),
    native(MATH, c"max", c"(II)I", DR_ST, dr_math_max),
    native(
        SYS,
        c"arraycopy",
        c"(Ljava/lang/Object;ILjava/lang/Object;II)V",
        DR_ST,
        dr_sys_arraycopy,
    ),
    native(SYS, c"currentTimeMillis", c"()J", DR_ST, dr_sys_millis),
    native(THR, c"<init>", c"()V", DR_PU, dr_nop),
    native(
        THR,
        c"<init>",
        c"(Ljava/lang/String;)V",
        DR_PU,
        dr_thr_init_msg,
    ),
    native(
        THR,
        c"getMessage",
        c"()Ljava/lang/String;",
        DR_PU,
        dr_thr_message,
    ),
    native(SB, c"<init>", c"()V", DR_PU, dr_nop),
    native(
        SB,
        c"append",
        c"(Ljava/lang/String;)Ljava/lang/StringBuilder;",
        DR_PU,
        dr_sb_append_obj,
    ),
    native(
        SB,
        c"append",
        c"(Ljava/lang/CharSequence;)Ljava/lang/StringBuilder;",
        DR_PU,
        dr_sb_append_obj,
    ),
    native(
        SB,
        c"append",
        c"(Ljava/lang/Object;)Ljava/lang/StringBuilder;",
        DR_PU,
        dr_sb_append_obj,
    ),
    native(
        SB,
        c"append",
        c"(I)Ljava/lang/StringBuilder;",
        DR_PU,
        dr_sb_append_i,
    ),
    native(
        SB,
        c"append",
        c"(J)Ljava/lang/StringBuilder;",
        DR_PU,
        dr_sb_append_j,
    ),
    native(
        SB,
        c"append",
        c"(C)Ljava/lang/StringBuilder;",
        DR_PU,
        dr_sb_append_c,
    ),
    native(
        SB,
        c"append",
        c"(Z)Ljava/lang/StringBuilder;",
        DR_PU,
        dr_sb_append_z,
    ),
    native(
        SB,
        c"toString",
        c"()Ljava/lang/String;",
        DR_PU,
        dr_sb_tostring,
    ),
    native(SB, c"length", c"()I", DR_PU, dr_sb_length),
];

static G_ANDROID: [t_dnative; 26] = [
    native(CTX, c"<init>", c"()V", DR_PU, dr_nop),
    native(BUNDLE, c"<init>", c"()V", DR_PU, dr_nop),
    native(ACT, c"<init>", c"()V", DR_PU, dr_nop),
    native(ACT, c"onCreate", c"(Landroid/os/Bundle;)V", DR_PU, dr_nop),
    native(ACT, c"onStart", c"()V", DR_PU, dr_nop),
    native(ACT, c"onResume", c"()V", DR_PU, dr_nop),
    native(ACT, c"onPause", c"()V", DR_PU, dr_nop),
    native(ACT, c"onStop", c"()V", DR_PU, dr_nop),
    native(ACT, c"onDestroy", c"()V", DR_PU, dr_nop),
    native(
        ACT,
        c"setContentView",
        c"(Landroid/view/View;)V",
        DR_PU,
        dr_act_content,
    ),
    native(
        ACT,
        c"setTitle",
        c"(Ljava/lang/CharSequence;)V",
        DR_PU,
        dr_act_title,
    ),
    native(ACT, c"finish", c"()V", DR_PU, dr_act_finish),
    native(LOG, c"d", LOGSIG, DR_ST, dr_log_d),
    native(LOG, c"i", LOGSIG, DR_ST, dr_log_i),
    native(LOG, c"w", LOGSIG, DR_ST, dr_log_w),
    native(LOG, c"e", LOGSIG, DR_ST, dr_log_e),
    native(VIEW, c"<init>", CTXSIG, DR_PU, dr_view_init),
    native(
        VIEW,
        c"setOnClickListener",
        c"(Landroid/view/View$OnClickListener;)V",
        DR_PU,
        dr_view_listen,
    ),
    native(VG, c"<init>", CTXSIG, DR_PU, dr_view_init),
    native(
        VG,
        c"addView",
        c"(Landroid/view/View;)V",
        DR_PU,
        dr_vg_addview,
    ),
    native(TV, c"<init>", CTXSIG, DR_PU, dr_view_init),
    native(
        TV,
        c"setText",
        c"(Ljava/lang/CharSequence;)V",
        DR_PU,
        dr_tv_settext,
    ),
    native(
        TV,
        c"getText",
        c"()Ljava/lang/CharSequence;",
        DR_PU,
        dr_tv_gettext,
    ),
    native(BTN, c"<init>", CTXSIG, DR_PU, dr_view_init),
    native(LL, c"<init>", CTXSIG, DR_PU, dr_view_init),
    native(LL, c"setOrientation", c"(I)V", DR_PU, dr_ll_orient),
];

/// # Safety
/// `tab` n'est pas nul et il est exclusif le temps de l'écriture.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_classes(tab: *mut *const t_dbuiltin) -> u32 {
    // SAFETY: tab n'est pas nul.
    unsafe { tab.write(G_CLASSES.as_ptr()) };
    G_CLASSES.len() as u32
}

/// # Safety
/// `tab` n'est pas nul et il est exclusif le temps de l'écriture.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_tab_lang(tab: *mut *const t_dnative) -> u32 {
    // SAFETY: tab n'est pas nul.
    unsafe { tab.write(G_LANG.as_ptr()) };
    G_LANG.len() as u32
}

/// # Safety
/// `tab` n'est pas nul et il est exclusif le temps de l'écriture.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn dr_tab_android(tab: *mut *const t_dnative) -> u32 {
    // SAFETY: tab n'est pas nul.
    unsafe { tab.write(G_ANDROID.as_ptr()) };
    G_ANDROID.len() as u32
}
