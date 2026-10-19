use core::ffi::{c_char, c_int, c_void};

use crate::abi::{
    CN_CLOSED, CN_SELECT, CT_MENU, CTL_ENABLED, CTL_FOCUSABLE, CTL_MENU_ARROW, CTL_MENU_BORDER,
    CTL_MENU_GUTTER, CTL_MENU_ITEMS_MAX, CTL_MENU_LEVELS_MAX, CTL_MENU_PAD, CTL_MEV_DOWN,
    CTL_MEV_MOVE, CTL_MI_CHECKED, CTL_MI_DISABLED, CTL_MI_SEPARATOR, CTL_ST_CLOSED, CTL_ST_PRESSED,
    CTL_TEXT_MAX, CTL_VISIBLE, VK_DOWN, VK_END, VK_ESCAPE, VK_HOME, VK_LEFT, VK_RETURN, VK_RIGHT,
    VK_SPACE, VK_UP, t_ctl, t_ctlmenuitem, t_ctlmenuspec, t_ctlroot, t_ctlspec, t_ctltext, t_menu,
    t_mouseev, t_point, t_pool, t_rect, t_surface,
};
use crate::color::ctl_color_text;
use crate::ffi::{
    gfx_hline, gfx_line, luna_color_text, luna_menu_item, luna_menu_panel, luna_metrics,
    rect_contains, rect_make, rect_union,
};
use crate::focus::ctl_focus;
use crate::mem::ctl_free;
use crate::notify::ctl_notify;
use crate::raw::{scopy_distinct, slen};
use crate::scroll::ctl_sb_arrow;
use crate::text::{ctl_copy_text, ctl_mnemonic, ctl_text_draw, ctl_text_width};
use crate::tree::{ctl_dirty_add, ctl_find_id, ctl_in_tree, ctl_link_last, ctl_make, ctl_remove};
use crate::widgets::ctl_focus_ring;

fn menu_of(c: *const t_ctl) -> *mut t_menu {
    // SAFETY: c est non nul. Le privé d'un menu est un t_menu.
    unsafe { (*c).r#priv.cast() }
}

fn text_cost(text: *const c_char) -> usize {
    if text.is_null() {
        return 0;
    }
    let mut tmp = [0u8; CTL_TEXT_MAX];
    // SAFETY: tmp a CTL_TEXT_MAX octets, text est terminé.
    unsafe { ctl_copy_text(tmp.as_mut_ptr().cast(), text) };
    (slen(tmp.as_ptr().cast()) as usize + 8) & !7
}

/// # Safety
/// `it` est nul, ou couvre `n` éléments. `n` et `d` sont les compteurs du C.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mnu_pool(it: *const t_ctlmenuitem, n: u32, d: u32) -> usize {
    if it.is_null() || n == 0 || n > CTL_MENU_ITEMS_MAX || d >= CTL_MENU_LEVELS_MAX {
        return 0;
    }
    let mut total = n as usize * core::mem::size_of::<t_ctlmenuitem>();
    let mut i = 0u32;
    while i < n {
        // SAFETY: i < n, it couvre n éléments.
        let item = unsafe { &*it.add(i as usize) };
        total += text_cost(item.text);
        if !item.sub.is_null() && item.nsub != 0 {
            // SAFETY: sub couvre nsub éléments, d + 1 reste un compteur.
            let sub = unsafe { ctl_mnu_pool(item.sub, item.nsub, d + 1) };
            if sub == 0 {
                return 0;
            }
            total += sub;
        }
        i += 1;
    }
    total
}

fn take(p: *mut t_pool, n: usize) -> *mut c_void {
    // SAFETY: p est non nul.
    if n > unsafe { (*p).left } {
        return core::ptr::null_mut();
    }
    // SAFETY: p est inscriptible, cur couvre left octets.
    let mem = unsafe { (*p).cur };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe {
        (*p).cur = (*p).cur.add(n);
        (*p).left -= n;
    }
    mem.cast()
}

