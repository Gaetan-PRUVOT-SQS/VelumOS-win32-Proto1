use core::ffi::c_char;

use crate::abi::{
    FONT_UI, FONT_UI_BOLD, ICON_POWER, ICON_SEARCH, ICON_USER, LA_CENTER, LA_LEFT,
    LUNA_GLASS_LAUNCHER, LUNA_LAUNCHER_HEIGHT, LUNA_LAUNCHER_LOGOFF, LUNA_LAUNCHER_MARGIN_X,
    LUNA_LAUNCHER_NONE, LUNA_LAUNCHER_POWER, LUNA_LAUNCHER_TILES, LUNA_LAUNCHER_WIDTH,
    LUNA_LM_APPS_MAX, LUNA_LM_LABEL_MAX, LUNA_LM_NONE, LUNA_LM_QUERY_MAX, LUNA_LM_RECENTS,
    LUNA_LM_ROWS, LUNA_LV_ALL, LUNA_LV_PINNED, LUNA_LV_RECENT, LUNA_LV_SEARCH, LUNA_THEME_DARK,
    VT_R_TILE, t_color, t_launcher, t_launcherlayout, t_launchertile, t_lcring, t_lgrad, t_lmapp,
    t_lmodel, t_lrr, t_lstop, t_ltext, t_rect, t_surface,
};
use crate::glass::luna_glass;
use crate::ink::{lp_fill, lp_flat, lp_visible};
use crate::num::{lp_clamp, lp_clean, lp_in, lp_inset, lp_max, lp_min, lp_ok, lp_pt, lp_rect};
use crate::ops::luna_icon;
use crate::pal::theme_get;
use crate::shape::{lp_corner_ring, lp_rr_paint};
use crate::text::{lp_text, lp_text_len};

const LL_PAD: i32 = 18;
const LL_GAP: i32 = 6;
const LL_SEARCH_H: i32 = 48;
const LL_SEARCH_ICON: i32 = 26;
const LL_SEARCH_PAD: i32 = 10;
const LL_SEARCH_PAD_R: i32 = 18;
const LL_TITLE_H: i32 = 16;
const LL_TITLE_GAP: i32 = 10;
const LL_TITLE_PAD: i32 = 6;
const LL_TILE_H: i32 = 96;
const LL_TILE_TOP: i32 = 14;
const LL_FOOT_H: i32 = 36;
const LL_FOOT_PAD: i32 = 10;
const LL_FOOT_GAP: i32 = 11;
const LL_AVATAR: i32 = 34;
const LL_LOGOFF_W: i32 = 135;
const LL_ICON: i32 = 44;
const LL_POWER_ICON: i32 = 26;
const LL_LABEL_H: i32 = 16;
const LL_FOCUS_GAP: i32 = 2;
const LL_MIN_W: i32 = 320;
const LL_MIN_H: i32 = 289;
const LL_HOT_L: t_color = 0x9eff_ffff;
const LL_HOT_D: t_color = 0x1aff_ffff;
const LL_HOT_RIM_L: t_color = 0xffff_ffff;
const LL_HOT_RIM_D: t_color = 0x26ff_ffff;
const LL_BTN_RIM_L: t_color = 0xffff_ffff;
const LL_BTN_RIM_D: t_color = 0x1fff_ffff;
const LL_SEP_L: t_color = 0x99ff_ffff;
const LL_SEP_D: t_color = 0x14ff_ffff;
const LL_INPUT_RIM_L: t_color = 0x1414_1824;
const LL_INPUT_RIM_D: t_color = 0x1aff_ffff;
const LL_ST_HOT: i32 = 1;
const LL_ST_FOCUS: i32 = 2;
const LL_ROW_H: i32 = 26;
const LL_ROW_ICON: i32 = 20;
const LL_ROW_PAD: i32 = 8;
const LL_TAB_PAD: i32 = 4;
const LL_BAR_W: i32 = 3;
const LL_BAR_MIN: i32 = 12;

const G_LATIN1: &[u8] = b"aaaaaaaceeeeiiiidnoooooxouuuuytsaaaaaaaceeeeiiiidnoooooxouuuuyty";
const G_TAB_X: [i32; 3] = [0, 104, 202];
const G_TAB_W: [i32; 3] = [100, 94, 150];
const G_DIR: [[i32; 2]; 4] = [[1, 1], [-1, 1], [1, -1], [-1, -1]];

const PINNED: &[u8] = b"\xc3\x89pingl\xc3\xa9es\0";
const RECENTS: &[u8] = b"R\xc3\xa9cents\0";
const ALL_APPS: &[u8] = b"Toutes les apps\0";
const RESULTS: &[u8] = b"R\xc3\xa9sultats\0";
const SEARCH_HINT: &[u8] = b"Rechercher une app, un fichier, un r\xc3\xa9glage\0";
const LOGOFF_LABEL: &[u8] = b"Fermer la session\0";
const VIDE_0: &[u8] = b"\0";
const VIDE_1: &[u8] = b"Aucune appli r\xc3\xa9cente\0";
const VIDE_2: &[u8] = b"Aucune appli\0";
const VIDE_3: &[u8] = b"Aucun r\xc3\xa9sultat\0";

fn cstr(s: &[u8]) -> *const c_char {
    s.as_ptr().cast()
}

fn byte_at(p: *const c_char) -> u8 {
    // SAFETY: p vise un octet d'une chaîne C, au moins le terminateur.
    unsafe { *p.cast::<u8>() }
}

fn bump(p: &mut *const c_char, n: isize) {
    // SAFETY: n octets existent encore dans la chaîne, terminateur compris.
    *p = unsafe { (*p).offset(n) };
}

fn fold_next(p: &mut *const c_char) -> u32 {
    let s = *p;
    let c = u32::from(byte_at(s));
    if c == 0xc3 {
        let b1 = byte_at(unsafe {
            // SAFETY: le premier octet est 0xc3, le suivant existe (chaîne C).
            s.offset(1)
        });
        if (0x80..=0xbf).contains(&b1) {
            bump(p, 2);
            return u32::from(G_LATIN1[usize::from(b1 - 0x80)]);
        }
    }
    bump(p, 1);
    loop {
        let b = byte_at(*p);
        if (b & 0xc0) != 0x80 || c < 0xc0 {
            break;
        }
        bump(p, 1);
    }
    if (b'A' as u32..=b'Z' as u32).contains(&c) {
        return c + u32::from(b'a' - b'A');
    }
    c
}

pub(crate) fn lp_lm_fold(out: *mut u32, max: u32, s: *const c_char) -> u32 {
    let mut p = s;
    let mut n = 0u32;
    while byte_at(p) != 0 && n < max {
        let cp = fold_next(&mut p);
        // SAFETY: n < max et out a max cases, comme l'écriture C.
        unsafe { *out.add(n as usize) = cp };
        n += 1;
    }
    n
}

fn same_at(a: &[u32], b: &[u32], n: u32) -> bool {
    let mut i = 0u32;
    while i < n {
        if a[i as usize] != b[i as usize] {
            return false;
        }
        i += 1;
    }
    true
}

