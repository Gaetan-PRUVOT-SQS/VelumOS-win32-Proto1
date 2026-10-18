use core::ffi::{c_char, c_int, c_void};

use crate::check::info_ok;
use crate::mem::{pm_alloc, pm_free};
use crate::raw::{copy_ptr, lit, nlen};
use crate::text::{index_of, path_at, pm_registry_format, pm_registry_scan};
use crate::{
    DATA_SLOTS, E_ACCES, E_EXIST, E_INVAL, E_NOENT, E_NOMEM, E_NOSPC, E_OK, E_OVERFLOW, E_PERM,
    E_RANGE, PM_ACT_MAX, PM_APK_MAX, PM_LABEL_MAX, PM_MAX_PACKAGES, PM_ORIGIN_DATA,
    PM_ORIGIN_SYSTEM, PM_PATH_MAX, PM_PKG_MAX, PM_REG_MAX, PM_REG_NAME, t_pm, t_pmentry, t_pminfo,
    t_pmreg, t_pmwork, t_span, t_text,
};

const SUF_APK: &[u8] = b".apk\0";
const SUF_TMP: &[u8] = b".apk.tmp\0";
const SUF_BAK: &[u8] = b".apk.bak\0";
const SUF_REG: &[u8] = b".tmp\0";

fn narrow(v: i64) -> c_int {
    c_int::try_from(v).unwrap_or(E_OVERFLOW)
}

fn valid(pm: *const t_pm) -> bool {
    if pm.is_null() || !pm.is_aligned() {
        return false;
    }
    // SAFETY: pm non nul et aligné. Lecture des champs pour la validation.
    let pm = unsafe { &*pm };
    if pm.sys_root.is_null() || pm.data_root.is_null() || pm.inspect.is_none() {
        return false;
    }
    if pm.fs.is_null() || !pm.fs.is_aligned() {
        return false;
    }
    // SAFETY: fs non nul et aligné, lecture seule.
    let fs = unsafe { &*pm.fs };
    if fs.read.is_none()
        || fs.write_new.is_none()
        || fs.rename.is_none()
        || fs.unlink.is_none()
        || fs.mkdir.is_none()
    {
        return false;
    }
    pm.max_packages > 0
        && pm.max_packages <= PM_MAX_PACKAGES
        && pm.max_apk > 0
        && pm.max_apk <= PM_APK_MAX
        && pm.max_registry > 0
        && pm.max_registry <= PM_REG_MAX
}

fn ctx_of(pm: *mut t_pm) -> *mut c_void {
    // SAFETY: pm validé non nul avant l'appel des callbacks.
    unsafe { (*pm).ctx }
}

fn call_read(pm: *mut t_pm, path: *const c_char, text: t_text) -> i64 {
    // SAFETY: pm non nul. On copie le pointeur de fonction avant l'appel.
    let fs = unsafe { (*pm).fs };
    if fs.is_null() || !fs.is_aligned() {
        return i64::from(E_INVAL);
    }
    // SAFETY: fs non nul et aligné.
    let Some(read) = (unsafe { (*fs).read }) else {
        return i64::from(E_INVAL);
    };
    let ctx = ctx_of(pm);
    // SAFETY: read non nul, validé par pm_valid. path est une chaîne C. text suit le contrat du lecteur.
    unsafe { read(ctx, path, text) }
}

fn call_write(pm: *mut t_pm, path: *const c_char, data: t_span) -> c_int {
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let fs = unsafe { (*pm).fs };
    if fs.is_null() || !fs.is_aligned() {
        return E_INVAL;
    }
    // SAFETY: fs non nul et aligné.
    let Some(write) = (unsafe { (*fs).write_new }) else {
        return E_INVAL;
    };
    let ctx = ctx_of(pm);
    // SAFETY: write non nul. path est une chaîne C. data décrit les octets fournis par l'appelant.
    unsafe { write(ctx, path, data) }
}

fn call_rename(pm: *mut t_pm, from: *const c_char, to: *const c_char) -> c_int {
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let fs = unsafe { (*pm).fs };
    if fs.is_null() || !fs.is_aligned() {
        return E_INVAL;
    }
    // SAFETY: fs non nul et aligné.
    let Some(rename) = (unsafe { (*fs).rename }) else {
        return E_INVAL;
    };
    let ctx = ctx_of(pm);
    // SAFETY: rename non nul. from et to sont des chaînes C.
    unsafe { rename(ctx, from, to) }
}