fn copy_one(p: *mut t_pool, src: *const t_ctlmenuitem, dst: *mut t_ctlmenuitem) -> bool {
    // SAFETY: src est non nul.
    let src = unsafe { &*src };
    // SAFETY: dst est dans le pool, mis à zéro par ctl_alloc.
    unsafe {
        (*dst).id = src.id;
        (*dst).flags = src.flags & (CTL_MI_DISABLED | CTL_MI_CHECKED | CTL_MI_SEPARATOR);
    }
    if !src.text.is_null() {
        let mut tmp = [0u8; CTL_TEXT_MAX];
        // SAFETY: tmp a CTL_TEXT_MAX octets.
        unsafe { ctl_copy_text(tmp.as_mut_ptr().cast(), src.text) };
        let n = (slen(tmp.as_ptr().cast()) as usize + 8) & !7;
        let txt = take(p, n).cast::<c_char>();
        if txt.is_null() {
            return false;
        }
        let bytes = slen(tmp.as_ptr().cast()) as usize + 1;
        scopy_distinct(txt.cast(), tmp.as_ptr().cast(), bytes);
        // SAFETY: dst est inscriptible.
        unsafe { (*dst).text = txt };
    }
    if src.sub.is_null() || src.nsub == 0 {
        return true;
    }
    // SAFETY: sub couvre nsub éléments, p a encore de la place ou copy échoue.
    let sub = unsafe { ctl_mnu_copy(p, src.sub, src.nsub) };
    // SAFETY: dst est inscriptible.
    unsafe {
        (*dst).sub = sub;
        (*dst).nsub = src.nsub;
    }
    !sub.is_null()
}

/// # Safety
/// `p` est non nul. `it` couvre `n` éléments. Le pool a été mis à zéro.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mnu_copy(
    p: *mut t_pool,
    it: *const t_ctlmenuitem,
    n: u32,
) -> *mut t_ctlmenuitem {
    let arr = take(p, n as usize * core::mem::size_of::<t_ctlmenuitem>()).cast::<t_ctlmenuitem>();
    if arr.is_null() {
        return core::ptr::null_mut();
    }
    let mut i = 0u32;
    while i < n {
        // SAFETY: i < n, it et arr couvrent n éléments.
        if !copy_one(p, unsafe { it.add(i as usize) }, unsafe {
            arr.add(i as usize)
        }) {
            return core::ptr::null_mut();
        }
        i += 1;
    }
    arr
}

/// # Safety
/// `it` et `m` sont non nuls.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mnu_item_h(
    it: *const t_ctlmenuitem,
    m: *const crate::abi::t_lunametrics,
) -> i32 {
    // SAFETY: it et m sont non nuls.
    let it = unsafe { &*it };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let h = unsafe { (*m).menu_item_h };
    if it.flags & CTL_MI_SEPARATOR != 0 {
        h / 4 + 1
    } else {
        h
    }
}

/// # Safety
/// `m` est non nul. `lvl` est un niveau du menu.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mnu_item_rect(m: *const t_menu, lvl: u32, idx: u32) -> t_rect {
    // SAFETY: m est non nul, lvl < CTL_MENU_LEVELS_MAX pour un menu construit.
    let lv = unsafe { &(*m).lv[lvl as usize] };
    let mut mt = core::mem::MaybeUninit::<crate::abi::t_lunametrics>::uninit();
    // SAFETY: mt est local et inscriptible.
    luna_metrics(mt.as_mut_ptr());
    // SAFETY: luna_metrics a écrit toute la structure.
    let mt = unsafe { mt.assume_init() };
    let mut r = rect_make(
        lv.rect.x + CTL_MENU_BORDER,
        lv.rect.y + CTL_MENU_BORDER,
        lv.rect.w - 2 * CTL_MENU_BORDER,
        0,
    );
    let mut i = 0u32;
    while i < idx && i < lv.count {
        // SAFETY: i < count, items couvre count éléments.
        r.y += unsafe { ctl_mnu_item_h(lv.items.add(i as usize), &mt) };
        i += 1;
    }
    if idx < lv.count {
        // SAFETY: idx < count.
        r.h = unsafe { ctl_mnu_item_h(lv.items.add(idx as usize), &mt) };
    }
    r
}

/// # Safety
/// `m` est non nul. `lvl` est un niveau du menu.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mnu_item_at(m: *const t_menu, lvl: u32, p: t_point) -> i32 {
    // SAFETY: m est non nul.
    let lv = unsafe { &(*m).lv[lvl as usize] };
    if !rect_contains(lv.rect, p) {
        return -1;
    }
    let mut mt = core::mem::MaybeUninit::<crate::abi::t_lunametrics>::uninit();
    luna_metrics(mt.as_mut_ptr());
    // SAFETY: luna_metrics a écrit mt.
    let mt = unsafe { mt.assume_init() };
    let mut y = lv.rect.y + CTL_MENU_BORDER;
    let mut i = 0u32;
    while i < lv.count {
        // SAFETY: i < count.
        y += unsafe { ctl_mnu_item_h(lv.items.add(i as usize), &mt) };
        if p.y < y {
            return i as i32;
        }
        i += 1;
    }
    -1
}

/// # Safety
/// `m` est non nul. `depth` vaut au moins 1 pour un menu ouvert.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mnu_bounds(m: *const t_menu) -> t_rect {
    // SAFETY: m est non nul.
    let m = unsafe { &*m };
    let mut b = m.lv[0].rect;
    let mut i = 1u32;
    while i < m.depth {
        b = rect_union(b, m.lv[i as usize].rect);
        i += 1;
    }
    b
}

