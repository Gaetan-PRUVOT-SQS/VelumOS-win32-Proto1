use core::ffi::{c_int, c_void};

use crate::ffi::{
    AXML_ATTR_LABEL, AXML_ATTR_MIN_SDK, AXML_ATTR_NAME, AXML_ATTR_TARGET_SDK,
    AXML_ATTR_VERSION_CODE, AXML_ATTR_VERSION_NAME, AXML_END, AXML_START, RES_T_INT_DEC,
    RES_T_INT_HEX, RES_T_NULL, RES_T_REFERENCE, RES_T_STRING, arsc_open, arsc_string, axml_attr,
    axml_attr_count, axml_attr_find, axml_name, axml_next, axml_open, axml_string, t_arsc, t_axml,
    t_axmlattr, t_resquery, t_text,
};
use crate::mem::heap_free;
use crate::rd::{cstr_eq, cstr_has, cstr_len, strlcat, strlcpy, text_clean};
use crate::read::apk_read;
use crate::{
    APK_CLASS_MAX, APK_DESC_MAX, APK_ELEM_MAX, APK_LABEL_MAX, APK_PERM_LEN, APK_PERM_MAX,
    APK_PKG_MAX, APKR_ACTIVITE, APKR_LIBELLE, APKR_MANIFESTE, APKR_MEMOIRE, APKR_OK, APKR_PAQUET,
    E_INVAL, E_NOENT, E_NOMEM, E_NOTSUP, E_OK, E_OVERFLOW, E_RANGE, t_apk, t_apkmanifest, t_span,
};

struct Walk {
    x: t_axml,
    apk: *const t_apk,
    out: *mut t_apkmanifest,
    depth: u32,
    roots: u32,
    apps: u32,
    in_app: bool,
    in_act: bool,
    in_filter: bool,
    has_main: bool,
    has_launcher: bool,
    label_kind: u32,
    label_ref: u32,
    name: [u8; APK_ELEM_MAX],
    cand: [u8; APK_CLASS_MAX],
}

fn walk_blank(apk: *const t_apk, out: *mut t_apkmanifest) -> Walk {
    Walk {
        x: t_axml::zero(),
        apk,
        out,
        depth: 0,
        roots: 0,
        apps: 0,
        in_app: false,
        in_act: false,
        in_filter: false,
        has_main: false,
        has_launcher: false,
        label_kind: RES_T_NULL,
        label_ref: 0,
        name: [0; APK_ELEM_MAX],
        cand: [0; APK_CLASS_MAX],
    }
}

fn manifest<'a>(out: *mut t_apkmanifest) -> &'a mut t_apkmanifest {
    // SAFETY: out est non nul, aligné et exclusif pendant tout apk_manifest.
    unsafe { &mut *out }
}

fn narrow(n: i64) -> c_int {
    match c_int::try_from(n) {
        Ok(c) => c,
        Err(_) => E_OVERFLOW,
    }
}

fn name_char(c: u8, dollar: bool) -> bool {
    c.is_ascii_alphanumeric() || c == b'_' || (dollar && c == b'$')
}

fn name_dots(s: &[u8], dollar: bool) -> i32 {
    let n = cstr_len(s);
    let bytes = &s[..n];
    let mut dots = 0i32;
    let mut i = 0usize;
    while i < bytes.len() {
        let c = bytes[i];
        if c == b'.' {
            let edge = i == 0 || bytes[i - 1] == b'.' || i + 1 == bytes.len();
            if edge {
                return -1;
            }
            dots = match dots.checked_add(1) {
                Some(v) => v,
                None => return -1,
            };
        } else if !name_char(c, dollar) {
            return -1;
        }
        i += 1;
    }
    dots
}

fn pull(x: &t_axml, index: u32, dst: &mut [u8]) -> c_int {
    let text = t_text::from_buf(dst);
    // SAFETY: x reste lisible. dst couvre sa capacité, hors du document.
    let r = unsafe { axml_string(core::ptr::from_ref(x), index, text) };
    if r < 0 {
        return r;
    }
    if !dst.is_empty() && cstr_len(dst) == dst.len() {
        return E_INVAL;
    }
    r
}

fn man_str(x: &t_axml, res_id: u32, dst: &mut [u8]) -> c_int {
    let mut attr = t_axmlattr::zero();
    // SAFETY: x reste lisible. attr est exclusif.
    let r = unsafe {
        axml_attr_find(
            core::ptr::from_ref(x),
            res_id,
            core::ptr::from_mut(&mut attr),
        )
    };
    if r < 0 {
        return r;
    }
    if attr.r#type != RES_T_STRING {
        return E_NOTSUP;
    }
    let r = pull(x, attr.data, dst);
    if r < 0 {
        return r;
    }
    text_clean(dst)
}