fn call_unlink(pm: *mut t_pm, path: *const c_char) -> c_int {
    if path.is_null() {
        return E_INVAL;
    }
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let fs = unsafe { (*pm).fs };
    if fs.is_null() || !fs.is_aligned() {
        return E_INVAL;
    }
    // SAFETY: fs non nul et aligné.
    let Some(unlink) = (unsafe { (*fs).unlink }) else {
        return E_INVAL;
    };
    let ctx = ctx_of(pm);
    // SAFETY: unlink non nul. path est une chaîne C.
    unsafe { unlink(ctx, path) }
}

fn call_mkdir(pm: *mut t_pm, path: *const c_char) -> c_int {
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let fs = unsafe { (*pm).fs };
    if fs.is_null() || !fs.is_aligned() {
        return E_INVAL;
    }
    // SAFETY: fs non nul et aligné.
    let Some(mkdir) = (unsafe { (*fs).mkdir }) else {
        return E_INVAL;
    };
    let ctx = ctx_of(pm);
    // SAFETY: mkdir non nul. path est une chaîne C.
    unsafe { mkdir(ctx, path) }
}

fn drop_path(pm: *mut t_pm, path: *const c_char) {
    let _ = call_unlink(pm, path);
}

/// # Safety
/// `pm` est nul, ou bien aligné et son `fs.unlink` est nul ou callable.
/// `path` est nul ou une chaîne C.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_drop(pm: *mut t_pm, path: *const c_char) {
    if pm.is_null() || !pm.is_aligned() {
        return;
    }
    drop_path(pm, path);
}

fn stamp(e: *mut t_pmentry, n: u32, root: *const c_char, origin: u32) {
    let mut i = 0u32;
    while i < n {
        let Ok(at) = usize::try_from(i) else {
            return;
        };
        // SAFETY: at < n, e pointe n entrées exclusives.
        unsafe { (*e.add(at)).origin = origin };
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        let name = unsafe { (*e.add(at)).info.package.as_ptr().cast() };
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        let path = unsafe { core::ptr::addr_of_mut!((*e.add(at)).path).cast() };
        let _ = path_at(path, root, name, lit(SUF_APK));
        i += 1;
    }
}

fn load_root(pm: *mut t_pm, w: *mut t_pmwork, origin: u32) -> c_int {
    let data = origin == PM_ORIGIN_DATA;
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let root = unsafe {
        if data {
            (*pm).data_root
        } else {
            (*pm).sys_root
        }
    };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let entries = unsafe {
        if data {
            core::ptr::addr_of_mut!((*w).data).cast::<t_pmentry>()
        } else {
            core::ptr::addr_of_mut!((*w).sys).cast::<t_pmentry>()
        }
    };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let reg = unsafe { core::ptr::addr_of_mut!((*w).reg).cast() };
    if path_at(reg, root, lit(PM_REG_NAME), lit(b"\0")) < 0 {
        return E_INVAL;
    }
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let Ok(cap) = usize::try_from(unsafe { (*pm).max_registry }) else {
        return E_INVAL;
    };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let buf = unsafe { core::ptr::addr_of_mut!((*w).text).cast::<c_char>() };
    let mut got = call_read(pm, reg, t_text { p: buf, cap });
    if got == i64::from(E_NOENT) {
        got = 0;
    }
    if got < 0 {
        return narrow(got);
    }
    let Ok(n) = u64::try_from(got) else {
        return E_OVERFLOW;
    };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    if n > u64::from(unsafe { (*pm).max_registry }) {
        return E_OVERFLOW;
    }
    let Ok(len) = usize::try_from(n) else {
        return E_OVERFLOW;
    };
    let text = t_span { p: buf.cast(), len };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let max = unsafe { (*pm).max_packages };
    let mut parsed = t_pmreg {
        e: entries,
        max,
        n: 0,
        rejected: 0,
    };
    // SAFETY: parsed est local. entries pointe le tableau de w, max <= PM_MAX_PACKAGES.
    let err = unsafe { pm_registry_scan(text, &mut parsed) };
    if err < 0 {
        return err;
    }
    stamp(entries, parsed.n, root, origin);
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    unsafe {
        if data {
            (*w).data_n = parsed.n;
        } else {
            (*w).sys_n = parsed.n;
        }
    }
    E_OK
}