fn item_w(it: *const t_ctlmenuitem) -> i32 {
    // SAFETY: it est non nul.
    let it = unsafe { &*it };
    if it.flags & CTL_MI_SEPARATOR != 0 || it.text.is_null() {
        return 0;
    }
    let tx = t_ctltext {
        r#box: rect_make(0, 0, 0, 0),
        text: it.text,
        flags: 0,
        color: 0,
        enabled: false,
        mnemonic: true,
        font: core::ptr::null(),
    };
    // SAFETY: tx est local et complet.
    unsafe { ctl_text_width(&tx) }
}

fn menu_size(m: *const t_menu, lvl: u32) -> t_point {
    // SAFETY: m est non nul.
    let lv = unsafe { &(*m).lv[lvl as usize] };
    let mut mt = core::mem::MaybeUninit::<crate::abi::t_lunametrics>::uninit();
    luna_metrics(mt.as_mut_ptr());
    // SAFETY: luna_metrics a écrit mt.
    let mt = unsafe { mt.assume_init() };
    let mut sz = t_point {
        x: 0,
        y: 2 * CTL_MENU_BORDER,
    };
    let mut i = 0u32;
    while i < lv.count {
        // SAFETY: i < count.
        sz.y += unsafe { ctl_mnu_item_h(lv.items.add(i as usize), &mt) };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let w = item_w(unsafe { lv.items.add(i as usize) });
        if w > sz.x {
            // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
            sz.x = item_w(unsafe { lv.items.add(i as usize) });
        }
        i += 1;
    }
    sz.x += 2 * CTL_MENU_BORDER + CTL_MENU_GUTTER + CTL_MENU_ARROW;
    sz
}

fn sub_origin(m: *const t_menu, lvl: u32, sz: t_point, sw: i32) -> t_point {
    // SAFETY: m est non nul, lvl >= 1.
    let par = unsafe { &(*m).lv[lvl as usize - 1] };
    // SAFETY: m est non nul.
    let it = unsafe { ctl_mnu_item_rect(m, lvl - 1, par.sel as u32) };
    let mut o = t_point {
        x: par.rect.x + par.rect.w - CTL_MENU_BORDER,
        y: it.y - CTL_MENU_BORDER,
    };
    if o.x + sz.x > sw {
        o.x = par.rect.x - sz.x + CTL_MENU_BORDER;
    }
    o
}

fn place(s: *const t_surface, r: *mut t_rect) {
    // SAFETY: s et r sont non nuls.
    let sw = unsafe { (*s).w };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let sh = unsafe { (*s).h };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let mut x = unsafe { (*r).x };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let mut y = unsafe { (*r).y };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let w = unsafe { (*r).w };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let h = unsafe { (*r).h };
    if x + w > sw {
        x = sw - w;
    }
    if y + h > sh {
        y = sh - h;
    }
    if x < 0 {
        x = 0;
    }
    if y < 0 {
        y = 0;
    }
    // SAFETY: r est inscriptible.
    unsafe {
        (*r).x = x;
        (*r).y = y;
    }
}

/// # Safety
/// `r` et `c` sont non nuls. `r.surface` est vivante. `lvl` est un niveau du menu.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mnu_layout(r: *mut t_ctlroot, c: *mut t_ctl, lvl: u32) {
    let m = menu_of(c);
    let sz = menu_size(m, lvl);
    // SAFETY: m est non nul.
    let mut o = unsafe {
        t_point {
            x: (*m).lv[lvl as usize].rect.x,
            y: (*m).lv[lvl as usize].rect.y,
        }
    };
    if lvl > 0 {
        // SAFETY: r.surface est non nul.
        let sw = unsafe { (*(*r).surface).w };
        o = sub_origin(m, lvl, sz, sw);
    }
    // SAFETY: m est inscriptible.
    unsafe { (*m).lv[lvl as usize].rect = rect_make(o.x, o.y, sz.x, sz.y) };
    // SAFETY: surface et rect du niveau sont vivants.
    place(unsafe { (*r).surface }, unsafe {
        &mut (*m).lv[lvl as usize].rect
    });
}

/// # Safety
/// `r` et `c` sont non nuls. `c` est un menu.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mnu_refresh(r: *mut t_ctlroot, c: *mut t_ctl) {
    // SAFETY: c est non nul.
    let old = unsafe { (*c).rect };
    // SAFETY: le privé est un t_menu.
    let b = unsafe { ctl_mnu_bounds(menu_of(c)) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*c).rect = b };
    // SAFETY: r a une surface.
    unsafe { ctl_dirty_add(r, old) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let now = unsafe { (*c).rect };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_dirty_add(r, now) };
}