fn score_at(l: &[u32], at: u32) -> i32 {
    if at == 0 {
        return 3;
    }
    let prev = l[(at - 1) as usize];
    if prev == u32::from(b' ')
        || prev == u32::from(b'-')
        || prev == u32::from(b'.')
        || prev == u32::from(b'_')
    {
        return 2;
    }
    1
}

pub(crate) fn luna_lm_match(label: *const c_char, query: *const c_char) -> i32 {
    if label.is_null() || query.is_null() {
        return 0;
    }
    let mut l = [0u32; LUNA_LM_LABEL_MAX];
    let mut q = [0u32; LUNA_LM_QUERY_MAX];
    let nl = lp_lm_fold(l.as_mut_ptr(), LUNA_LM_LABEL_MAX as u32, label);
    let nq = lp_lm_fold(q.as_mut_ptr(), LUNA_LM_QUERY_MAX as u32, query);
    if nq == 0 {
        return 1;
    }
    let mut best = 0i32;
    let mut at = 0u32;
    while nq <= nl && at <= nl - nq {
        if same_at(&l[at as usize..], &q, nq) {
            let sc = score_at(&l, at);
            if sc > best {
                best = sc;
            }
        }
        at += 1;
    }
    best
}

pub(crate) fn lp_lm_cmp(a: *const c_char, b: *const c_char) -> i32 {
    let mut fa = [0u32; LUNA_LM_LABEL_MAX];
    let mut fb = [0u32; LUNA_LM_LABEL_MAX];
    let na = lp_lm_fold(fa.as_mut_ptr(), LUNA_LM_LABEL_MAX as u32, a);
    let nb = lp_lm_fold(fb.as_mut_ptr(), LUNA_LM_LABEL_MAX as u32, b);
    let mut i = 0u32;
    while i < na && i < nb {
        if fa[i as usize] != fb[i as usize] {
            return if fa[i as usize] < fb[i as usize] {
                -1
            } else {
                1
            };
        }
        i += 1;
    }
    if na == nb {
        return 0;
    }
    if na < nb { -1 } else { 1 }
}

pub(crate) fn luna_lm_init(m: *mut t_lmodel) {
    if m.is_null() {
        return;
    }
    // SAFETY: m non nul, taille d'un t_lmodel, comme memset.
    unsafe {
        core::ptr::write_bytes(m, 0, 1);
        (*m).sel = LUNA_LM_NONE;
        (*m).hot = LUNA_LM_NONE;
    }
}

pub(crate) fn luna_lm_set_apps(m: *mut t_lmodel, apps: *const t_lmapp, n: u32) -> i32 {
    if m.is_null() || n > LUNA_LM_APPS_MAX || (apps.is_null() && n > 0) {
        return -1;
    }
    let mut i = 0u32;
    while i < n {
        // SAFETY: i < n <= 80, apps et m non nuls.
        unsafe {
            (*m).apps[i as usize] = *apps.add(i as usize);
            (*m).apps[i as usize].label[LUNA_LM_LABEL_MAX - 1] = 0;
        }
        i += 1;
    }
    // SAFETY: m non nul.
    unsafe { (*m).napps = n };
    lp_lm_refresh(m);
    0
}

fn utf8_encode(c: u32, out: &mut [c_char; 4]) -> u32 {
    if c < 0x80 {
        out[0] = c as c_char;
        return 1;
    }
    if c < 0x800 {
        out[0] = (0xc0 | (c >> 6)) as u8 as c_char;
        out[1] = (0x80 | (c & 0x3f)) as u8 as c_char;
        return 2;
    }
    if c < 0x10000 {
        out[0] = (0xe0 | (c >> 12)) as u8 as c_char;
        out[1] = (0x80 | ((c >> 6) & 0x3f)) as u8 as c_char;
        out[2] = (0x80 | (c & 0x3f)) as u8 as c_char;
        return 3;
    }
    out[0] = (0xf0 | (c >> 18)) as u8 as c_char;
    out[1] = (0x80 | ((c >> 12) & 0x3f)) as u8 as c_char;
    out[2] = (0x80 | ((c >> 6) & 0x3f)) as u8 as c_char;
    out[3] = (0x80 | (c & 0x3f)) as u8 as c_char;
    4
}

fn printable(c: u32) -> bool {
    if c < 0x20 || (0x7f..0xa0).contains(&c) {
        return false;
    }
    c <= 0x10_ffff && !(0xd800..=0xdfff).contains(&c)
}

pub(crate) fn luna_lm_type(m: *mut t_lmodel, codepoint: u32) -> i32 {
    if m.is_null() || !printable(codepoint) {
        return 0;
    }
    let mut enc = [0 as c_char; 4];
    let n = utf8_encode(codepoint, &mut enc);
    // SAFETY: m non nul.
    let qlen = unsafe { (*m).qlen };
    if qlen.wrapping_add(n) >= LUNA_LM_QUERY_MAX as u32 {
        return 0;
    }
    // SAFETY: qlen + n < 48, enc contient n octets.
    unsafe {
        let dst = (*m).query.as_mut_ptr().wrapping_add(qlen as usize);
        core::ptr::copy_nonoverlapping(enc.as_ptr(), dst, n as usize);
        (*m).qlen = qlen + n;
        (*m).query[(*m).qlen as usize] = 0;
    }
    lp_lm_refresh(m);
    1
}

pub(crate) fn luna_lm_backspace(m: *mut t_lmodel) -> i32 {
    if m.is_null() {
        return 0;
    }
    // SAFETY: m non nul.
    unsafe {
        if (*m).qlen == 0 {
            return 0;
        }
        (*m).qlen -= 1;
        while (*m).qlen > 0 && ((*m).query[(*m).qlen as usize] as u8 & 0xc0) == 0x80 {
            (*m).qlen -= 1;
        }
        (*m).query[(*m).qlen as usize] = 0;
    }
    lp_lm_refresh(m);
    1
}

pub(crate) fn luna_lm_clear_query(m: *mut t_lmodel) {
    if m.is_null() {
        return;
    }
    // SAFETY: m non nul.
    unsafe {
        (*m).qlen = 0;
        (*m).query[0] = 0;
    }
    lp_lm_refresh(m);
}

pub(crate) fn luna_lm_touch(m: *mut t_lmodel, id: u32) {
    if m.is_null() {
        return;
    }
    // SAFETY: m non nul. recent a LUNA_LM_RECENTS cases.
    unsafe {
        let mut i = 0u32;
        while i < (*m).nrecent && (*m).recent[i as usize] != id {
            i += 1;
        }
        if i == (*m).nrecent && (*m).nrecent < LUNA_LM_RECENTS {
            (*m).nrecent += 1;
        }
        if i == (*m).nrecent {
            i -= 1;
        }
        while i > 0 {
            (*m).recent[i as usize] = (*m).recent[(i - 1) as usize];
            i -= 1;
        }
        (*m).recent[0] = id;
    }
    lp_lm_refresh(m);
}