/// # Safety
/// `pm` est nul ou aligné. `out` est nul ou aligné ; en cas de succès il reçoit un bloc de `pm_alloc`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_work_open(pm: *mut t_pm, out: *mut *mut t_pmwork) -> c_int {
    if out.is_null() || !out.is_aligned() {
        return E_INVAL;
    }
    // SAFETY: out non nul et aligné, exclusif.
    unsafe { out.write(core::ptr::null_mut()) };
    if !valid(pm) {
        return E_INVAL;
    }
    let bytes = core::mem::size_of::<t_pmwork>();
    // SAFETY: bytes est la taille de t_pmwork, non nulle.
    let p = unsafe { pm_alloc(bytes) };
    if p.is_null() {
        return E_NOMEM;
    }
    // SAFETY: p vient de pm_alloc, bytes octets exclusifs.
    unsafe { core::ptr::write_bytes(p.cast::<u8>(), 0, bytes) };
    let w = p.cast::<t_pmwork>();
    let mut err = load_root(pm, w, PM_ORIGIN_SYSTEM);
    if err == E_OK {
        err = load_root(pm, w, PM_ORIGIN_DATA);
    }
    if err < 0 {
        // SAFETY: p vient de pm_alloc et n'est plus utilisé.
        unsafe { pm_free(p) };
        return err;
    }
    // SAFETY: out non nul et aligné.
    unsafe { out.write(w) };
    E_OK
}

fn store(pm: *mut t_pm, w: *mut t_pmwork) -> c_int {
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let Ok(max_reg) = usize::try_from(unsafe { (*pm).max_registry }) else {
        return E_INVAL;
    };
    let Some(cap) = max_reg.checked_add(1) else {
        return E_OVERFLOW;
    };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let textp = unsafe { core::ptr::addr_of_mut!((*w).text).cast::<c_char>() };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let data_n = unsafe { (*w).data_n };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let data = unsafe { core::ptr::addr_of!((*w).data).cast::<t_pmentry>() };
    // SAFETY: data pointe le tableau data. textp pointe TEXT_CAP octets, cap <= TEXT_CAP.
    let n = unsafe { pm_registry_format(data, data_n, t_text { p: textp, cap }) };
    if n < 0 {
        return n;
    }
    let Ok(len) = usize::try_from(n) else {
        return E_OVERFLOW;
    };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let tmp = unsafe { core::ptr::addr_of!((*w).reg_tmp).cast::<c_char>() };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let reg = unsafe { core::ptr::addr_of!((*w).reg).cast::<c_char>() };
    drop_path(pm, tmp);
    let mut rc = call_write(
        pm,
        tmp,
        t_span {
            p: textp.cast(),
            len,
        },
    );
    if rc >= 0 {
        rc = call_rename(pm, tmp, reg);
    }
    if rc < 0 {
        drop_path(pm, tmp);
        return rc;
    }
    E_OK
}

/// # Safety
/// `pm` et `w` sont nuls ou alignés. `w` vient de `pm_work_open` et `reg_tmp` est déjà rempli.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_store(pm: *mut t_pm, w: *mut t_pmwork) -> c_int {
    if pm.is_null() || !pm.is_aligned() || w.is_null() || !w.is_aligned() {
        return E_INVAL;
    }
    store(pm, w)
}

fn copy_at(out: *mut t_pmentry, n: u32, src: t_pmentry) -> bool {
    let Ok(i) = usize::try_from(n) else {
        return false;
    };
    if out.is_null() {
        return false;
    }
    // SAFETY: out non nul, i < max fourni par l'appelant qui a réservé les places.
    unsafe { out.add(i).write(src) };
    true
}

