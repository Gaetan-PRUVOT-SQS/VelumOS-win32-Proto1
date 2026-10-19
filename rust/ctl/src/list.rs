use core::ffi::{c_char, c_int};

use crate::abi::{
    CN_DBLCLK, CN_ENTER, CN_SCROLL, CN_SELECT, CT_LIST, CTL_ELLIPSIS, CTL_FRAME,
    CTL_LIST_ITEMS_MAX, CTL_ROW_PAD, CTL_TEXT_MAX, E_INVAL, E_NOMEM, E_NOSPC, E_RANGE,
    INP_KEY_DOWN, INPM_ALT, INPM_CTRL, VK_DOWN, VK_END, VK_HOME, VK_NEXT, VK_PRIOR, VK_RETURN,
    VK_UP, t_ctl, t_ctlroot, t_ctltext, t_inpevent, t_list, t_lstgeom, t_lunametrics, t_mouseev,
    t_point, t_rect, t_sbctx,
};
use crate::color::{ctl_color_back, ctl_color_selected_text, ctl_color_text};
use crate::ffi::{
    gfx_fill, gfx_set_clip, luna_edit_frame, luna_metrics, rect_contains, rect_intersect, rect_make,
};
use crate::mem::{ctl_alloc, ctl_free};
use crate::mouse::ctl_wheel_delta;
use crate::notify::ctl_notify;
use crate::raw::{scopy, scopy_distinct, slen};
use crate::scroll::{ctl_sb_mouse, ctl_sb_paint, ctl_sb_set};
use crate::text::{ctl_copy_text, ctl_font_of, ctl_text_draw};
use crate::tree::{ctl_dirty_add, ctl_enabled, ctl_in_tree};
use crate::utf8::ctl_u8_next;
use crate::widgets::ctl_focus_ring_row;

const SBR_MOVED: c_int = crate::abi::CTL_SBR_MOVED;
const SBR_NONE: c_int = crate::abi::CTL_SBR_NONE;
const MEV_DOWN: u32 = crate::abi::CTL_MEV_DOWN;
const MEV_WHEEL: u32 = crate::abi::CTL_MEV_WHEEL;

fn list_ptr(c: *mut t_ctl) -> *mut t_list {
    // SAFETY: l'appelant garantit un contrôle liste.
    unsafe { (*c).r#priv.cast() }
}

/// # Safety
/// `c` est non nul. `c.priv` est un `t_list` mis à zéro.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_lst_init(c: *mut t_ctl) {
    // SAFETY: le privé d'une liste est un t_list.
    let l = unsafe { &mut *list_ptr(c) };
    l.sel = -1;
    l.sb.page = 1;
}

/// # Safety
/// `r` et `list` sont nuls ou vivants. `list` est une liste dans l'arbre.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_lst_of(r: *const t_ctlroot, list: *const t_ctl) -> *mut t_list {
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if r.is_null() || list.is_null() || unsafe { (*r).root.is_null() } {
        return core::ptr::null_mut();
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if unsafe { (*list).r#type } != CT_LIST || unsafe { !ctl_in_tree(r, list) } {
        return core::ptr::null_mut();
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*list).r#priv.cast() }
}

/// # Safety
/// `l` est non nul. Ses chaînes viennent de `ctl_alloc`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_lst_drop(l: *mut t_list) {
    // SAFETY: l est non nul.
    let n = unsafe { (*l).count };
    let mut i = 0u32;
    while i < n {
        // SAFETY: i < count, items[i] vient de ctl_alloc.
        unsafe { ctl_free((*(*l).items.add(i as usize)).cast()) };
        i += 1;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_free((*l).items.cast()) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*l).items = core::ptr::null_mut() };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*l).count = 0 };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*l).cap = 0 };
}

/// # Safety
/// `c` est non nul. `c.priv` est un `t_list`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_lst_destroy(c: *mut t_ctl) {
    // SAFETY: le privé est un t_list alloué avec le contrôle.
    unsafe { ctl_lst_drop(list_ptr(c)) };
}