pub(crate) fn luna_lm_view(m: *const t_lmodel) -> i32 {
    if m.is_null() {
        return LUNA_LV_PINNED;
    }
    // SAFETY: m non nul.
    let m = unsafe { &*m };
    if m.qlen > 0 {
        return LUNA_LV_SEARCH;
    }
    m.tab
}

pub(crate) fn luna_lm_set_tab(m: *mut t_lmodel, tab: i32) {
    if m.is_null() {
        return;
    }
    // SAFETY: m non nul.
    unsafe { (*m).tab = lp_clamp(tab, LUNA_LV_PINNED, LUNA_LV_ALL) };
    lp_lm_refresh(m);
}

fn list_search(m: &mut t_lmodel) {
    let mut rank = 3i32;
    while rank >= 1 {
        let mut i = 0u32;
        while i < m.napps {
            if luna_lm_match(m.apps[i as usize].label.as_ptr(), m.query.as_ptr()) == rank {
                m.shown[m.nshown as usize] = i;
                m.nshown += 1;
            }
            i += 1;
        }
        rank -= 1;
    }
}

fn list_recent(m: &mut t_lmodel) {
    let mut r = 0u32;
    while r < m.nrecent {
        let mut i = 0u32;
        while i < m.napps && m.apps[i as usize].id != m.recent[r as usize] {
            i += 1;
        }
        if i < m.napps {
            m.shown[m.nshown as usize] = i;
            m.nshown += 1;
        }
        r += 1;
    }
}

fn list_all(m: &mut t_lmodel) {
    let mut i = 0u32;
    while i < m.napps {
        let v = i;
        let mut j = m.nshown;
        while j > 0
            && lp_lm_cmp(
                m.apps[m.shown[(j - 1) as usize] as usize].label.as_ptr(),
                m.apps[v as usize].label.as_ptr(),
            ) > 0
        {
            m.shown[j as usize] = m.shown[(j - 1) as usize];
            j -= 1;
        }
        m.shown[j as usize] = v;
        m.nshown += 1;
        i += 1;
    }
}

pub(crate) fn lp_lm_refresh(m: *mut t_lmodel) {
    // SAFETY: le C ne teste pas m. L'appelant passe un modèle vivant.
    let m = unsafe { &mut *m };
    m.nshown = 0;
    let v = if m.qlen > 0 { LUNA_LV_SEARCH } else { m.tab };
    if v == LUNA_LV_SEARCH {
        list_search(m);
    } else if v == LUNA_LV_RECENT {
        list_recent(m);
    } else if v == LUNA_LV_ALL {
        list_all(m);
    }
    m.top = 0;
    m.sel = LUNA_LM_NONE;
    if m.nshown > 0 {
        m.sel = 0;
    }
}

pub(crate) fn luna_lm_move(m: *mut t_lmodel, delta: i32) -> i32 {
    if m.is_null() {
        return LUNA_LM_NONE;
    }
    // SAFETY: m non nul.
    let m = unsafe { &mut *m };
    if m.nshown == 0 {
        return LUNA_LM_NONE;
    }
    let n = m.nshown as i32;
    if m.sel < 0 {
        m.sel = 0;
    } else {
        m.sel = lp_clamp(m.sel.wrapping_add(delta), 0, n - 1);
    }
    if m.sel < m.top {
        m.top = m.sel;
    }
    if m.sel >= m.top.wrapping_add(LUNA_LM_ROWS) {
        m.top = m.sel.wrapping_sub(LUNA_LM_ROWS).wrapping_add(1);
    }
    m.sel
}

pub(crate) fn luna_lm_scroll(m: *mut t_lmodel, delta: i32) -> i32 {
    if m.is_null() {
        return 0;
    }
    // SAFETY: m non nul.
    let m = unsafe { &mut *m };
    let max = lp_max((m.nshown as i32).wrapping_sub(LUNA_LM_ROWS), 0);
    m.top = lp_clamp(m.top.wrapping_add(delta), 0, max);
    m.top
}

pub(crate) fn luna_lm_select(m: *mut t_lmodel, row: i32) -> i32 {
    if m.is_null() {
        return LUNA_LM_NONE;
    }
    // SAFETY: m non nul.
    let m = unsafe { &mut *m };
    if !(0..LUNA_LM_ROWS).contains(&row) || m.top.wrapping_add(row) >= m.nshown as i32 {
        return LUNA_LM_NONE;
    }
    m.sel = m.top.wrapping_add(row);
    m.sel
}

pub(crate) fn luna_lm_selected(m: *const t_lmodel, id: *mut u32) -> i32 {
    if m.is_null() || id.is_null() {
        return 0;
    }
    // SAFETY: m et id non nuls.
    let m = unsafe { &*m };
    if m.sel < 0 || m.sel >= m.nshown as i32 {
        return 0;
    }
    let app = m.shown[m.sel as usize] as usize;
    // SAFETY: id non nul.
    unsafe { *id = m.apps[app].id };
    1
}

pub(crate) fn luna_lm_row(m: *const t_lmodel, row: i32) -> *const t_lmapp {
    if m.is_null() {
        return core::ptr::null();
    }
    // SAFETY: m non nul.
    let m = unsafe { &*m };
    if !(0..LUNA_LM_ROWS).contains(&row) || m.top.wrapping_add(row) >= m.nshown as i32 {
        return core::ptr::null();
    }
    let app = m.shown[m.top.wrapping_add(row) as usize] as usize;
    &m.apps[app]
}

fn layout_tiles(out: &mut t_launcherlayout, y: i32) {
    let span = out.search.w.wrapping_add(LL_GAP);
    let mut i = 0i32;
    while i < LUNA_LAUNCHER_TILES {
        out.tile[i as usize] = lp_rect(0, 0, 0, 0);
        let left = out
            .search
            .x
            .wrapping_add(i.wrapping_mul(span) / LUNA_LAUNCHER_TILES);
        let right = out
            .search
            .x
            .wrapping_add((i + 1).wrapping_mul(span) / LUNA_LAUNCHER_TILES)
            .wrapping_sub(LL_GAP);
        if i < out.ntiles {
            out.tile[i as usize] = lp_rect(left, y, lp_max(right.wrapping_sub(left), 0), LL_TILE_H);
        }
        i += 1;
    }
}