/// # Safety
/// `pm` suit `pm_work_open`. `out` est nul seulement si `max` vaut 0, sinon `max` entrées exclusives.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_list(pm: *mut t_pm, out: *mut t_pmentry, max: u32) -> c_int {
    if out.is_null() && max != 0 {
        return E_INVAL;
    }
    if !out.is_null() && !out.is_aligned() {
        return E_INVAL;
    }
    let mut w = core::ptr::null_mut();
    // SAFETY: w est local. pm est vérifié par pm_work_open.
    let err = unsafe { pm_work_open(pm, &mut w) };
    if err < 0 {
        return err;
    }
    let mut n = 0u32;
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let sys_n = unsafe { (*w).sys_n.min(PM_MAX_PACKAGES) };
    let mut i = 0u32;
    while i < sys_n && n < max {
        let Ok(at) = usize::try_from(i) else {
            break;
        };
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        let src = unsafe { (*w).sys[at] };
        if !copy_at(out, n, src) {
            break;
        }
        n += 1;
        i += 1;
    }
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let data_n = unsafe { (*w).data_n.min(DATA_SLOTS as u32) };
    i = 0;
    while i < data_n && n < max {
        let Ok(at) = usize::try_from(i) else {
            break;
        };
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        let pkg = unsafe { (*w).data[at].info.package.as_ptr().cast() };
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        let sys = unsafe { (*w).sys.as_ptr() };
        if index_of(sys, sys_n, pkg) < 0 {
            // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
            let src = unsafe { (*w).data[at] };
            if !copy_at(out, n, src) {
                break;
            }
            n += 1;
        }
        i += 1;
    }
    // SAFETY: w vient de pm_work_open.
    unsafe { pm_free(w.cast()) };
    c_int::try_from(n).unwrap_or(E_OVERFLOW)
}

/// # Safety
/// `package` est nul ou une chaîne C. `out` est nul ou une entrée exclusive alignée.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_find(
    pm: *mut t_pm,
    package: *const c_char,
    out: *mut t_pmentry,
) -> c_int {
    if package.is_null() || out.is_null() || !out.is_aligned() {
        return E_INVAL;
    }
    let mut w = core::ptr::null_mut();
    // SAFETY: w est local. pm est vérifié par pm_work_open.
    let err = unsafe { pm_work_open(pm, &mut w) };
    if err < 0 {
        return err;
    }
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let sys_n = unsafe { (*w).sys_n.min(PM_MAX_PACKAGES) };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let mut found = index_of(unsafe { (*w).sys.as_ptr() }, sys_n, package);
    if found >= 0 {
        let Ok(at) = usize::try_from(found) else {
            // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
            unsafe { pm_free(w.cast()) };
            return E_INVAL;
        };
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        let src = unsafe { (*w).sys[at] };
        // SAFETY: out non nul et aligné.
        unsafe { out.write(src) };
    } else if found != E_INVAL {
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        let data_n = unsafe { (*w).data_n.min(DATA_SLOTS as u32) };
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        found = index_of(unsafe { (*w).data.as_ptr() }, data_n, package);
        if found >= 0 {
            let Ok(at) = usize::try_from(found) else {
                // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
                unsafe { pm_free(w.cast()) };
                return E_INVAL;
            };
            // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
            let src = unsafe { (*w).data[at] };
            // SAFETY: out non nul et aligné.
            unsafe { out.write(src) };
        }
    }
    // SAFETY: w vient de pm_work_open.
    unsafe { pm_free(w.cast()) };
    if found < 0 {
        return if found == E_INVAL { E_INVAL } else { E_NOENT };
    }
    E_OK
}

fn shift_data(w: *mut t_pmwork, at: u32, data_n: u32) -> c_int {
    let mut k = at;
    while let Some(next) = k.checked_add(1) {
        if next >= data_n {
            break;
        }
        let Ok(a) = usize::try_from(k) else {
            return E_INVAL;
        };
        let Ok(b) = usize::try_from(next) else {
            return E_INVAL;
        };
        if a >= DATA_SLOTS || b >= DATA_SLOTS {
            return E_INVAL;
        }
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        let src = unsafe { (*w).data[b] };
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe { (*w).data[a] = src };
        k = next;
    }
    let Some(left) = data_n.checked_sub(1) else {
        return E_INVAL;
    };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    unsafe { (*w).data_n = left };
    E_OK
}