fn grow(l: *mut t_list) -> bool {
    // SAFETY: l est non nul.
    let count = unsafe { (*l).count };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let cap0 = unsafe { (*l).cap };
    if count < cap0 {
        return true;
    }
    let mut cap = cap0.wrapping_mul(2);
    if cap == 0 {
        cap = 8;
    }
    let bytes = (cap as usize).saturating_mul(core::mem::size_of::<*mut c_char>());
    // SAFETY: taille du nouveau tableau.
    let n = unsafe { ctl_alloc(bytes) }.cast::<*mut c_char>();
    if n.is_null() {
        return false;
    }
    if count != 0 {
        scopy_distinct(
            n.cast(),
            // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
            unsafe { (*l).items.cast() },
            (count as usize) * core::mem::size_of::<*mut c_char>(),
        );
    }
    // SAFETY: l'ancien tableau vient de ctl_alloc, ou est nul.
    unsafe { ctl_free((*l).items.cast()) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*l).items = n };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*l).cap = cap };
    true
}

/// # Safety
/// `r` et `list` sont nuls ou vivants. `item` est nul ou terminé.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_list_add(
    r: *mut t_ctlroot,
    list: *mut t_ctl,
    item: *const c_char,
) -> c_int {
    // SAFETY: r et list suivent ctl_lst_of.
    let l = unsafe { ctl_lst_of(r, list) };
    if l.is_null() || item.is_null() {
        return E_INVAL;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if unsafe { (*l).count } >= CTL_LIST_ITEMS_MAX {
        return E_NOSPC;
    }
    let mut tmp = [0u8; CTL_TEXT_MAX];
    // SAFETY: tmp a CTL_TEXT_MAX octets, item est terminé.
    unsafe { ctl_copy_text(tmp.as_mut_ptr().cast(), item) };
    let nlen = slen(tmp.as_ptr().cast());
    // SAFETY: un octet de plus pour le NUL.
    let copy = unsafe { ctl_alloc(nlen as usize + 1) }.cast::<c_char>();
    if copy.is_null() {
        return E_NOMEM;
    }
    if !grow(l) {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_free(copy.cast()) };
        return E_NOMEM;
    }
    scopy_distinct(copy.cast(), tmp.as_ptr().cast(), nlen as usize + 1);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let count = unsafe { (*l).count };
    // SAFETY: grow a réservé count < cap.
    unsafe { *(*l).items.add(count as usize) = copy };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*l).count = count + 1 };
    // SAFETY: list est la liste.
    unsafe { ctl_lst_sync(list) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*list).rect };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_dirty_add(r, rect) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    (unsafe { (*l).count } as c_int) - 1
}

/// # Safety
/// `r` et `list` sont nuls ou vivants.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_list_remove(
    r: *mut t_ctlroot,
    list: *mut t_ctl,
    index: c_int,
) -> c_int {
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let l = unsafe { ctl_lst_of(r, list) };
    if l.is_null() {
        return E_INVAL;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let count = unsafe { (*l).count };
    if index < 0 || index as u32 >= count {
        return E_RANGE;
    }
    // SAFETY: index est dans le tableau.
    unsafe { ctl_free((*(*l).items.add(index as usize)).cast()) };
    let tail = count - index as u32 - 1;
    scopy(
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { (*l).items.add(index as usize) }.cast(),
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { (*l).items.add(index as usize + 1) }.cast(),
        tail as usize * core::mem::size_of::<*mut c_char>(),
    );
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*l).count = count - 1 };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let sel = unsafe { (*l).sel };
    if sel == index {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { (*l).sel = -1 };
    } else if sel > index {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { (*l).sel = sel - 1 };
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_lst_sync(list) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*list).rect };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_dirty_add(r, rect) };
    0
}

/// # Safety
/// `r` et `list` sont nuls ou vivants.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_list_clear(r: *mut t_ctlroot, list: *mut t_ctl) {
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let l = unsafe { ctl_lst_of(r, list) };
    if l.is_null() {
        return;
    }
    // SAFETY: l est la liste.
    unsafe { ctl_lst_drop(l) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*l).sel = -1 };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*l).sb.pos = 0 };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_lst_sync(list) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*list).rect };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_dirty_add(r, rect) };
}

/// # Safety
/// `list` est nul, ou son privé est un `t_list` si c'est une liste.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_list_selected(list: *const t_ctl) -> c_int {
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if list.is_null() || unsafe { (*list).r#type } != CT_LIST {
        return -1;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*(*list).r#priv.cast::<t_list>()).sel }
}

/// # Safety
/// `list` est nul, ou son privé est un `t_list` si c'est une liste.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_list_count(list: *const t_ctl) -> c_int {
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if list.is_null() || unsafe { (*list).r#type } != CT_LIST {
        return E_INVAL;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*(*list).r#priv.cast::<t_list>()).count as c_int }
}