fn layout_foot(out: &mut t_launcherlayout, fy: i32) {
    let p = out.panel;
    out.avatar = lp_rect(
        p.x.wrapping_add(LL_PAD).wrapping_add(LL_FOOT_PAD),
        fy.wrapping_add((LL_FOOT_H - LL_AVATAR) / 2),
        LL_AVATAR,
        LL_AVATAR,
    );
    out.poweroff = lp_rect(
        p.x.wrapping_add(p.w)
            .wrapping_sub(LL_PAD)
            .wrapping_sub(LL_FOOT_PAD)
            .wrapping_sub(LL_FOOT_H),
        fy,
        LL_FOOT_H,
        LL_FOOT_H,
    );
    out.logoff = lp_rect(
        out.poweroff
            .x
            .wrapping_sub(LL_FOOT_PAD)
            .wrapping_sub(LL_LOGOFF_W),
        fy,
        LL_LOGOFF_W,
        LL_FOOT_H,
    );
    let name_x = out
        .avatar
        .x
        .wrapping_add(LL_AVATAR)
        .wrapping_add(LL_FOOT_PAD);
    out.user = lp_rect(
        name_x,
        fy,
        lp_max(
            out.logoff.x.wrapping_sub(LL_FOOT_PAD).wrapping_sub(name_x),
            0,
        ),
        LL_FOOT_H,
    );
}

pub(crate) fn luna_launcher_layout(panel: t_rect, ntiles: i32, out: *mut t_launcherlayout) {
    if out.is_null() {
        return;
    }
    let panel = lp_clean(panel);
    let inner = lp_max(panel.w.wrapping_sub(2 * LL_PAD), 0);
    let fy = panel.y.wrapping_add(lp_max(
        panel.h.wrapping_sub(LL_PAD).wrapping_sub(LL_FOOT_H),
        0,
    ));
    // SAFETY: out non nul.
    let out = unsafe { &mut *out };
    out.panel = panel;
    out.ntiles = lp_clamp(ntiles, 0, LUNA_LAUNCHER_TILES);
    out.search = lp_rect(
        panel.x.wrapping_add(LL_PAD),
        panel.y.wrapping_add(LL_PAD),
        inner,
        LL_SEARCH_H,
    );
    out.title = lp_rect(
        out.search.x,
        out.search.y.wrapping_add(LL_SEARCH_H).wrapping_add(LL_PAD),
        inner,
        LL_TITLE_H,
    );
    out.sep = lp_rect(out.search.x, fy.wrapping_sub(LL_FOOT_GAP), inner, 1);
    let tiles_y = out
        .title
        .y
        .wrapping_add(LL_TITLE_H)
        .wrapping_add(LL_TITLE_GAP);
    layout_tiles(out, tiles_y);
    layout_foot(out, fy);
}

pub(crate) fn luna_launcher_hit(l: *const t_launcherlayout, x: i32, y: i32) -> i32 {
    if l.is_null() {
        return LUNA_LAUNCHER_NONE;
    }
    // SAFETY: l non nul.
    let l = unsafe { &*l };
    let p = lp_pt(x, y);
    let mut i = 0i32;
    while i < l.ntiles && i < LUNA_LAUNCHER_TILES {
        if lp_in(l.tile[i as usize], p) {
            return i;
        }
        i += 1;
    }
    if lp_in(l.logoff, p) {
        return LUNA_LAUNCHER_LOGOFF;
    }
    if lp_in(l.poweroff, p) {
        return LUNA_LAUNCHER_POWER;
    }
    LUNA_LAUNCHER_NONE
}

pub(crate) fn luna_launcher_place(screen_h: i32, bar_h: i32) -> t_rect {
    let y = lp_max(
        screen_h
            .wrapping_sub(bar_h)
            .wrapping_sub(LUNA_LAUNCHER_HEIGHT),
        0,
    );
    lp_rect(
        LUNA_LAUNCHER_MARGIN_X,
        y,
        LUNA_LAUNCHER_WIDTH,
        LUNA_LAUNCHER_HEIGHT,
    )
}

pub(crate) fn luna_launcher_place_in(screen_w: i32, screen_h: i32, bar_h: i32) -> t_rect {
    let bottom = lp_max(screen_h.wrapping_sub(lp_max(bar_h, 0)), 0);
    let w = lp_clamp(
        screen_w.wrapping_sub(2 * LUNA_LAUNCHER_MARGIN_X),
        0,
        LUNA_LAUNCHER_WIDTH,
    );
    let h = lp_min(bottom, LUNA_LAUNCHER_HEIGHT);
    lp_rect(LUNA_LAUNCHER_MARGIN_X, bottom.wrapping_sub(h), w, h)
}

pub(crate) fn luna_launcher_tab_rect(l: *const t_launcherlayout, tab: i32) -> t_rect {
    if l.is_null() || !(0..=LUNA_LV_ALL).contains(&tab) {
        return lp_rect(0, 0, 0, 0);
    }
    // SAFETY: l non nul, tab dans 0..=2.
    let l = unsafe { &*l };
    let i = tab as usize;
    lp_rect(
        l.title.x.wrapping_add(G_TAB_X[i]),
        l.title.y.wrapping_sub(LL_TAB_PAD),
        lp_min(G_TAB_W[i], lp_max(l.title.w.wrapping_sub(G_TAB_X[i]), 0)),
        l.title.h.wrapping_add(2 * LL_TAB_PAD),
    )
}

pub(crate) fn luna_launcher_row_rect(l: *const t_launcherlayout, row: i32) -> t_rect {
    if l.is_null() || !(0..LUNA_LM_ROWS).contains(&row) {
        return lp_rect(0, 0, 0, 0);
    }
    // SAFETY: l non nul.
    let l = unsafe { &*l };
    let top = l
        .title
        .y
        .wrapping_add(LL_TITLE_H)
        .wrapping_add(LL_TITLE_GAP);
    lp_rect(
        l.search.x,
        top.wrapping_add(row.wrapping_mul(LL_ROW_H)),
        lp_max(
            l.search
                .w
                .wrapping_sub(LL_BAR_W)
                .wrapping_sub(LL_FOCUS_GAP * 2),
            0,
        ),
        LL_ROW_H,
    )
}

pub(crate) fn luna_launcher_tab_hit(l: *const t_launcherlayout, x: i32, y: i32) -> i32 {
    let mut i = 0i32;
    while !l.is_null() && i <= LUNA_LV_ALL {
        if lp_in(luna_launcher_tab_rect(l, i), lp_pt(x, y)) {
            return i;
        }
        i += 1;
    }
    LUNA_LM_NONE
}

pub(crate) fn luna_launcher_row_hit(l: *const t_launcherlayout, x: i32, y: i32) -> i32 {
    let mut i = 0i32;
    while !l.is_null() && i < LUNA_LM_ROWS {
        if lp_in(luna_launcher_row_rect(l, i), lp_pt(x, y)) {
            return i;
        }
        i += 1;
    }
    LUNA_LM_NONE
}

pub(crate) fn lp_launch_tone(light: t_color, dark: t_color) -> t_color {
    if theme_get() == LUNA_THEME_DARK {
        dark
    } else {
        light
    }
}

pub(crate) fn lp_launch_fill(s: *mut t_surface, r: t_rect, rad: i32, c: t_color) {
    let r = lp_clean(r);
    if !lp_ok(s) || r.w <= 0 || r.h <= 0 || (c >> 24) == 0 {
        return;
    }
    let mut st = t_lstop { at: 0, c: 0 };
    let mut g = t_lgrad {
        st: core::ptr::null(),
        n: 0,
        tint: 0,
        tint_t: 0,
    };
    lp_flat(&mut g, &mut st, c);
    let rr = t_lrr {
        r,
        rad: [rad, rad, rad, rad],
        g: &g,
        y0: r.y,
        hole: lp_rect(0, 0, 0, 0),
        vis: lp_visible(s, r),
    };
    lp_rr_paint(s, &rr);
}

