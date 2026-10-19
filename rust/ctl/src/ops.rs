use crate::abi::{CT_TYPES, t_ctlops};
use crate::edit::{ctl_edt_init, ctl_edt_key, ctl_edt_mouse, ctl_edt_paint};
use crate::list::{ctl_lst_destroy, ctl_lst_init, ctl_lst_key, ctl_lst_mouse, ctl_lst_paint};
use crate::menu::{ctl_mnu_key, ctl_mnu_mouse, ctl_mnu_paint};
use crate::radio::{ctl_rad_activate, ctl_rad_key};
use crate::scroll::{ctl_scr_init, ctl_scr_key, ctl_scr_mouse, ctl_scr_paint};
use crate::widgets::{
    ctl_btn_activate, ctl_btn_key, ctl_btn_paint, ctl_chk_activate, ctl_chk_key, ctl_chk_paint,
    ctl_grp_paint, ctl_lbl_activate, ctl_lbl_paint, ctl_pnl_paint, ctl_press_mouse, ctl_prg_paint,
};

#[unsafe(no_mangle)]
pub static g_ctl_ops: [t_ctlops; CT_TYPES as usize] = [
    t_ctlops {
        priv_size: 0,
        supported: true,
        hot: false,
        init: None,
        destroy: None,
        paint: Some(ctl_pnl_paint),
        mouse: None,
        key: None,
        activate: None,
    },
    t_ctlops {
        priv_size: 0,
        supported: true,
        hot: false,
        init: None,
        destroy: None,
        paint: Some(ctl_lbl_paint),
        mouse: None,
        key: None,
        activate: Some(ctl_lbl_activate),
    },
    t_ctlops {
        priv_size: 0,
        supported: true,
        hot: true,
        init: None,
        destroy: None,
        paint: Some(ctl_btn_paint),
        mouse: Some(ctl_press_mouse),
        key: Some(ctl_btn_key),
        activate: Some(ctl_btn_activate),
    },
    t_ctlops {
        priv_size: 0,
        supported: true,
        hot: true,
        init: None,
        destroy: None,
        paint: Some(ctl_chk_paint),
        mouse: Some(ctl_press_mouse),
        key: Some(ctl_chk_key),
        activate: Some(ctl_chk_activate),
    },
    t_ctlops {
        priv_size: 0,
        supported: true,
        hot: true,
        init: None,
        destroy: None,
        paint: Some(ctl_chk_paint),
        mouse: Some(ctl_press_mouse),
        key: Some(ctl_rad_key),
        activate: Some(ctl_rad_activate),
    },
    t_ctlops {
        priv_size: 0,
        supported: true,
        hot: false,
        init: None,
        destroy: None,
        paint: Some(ctl_grp_paint),
        mouse: None,
        key: None,
        activate: Some(ctl_lbl_activate),
    },
    t_ctlops {
        priv_size: core::mem::size_of::<crate::abi::t_edit>(),
        supported: true,
        hot: false,
        init: Some(ctl_edt_init),
        destroy: None,
        paint: Some(ctl_edt_paint),
        mouse: Some(ctl_edt_mouse),
        key: Some(ctl_edt_key),
        activate: None,
    },
    t_ctlops {
        priv_size: core::mem::size_of::<crate::abi::t_list>(),
        supported: true,
        hot: false,
        init: Some(ctl_lst_init),
        destroy: Some(ctl_lst_destroy),
        paint: Some(ctl_lst_paint),
        mouse: Some(ctl_lst_mouse),
        key: Some(ctl_lst_key),
        activate: None,
    },
    t_ctlops {
        priv_size: core::mem::size_of::<crate::abi::t_scrst>(),
        supported: true,
        hot: false,
        init: Some(ctl_scr_init),
        destroy: None,
        paint: Some(ctl_scr_paint),
        mouse: Some(ctl_scr_mouse),
        key: Some(ctl_scr_key),
        activate: None,
    },
    t_ctlops {
        priv_size: core::mem::size_of::<crate::abi::t_progress>(),
        supported: true,
        hot: false,
        init: None,
        destroy: None,
        paint: Some(ctl_prg_paint),
        mouse: None,
        key: None,
        activate: None,
    },
    t_ctlops {
        priv_size: core::mem::size_of::<crate::abi::t_menu>(),
        supported: true,
        hot: false,
        init: None,
        destroy: None,
        paint: Some(ctl_mnu_paint),
        mouse: Some(ctl_mnu_mouse),
        key: Some(ctl_mnu_key),
        activate: None,
    },
    t_ctlops {
        priv_size: 0,
        supported: false,
        hot: false,
        init: None,
        destroy: None,
        paint: None,
        mouse: None,
        key: None,
        activate: None,
    },
];

/// # Safety
/// `ty` est un entier de type. Au-delà de `CT_TYPES`, le pointeur rendu est nul.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_ops_get(ty: u32) -> *const t_ctlops {
    if ty >= CT_TYPES {
        core::ptr::null()
    } else {
        &raw const g_ctl_ops[ty as usize]
    }
}