fn man_u32(x: &t_axml, res_id: u32, slot: &mut u32) -> c_int {
    let mut attr = t_axmlattr::zero();
    // SAFETY: x reste lisible. attr est exclusif.
    let r = unsafe {
        axml_attr_find(
            core::ptr::from_ref(x),
            res_id,
            core::ptr::from_mut(&mut attr),
        )
    };
    if r == E_NOENT {
        return E_OK;
    }
    if r < 0 {
        return r;
    }
    if attr.r#type != RES_T_INT_DEC && attr.r#type != RES_T_INT_HEX {
        return E_NOTSUP;
    }
    *slot = attr.data;
    E_OK
}

fn man_package(x: &t_axml, dst: &mut [u8]) -> c_int {
    let mut name = [0u8; APK_ELEM_MAX];
    // SAFETY: x reste lisible.
    let n = unsafe { axml_attr_count(core::ptr::from_ref(x)) };
    let mut i = 0u32;
    while i < n {
        let mut attr = t_axmlattr::zero();
        // SAFETY: x reste lisible. attr est exclusif. i est l'indice demandé.
        let r = unsafe { axml_attr(core::ptr::from_ref(x), i, core::ptr::from_mut(&mut attr)) };
        if r < 0 {
            return E_INVAL;
        }
        if attr.res_id == 0 && attr.r#type == RES_T_STRING {
            let nr = pull(x, attr.name, &mut name);
            if nr >= 0 && cstr_eq(&name, b"package") {
                if pull(x, attr.data, dst) < 0 {
                    return E_INVAL;
                }
                return E_OK;
            }
        }
        i += 1;
    }
    E_NOENT
}

fn on_root(w: &mut Walk) -> c_int {
    let prev = w.roots;
    w.roots = w.roots.wrapping_add(1);
    if prev != 0 || !cstr_eq(&w.name, b"manifest") {
        return E_INVAL;
    }
    let x = w.x;
    let out = w.out;
    let r = man_package(&x, &mut manifest(out).package);
    if r < 0 {
        manifest(out).reason = APKR_PAQUET;
        return E_INVAL;
    }
    let r = man_u32(&x, AXML_ATTR_VERSION_CODE, &mut manifest(out).version_code);
    if r < 0 {
        return r;
    }
    let r = man_str(&x, AXML_ATTR_VERSION_NAME, &mut manifest(out).version_name);
    if r < 0 && r != E_NOENT {
        return r;
    }
    E_OK
}

fn on_sdk(w: &mut Walk) -> c_int {
    let x = w.x;
    let out = w.out;
    let r = man_u32(&x, AXML_ATTR_MIN_SDK, &mut manifest(out).min_sdk);
    if r == 0 {
        return man_u32(&x, AXML_ATTR_TARGET_SDK, &mut manifest(out).target_sdk);
    }
    r
}

fn on_perm(w: &mut Walk) -> c_int {
    let x = w.x;
    let m = manifest(w.out);
    let Ok(i) = usize::try_from(m.perm_count) else {
        return E_RANGE;
    };
    if i >= APK_PERM_MAX {
        return E_RANGE;
    }
    let r = man_str(&x, AXML_ATTR_NAME, &mut m.perms[i]);
    if r < 0 {
        return r;
    }
    m.perm_count += 1;
    E_OK
}

fn on_app(w: &mut Walk) -> c_int {
    let prev = w.apps;
    w.apps = w.apps.wrapping_add(1);
    if prev != 0 {
        return E_INVAL;
    }
    w.in_app = true;
    let x = w.x;
    let mut attr = t_axmlattr::zero();
    // SAFETY: x est une copie des champs lus par axml. attr est exclusif.
    let r = unsafe {
        axml_attr_find(
            core::ptr::from_ref(&x),
            AXML_ATTR_LABEL,
            core::ptr::from_mut(&mut attr),
        )
    };
    if r == E_NOENT {
        return E_OK;
    }
    if r < 0 {
        return r;
    }
    w.label_kind = attr.r#type;
    w.label_ref = attr.data;
    if attr.r#type == RES_T_REFERENCE {
        return E_OK;
    }
    manifest(w.out).reason = APKR_LIBELLE;
    let r = man_str(&x, AXML_ATTR_LABEL, &mut manifest(w.out).label);
    if r == 0 {
        manifest(w.out).reason = APKR_MANIFESTE;
    }
    r
}

fn on_activity(w: &mut Walk) -> c_int {
    w.in_act = true;
    w.cand[0] = 0;
    let x = w.x;
    let r = man_str(&x, AXML_ATTR_NAME, &mut w.cand);
    if r == E_NOENT {
        w.cand[0] = 0;
        return E_OK;
    }
    r
}

