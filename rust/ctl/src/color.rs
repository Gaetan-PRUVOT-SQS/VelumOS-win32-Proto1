use crate::abi::{LUNA_THEME_DARK, t_color};
use crate::ffi::{gfx_lerp, luna_color_panel, luna_color_text, luna_color_window, luna_theme_get};

/// # Safety
/// Aucun pointeur.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_color_text(enabled: bool) -> t_color {
    if enabled {
        return luna_color_text();
    }
    gfx_lerp(luna_color_window(), luna_color_text(), 120)
}

/// # Safety
/// Aucun pointeur.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_color_shadow() -> t_color {
    0xffff_ffff
}

/// # Safety
/// Aucun pointeur.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_color_selected_text() -> t_color {
    0xffff_ffff
}

/// # Safety
/// Aucun pointeur.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_color_back(enabled: bool) -> t_color {
    if !enabled {
        return luna_color_window();
    }
    if luna_theme_get() == LUNA_THEME_DARK {
        return luna_color_panel();
    }
    0xffff_ffff
}

/// # Safety
/// Aucun pointeur.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_color_track() -> t_color {
    gfx_lerp(luna_color_window(), 0xffff_ffff, 128)
}