fn ring_corners(s: *mut t_surface, r: t_rect, rad: i32, c: t_color) {
    let mut i = 0;
    while i < 4 {
        let dx = G_DIR[i][0];
        let dy = G_DIR[i][1];
        let k = t_lcring {
            dx,
            dy,
            x: r.x
                .wrapping_add(i32::from(dx < 0).wrapping_mul(r.w.wrapping_sub(1))),
            y: r.y
                .wrapping_add(i32::from(dy < 0).wrapping_mul(r.h.wrapping_sub(1))),
            rad,
            c,
        };
        lp_corner_ring(s, &k);
        i += 1;
    }
}

pub(crate) fn lp_launch_ring(s: *mut t_surface, r: t_rect, rad: i32, c: t_color) {
    let r = lp_clean(r);
    if !lp_ok(s) || r.w < 2 || r.h < 2 {
        return;
    }
    let rad = lp_clamp(rad, 0, lp_min(r.w, r.h) / 2);
    ring_corners(s, r, rad, c);
    lp_fill(
        s,
        lp_rect(
            r.x.wrapping_add(rad),
            r.y,
            r.w.wrapping_sub(rad.wrapping_mul(2)),
            1,
        ),
        c,
    );
    lp_fill(
        s,
        lp_rect(
            r.x.wrapping_add(rad),
            r.y.wrapping_add(r.h).wrapping_sub(1),
            r.w.wrapping_sub(rad.wrapping_mul(2)),
            1,
        ),
        c,
    );
    lp_fill(
        s,
        lp_rect(
            r.x,
            r.y.wrapping_add(rad),
            1,
            r.h.wrapping_sub(rad.wrapping_mul(2)),
        ),
        c,
    );
    lp_fill(
        s,
        lp_rect(
            r.x.wrapping_add(r.w).wrapping_sub(1),
            r.y.wrapping_add(rad),
            1,
            r.h.wrapping_sub(rad.wrapping_mul(2)),
        ),
        c,
    );
}

pub(crate) fn lp_launch_focus(s: *mut t_surface, r: t_rect, rad: i32) {
    let c = unsafe {
        // SAFETY: lp_pal vise une palette statique.
        (*crate::pal::lp_pal()).accent
    };
    lp_launch_ring(
        s,
        lp_inset(r, -(LL_FOCUS_GAP + 2)),
        rad.wrapping_add(LL_FOCUS_GAP + 2),
        c,
    );
    lp_launch_ring(
        s,
        lp_inset(r, -(LL_FOCUS_GAP + 1)),
        rad.wrapping_add(LL_FOCUS_GAP + 1),
        c,
    );
}

fn text_at(
    s: *mut t_surface,
    font: i32,
    color: t_color,
    text: *const c_char,
    r: t_rect,
    align: i32,
) {
    let t = t_ltext {
        font,
        color,
        shadow: 0,
        text,
        box_: r,
        align,
    };
    lp_text(s, &t);
}

fn tile_back(s: *mut t_surface, r: t_rect, st: i32) {
    if st == 0 {
        return;
    }
    lp_launch_fill(s, r, VT_R_TILE, lp_launch_tone(LL_HOT_L, LL_HOT_D));
    lp_fill(
        s,
        lp_rect(
            r.x.wrapping_add(VT_R_TILE),
            r.y,
            r.w.wrapping_sub(2 * VT_R_TILE),
            1,
        ),
        lp_launch_tone(LL_HOT_RIM_L, LL_HOT_RIM_D),
    );
    if (st & LL_ST_FOCUS) != 0 {
        lp_launch_focus(s, r, VT_R_TILE);
    }
}

fn tile_label(s: *mut t_surface, r: t_rect, text: *const c_char) {
    if text.is_null() {
        return;
    }
    let color = unsafe {
        // SAFETY: palette statique.
        (*crate::pal::lp_pal()).ink
    };
    text_at(
        s,
        FONT_UI_BOLD,
        color,
        text,
        lp_rect(
            r.x.wrapping_add(4),
            r.y.wrapping_add(LL_TILE_TOP)
                .wrapping_add(LL_ICON)
                .wrapping_add(8),
            r.w.wrapping_sub(8),
            LL_LABEL_H,
        ),
        LA_CENTER,
    );
}

pub(crate) fn lp_launch_tile(s: *mut t_surface, r: t_rect, t: *const t_launchertile, st: i32) {
    if t.is_null() {
        return;
    }
    // SAFETY: t non nul.
    let t = unsafe { &*t };
    tile_back(s, r, st);
    luna_icon(
        s,
        lp_pt(
            r.x.wrapping_add((r.w.wrapping_sub(LL_ICON)) / 2),
            r.y.wrapping_add(LL_TILE_TOP),
        ),
        t.icon,
        LL_ICON,
    );
    tile_label(s, r, t.label);
}

fn search_chrome(s: *mut t_surface, r: t_rect) {
    let input = unsafe {
        // SAFETY: palette statique.
        (*crate::pal::lp_pal()).input
    };
    lp_launch_fill(s, r, r.h / 2, input);
    lp_fill(
        s,
        lp_rect(r.x.wrapping_add(r.h / 2), r.y, r.w.wrapping_sub(r.h), 1),
        lp_launch_tone(LL_INPUT_RIM_L, LL_INPUT_RIM_D),
    );
    luna_icon(
        s,
        lp_pt(
            r.x.wrapping_add(LL_SEARCH_PAD),
            r.y.wrapping_add((r.h.wrapping_sub(LL_SEARCH_ICON)) / 2),
        ),
        ICON_SEARCH,
        LL_SEARCH_ICON,
    );
}

fn search_box(r: t_rect) -> t_rect {
    lp_rect(
        r.x.wrapping_add(LL_SEARCH_PAD)
            .wrapping_add(LL_SEARCH_ICON)
            .wrapping_add(LL_SEARCH_PAD),
        r.y,
        r.w.wrapping_sub(2 * LL_SEARCH_PAD)
            .wrapping_sub(LL_SEARCH_ICON)
            .wrapping_sub(LL_SEARCH_PAD_R),
        r.h,
    )
}

pub(crate) fn lp_launch_search(s: *mut t_surface, l: *const t_launcherlayout) {
    // SAFETY: l non nul, comme le C.
    let r = unsafe { (*l).search };
    search_chrome(s, r);
    let muted = unsafe {
        // SAFETY: palette statique.
        (*crate::pal::lp_pal()).muted
    };
    text_at(s, FONT_UI, muted, cstr(SEARCH_HINT), search_box(r), LA_LEFT);
}