/// # Safety
/// `r` et `c` sont non nuls. `l` est un niveau ouvert.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mnu_select(r: *mut t_ctlroot, c: *mut t_ctl, l: u32, i: i32) {
    let m = menu_of(c);
    // SAFETY: m est non nul.
    let sel = unsafe { (*m).lv[l as usize].sel };
    if sel == i {
        return;
    }
    if sel >= 0 {
        // SAFETY: m est non nul.
        let row = unsafe { ctl_mnu_item_rect(m, l, sel as u32) };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_dirty_add(r, row) };
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*m).lv[l as usize].sel = i };
    if i >= 0 {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let row = unsafe { ctl_mnu_item_rect(m, l, i as u32) };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_dirty_add(r, row) };
    }
    // SAFETY: m est non nul.
    if unsafe { (*m).depth } > l + 1 {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { (*m).depth = l + 1 };
        // SAFETY: r et c sont vivants.
        unsafe { ctl_mnu_refresh(r, c) };
    }
}

/// # Safety
/// `r` et `c` sont non nuls. `l` est le dernier niveau ouvert, ou la fonction ne fait rien.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mnu_open_sub(r: *mut t_ctlroot, c: *mut t_ctl, l: u32) {
    let m = menu_of(c);
    // SAFETY: m est non nul.
    if unsafe { (*m).depth } != l + 1 || l + 1 >= CTL_MENU_LEVELS_MAX {
        return;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let sel = unsafe { (*m).lv[l as usize].sel };
    if sel < 0 {
        return;
    }
    // SAFETY: sel >= 0 et les items du niveau sont en place.
    let it = unsafe { &*(*m).lv[l as usize].items.add(sel as usize) };
    if it.sub.is_null() || it.nsub == 0 || it.flags & CTL_MI_DISABLED != 0 {
        return;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe {
        (*m).lv[l as usize + 1].items = it.sub;
        (*m).lv[l as usize + 1].count = it.nsub;
        (*m).lv[l as usize + 1].sel = -1;
        (*m).depth = l + 2;
    }
    // SAFETY: le niveau l + 1 vient d'être armé.
    unsafe { ctl_mnu_layout(r, c, l + 1) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_mnu_refresh(r, c) };
}

/// # Safety
/// `r` et `c` sont non nuls.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mnu_pop(r: *mut t_ctlroot, c: *mut t_ctl) {
    let m = menu_of(c);
    // SAFETY: m est non nul.
    if unsafe { (*m).depth } <= 1 {
        return;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*m).depth -= 1 };
    // SAFETY: r et c sont vivants.
    unsafe { ctl_mnu_refresh(r, c) };
}

/// # Safety
/// `r` et `c` sont non nuls. `c` est un menu dans l'arbre.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mnu_end(r: *mut t_ctlroot, c: *mut t_ctl) {
    // SAFETY: c est non nul.
    if unsafe { (*c).state } & CTL_ST_CLOSED != 0 {
        return;
    }
    let m = menu_of(c);
    // SAFETY: m est non nul.
    let back = unsafe { (*m).prev };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe {
        (*c).state |= CTL_ST_CLOSED;
        (*c).flags &= !CTL_VISIBLE;
        if (*r).capture == c {
            (*r).capture = core::ptr::null_mut();
        }
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let rect = unsafe { (*c).rect };
    // SAFETY: r a une surface.
    unsafe { ctl_dirty_add(r, rect) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if unsafe { (*r).focus } != c {
        return;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*r).focus = core::ptr::null_mut() };
    // SAFETY: back est nul ou un contrôle de l'arbre.
    unsafe { ctl_focus(r, back) };
}

/// # Safety
/// `r` et `c` sont non nuls.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mnu_cancel(r: *mut t_ctlroot, c: *mut t_ctl) {
    // SAFETY: r et c sont non nuls.
    unsafe { ctl_mnu_end(r, c) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_notify(r, c, CN_CLOSED) };
}

/// # Safety
/// `m` est non nul. `lvl` est un niveau, `from` un indice ou -1, `dir` vaut 1 ou -1.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mnu_step(m: *const t_menu, lvl: u32, from: i32, dir: i32) -> i32 {
    // SAFETY: m est non nul.
    let lv = unsafe { &(*m).lv[lvl as usize] };
    let n = lv.count as i32;
    let mut i = from;
    let mut k = 0i32;
    while k < n {
        i = (i + dir + n) % n;
        // SAFETY: i est dans [0, n).
        let flags = unsafe { (*lv.items.add(i as usize)).flags };
        if flags & CTL_MI_SEPARATOR == 0 {
            return i;
        }
        k += 1;
    }
    -1
}