fn unregister(pm: *mut t_pm, w: *mut t_pmwork, package: *const c_char) -> c_int {
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let sys_n = unsafe { (*w).sys_n.min(PM_MAX_PACKAGES) };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let sys_hit = index_of(unsafe { (*w).sys.as_ptr() }, sys_n, package);
    if sys_hit == E_INVAL {
        return E_INVAL;
    }
    if sys_hit >= 0 {
        return E_PERM;
    }
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let data_n = unsafe { (*w).data_n };
    let cap = data_n.min(DATA_SLOTS as u32);
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let i = index_of(unsafe { (*w).data.as_ptr() }, cap, package);
    if i < 0 {
        return if i == E_INVAL { E_INVAL } else { E_NOENT };
    }
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let root = unsafe { (*pm).data_root };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let reg_tmp = unsafe { core::ptr::addr_of_mut!((*w).reg_tmp).cast() };
    if path_at(reg_tmp, root, lit(PM_REG_NAME), lit(SUF_REG)) < 0 {
        return E_INVAL;
    }
    let Ok(iu) = u32::try_from(i) else {
        return E_INVAL;
    };
    let Ok(at) = usize::try_from(iu) else {
        return E_INVAL;
    };
    if at >= DATA_SLOTS {
        return E_INVAL;
    }
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let path = unsafe { (*w).data[at].path };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    unsafe { (*w).apk = path };
    let shifted = shift_data(w, iu, data_n);
    if shifted < 0 {
        return shifted;
    }
    store(pm, w)
}

/// # Safety
/// `package` est nul ou une chaîne C. `pm` suit `pm_work_open`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_remove(pm: *mut t_pm, package: *const c_char) -> c_int {
    if package.is_null() {
        return E_INVAL;
    }
    let mut w = core::ptr::null_mut();
    // SAFETY: w est local. pm est vérifié par pm_work_open.
    let err = unsafe { pm_work_open(pm, &mut w) };
    if err < 0 {
        return err;
    }
    let mut err = unregister(pm, w, package);
    if err >= 0 {
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        let path = unsafe { core::ptr::addr_of!((*w).apk).cast() };
        err = call_unlink(pm, path);
        if err == E_NOENT {
            err = E_OK;
        }
    }
    // SAFETY: w vient de pm_work_open.
    unsafe { pm_free(w.cast()) };
    if err < 0 { err } else { E_OK }
}

fn examine(pm: *mut t_pm, w: *mut t_pmwork, apk: t_span) -> c_int {
    if apk.p.is_null() || apk.len == 0 {
        return E_INVAL;
    }
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let max = unsafe { (*pm).max_apk };
    let Ok(len) = u32::try_from(apk.len) else {
        return E_RANGE;
    };
    if len > max {
        return E_RANGE;
    }
    let mut raw = t_pminfo::zero();
    // SAFETY: pm validé, inspect est Some.
    let Some(inspect) = (unsafe { (*pm).inspect }) else {
        return E_INVAL;
    };
    // SAFETY: raw est local et exclusif. apk est celui de l'appelant.
    let err = unsafe { inspect(apk, &mut raw) };
    if err < 0 {
        return err;
    }
    if !info_ok(&raw) {
        return E_INVAL;
    }
    let mut info = t_pminfo::zero();
    let pn = nlen(raw.package.as_ptr().cast(), PM_PKG_MAX);
    let ln = nlen(raw.label.as_ptr().cast(), PM_LABEL_MAX);
    let an = nlen(raw.activity.as_ptr().cast(), PM_ACT_MAX);
    copy_ptr(&mut info.package, raw.package.as_ptr(), pn);
    copy_ptr(&mut info.label, raw.label.as_ptr(), ln);
    copy_ptr(&mut info.activity, raw.activity.as_ptr(), an);
    info.cert = raw.cert;
    info.version_code = raw.version_code;
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    unsafe { (*w).info = info };
    E_OK
}