pub(crate) fn lp_launch_title(s: *mut t_surface, l: *const t_launcherlayout) {
    // SAFETY: l non nul.
    let title = unsafe { (*l).title };
    let ink = unsafe {
        // SAFETY: palette statique.
        (*crate::pal::lp_pal()).ink
    };
    text_at(
        s,
        FONT_UI_BOLD,
        ink,
        cstr(PINNED),
        lp_rect(
            title.x.wrapping_add(LL_TITLE_PAD),
            title.y,
            title.w.wrapping_sub(2 * LL_TITLE_PAD),
            title.h,
        ),
        LA_LEFT,
    );
}

fn state_of(l: &t_launcher, id: i32) -> i32 {
    let mut st = 0;
    if l.hot == id {
        st |= LL_ST_HOT;
    }
    if l.focus == id {
        st |= LL_ST_FOCUS;
    }
    st
}

pub(crate) fn lp_launch_state(l: *const t_launcher, id: i32) -> i32 {
    // SAFETY: l non nul, comme le C.
    let l = unsafe { &*l };
    state_of(l, id)
}

pub(crate) fn lp_launch_tiles(
    s: *mut t_surface,
    l: *const t_launcher,
    lay: *const t_launcherlayout,
) {
    // SAFETY: l non nul.
    let l = unsafe { &*l };
    // SAFETY: lay non nul.
    let lay = unsafe { &*lay };
    let mut i = 0i32;
    while i < lay.ntiles && i < LUNA_LAUNCHER_TILES {
        let tile = l.tiles.wrapping_add(i as usize);
        lp_launch_tile(s, lay.tile[i as usize], tile, state_of(l, i));
        i += 1;
    }
}

fn foot_button(s: *mut t_surface, r: t_rect, st: i32) {
    let pal = unsafe {
        // SAFETY: palette statique.
        *crate::pal::lp_pal()
    };
    let mut c = pal.control;
    if st != 0 {
        c = pal.sel;
    }
    lp_launch_fill(s, r, r.h / 2, c);
    lp_fill(
        s,
        lp_rect(r.x.wrapping_add(r.h / 2), r.y, r.w.wrapping_sub(r.h), 1),
        lp_launch_tone(LL_BTN_RIM_L, LL_BTN_RIM_D),
    );
    if (st & LL_ST_FOCUS) != 0 {
        lp_launch_focus(s, r, r.h / 2);
    }
}

fn foot_user(s: *mut t_surface, l: &t_launcher, lay: &t_launcherlayout) {
    luna_icon(s, lp_pt(lay.avatar.x, lay.avatar.y), ICON_USER, LL_AVATAR);
    if l.user.is_null() {
        return;
    }
    let ink = unsafe {
        // SAFETY: palette statique.
        (*crate::pal::lp_pal()).ink
    };
    text_at(s, FONT_UI_BOLD, ink, l.user, lay.user, LA_LEFT);
}

fn foot_logoff(s: *mut t_surface, l: &t_launcher, lay: &t_launcherlayout) {
    foot_button(s, lay.logoff, state_of(l, LUNA_LAUNCHER_LOGOFF));
    let ink = unsafe {
        // SAFETY: palette statique.
        (*crate::pal::lp_pal()).ink
    };
    text_at(
        s,
        FONT_UI_BOLD,
        ink,
        cstr(LOGOFF_LABEL),
        lay.logoff,
        LA_CENTER,
    );
}

fn foot_power(s: *mut t_surface, l: &t_launcher, lay: &t_launcherlayout) {
    let r = lay.poweroff;
    foot_button(s, r, state_of(l, LUNA_LAUNCHER_POWER));
    luna_icon(
        s,
        lp_pt(
            r.x.wrapping_add((r.w.wrapping_sub(LL_POWER_ICON)) / 2),
            r.y.wrapping_add((r.h.wrapping_sub(LL_POWER_ICON)) / 2),
        ),
        ICON_POWER,
        LL_POWER_ICON,
    );
}

pub(crate) fn lp_launch_foot(
    s: *mut t_surface,
    l: *const t_launcher,
    lay: *const t_launcherlayout,
) {
    // SAFETY: l non nul.
    let l = unsafe { &*l };
    // SAFETY: lay non nul.
    let lay = unsafe { &*lay };
    lp_fill(s, lay.sep, lp_launch_tone(LL_SEP_L, LL_SEP_D));
    foot_user(s, l, lay);
    foot_logoff(s, l, lay);
    foot_power(s, l, lay);
}

pub(crate) fn luna_launcher(s: *mut t_surface, l: *const t_launcher, panel: t_rect) {
    let panel = lp_clean(panel);
    if !lp_ok(s) || l.is_null() || panel.w < LL_MIN_W || panel.h < LL_MIN_H {
        return;
    }
    // SAFETY: l non nul.
    let lr = unsafe { &*l };
    let n = if lr.tiles.is_null() { 0 } else { lr.ntiles };
    let mut lay = t_launcherlayout {
        panel: lp_rect(0, 0, 0, 0),
        search: lp_rect(0, 0, 0, 0),
        title: lp_rect(0, 0, 0, 0),
        tile: [lp_rect(0, 0, 0, 0); 4],
        sep: lp_rect(0, 0, 0, 0),
        avatar: lp_rect(0, 0, 0, 0),
        user: lp_rect(0, 0, 0, 0),
        logoff: lp_rect(0, 0, 0, 0),
        poweroff: lp_rect(0, 0, 0, 0),
        ntiles: 0,
    };
    luna_launcher_layout(panel, n, &mut lay);
    luna_glass(s, panel, LUNA_GLASS_LAUNCHER);
    lp_launch_search(s, &lay);
    lp_launch_title(s, &lay);
    lp_launch_tiles(s, l, &lay);
    lp_launch_foot(s, l, &lay);
}

fn tab_label(i: i32) -> *const c_char {
    match i {
        0 => cstr(PINNED),
        1 => cstr(RECENTS),
        _ => cstr(ALL_APPS),
    }
}

fn tab_one(s: *mut t_surface, l: &t_launcherlayout, i: i32, active: i32) {
    let r = luna_launcher_tab_rect(l, i);
    let pal = unsafe {
        // SAFETY: palette statique.
        *crate::pal::lp_pal()
    };
    let mut font = FONT_UI;
    let mut color = pal.muted;
    if i == active {
        font = FONT_UI_BOLD;
        color = pal.ink;
        lp_fill(
            s,
            lp_rect(
                r.x.wrapping_add(LL_TITLE_PAD),
                l.title.y.wrapping_add(l.title.h).wrapping_add(1),
                r.w.wrapping_sub(2 * LL_TITLE_PAD),
                2,
            ),
            pal.accent,
        );
    }
    text_at(
        s,
        font,
        color,
        tab_label(i),
        lp_rect(
            r.x.wrapping_add(LL_TITLE_PAD),
            l.title.y,
            r.w.wrapping_sub(2 * LL_TITLE_PAD),
            l.title.h,
        ),
        LA_LEFT,
    );
}