fn mn_match(it: *const t_ctlmenuitem, ch: u32) -> bool {
    // SAFETY: it est non nul.
    let it = unsafe { &*it };
    if it.flags & (CTL_MI_SEPARATOR | CTL_MI_DISABLED) != 0 || it.text.is_null() {
        return false;
    }
    // SAFETY: text est terminé.
    unsafe { ctl_mnemonic(it.text) as u32 == ch }
}

/// # Safety
/// `m` est non nul. `lvl` est un niveau.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mnu_count(m: *const t_menu, lvl: u32, ch: u32) -> u32 {
    // SAFETY: m est non nul.
    let lv = unsafe { &(*m).lv[lvl as usize] };
    let mut i = 0u32;
    let mut n = 0u32;
    while i < lv.count {
        // SAFETY: i < count.
        if mn_match(unsafe { lv.items.add(i as usize) }, ch) {
            n += 1;
        }
        i += 1;
    }
    n
}

/// # Safety
/// `m` est non nul. `lvl` est un niveau dont le compte est non nul, ou la fonction rend -1.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mnu_find(m: *const t_menu, lvl: u32, ch: u32) -> i32 {
    // SAFETY: m est non nul.
    let lv = unsafe { &(*m).lv[lvl as usize] };
    let n = lv.count as i32;
    let mut i = lv.sel;
    let mut k = 0i32;
    while k < n {
        i = (i + 1 + n) % n;
        // SAFETY: i est dans [0, n).
        if mn_match(unsafe { lv.items.add(i as usize) }, ch) {
            return i;
        }
        k += 1;
    }
    -1
}

/// # Safety
/// `r` et `c` sont non nuls. `l` est un niveau, `i` un indice de ce niveau.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mnu_activate(r: *mut t_ctlroot, c: *mut t_ctl, l: u32, i: i32) {
    let m = menu_of(c);
    // SAFETY: i est un indice du niveau.
    let it = unsafe { &*(*m).lv[l as usize].items.add(i as usize) };
    if it.flags & (CTL_MI_SEPARATOR | CTL_MI_DISABLED) != 0 {
        return;
    }
    let sub = it.sub;
    let nsub = it.nsub;
    let id = it.id;
    // SAFETY: r et c sont vivants.
    unsafe { ctl_mnu_select(r, c, l, i) };
    if !sub.is_null() && nsub != 0 {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_mnu_open_sub(r, c, l) };
        // SAFETY: m est non nul.
        if unsafe { (*m).depth } == l + 2 {
            // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
            let step = unsafe { ctl_mnu_step(m, l + 1, -1, 1) };
            // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
            unsafe { ctl_mnu_select(r, c, l + 1, step) };
        }
        return;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*m).picked = id };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_mnu_end(r, c) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_notify(r, c, CN_SELECT) };
}

/// # Safety
/// `menu` est nul, ou son privé est un `t_menu` s'il est un menu.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_menu_result(menu: *const t_ctl) -> c_int {
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if menu.is_null() || unsafe { (*menu).r#type } != CT_MENU {
        return 0;
    }
    // SAFETY: le privé d'un menu est un t_menu.
    unsafe { (*(*menu).r#priv.cast::<t_menu>()).picked as c_int }
}

/// # Safety
/// `r` et `menu` sont nuls ou vivants.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_menu_close(r: *mut t_ctlroot, menu: *mut t_ctl) {
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if r.is_null() || menu.is_null() || unsafe { (*r).root.is_null() } {
        return;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if unsafe { (*menu).r#type } != CT_MENU || unsafe { !ctl_in_tree(r, menu) } {
        return;
    }
    // SAFETY: menu est un menu de l'arbre.
    unsafe { ctl_mnu_end(r, menu) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_remove(r, menu) };
}

fn level_at(m: *const t_menu, p: t_point) -> i32 {
    // SAFETY: m est non nul.
    let depth = unsafe { (*m).depth };
    let mut l = depth as i32 - 1;
    while l >= 0 {
        // SAFETY: l est dans [0, depth).
        let rect = unsafe { (*m).lv[l as usize].rect };
        if rect_contains(rect, p) {
            return l;
        }
        l -= 1;
    }
    -1
}

fn hover(r: *mut t_ctlroot, c: *mut t_ctl, l: u32, p: t_point) {
    let m = menu_of(c);
    // SAFETY: m est non nul.
    let i = unsafe { ctl_mnu_item_at(m, l, p) };
    if i < 0 {
        return;
    }
    // SAFETY: i est un indice du niveau.
    let flags = unsafe { (*(*m).lv[l as usize].items.add(i as usize)).flags };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let sub = unsafe { (*(*m).lv[l as usize].items.add(i as usize)).sub };
    if flags & CTL_MI_SEPARATOR != 0 {
        return;
    }
    // SAFETY: r et c sont vivants.
    unsafe { ctl_mnu_select(r, c, l, i) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if unsafe { (*m).depth } == l + 1 && !sub.is_null() {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_mnu_open_sub(r, c, l) };
    }
}

fn press(r: *mut t_ctlroot, c: *mut t_ctl, l: u32, p: t_point) {
    let m = menu_of(c);
    // SAFETY: m est non nul.
    let i = unsafe { ctl_mnu_item_at(m, l, p) };
    if i >= 0 {
        // SAFETY: i est un indice.
        unsafe { ctl_mnu_activate(r, c, l, i) };
    }
}

/// # Safety
/// `r`, `c` et `e` sont non nuls. `c.priv` est un `t_menu`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mnu_mouse(
    r: *mut t_ctlroot,
    c: *mut t_ctl,
    e: *const t_mouseev,
) -> bool {
    // SAFETY: e est non nul.
    let e = unsafe { &*e };
    let m = menu_of(c);
    let l = level_at(m, e.at);
    if e.kind == CTL_MEV_DOWN && l < 0 {
        // SAFETY: r et c sont vivants.
        unsafe { ctl_mnu_cancel(r, c) };
    } else if e.kind == CTL_MEV_DOWN {
        press(r, c, l as u32, e.at);
    } else if e.kind == CTL_MEV_MOVE && l >= 0 {
        hover(r, c, l as u32, e.at);
    }
    true
}