fn on_filter_item(w: &mut Walk) {
    let x = w.x;
    let mut v = [0u8; APK_PERM_LEN];
    if man_str(&x, AXML_ATTR_NAME, &mut v) < 0 {
        return;
    }
    if cstr_eq(&w.name, b"action") && cstr_eq(&v, b"android.intent.action.MAIN") {
        w.has_main = true;
    }
    if cstr_eq(&w.name, b"category") && cstr_eq(&v, b"android.intent.category.LAUNCHER") {
        w.has_launcher = true;
    }
}

fn man_on_child(w: &mut Walk) -> c_int {
    if w.depth == 3 && w.in_app && cstr_eq(&w.name, b"activity") {
        return on_activity(w);
    }
    if w.depth == 4 && w.in_act && cstr_eq(&w.name, b"intent-filter") {
        w.in_filter = true;
    }
    if w.depth == 5 && w.in_filter {
        on_filter_item(w);
    }
    E_OK
}

fn man_on_start(w: &mut Walk) -> c_int {
    w.depth = match w.depth.checked_add(1) {
        Some(d) => d,
        None => return E_INVAL,
    };
    if w.depth == 1 {
        return on_root(w);
    }
    if w.depth == 2 && cstr_eq(&w.name, b"uses-sdk") {
        return on_sdk(w);
    }
    if w.depth == 2 && cstr_eq(&w.name, b"uses-permission") {
        return on_perm(w);
    }
    if w.depth == 2 && cstr_eq(&w.name, b"application") {
        return on_app(w);
    }
    man_on_child(w)
}

fn on_end(w: &mut Walk) -> c_int {
    if w.depth == 0 {
        return E_INVAL;
    }
    if w.depth == 4 && w.in_filter {
        let take = w.has_main && w.has_launcher;
        let cand = w.cand;
        if take && manifest(w.out).activity[0] == 0 {
            manifest(w.out).activity.copy_from_slice(&cand);
        }
        w.in_filter = false;
        w.has_main = false;
        w.has_launcher = false;
    }
    if w.depth == 3 {
        w.in_act = false;
    }
    if w.depth == 2 {
        w.in_app = false;
    }
    w.depth = match w.depth.checked_sub(1) {
        Some(d) => d,
        None => return E_INVAL,
    };
    E_OK
}

fn man_walk(w: &mut Walk) -> c_int {
    let mut r = E_OK;
    // SAFETY: w.x est exclusif et son fichier reste lisible.
    let mut ev = unsafe { axml_next(core::ptr::from_mut(&mut w.x)) };
    while ev > 0 && r == 0 {
        if ev == AXML_START {
            let text = t_text::from_buf(&mut w.name);
            // SAFETY: w.x est lisible. text vise w.name, capacité APK_ELEM_MAX.
            let nr = unsafe { axml_name(core::ptr::from_ref(&w.x), text) };
            if nr < 0 {
                w.name[0] = 0;
            }
            r = man_on_start(w);
        } else if ev == AXML_END {
            r = on_end(w);
        }
        if r == 0 {
            // SAFETY: w.x reste exclusif et lisible.
            ev = unsafe { axml_next(core::ptr::from_mut(&mut w.x)) };
        }
    }
    if r < 0 {
        return r;
    }
    ev
}

fn class_full(m: &mut t_apkmanifest) -> c_int {
    if m.activity[0] == 0 {
        return E_NOENT;
    }
    let mut full = [0u8; APK_CLASS_MAX];
    if m.activity[0] == b'.' || !cstr_has(&m.activity, b'.') {
        let _n = strlcpy(&mut full, &m.package);
        if m.activity[0] != b'.' {
            let _n = strlcat(&mut full, b".");
        }
    }
    let n = strlcat(&mut full, &m.activity);
    if n >= APK_CLASS_MAX {
        return E_RANGE;
    }
    if name_dots(&full, true) < 1 {
        return E_INVAL;
    }
    let Some(end) = n.checked_add(1) else {
        return E_RANGE;
    };
    m.activity[..end].copy_from_slice(&full[..end]);
    E_OK
}

fn class_desc(m: &mut t_apkmanifest) -> c_int {
    let n = cstr_len(&m.activity);
    let Some(semi) = n.checked_add(1) else {
        return E_RANGE;
    };
    let Some(end) = n.checked_add(2) else {
        return E_RANGE;
    };
    if end >= APK_DESC_MAX {
        return E_RANGE;
    }
    m.activity_desc[0] = b'L';
    let mut i = 0usize;
    while i < n {
        let c = m.activity[i];
        m.activity_desc[i + 1] = if c == b'.' { b'/' } else { c };
        i += 1;
    }
    m.activity_desc[semi] = b';';
    m.activity_desc[end] = 0;
    E_OK
}