fn check_rules(pm: *mut t_pm, w: *mut t_pmwork) -> c_int {
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let pkg = unsafe { (*w).info.package.as_ptr().cast::<c_char>() };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let sys_n = unsafe { (*w).sys_n.min(PM_MAX_PACKAGES) };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let data_n = unsafe { (*w).data_n.min(DATA_SLOTS as u32) };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let max = unsafe { (*pm).max_packages };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let sys_i = index_of(unsafe { (*w).sys.as_ptr() }, sys_n, pkg);
    if sys_i == E_INVAL {
        return E_INVAL;
    }
    if sys_i >= 0 {
        return E_PERM;
    }
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let i = index_of(unsafe { (*w).data.as_ptr() }, data_n, pkg);
    if i == E_INVAL {
        return E_INVAL;
    }
    let fresh = i < 0;
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    unsafe {
        (*w).fresh = u32::from(fresh);
        (*w).slot = data_n;
    }
    if fresh {
        let Some(used) = sys_n.checked_add(data_n) else {
            return E_NOSPC;
        };
        if used >= max {
            return E_NOSPC;
        }
        return E_OK;
    }
    let Ok(iu) = u32::try_from(i) else {
        return E_INVAL;
    };
    let Ok(at) = usize::try_from(iu) else {
        return E_INVAL;
    };
    if at >= DATA_SLOTS {
        return E_INVAL;
    }
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    unsafe { (*w).slot = iu };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let old = unsafe { (*w).data[at].info };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let info = unsafe { (*w).info };
    if old.package != info.package {
        return E_EXIST;
    }
    if old.cert != info.cert {
        return E_ACCES;
    }
    if info.version_code < old.version_code {
        return E_INVAL;
    }
    E_OK
}

fn fill_entry(w: *mut t_pmwork) -> c_int {
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let slot = unsafe { (*w).slot };
    let Ok(i) = usize::try_from(slot) else {
        return E_INVAL;
    };
    if i >= DATA_SLOTS {
        return E_INVAL;
    }
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let apk = unsafe { (*w).apk };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let info = unsafe { (*w).info };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    unsafe {
        (*w).data[i] = t_pmentry::zero();
        (*w).data[i].info = info;
        (*w).data[i].origin = PM_ORIGIN_DATA;
        (*w).data[i].path = apk;
    }
    E_OK
}

fn publish_fresh(pm: *mut t_pm, w: *mut t_pmwork) -> c_int {
    if fill_entry(w) < 0 {
        return E_INVAL;
    }
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let n = unsafe { (*w).data_n };
    let Some(next) = n.checked_add(1) else {
        return E_OVERFLOW;
    };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    unsafe { (*w).data_n = next };
    let err = store(pm, w);
    if err < 0 {
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        let path = unsafe { core::ptr::addr_of!((*w).apk).cast() };
        drop_path(pm, path);
    }
    err
}

fn copy_out(pm: *mut t_pm, src: *const c_char, dst: *const c_char) -> c_int {
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let Ok(max_apk) = usize::try_from(unsafe { (*pm).max_apk }) else {
        return E_INVAL;
    };
    let mut cap = 64usize;
    if cap > max_apk {
        cap = max_apk;
    }
    let mut buf: *mut u8 = core::ptr::null_mut();
    let mut n = i64::from(E_OVERFLOW);
    while n == i64::from(E_OVERFLOW) {
        // SAFETY: buf est nul ou vient de pm_alloc dans cette boucle.
        unsafe { pm_free(buf.cast()) };
        // SAFETY: cap est borné par max_apk.
        buf = unsafe { pm_alloc(cap).cast() };
        if buf.is_null() {
            return E_NOMEM;
        }
        n = call_read(pm, src, t_text { p: buf.cast(), cap });
        if n != i64::from(E_OVERFLOW) || cap >= max_apk {
            break;
        }
        if cap > max_apk / 2 {
            cap = max_apk;
        } else {
            cap = cap.saturating_mul(2);
        }
    }
    if n < 0 {
        // SAFETY: buf vient de pm_alloc.
        unsafe { pm_free(buf.cast()) };
        return narrow(n);
    }
    let Ok(len) = usize::try_from(n) else {
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe { pm_free(buf.cast()) };
        return E_OVERFLOW;
    };
    if len > cap {
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        unsafe { pm_free(buf.cast()) };
        return E_OVERFLOW;
    }
    let err = call_write(pm, dst, t_span { p: buf, len });
    // SAFETY: buf vient de pm_alloc.
    unsafe { pm_free(buf.cast()) };
    err
}