/// # Safety
/// `list` est nul ou une liste. `out` est nul, ou couvre `cap` octets.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_list_text(
    list: *const t_ctl,
    index: c_int,
    out: *mut c_char,
    cap: u32,
) -> c_int {
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if list.is_null() || unsafe { (*list).r#type } != CT_LIST || out.is_null() || cap == 0 {
        return E_INVAL;
    }
    // SAFETY: le privé est un t_list.
    let l = unsafe { &*(*list).r#priv.cast::<t_list>() };
    if index < 0 || index as u32 >= l.count {
        return E_RANGE;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let src = unsafe { *l.items.add(index as usize) };
    let len = slen(src);
    let mut n = 0u32;
    while n < len {
        // SAFETY: src couvre len octets.
        let next = unsafe { ctl_u8_next(src, len, n) };
        if next > cap - 1 {
            break;
        }
        n = next;
    }
    scopy_distinct(out.cast(), src.cast(), n as usize);
    // SAFETY: cap > 0 et n <= cap - 1.
    unsafe { out.add(n as usize).write(0) };
    n as c_int
}

/// # Safety
/// `c` est une liste non nulle.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_lst_geom(c: *const t_ctl, g: *mut t_lstgeom) {
    let mut m = t_lunametrics {
        caption_h: 0,
        frame_w: 0,
        frame_bottom: 0,
        btn_w: 0,
        btn_h: 0,
        taskbar_h: 0,
        start_w: 0,
        tray_w: 0,
        scroll_w: 0,
        menu_item_h: 0,
        icon_small: 0,
        icon_large: 0,
    };
    luna_metrics(&mut m);
    // SAFETY: c et g sont non nuls.
    let rect = unsafe { (*c).rect };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let count = unsafe { (*(*c).r#priv.cast::<t_list>()).count };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let font = unsafe { ctl_font_of(c) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let g = unsafe { &mut *g };
    g.rowh = 1;
    if !font.is_null() {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let h = unsafe { (*font).height };
        if h > 0 {
            g.rowh = h + 2 * CTL_ROW_PAD;
        }
    }
    let inn = rect_make(
        rect.x + CTL_FRAME,
        rect.y + CTL_FRAME,
        rect.w - 2 * CTL_FRAME,
        rect.h - 2 * CTL_FRAME,
    );
    g.rows = inn.h / g.rowh;
    if g.rows < 1 {
        g.rows = 1;
    }
    g.bar_on = i64::from(count) > i64::from(g.rows) && inn.w > m.scroll_w;
    g.items = inn;
    g.bar = rect_make(0, 0, 0, 0);
    if !g.bar_on {
        return;
    }
    g.items.w -= m.scroll_w;
    g.bar = rect_make(inn.x + inn.w - m.scroll_w, inn.y, m.scroll_w, inn.h);
}

/// # Safety
/// `c` est une liste non nulle.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_lst_sync(c: *mut t_ctl) {
    let mut g = t_lstgeom {
        items: rect_make(0, 0, 0, 0),
        bar: rect_make(0, 0, 0, 0),
        rowh: 1,
        rows: 1,
        bar_on: false,
    };
    // SAFETY: c est une liste, g est local.
    unsafe { ctl_lst_geom(c, &mut g) };
    let l = list_ptr(c);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let count = unsafe { (*l).count };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*l).sb.page = g.rows };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*l).sb.max = 0 };
    if i64::from(count) > i64::from(g.rows) {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { (*l).sb.max = count as i32 - g.rows };
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let pos = unsafe { (*l).sb.pos };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_sb_set(&raw mut (*l).sb, pos) };
}

/// # Safety
/// `c` est une liste non nulle.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_lst_show(c: *mut t_ctl, idx: i32) {
    // SAFETY: c est une liste.
    unsafe { ctl_lst_sync(c) };
    let l = list_ptr(c);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let pos = unsafe { (*l).sb.pos };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let page = unsafe { (*l).sb.page };
    if idx < pos {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_sb_set(&raw mut (*l).sb, idx) };
    } else if idx >= pos + page {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_sb_set(&raw mut (*l).sb, idx - page + 1) };
    }
}