pub(crate) fn lp_launch_tabs(s: *mut t_surface, l: *const t_launcherlayout, active: i32) {
    // SAFETY: l non nul.
    let l = unsafe { &*l };
    if active > LUNA_LV_ALL {
        let ink = unsafe {
            // SAFETY: palette statique.
            (*crate::pal::lp_pal()).ink
        };
        text_at(
            s,
            FONT_UI_BOLD,
            ink,
            cstr(RESULTS),
            lp_rect(
                l.title.x.wrapping_add(LL_TITLE_PAD),
                l.title.y,
                l.title.w.wrapping_sub(2 * LL_TITLE_PAD),
                l.title.h,
            ),
            LA_LEFT,
        );
        return;
    }
    let mut i = 0i32;
    while i <= LUNA_LV_ALL {
        tab_one(s, l, i, active);
        i += 1;
    }
}

pub(crate) fn lp_launch_query(s: *mut t_surface, l: *const t_launcherlayout, query: *const c_char) {
    lp_launch_search(s, l);
    if query.is_null() || byte_at(query) == 0 {
        return;
    }
    // SAFETY: l non nul.
    let r = unsafe { (*l).search };
    search_chrome(s, r);
    let n = lp_min(lp_text_len(query), LUNA_LM_QUERY_MAX as i32) as usize;
    let mut shown = [0 as c_char; LUNA_LM_QUERY_MAX + 2];
    // SAFETY: n <= 48, query a n octets, shown en a 50.
    unsafe { core::ptr::copy_nonoverlapping(query, shown.as_mut_ptr(), n) };
    shown[n] = b'|' as c_char;
    shown[n + 1] = 0;
    let ink = unsafe {
        // SAFETY: palette statique.
        (*crate::pal::lp_pal()).ink
    };
    text_at(s, FONT_UI, ink, shown.as_ptr(), search_box(r), LA_LEFT);
}

fn vide_at(v: i32) -> *const c_char {
    match v {
        1 => cstr(VIDE_1),
        2 => cstr(VIDE_2),
        3 => cstr(VIDE_3),
        _ => cstr(VIDE_0),
    }
}

fn row_one(s: *mut t_surface, l: &t_launcherlayout, m: &t_lmodel, i: i32) {
    let a = luna_lm_row(m, i);
    if a.is_null() {
        return;
    }
    // SAFETY: a non nul, renvoyé par luna_lm_row.
    let a = unsafe { &*a };
    let r = luna_launcher_row_rect(l, i);
    if m.hot == i || m.sel == m.top.wrapping_add(i) {
        lp_launch_fill(s, r, VT_R_TILE, lp_launch_tone(LL_HOT_L, LL_HOT_D));
    }
    luna_icon(
        s,
        lp_pt(
            r.x.wrapping_add(LL_ROW_PAD),
            r.y.wrapping_add((r.h.wrapping_sub(LL_ROW_ICON)) / 2),
        ),
        a.icon,
        LL_ROW_ICON,
    );
    let ink = unsafe {
        // SAFETY: palette statique.
        (*crate::pal::lp_pal()).ink
    };
    text_at(
        s,
        FONT_UI_BOLD,
        ink,
        a.label.as_ptr(),
        lp_rect(
            r.x.wrapping_add(2 * LL_ROW_PAD).wrapping_add(LL_ROW_ICON),
            r.y,
            r.w.wrapping_sub(3 * LL_ROW_PAD).wrapping_sub(LL_ROW_ICON),
            r.h,
        ),
        LA_LEFT,
    );
}

fn rows_bar(s: *mut t_surface, l: &t_launcherlayout, m: &t_lmodel) {
    if m.nshown as i32 <= LUNA_LM_ROWS {
        return;
    }
    let first = luna_launcher_row_rect(l, 0);
    let track = LUNA_LM_ROWS * LL_ROW_H;
    let thumb = lp_max(
        track.wrapping_mul(LUNA_LM_ROWS) / m.nshown as i32,
        LL_BAR_MIN,
    );
    let span = (m.nshown as i32).wrapping_sub(LUNA_LM_ROWS);
    let x = l.search.x.wrapping_add(l.search.w).wrapping_sub(LL_BAR_W);
    lp_launch_fill(
        s,
        lp_rect(x, first.y, LL_BAR_W, track),
        1,
        lp_launch_tone(LL_SEP_L, LL_SEP_D),
    );
    let y = first
        .y
        .wrapping_add(m.top.wrapping_mul(track.wrapping_sub(thumb)) / span);
    let muted = unsafe {
        // SAFETY: palette statique.
        (*crate::pal::lp_pal()).muted
    };
    lp_launch_fill(s, lp_rect(x, y, LL_BAR_W, thumb), 1, muted);
}

pub(crate) fn lp_launch_rows(s: *mut t_surface, l: *const t_launcherlayout, m: *const t_lmodel) {
    // SAFETY: l non nul.
    let l = unsafe { &*l };
    // SAFETY: m non nul.
    let m = unsafe { &*m };
    if m.nshown == 0 {
        let mut box_ = luna_launcher_row_rect(l, 0);
        box_.x = box_.x.wrapping_add(LL_ROW_PAD);
        let muted = unsafe {
            // SAFETY: palette statique.
            (*crate::pal::lp_pal()).muted
        };
        let v = if m.qlen > 0 { LUNA_LV_SEARCH } else { m.tab };
        text_at(s, FONT_UI, muted, vide_at(v), box_, LA_LEFT);
        return;
    }
    let mut i = 0i32;
    while i < LUNA_LM_ROWS {
        row_one(s, l, m, i);
        i += 1;
    }
    rows_bar(s, l, m);
}

pub(crate) fn luna_launcher_view(
    s: *mut t_surface,
    l: *const t_launcher,
    m: *const t_lmodel,
    panel: t_rect,
) {
    if m.is_null() {
        luna_launcher(s, l, panel);
        return;
    }
    let panel = lp_clean(panel);
    if !lp_ok(s) || l.is_null() || panel.w < LL_MIN_W || panel.h < LL_MIN_H {
        return;
    }
    // SAFETY: l non nul.
    let lr = unsafe { &*l };
    // SAFETY: m non nul.
    let mr = unsafe { &*m };
    let n = if lr.tiles.is_null() { 0 } else { lr.ntiles };
    let mut lay = t_launcherlayout {
        panel: lp_rect(0, 0, 0, 0),
        search: lp_rect(0, 0, 0, 0),
        title: lp_rect(0, 0, 0, 0),
        tile: [lp_rect(0, 0, 0, 0); 4],
        sep: lp_rect(0, 0, 0, 0),
        avatar: lp_rect(0, 0, 0, 0),
        user: lp_rect(0, 0, 0, 0),
        logoff: lp_rect(0, 0, 0, 0),
        poweroff: lp_rect(0, 0, 0, 0),
        ntiles: 0,
    };
    luna_launcher_layout(panel, n, &mut lay);
    luna_glass(s, panel, LUNA_GLASS_LAUNCHER);
    lp_launch_query(s, &lay, mr.query.as_ptr());
    let v = if mr.qlen > 0 { LUNA_LV_SEARCH } else { mr.tab };
    lp_launch_tabs(s, &lay, v);
    if v == LUNA_LV_PINNED {
        lp_launch_tiles(s, l, &lay);
    } else {
        lp_launch_rows(s, &lay, m);
    }
    lp_launch_foot(s, l, &lay);
}