fn move_sel(r: *mut t_ctlroot, c: *mut t_ctl, dir: i32) {
    let m = menu_of(c);
    // SAFETY: m est non nul, depth >= 1.
    let l = unsafe { (*m).depth } - 1;
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let mut from = unsafe { (*m).lv[l as usize].sel };
    if dir < 0 && from < 0 {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        from = unsafe { (*m).lv[l as usize].count } as i32;
    }
    // SAFETY: m est non nul.
    from = unsafe { ctl_mnu_step(m, l, from, dir) };
    if from >= 0 {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_mnu_select(r, c, l, from) };
    }
}

fn edge(r: *mut t_ctlroot, c: *mut t_ctl, last: bool) {
    let m = menu_of(c);
    // SAFETY: m est non nul.
    let l = unsafe { (*m).depth } - 1;
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let count = unsafe { (*m).lv[l as usize].count } as i32;
    let i = if last {
        // SAFETY: m est non nul.
        unsafe { ctl_mnu_step(m, l, count, -1) }
    } else {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_mnu_step(m, l, -1, 1) }
    };
    if i >= 0 {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_mnu_select(r, c, l, i) };
    }
}

fn mnemonic(r: *mut t_ctlroot, c: *mut t_ctl, mut code: u32) {
    if code >= b'a' as u32 && code <= b'z' as u32 {
        code = code - b'a' as u32 + b'A' as u32;
    }
    let m = menu_of(c);
    // SAFETY: m est non nul.
    let l = unsafe { (*m).depth } - 1;
    // SAFETY: m est non nul.
    let i = unsafe { ctl_mnu_find(m, l, code) };
    if i < 0 {
        return;
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if unsafe { ctl_mnu_count(m, l, code) } == 1 {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_mnu_activate(r, c, l, i) };
    } else {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_mnu_select(r, c, l, i) };
    }
}

fn horizontal(r: *mut t_ctlroot, c: *mut t_ctl, code: u32) {
    let m = menu_of(c);
    // SAFETY: m est non nul.
    let l = unsafe { (*m).depth } - 1;
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let sel = unsafe { (*m).lv[l as usize].sel };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let depth = unsafe { (*m).depth };
    if code == VK_LEFT || code == VK_ESCAPE {
        if depth > 1 {
            // SAFETY: r et c sont vivants.
            unsafe { ctl_mnu_pop(r, c) };
        } else if code == VK_ESCAPE {
            // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
            unsafe { ctl_mnu_cancel(r, c) };
        }
        return;
    }
    if sel >= 0 {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_mnu_open_sub(r, c, l) };
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if sel >= 0 && unsafe { (*m).depth } == l + 2 {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let step = unsafe { ctl_mnu_step(m, l + 1, -1, 1) };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_mnu_select(r, c, l + 1, step) };
    }
}