fn copy_pkg_label(m: &mut t_apkmanifest) {
    let n = cstr_len(&m.package);
    let copy = n.min(APK_LABEL_MAX - 1);
    m.label[..copy].copy_from_slice(&m.package[..copy]);
    m.label[copy] = 0;
}

fn label_resolve(w: &mut Walk) -> c_int {
    if w.label_kind == RES_T_NULL {
        copy_pkg_label(manifest(w.out));
    }
    if w.label_kind != RES_T_REFERENCE {
        return E_OK;
    }
    let mut buf: *mut u8 = core::ptr::null_mut();
    let name = c"resources.arsc";
    // SAFETY: w.apk vise l'archive ouverte. name est une chaîne C statique. buf est exclusif.
    let n = unsafe { apk_read(w.apk, name.as_ptr(), core::ptr::from_mut(&mut buf)) };
    if n < 0 {
        return narrow(n);
    }
    let Ok(len) = usize::try_from(n) else {
        heap_free(buf.cast::<c_void>());
        return E_OVERFLOW;
    };
    let file = t_span { p: buf, len };
    let mut arsc = t_arsc::zero();
    // SAFETY: buf couvre len octets initialisés. arsc est exclusif.
    let mut r = unsafe { arsc_open(core::ptr::from_mut(&mut arsc), file) };
    if r == 0 {
        let lang = c"fr";
        let q = t_resquery {
            res_id: w.label_ref,
            lang: lang.as_ptr(),
        };
        let text = t_text::from_buf(&mut manifest(w.out).label);
        // SAFETY: arsc et q restent lisibles le temps de l'appel. text vise le libellé.
        r = unsafe { arsc_string(core::ptr::from_ref(&arsc), core::ptr::from_ref(&q), text) };
    }
    heap_free(buf.cast::<c_void>());
    if r >= 0 {
        r = text_clean(&manifest(w.out).label);
    }
    r
}

fn man_finish(w: &mut Walk) -> c_int {
    {
        let m = manifest(w.out);
        m.reason = APKR_PAQUET;
        if cstr_len(&m.package) > APK_PKG_MAX - 1 || name_dots(&m.package, false) < 1 {
            return E_INVAL;
        }
        m.reason = APKR_ACTIVITE;
        let r = class_full(m);
        if r < 0 {
            return r;
        }
        if class_desc(m) < 0 {
            return E_RANGE;
        }
        m.reason = APKR_LIBELLE;
    }
    let r = label_resolve(w);
    let m = manifest(w.out);
    if r == E_NOMEM {
        m.reason = APKR_MEMOIRE;
    }
    if r < 0 {
        return r;
    }
    m.reason = APKR_OK;
    E_OK
}

fn man_result(out: &mut t_apkmanifest, r: c_int) -> c_int {
    if r >= 0 {
        return E_OK;
    }
    let mut reason = out.reason;
    if reason == APKR_OK {
        reason = APKR_MANIFESTE;
    }
    *out = t_apkmanifest::zero();
    out.reason = reason;
    r
}

/// # Safety
/// `a` est nul, ou bien aligné, et l'archive reste lisible si elle a été ouverte.
/// `out` est nul, ou bien aligné et exclusif. Un refus remet les champs à zéro
/// sauf `out.reason`. Le tampon lu est libéré avant le retour.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn apk_manifest(a: *const t_apk, out: *mut t_apkmanifest) -> c_int {
    if out.is_null() || !out.is_aligned() {
        return E_INVAL;
    }
    // SAFETY: out est non nul et aligné, exclusif le temps de l'appel.
    let out = unsafe { &mut *out };
    *out = t_apkmanifest::zero();
    out.reason = APKR_MANIFESTE;
    let mut buf: *mut u8 = core::ptr::null_mut();
    let name = c"AndroidManifest.xml";
    // SAFETY: a est nul ou aligné. name est une chaîne C statique. buf est exclusif.
    let n = unsafe { apk_read(a, name.as_ptr(), core::ptr::from_mut(&mut buf)) };
    if n == i64::from(E_NOMEM) {
        out.reason = APKR_MEMOIRE;
    }
    if n < 0 {
        return narrow(n);
    }
    let Ok(len) = usize::try_from(n) else {
        heap_free(buf.cast::<c_void>());
        return E_OVERFLOW;
    };
    let mut w = walk_blank(a, core::ptr::from_mut(out));
    let file = t_span { p: buf, len };
    // SAFETY: w.x est exclusif. buf couvre len octets jusqu'au free ci-dessous.
    let mut r = unsafe { axml_open(core::ptr::from_mut(&mut w.x), file) };
    if r == 0 {
        r = man_walk(&mut w);
    }
    if r == 0 {
        r = man_finish(&mut w);
    }
    heap_free(buf.cast::<c_void>());
    man_result(out, r)
}