/// # Safety
/// `c` est une liste non nulle.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_lst_item_rect(c: *const t_ctl, idx: i32) -> t_rect {
    let mut g = t_lstgeom {
        items: rect_make(0, 0, 0, 0),
        bar: rect_make(0, 0, 0, 0),
        rowh: 1,
        rows: 1,
        bar_on: false,
    };
    // SAFETY: c est une liste.
    unsafe { ctl_lst_geom(c, &mut g) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let pos = unsafe { (*(*c).r#priv.cast::<t_list>()).sb.pos };
    rect_make(
        g.items.x,
        g.items.y + (idx - pos) * g.rowh,
        g.items.w,
        g.rowh,
    )
}

/// # Safety
/// `c` est une liste non nulle.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_lst_index_at(c: *const t_ctl, p: t_point) -> i32 {
    let mut g = t_lstgeom {
        items: rect_make(0, 0, 0, 0),
        bar: rect_make(0, 0, 0, 0),
        rowh: 1,
        rows: 1,
        bar_on: false,
    };
    // SAFETY: c est une liste.
    unsafe { ctl_lst_geom(c, &mut g) };
    if !rect_contains(g.items, p) {
        return -1;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let l = unsafe { &*(*c).r#priv.cast::<t_list>() };
    let idx = l.sb.pos + (p.y - g.items.y) / g.rowh;
    if idx < 0 || idx >= l.count as i32 {
        return -1;
    }
    idx
}

/// # Safety
/// `r` et `list` sont nuls ou vivants.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_list_select(
    r: *mut t_ctlroot,
    list: *mut t_ctl,
    index: c_int,
) -> c_int {
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let l = unsafe { ctl_lst_of(r, list) };
    if l.is_null() {
        return E_INVAL;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let count = unsafe { (*l).count };
    if index < -1 || index >= count as c_int {
        return E_RANGE;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if index == unsafe { (*l).sel } {
        return 0;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*l).sel = index };
    if index >= 0 {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_lst_show(list, index) };
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*list).rect };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_dirty_add(r, rect) };
    0
}

/// # Safety
/// `r` et `c` sont non nuls. `c` est une liste. `idx` est un indice de ligne.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_lst_pick(r: *mut t_ctlroot, c: *mut t_ctl, idx: i32) -> bool {
    let l = list_ptr(c);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if idx == unsafe { (*l).sel } {
        return true;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*l).sel = idx };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_lst_show(c, idx) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*c).rect };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_dirty_add(r, rect) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_notify(r, c, CN_SELECT) }
}

fn focus_row(l: &t_list) -> i32 {
    if l.sel >= 0 { l.sel } else { l.sb.pos }
}

fn draw_item(r: *mut t_ctlroot, c: *const t_ctl, idx: i32) {
    // SAFETY: c est une liste, idx est dans les lignes peintes.
    let l = unsafe { &*(*c).r#priv.cast::<t_list>() };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let row = unsafe { ctl_lst_item_rect(c, idx) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let en = unsafe { ctl_enabled(c) };
    let sel = idx == l.sel && en;
    let mut tx = t_ctltext {
        r#box: rect_make(row.x + CTL_FRAME, row.y, row.w - 2 * CTL_FRAME, row.h),
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        text: unsafe { *l.items.add(idx as usize) },
        flags: CTL_ELLIPSIS,
        enabled: true,
        mnemonic: false,
        font: core::ptr::null(),
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        color: unsafe { ctl_color_text(en) },
    };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if unsafe { (*c).font } != 0 {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        tx.font = unsafe { ctl_font_of(c) };
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let surface = unsafe { (*r).surface };
    if sel {
        gfx_fill(surface, row, crate::ffi::luna_color_selection());
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        tx.color = unsafe { ctl_color_selected_text() };
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_text_draw(surface, &tx) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if unsafe { (*r).focus.cast_const() == c } && idx == focus_row(l) {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_focus_ring_row(surface, row, sel) };
    }
}

/// # Safety
/// `r` et `c` sont non nuls. `c` est une liste. La surface est vivante.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_lst_paint(r: *mut t_ctlroot, c: *mut t_ctl) {
    // SAFETY: c est une liste.
    let en = unsafe { ctl_enabled(c) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_lst_sync(c) };
    let mut g = t_lstgeom {
        items: rect_make(0, 0, 0, 0),
        bar: rect_make(0, 0, 0, 0),
        rowh: 1,
        rows: 1,
        bar_on: false,
    };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_lst_geom(c, &mut g) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let surface = unsafe { (*r).surface };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let saved = unsafe { (*surface).clip };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*c).rect };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    gfx_fill(surface, rect, unsafe { ctl_color_back(en) });
    luna_edit_frame(surface, rect, en);
    gfx_set_clip(surface, rect_intersect(saved, g.items));
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let pos = unsafe { (*list_ptr(c)).sb.pos };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let count = unsafe { (*list_ptr(c)).count };
    let mut i = pos;
    while i < count as i32 && i <= pos + g.rows {
        draw_item(r, c, i);
        i += 1;
    }
    gfx_set_clip(surface, saved);
    if g.bar_on {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let st = unsafe { &raw const (*list_ptr(c)).sb };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_sb_paint(surface, g.bar, st, en) };
    }
}