/// # Safety
/// `r`, `c` et `ev` sont non nuls. `c.priv` est un `t_menu` de profondeur au moins 1.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mnu_key(
    r: *mut t_ctlroot,
    c: *mut t_ctl,
    ev: *const crate::abi::t_inpevent,
) -> bool {
    // SAFETY: ev est non nul.
    let ev = unsafe { &*ev };
    let m = menu_of(c);
    // SAFETY: m est non nul.
    let l = unsafe { (*m).depth } - 1;
    if ev.r#type == crate::abi::INP_CHAR {
        mnemonic(r, c, ev.code);
    } else if ev.code == VK_UP || ev.code == VK_DOWN {
        let dir = 2 * i32::from(ev.code == VK_DOWN) - 1;
        move_sel(r, c, dir);
    } else if ev.code == VK_HOME || ev.code == VK_END {
        edge(r, c, ev.code == VK_END);
    } else if ev.code == VK_LEFT || ev.code == VK_RIGHT || ev.code == VK_ESCAPE {
        horizontal(r, c, ev.code);
    } else if (ev.code == VK_RETURN || ev.code == VK_SPACE)
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        && unsafe { (*m).lv[l as usize].sel } >= 0
    {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let sel = unsafe { (*m).lv[l as usize].sel };
        // SAFETY: sel >= 0.
        unsafe { ctl_mnu_activate(r, c, l, sel) };
    }
    true
}

fn marks(s: *mut t_surface, row: t_rect, it: *const t_ctlmenuitem, col: u32) {
    // SAFETY: it est non nul.
    let it = unsafe { &*it };
    if it.flags & CTL_MI_CHECKED != 0 {
        let mid = t_point {
            x: row.x + CTL_MENU_GUTTER / 2,
            y: row.y + row.h / 2,
        };
        gfx_line(
            s,
            t_point {
                x: mid.x - 3,
                y: mid.y,
            },
            t_point {
                x: mid.x - 1,
                y: mid.y + 2,
            },
            col,
        );
        gfx_line(
            s,
            t_point {
                x: mid.x - 1,
                y: mid.y + 2,
            },
            t_point {
                x: mid.x + 3,
                y: mid.y - 2,
            },
            col,
        );
    }
    if !it.sub.is_null() && it.nsub != 0 {
        // SAFETY: s est une surface. dir 3 est la flèche droite du C.
        unsafe {
            ctl_sb_arrow(
                s,
                rect_make(row.x + row.w - CTL_MENU_ARROW, row.y, CTL_MENU_ARROW, row.h),
                3,
                col,
            );
        }
    }
}

fn sep(s: *mut t_surface, row: t_rect) {
    // SAFETY: pas de pointeur autre que la surface.
    unsafe {
        gfx_hline(
            s,
            t_point {
                x: row.x + CTL_MENU_PAD,
                y: row.y + row.h / 2,
            },
            row.w - 2 * CTL_MENU_PAD,
            ctl_color_text(false),
        );
    }
}

fn highlight(r: *mut t_ctlroot, row: t_rect, sel: bool, enabled: bool) -> u32 {
    if sel && enabled {
        // SAFETY: r.surface est non nul.
        luna_menu_item(unsafe { (*r).surface }, row, crate::abi::LS_HOT);
        return luna_color_text();
    }
    if sel {
        // SAFETY: la surface est vivante.
        unsafe { ctl_focus_ring((*r).surface, row) };
    }
    // SAFETY: pas de pointeur.
    unsafe { ctl_color_text(enabled) }
}

fn item(r: *mut t_ctlroot, m: *const t_menu, l: u32, i: u32) {
    // SAFETY: m est non nul, i < count.
    let it = unsafe { &*(*m).lv[l as usize].items.add(i as usize) };
    // SAFETY: m est non nul.
    let row = unsafe { ctl_mnu_item_rect(m, l, i) };
    if it.flags & CTL_MI_SEPARATOR != 0 {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        sep(unsafe { (*r).surface }, row);
        return;
    }
    let enabled = it.flags & CTL_MI_DISABLED == 0;
    // SAFETY: m est non nul.
    let sel = unsafe { i as i32 == (*m).lv[l as usize].sel };
    let color = highlight(r, row, sel, enabled);
    let tx = t_ctltext {
        r#box: rect_make(
            row.x + CTL_MENU_GUTTER,
            row.y,
            row.w - CTL_MENU_GUTTER - CTL_MENU_ARROW,
            row.h,
        ),
        text: it.text,
        flags: 0,
        color,
        enabled,
        mnemonic: true,
        font: core::ptr::null(),
    };
    // SAFETY: tx est local, la surface est vivante.
    unsafe { ctl_text_draw((*r).surface, &tx) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    marks(unsafe { (*r).surface }, row, it, color);
}