pub(crate) mod ffi {
    use core::ffi::c_char;

    use crate::abi::{
        t_color, t_launcher, t_launcherlayout, t_launchertile, t_lmapp, t_lmodel, t_rect, t_surface,
    };

    c_fn!(lp_launch_state(l: *const t_launcher, id: i32) -> i32 { super::lp_launch_state(l, id) });
    c_fn!(lp_launch_tiles(s: *mut t_surface, l: *const t_launcher, lay: *const t_launcherlayout) {
        super::lp_launch_tiles(s, l, lay)
    });
    c_fn!(luna_launcher(s: *mut t_surface, l: *const t_launcher, panel: t_rect) {
        super::luna_launcher(s, l, panel)
    });
    c_fn!(lp_lm_fold(out: *mut u32, max: u32, s: *const c_char) -> u32 { super::lp_lm_fold(out, max, s) });
    c_fn!(luna_lm_match(label: *const c_char, query: *const c_char) -> i32 {
        super::luna_lm_match(label, query)
    });
    c_fn!(lp_lm_cmp(a: *const c_char, b: *const c_char) -> i32 { super::lp_lm_cmp(a, b) });
    c_fn!(luna_lm_init(m: *mut t_lmodel) { super::luna_lm_init(m) });
    c_fn!(luna_lm_set_apps(m: *mut t_lmodel, apps: *const t_lmapp, n: u32) -> i32 {
        super::luna_lm_set_apps(m, apps, n)
    });
    c_fn!(luna_lm_type(m: *mut t_lmodel, codepoint: u32) -> i32 { super::luna_lm_type(m, codepoint) });
    c_fn!(luna_lm_backspace(m: *mut t_lmodel) -> i32 { super::luna_lm_backspace(m) });
    c_fn!(luna_lm_clear_query(m: *mut t_lmodel) { super::luna_lm_clear_query(m) });
    c_fn!(luna_lm_touch(m: *mut t_lmodel, id: u32) { super::luna_lm_touch(m, id) });
    c_fn!(luna_lm_view(m: *const t_lmodel) -> i32 { super::luna_lm_view(m) });
    c_fn!(luna_lm_set_tab(m: *mut t_lmodel, tab: i32) { super::luna_lm_set_tab(m, tab) });
    c_fn!(lp_lm_refresh(m: *mut t_lmodel) { super::lp_lm_refresh(m) });
    c_fn!(luna_lm_move(m: *mut t_lmodel, delta: i32) -> i32 { super::luna_lm_move(m, delta) });
    c_fn!(luna_lm_scroll(m: *mut t_lmodel, delta: i32) -> i32 { super::luna_lm_scroll(m, delta) });
    c_fn!(luna_lm_select(m: *mut t_lmodel, row: i32) -> i32 { super::luna_lm_select(m, row) });
    c_fn!(luna_lm_selected(m: *const t_lmodel, id: *mut u32) -> i32 { super::luna_lm_selected(m, id) });
    c_fn!(luna_lm_row(m: *const t_lmodel, row: i32) -> *const t_lmapp { super::luna_lm_row(m, row) });
    c_fn!(luna_launcher_layout(panel: t_rect, ntiles: i32, out: *mut t_launcherlayout) {
        super::luna_launcher_layout(panel, ntiles, out)
    });
    c_fn!(luna_launcher_hit(l: *const t_launcherlayout, x: i32, y: i32) -> i32 {
        super::luna_launcher_hit(l, x, y)
    });
    c_fn!(luna_launcher_place(screen_h: i32, bar_h: i32) -> t_rect {
        super::luna_launcher_place(screen_h, bar_h)
    });
    c_fn!(luna_launcher_place_in(screen_w: i32, screen_h: i32, bar_h: i32) -> t_rect {
        super::luna_launcher_place_in(screen_w, screen_h, bar_h)
    });
    c_fn!(luna_launcher_tab_rect(l: *const t_launcherlayout, tab: i32) -> t_rect {
        super::luna_launcher_tab_rect(l, tab)
    });
    c_fn!(luna_launcher_row_rect(l: *const t_launcherlayout, row: i32) -> t_rect {
        super::luna_launcher_row_rect(l, row)
    });
    c_fn!(luna_launcher_tab_hit(l: *const t_launcherlayout, x: i32, y: i32) -> i32 {
        super::luna_launcher_tab_hit(l, x, y)
    });
    c_fn!(luna_launcher_row_hit(l: *const t_launcherlayout, x: i32, y: i32) -> i32 {
        super::luna_launcher_row_hit(l, x, y)
    });
    c_fn!(lp_launch_fill(s: *mut t_surface, r: t_rect, rad: i32, c: t_color) {
        super::lp_launch_fill(s, r, rad, c)
    });
    c_fn!(lp_launch_ring(s: *mut t_surface, r: t_rect, rad: i32, c: t_color) {
        super::lp_launch_ring(s, r, rad, c)
    });
    c_fn!(lp_launch_focus(s: *mut t_surface, r: t_rect, rad: i32) { super::lp_launch_focus(s, r, rad) });
    c_fn!(lp_launch_tone(light: t_color, dark: t_color) -> t_color { super::lp_launch_tone(light, dark) });
    c_fn!(lp_launch_tile(s: *mut t_surface, r: t_rect, t: *const t_launchertile, st: i32) {
        super::lp_launch_tile(s, r, t, st)
    });
    c_fn!(lp_launch_search(s: *mut t_surface, l: *const t_launcherlayout) { super::lp_launch_search(s, l) });
    c_fn!(lp_launch_title(s: *mut t_surface, l: *const t_launcherlayout) { super::lp_launch_title(s, l) });
    c_fn!(lp_launch_foot(s: *mut t_surface, l: *const t_launcher, lay: *const t_launcherlayout) {
        super::lp_launch_foot(s, l, lay)
    });
    c_fn!(lp_launch_tabs(s: *mut t_surface, l: *const t_launcherlayout, active: i32) {
        super::lp_launch_tabs(s, l, active)
    });
    c_fn!(lp_launch_query(s: *mut t_surface, l: *const t_launcherlayout, query: *const c_char) {
        super::lp_launch_query(s, l, query)
    });
    c_fn!(lp_launch_rows(s: *mut t_surface, l: *const t_launcherlayout, m: *const t_lmodel) {
        super::lp_launch_rows(s, l, m)
    });
    c_fn!(luna_launcher_view(s: *mut t_surface, l: *const t_launcher, m: *const t_lmodel, panel: t_rect) {
        super::luna_launcher_view(s, l, m, panel)
    });
}