fn publish_update(pm: *mut t_pm, w: *mut t_pmwork) -> c_int {
    let mut bak = [0u8; PM_PATH_MAX];
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let root = unsafe { (*pm).data_root };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let pkg = unsafe { (*w).info.package.as_ptr().cast() };
    if path_at(bak.as_mut_ptr().cast(), root, pkg, lit(SUF_BAK)) < 0 {
        return E_INVAL;
    }
    if fill_entry(w) < 0 {
        return E_INVAL;
    }
    drop_path(pm, bak.as_ptr().cast());
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let apk = unsafe { core::ptr::addr_of!((*w).apk).cast::<c_char>() };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let tmp = unsafe { core::ptr::addr_of!((*w).apk_tmp).cast::<c_char>() };
    let err = copy_out(pm, apk, bak.as_ptr().cast());
    if err < 0 {
        drop_path(pm, bak.as_ptr().cast());
        return err;
    }
    let err = call_rename(pm, tmp, apk);
    if err < 0 {
        drop_path(pm, bak.as_ptr().cast());
        return err;
    }
    let err = store(pm, w);
    if err < 0 {
        let back = call_rename(pm, bak.as_ptr().cast(), apk);
        if back < 0 {
            return back;
        }
        return err;
    }
    drop_path(pm, bak.as_ptr().cast());
    E_OK
}

fn commit(pm: *mut t_pm, w: *mut t_pmwork, apk: t_span) -> c_int {
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let root = unsafe { (*pm).data_root };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let pkg = unsafe { (*w).info.package.as_ptr().cast() };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let apk_dst = unsafe { core::ptr::addr_of_mut!((*w).apk).cast() };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let tmp = unsafe { core::ptr::addr_of_mut!((*w).apk_tmp).cast() };
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let reg_tmp = unsafe { core::ptr::addr_of_mut!((*w).reg_tmp).cast() };
    if path_at(apk_dst, root, pkg, lit(SUF_APK)) < 0
        || path_at(tmp, root, pkg, lit(SUF_TMP)) < 0
        || path_at(reg_tmp, root, lit(PM_REG_NAME), lit(SUF_REG)) < 0
    {
        return E_INVAL;
    }
    let made = call_mkdir(pm, root);
    if made < 0 && made != E_EXIST {
        return made;
    }
    drop_path(pm, tmp);
    let mut err = call_write(pm, tmp, apk);
    // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
    let fresh = unsafe { (*w).fresh } != 0;
    if err >= 0 && fresh {
        err = call_rename(pm, tmp, apk_dst);
    }
    if err >= 0 {
        err = if fresh {
            publish_fresh(pm, w)
        } else {
            publish_update(pm, w)
        };
    }
    if err < 0 {
        drop_path(pm, tmp);
    }
    err
}

/// # Safety
/// `pm` suit `pm_work_open`. `apk.p` est nul ou pointe `apk.len` octets.
/// `out` est nul, ou bien aligné et exclusif.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn pm_install(pm: *mut t_pm, apk: t_span, out: *mut t_pmentry) -> c_int {
    if !out.is_null() && !out.is_aligned() {
        return E_INVAL;
    }
    let mut w = core::ptr::null_mut();
    // SAFETY: w est local. pm est vérifié par pm_work_open.
    let err = unsafe { pm_work_open(pm, &mut w) };
    if err < 0 {
        return err;
    }
    let mut err = examine(pm, w, apk);
    if err >= 0 {
        err = check_rules(pm, w);
    }
    if err >= 0 {
        err = commit(pm, w, apk);
    }
    if err >= 0 && !out.is_null() {
        // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
        let slot = unsafe { (*w).slot };
        if let Ok(i) = usize::try_from(slot)
            && i < DATA_SLOTS
        {
            // SAFETY: Pointeur contrôlé avant cet accès, borné et exclusif le temps de l'appel.
            let src = unsafe { (*w).data[i] };
            // SAFETY: out non nul et aligné.
            unsafe { out.write(src) };
        }
    }
    // SAFETY: w vient de pm_work_open.
    unsafe { pm_free(w.cast()) };
    if err < 0 { err } else { E_OK }
}