fn lst_wheel(r: *mut t_ctlroot, c: *mut t_ctl, e: &t_mouseev) -> bool {
    // SAFETY: c est une liste.
    unsafe { ctl_lst_sync(c) };
    let l = list_ptr(c);
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let pos = unsafe { (*l).sb.pos - ctl_wheel_delta(e.wheel) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if unsafe { ctl_sb_set(&raw mut (*l).sb, pos) } {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let rect = unsafe { (*c).rect };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_dirty_add(r, rect) };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_notify(r, c, CN_SCROLL) };
    }
    true
}

fn lst_down(r: *mut t_ctlroot, c: *mut t_ctl, e: &t_mouseev) -> bool {
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let idx = unsafe { ctl_lst_index_at(c, e.at) };
    if idx < 0 {
        return true;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if unsafe { ctl_lst_pick(r, c, idx) } && e.dbl {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_notify(r, c, CN_DBLCLK) };
    }
    true
}

/// # Safety
/// `r`, `c` et `e` sont non nuls. `c` est une liste.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_lst_mouse(
    r: *mut t_ctlroot,
    c: *mut t_ctl,
    e: *const t_mouseev,
) -> bool {
    // SAFETY: e est non nul.
    let ev = unsafe { &*e };
    if ev.kind == MEV_WHEEL {
        return lst_wheel(r, c, ev);
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_lst_sync(c) };
    let mut g = t_lstgeom {
        items: rect_make(0, 0, 0, 0),
        bar: rect_make(0, 0, 0, 0),
        rowh: 1,
        rows: 1,
        bar_on: false,
    };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_lst_geom(c, &mut g) };
    if ev.kind == MEV_DOWN && !rect_contains(g.bar, ev.at) {
        return lst_down(r, c, ev);
    }
    let x = t_sbctx {
        r,
        c,
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        st: unsafe { &raw mut (*list_ptr(c)).sb },
        bar: g.bar,
    };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let res = unsafe { ctl_sb_mouse(&x, e) };
    if res == SBR_MOVED {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_notify(r, c, CN_SCROLL) };
    }
    res != SBR_NONE
}

fn clamp_idx(v: i32, hi: i32) -> i32 {
    if v < 0 {
        return 0;
    }
    if v > hi { hi } else { v }
}

fn is_nav(code: u32) -> bool {
    code == VK_UP
        || code == VK_DOWN
        || code == VK_HOME
        || code == VK_END
        || code == VK_PRIOR
        || code == VK_NEXT
}

fn nav(sel: i32, count: u32, code: u32, rows: i32) -> i32 {
    let last = count as i32 - 1;
    if code == VK_HOME {
        return 0;
    }
    if code == VK_END {
        return last;
    }
    if sel < 0 {
        return 0;
    }
    if code == VK_UP {
        return clamp_idx(sel - 1, last);
    }
    if code == VK_DOWN {
        return clamp_idx(sel + 1, last);
    }
    if code == VK_PRIOR {
        return clamp_idx(sel - rows, last);
    }
    clamp_idx(sel + rows, last)
}

/// # Safety
/// `r`, `c` et `ev` sont non nuls. `c` est une liste.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_lst_key(
    r: *mut t_ctlroot,
    c: *mut t_ctl,
    ev: *const t_inpevent,
) -> bool {
    // SAFETY: ev est non nul.
    let ev = unsafe { &*ev };
    if ev.r#type != INP_KEY_DOWN || ev.mods & (INPM_ALT | INPM_CTRL) != 0 {
        return false;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let sel = unsafe { (*list_ptr(c)).sel };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let count = unsafe { (*list_ptr(c)).count };
    if ev.code == VK_RETURN {
        if sel >= 0 {
            // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
            unsafe { ctl_notify(r, c, CN_ENTER) };
        }
        return true;
    }
    if !is_nav(ev.code) {
        return false;
    }
    if count == 0 {
        return true;
    }
    let mut g = t_lstgeom {
        items: rect_make(0, 0, 0, 0),
        bar: rect_make(0, 0, 0, 0),
        rowh: 1,
        rows: 1,
        bar_on: false,
    };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_lst_geom(c, &mut g) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_lst_pick(r, c, nav(sel, count, ev.code, g.rows)) };
    true
}