/// # Safety
/// `r` et `c` sont non nuls. `r.surface` est vivante. `c.priv` est un `t_menu`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_mnu_paint(r: *mut t_ctlroot, c: *mut t_ctl) {
    let m = menu_of(c);
    // SAFETY: m est non nul.
    let depth = unsafe { (*m).depth };
    let mut l = 0u32;
    while l < depth {
        // SAFETY: la surface est vivante.
        luna_menu_panel(unsafe { (*r).surface }, unsafe { (*m).lv[l as usize].rect });
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let count = unsafe { (*m).lv[l as usize].count };
        let mut i = 0u32;
        while i < count {
            item(r, m, l, i);
            i += 1;
        }
        l += 1;
    }
}

fn fill(c: *mut t_ctl, spec: *const t_ctlmenuspec, pool: usize) -> bool {
    let m = menu_of(c);
    let mut p = t_pool {
        // SAFETY: le privé est un t_menu, le pool suit la structure.
        cur: unsafe {
            (*c).r#priv
                .cast::<c_char>()
                .add(core::mem::size_of::<t_menu>())
        },
        left: pool,
    };
    // SAFETY: spec est non nul.
    let items = unsafe { (*spec).items };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let count = unsafe { (*spec).count };
    // SAFETY: p est local, items couvre count éléments.
    let copied = unsafe { ctl_mnu_copy(&mut p, items, count) };
    // SAFETY: m est inscriptible. c.rect est le rectangle initial.
    unsafe {
        (*m).lv[0].items = copied;
        (*m).lv[0].count = count;
        (*m).lv[0].sel = -1;
        (*m).lv[0].rect = (*c).rect;
        (*m).depth = 1;
    }
    !copied.is_null()
}

fn build(spec: *const t_ctlmenuspec, pool: usize) -> *mut t_ctl {
    // SAFETY: spec est non nul.
    let spec = unsafe { &*spec };
    let cs = t_ctlspec {
        r#type: CT_MENU,
        id: spec.id,
        flags: CTL_VISIBLE | CTL_ENABLED | CTL_FOCUSABLE,
        rect: rect_make(spec.at.x, spec.at.y, 0, 0),
        text: core::ptr::null(),
    };
    // SAFETY: cs est local.
    let c = unsafe { ctl_make(&cs, pool) };
    if c.is_null() {
        return core::ptr::null_mut();
    }
    if fill(c, spec, pool) {
        return c;
    }
    // SAFETY: c vient de ctl_alloc et n'est pas encore dans l'arbre.
    unsafe { ctl_free(c.cast()) };
    core::ptr::null_mut()
}

fn close_others(r: *mut t_ctlroot) {
    // SAFETY: r.root est non nul.
    let mut kid = unsafe { (*(*r).root).first };
    while !kid.is_null() {
        // SAFETY: kid est un enfant de la racine.
        let next = unsafe { (*kid).next };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        if unsafe { (*kid).r#type } == CT_MENU {
            // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
            unsafe { ctl_mnu_end(r, kid) };
        }
        kid = next;
    }
}

fn open(r: *mut t_ctlroot, c: *mut t_ctl) {
    let m = menu_of(c);
    close_others(r);
    // SAFETY: r est non nul.
    let capture = unsafe { (*r).capture };
    if !capture.is_null() {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { (*capture).state &= !CTL_ST_PRESSED };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let rect = unsafe { (*capture).rect };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_dirty_add(r, rect) };
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    let focus = unsafe { (*r).focus };
    if !focus.is_null() {
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        let rect = unsafe { (*focus).rect };
        // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
        unsafe { ctl_dirty_add(r, rect) };
    }
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { (*m).prev = focus };
    // SAFETY: c n'est pas encore lié.
    unsafe { ctl_link_last((*r).root, c) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_mnu_layout(r, c, 0) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe { ctl_mnu_refresh(r, c) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    unsafe {
        (*r).focus = c;
        (*r).capture = c;
    }
}

/// # Safety
/// `r` et `spec` sont nuls ou vivants. `spec.items` couvre `spec.count` éléments.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn ctl_menu_popup(
    r: *mut t_ctlroot,
    spec: *const t_ctlmenuspec,
) -> *mut t_ctl {
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if r.is_null() || spec.is_null() || unsafe { (*r).root.is_null() } {
        return core::ptr::null_mut();
    }
    // SAFETY: spec est non nul.
    let spec_ref = unsafe { &*spec };
    // SAFETY: items couvre count, ou count vaut 0.
    let pool = unsafe { ctl_mnu_pool(spec_ref.items, spec_ref.count, 0) };
    // SAFETY: pointeur du contrat C, aligné et vivant, ou arguments déjà bornés.
    if pool == 0 || (spec_ref.id != 0 && unsafe { !ctl_find_id(r, spec_ref.id).is_null() }) {
        return core::ptr::null_mut();
    }
    let c = build(spec, pool);
    if c.is_null() {
        return core::ptr::null_mut();
    }
    open(r, c);
    c
}
