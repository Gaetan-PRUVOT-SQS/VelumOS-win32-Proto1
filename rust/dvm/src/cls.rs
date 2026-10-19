use core::ffi::{c_char, c_int};

use crate::heap::{
    cstr, dvm_new_inner, dvm_pin_inner, dvm_unpin_inner, rt_alloc, rt_obj, rt_words,
};
use crate::mem::{c_calloc, c_free, c_strcmp, c_strlen};
use crate::{
    ACC_ABSTRACT, ACC_FINAL, ACC_INTERFACE, ACC_PUBLIC, DCLASSES_HARD, DCORE_CHARSEQ, DCORE_CLASS,
    DCORE_N, DCORE_OBJECT, DCORE_STRING, DCORE_THROWABLE, DDESC_MAX, DJOIN_MAX, DMSG_MAX, DOK_ANY,
    DOK_ARRAY, DOK_OBJECT, DOK_STRING, DPAYLOAD_MAX, DSTACK_HARD, DSTR_MAX, DVM_CLASSES_DEFAULT,
    DVM_DEPTH_DEFAULT, DVM_HEAP_DEFAULT, DVM_NULL, DVM_STACK_WORDS_DEFAULT, DVM_THROWN, E_EXIST,
    E_INVAL, E_NOENT, E_NOMEM, E_OVERFLOW, E_RANGE, t_darrview, t_dbuiltin, t_dclass, t_dlimits,
    t_dnewobj, t_dref, t_dstr16, t_dvm, t_text,
};

pub(crate) unsafe fn rt_find(vm: *const t_dvm, desc: *const c_char) -> *mut t_dclass {
    // SAFETY: vm non nul quand desc est lu. classes vient de dvm_create.
    unsafe {
        let mut i = 0u32;
        let n = (*(*vm).classes).n;
        while !desc.is_null() && i < n {
            let c = *(*(*vm).classes).tab.add(i as usize);
            if c_strcmp((*c).desc, desc) == 0 {
                return c;
            }
            i += 1;
        }
        core::ptr::null_mut()
    }
}

pub(crate) unsafe fn rt_dup(s: *const c_char) -> *mut c_char {
    if s.is_null() {
        return core::ptr::null_mut();
    }
    // SAFETY: s est une chaîne C.
    unsafe {
        let mut n = c_strlen(s) + 1;
        let d = c_calloc(1, n).cast::<c_char>();
        while !d.is_null() && n > 0 {
            n -= 1;
            *d.add(n) = *s.add(n);
        }
        d
    }
}

unsafe fn rt_parents(vm: *const t_dvm, b: *const t_dbuiltin, c: *mut t_dclass) -> c_int {
    // SAFETY: b et c non nuls, vm a ses classes.
    unsafe {
        (*c).super_ = rt_find(vm, (*b).super_desc);
        (*c).iface = rt_find(vm, (*b).iface_desc);
        if (!(*b).super_desc.is_null() && (*c).super_.is_null())
            || (!(*b).iface_desc.is_null() && (*c).iface.is_null())
        {
            return E_NOENT;
        }
        if !(*c).super_.is_null() && ((*(*c).super_).access & (ACC_INTERFACE | ACC_FINAL)) != 0 {
            return E_INVAL;
        }
        if !(*c).iface.is_null() && ((*(*c).iface).access & ACC_INTERFACE) == 0 {
            return E_INVAL;
        }
        (*c).access = (*b).access;
        (*c).ifaces = core::ptr::addr_of_mut!((*c).iface);
        (*c).nifaces = u32::from(!(*c).iface.is_null());
        if !(*c).super_.is_null() {
            (*c).words = (*(*c).super_).words;
            (*c).pay_off = (*(*c).super_).pay_end;
        }
        let align = (*b).payload_bytes.wrapping_add(7) & !7;
        (*c).pay_end = (*c).pay_off.wrapping_add(align);
        0
    }
}

pub(crate) unsafe fn rt_define(
    vm: *mut t_dvm,
    b: *const t_dbuiltin,
    out: *mut *mut t_dclass,
) -> c_int {
    if b.is_null()
        // SAFETY: b non nul, le test précédent a court-circuité.
        || unsafe { (*b).desc.is_null() || (*b).payload_bytes > DPAYLOAD_MAX }
    {
        return E_INVAL;
    }
    // SAFETY: b non nul, desc non nul.
    unsafe {
        if c_strlen((*b).desc) > DDESC_MAX {
            return E_INVAL;
        }
        if !rt_find(vm, (*b).desc).is_null() {
            return E_EXIST;
        }
        if (*(*vm).classes).n >= (*(*vm).classes).max {
            return E_RANGE;
        }
        let c = c_calloc(1, core::mem::size_of::<t_dclass>()).cast::<t_dclass>();
        if c.is_null() {
            return E_NOMEM;
        }
        let mut r = rt_parents(vm, b, c);
        if r == 0 {
            (*c).desc = rt_dup((*b).desc);
        }
        if r == 0 && (*c).desc.is_null() {
            r = E_NOMEM;
        }
        if r != 0 {
            c_free(c.cast());
            return r;
        }
        let n = (*(*vm).classes).n;
        *(*(*vm).classes).tab.add(n as usize) = c;
        (*(*vm).classes).n = n + 1;
        *out = c;
        0
    }
}

pub(crate) unsafe fn rt_oom_retype(vm: *mut t_dvm) {
    // SAFETY: vm vient d'un define réussi, classes et tas existent.
    unsafe {
        let c = rt_find(vm, cstr(b"Ljava/lang/OutOfMemoryError;\0"));
        let base = (*(*vm).classes).core[DCORE_THROWABLE];
        let o = rt_obj(vm, (*(*vm).heap).oom, DOK_OBJECT);
        if c.is_null() || o.is_null() || (*o).cls != base || rt_assignable(c, base) == 0 {
            return;
        }
        if (*c).words != (*base).words || (*c).pay_end != (*base).pay_end {
            return;
        }
        (*o).cls = c;
    }
}

pub(crate) unsafe fn dvm_builtins_inner(vm: *mut t_dvm, tab: *const t_dbuiltin, n: u32) -> c_int {
    if vm.is_null() || (tab.is_null() && n != 0) {
        return E_INVAL;
    }
    // SAFETY: vm non nul. tab est nul seulement si n vaut 0.
    unsafe {
        let mut i = 0u32;
        while i < n {
            let b = tab.add(i as usize);
            if (*b).desc.is_null() || *(*b).desc != b'L' as c_char {
                return E_INVAL;
            }
            let mut c = core::ptr::null_mut();
            let r = rt_define(vm, b, &mut c);
            if r != 0 {
                return r;
            }
            i += 1;
        }
        rt_oom_retype(vm);
        0
    }
}

unsafe fn rt_implements(mut c: *const t_dclass, to: *const t_dclass) -> i32 {
    // SAFETY: c est nul ou une classe. La chaîne super est acyclique par construction.
    unsafe {
        while !c.is_null() {
            let mut i = 0u32;
            while i < (*c).nifaces {
                let iface = *(*c).ifaces.add(i as usize);
                if iface == to.cast_mut() || rt_implements(iface, to) != 0 {
                    return 1;
                }
                i += 1;
            }
            c = (*c).super_;
        }
        0
    }
}

pub(crate) unsafe fn rt_assignable(mut from: *const t_dclass, to: *const t_dclass) -> i32 {
    if from.is_null() || to.is_null() {
        return 0;
    }
    if from == to {
        return 1;
    }
    // SAFETY: from et to non nuls.
    unsafe {
        if (*from).is_array != 0 && (*to).is_array != 0 {
            if (*from).is_ref != 0 && (*to).is_ref != 0 {
                return rt_assignable((*from).elem, (*to).elem);
            }
            return 0;
        }
        if rt_implements(from, to) != 0 {
            return 1;
        }
        from = (*from).super_;
        while !from.is_null() && from != to {
            from = (*from).super_;
        }
        i32::from(!from.is_null())
    }
}

pub(crate) unsafe fn dvm_class_name_inner(c: *const t_dclass) -> *const c_char {
    if c.is_null() {
        return core::ptr::null();
    }
    // SAFETY: c non nul.
    unsafe { (*c).desc }
}

pub(crate) unsafe fn dvm_is_instance_inner(vm: *mut t_dvm, r: t_dref, c: *const t_dclass) -> i32 {
    // SAFETY: rt_obj vérifie vm.
    unsafe {
        let o = rt_obj(vm, r, DOK_ANY);
        if o.is_null() || c.is_null() {
            return 0;
        }
        rt_assignable((*o).cls, c)
    }
}

fn prim_width(c: u8) -> u8 {
    if c == b'Z' || c == b'B' {
        return 1;
    }
    if c == b'C' || c == b'S' {
        return 2;
    }
    if c == b'I' || c == b'F' {
        return 4;
    }
    if c == b'J' || c == b'D' {
        return 8;
    }
    0
}

unsafe fn rt_elem(vm: *mut t_dvm, e: *const c_char, cls: *mut *mut t_dclass, w: *mut u8) -> c_int {
    // SAFETY: e pointe au moins un octet (le descripteur est non vide ici).
    unsafe {
        *cls = core::ptr::null_mut();
        *w = prim_width(*e as u8);
        if *w != 0 && *e.add(1) == 0 {
            return 0;
        }
        *w = 4;
        if *e == b'L' as c_char || *e == b'[' as c_char {
            return dvm_class_inner(vm, e, cls);
        }
        E_INVAL
    }
}

pub(crate) unsafe fn rt_array_class(
    vm: *mut t_dvm,
    desc: *const c_char,
    out: *mut *mut t_dclass,
) -> c_int {
    // SAFETY: desc non nul, vérifié par l'appelant ou par dvm_class.
    unsafe {
        *out = core::ptr::null_mut();
        if *desc != b'[' as c_char || c_strlen(desc) > DDESC_MAX {
            return E_INVAL;
        }
        let mut elem = core::ptr::null_mut();
        let mut width = 0u8;
        let r = rt_elem(vm, desc.add(1), &mut elem, &mut width);
        if r != 0 {
            return r;
        }
        let b = t_dbuiltin {
            desc,
            super_desc: cstr(b"Ljava/lang/Object;\0"),
            iface_desc: core::ptr::null(),
            access: ACC_PUBLIC | ACC_FINAL,
            payload_bytes: 0,
        };
        let r = rt_define(vm, &b, out);
        if r != 0 {
            return r;
        }
        (**out).elem = elem;
        (**out).width = width;
        (**out).is_ref = u8::from(!elem.is_null());
        (**out).is_array = 1;
        0
    }
}

pub(crate) unsafe fn dvm_class_inner(
    vm: *mut t_dvm,
    desc: *const c_char,
    out: *mut *mut t_dclass,
) -> c_int {
    if vm.is_null() || desc.is_null() || out.is_null() {
        return E_INVAL;
    }
    // SAFETY: les trois pointeurs sont non nuls.
    unsafe {
        *out = rt_find(vm, desc);
        if !(*out).is_null() {
            return 0;
        }
        if *desc == b'[' as c_char {
            return rt_array_class(vm, desc, out);
        }
        crate::dexrt::rt_link(vm, desc, out)
    }
}

pub(crate) unsafe fn dvm_array_new_inner(
    vm: *mut t_dvm,
    desc: *const c_char,
    len: i32,
    out: *mut t_dref,
) -> c_int {
    if vm.is_null() || desc.is_null() || out.is_null()
        // SAFETY: desc non nul, les tests précédents ont court-circuité.
        || unsafe { *desc != b'[' as c_char }
    {
        return E_INVAL;
    }
    // SAFETY: pointeurs non nuls, desc commence par '['.
    unsafe {
        *out = DVM_NULL;
        let mut cls = core::ptr::null_mut();
        let r = dvm_class_inner(vm, desc, &mut cls);
        if r != 0 {
            return r;
        }
        if len < 0 {
            return crate::bridge::go_throw(
                vm,
                cstr(b"Ljava/lang/NegativeArraySizeException;\0"),
                core::ptr::null(),
            );
        }
        let rq = t_dnewobj {
            cls,
            len: len as u32,
            bytes: u64::from(len as u32).wrapping_mul(u64::from((*cls).width)),
            kind: DOK_ARRAY,
        };
        rt_alloc(vm, &rq, out)
    }
}

pub(crate) unsafe fn dvm_array_view_inner(
    vm: *mut t_dvm,
    r: t_dref,
    out: *mut t_darrview,
) -> c_int {
    // SAFETY: rt_obj vérifie vm.
    unsafe {
        let o = rt_obj(vm, r, DOK_ARRAY);
        if o.is_null() || out.is_null() {
            return E_INVAL;
        }
        *out = t_darrview {
            data: (*o).data,
            len: (*o).len,
            width: (*(*o).cls).width,
            is_ref: (*(*o).cls).is_ref,
        };
        0
    }
}

unsafe fn rt_utf8_cp(p: *mut *const u8, cp: *mut u32) -> c_int {
    // SAFETY: *p pointe une chaîne C encore non vide.
    unsafe {
        let s = *p;
        if *s >= 0xf8 || (*s & 0xc0) == 0x80 {
            return E_INVAL;
        }
        let n = u32::from(*s >= 0xc0) + u32::from(*s >= 0xe0) + u32::from(*s >= 0xf0);
        *cp = u32::from(*s) & (0xffu32 >> (n + 1 + u32::from(n > 0)));
        let mut i = 1u32;
        while i <= n {
            if (*s.add(i as usize) & 0xc0) != 0x80 {
                return E_INVAL;
            }
            *cp = (*cp << 6) | u32::from(*s.add(i as usize) & 0x3f);
            i += 1;
        }
        let mut low = 0u32;
        if n > 0 {
            low = 0x80u32 << (4 * (n - 1) + u32::from(n == 3));
        }
        if *cp < low || *cp > 0x10_ffff || (*cp >= 0xd800 && *cp < 0xe000) {
            return E_INVAL;
        }
        *p = s.add(n as usize + 1);
        0
    }
}

pub(crate) unsafe fn rt_utf8_to16(utf8: *const c_char, out: *mut u16) -> c_int {
    // SAFETY: utf8 est une chaîne C. out est nul ou assez grand pour le résultat.
    unsafe {
        let mut p = utf8.cast::<u8>();
        let mut n = 0u32;
        while *p != 0 {
            let mut cp = 0u32;
            if rt_utf8_cp(&mut p, &mut cp) != 0 {
                return E_INVAL;
            }
            if cp >= 0x1_0000 && !out.is_null() {
                *out.add(n as usize) = (0xd800 + ((cp - 0x1_0000) >> 10)) as u16;
                *out.add(n as usize + 1) = (0xdc00 + ((cp - 0x1_0000) & 0x3ff)) as u16;
            } else if !out.is_null() {
                *out.add(n as usize) = cp as u16;
            }
            n += 1 + u32::from(cp >= 0x1_0000);
            if n > DSTR_MAX {
                return E_RANGE;
            }
        }
        n as c_int
    }
}

unsafe fn rt_cp16(s: t_dstr16, i: *mut u32) -> u32 {
    // SAFETY: i < s.n, s.p vise s.n unités.
    unsafe {
        let c = u32::from(*s.p.add(*i as usize));
        *i += 1;
        if !(0xd800..0xe000).contains(&c) {
            return c;
        }
        if c >= 0xdc00 || *i >= s.n {
            return 0xfffd;
        }
        let d = u32::from(*s.p.add(*i as usize));
        if !(0xdc00..0xe000).contains(&d) {
            return 0xfffd;
        }
        *i += 1;
        0x1_0000 + ((c - 0xd800) << 10) + (d - 0xdc00)
    }
}

unsafe fn rt_put8(mut cp: u32, b: *mut u8) -> u32 {
    // SAFETY: b vise 4 octets.
    unsafe {
        let n = 1 + u32::from(cp >= 0x80) + u32::from(cp >= 0x800) + u32::from(cp >= 0x1_0000);
        if n == 1 {
            *b = cp as u8;
            return 1;
        }
        let mut i = n;
        while {
            i -= 1;
            i > 0
        } {
            *b.add(i as usize) = (0x80 | (cp & 0x3f)) as u8;
            cp >>= 6;
        }
        *b = ((0xf00u32 >> n) as u8) | (cp as u8);
        n
    }
}

pub(crate) unsafe fn rt_utf16_to8(s: t_dstr16, out: t_text) -> c_int {
    // SAFETY: s.p est nul seulement si s.n vaut 0. out.p est nul ou a out.cap octets.
    unsafe {
        let mut b = [0u8; 4];
        let mut i = 0u32;
        let mut n = 0usize;
        while i < s.n {
            let k = rt_put8(rt_cp16(s, &mut i), b.as_mut_ptr()) as usize;
            if out.p.is_null() || n + k >= out.cap {
                return E_OVERFLOW;
            }
            let mut j = 0usize;
            while j < k {
                *out.p.add(n) = b[j] as c_char;
                n += 1;
                j += 1;
            }
        }
        if out.p.is_null() || n >= out.cap {
            return E_OVERFLOW;
        }
        *out.p.add(n) = 0;
        n as c_int
    }
}

unsafe fn rt_str_alloc(vm: *mut t_dvm, n: u32, out: *mut t_dref) -> c_int {
    if n > DSTR_MAX {
        return E_RANGE;
    }
    // SAFETY: vm a le cœur String.
    unsafe {
        let rq = t_dnewobj {
            cls: (*(*vm).classes).core[DCORE_STRING],
            len: n,
            bytes: u64::from(n) * 2,
            kind: DOK_STRING,
        };
        rt_alloc(vm, &rq, out)
    }
}

pub(crate) unsafe fn dvm_string_utf16_new_inner(
    vm: *mut t_dvm,
    s: t_dstr16,
    out: *mut t_dref,
) -> c_int {
    if vm.is_null() || out.is_null() || (s.p.is_null() && s.n != 0) {
        return E_INVAL;
    }
    // SAFETY: pointeurs vérifiés.
    unsafe {
        let r = rt_str_alloc(vm, s.n, out);
        if r != 0 {
            return r;
        }
        let d = (*rt_obj(vm, *out, DOK_STRING)).data.cast::<u16>();
        let mut i = 0u32;
        while i < s.n {
            *d.add(i as usize) = *s.p.add(i as usize);
            i += 1;
        }
        0
    }
}

pub(crate) unsafe fn dvm_string_utf8_new_inner(
    vm: *mut t_dvm,
    utf8: *const c_char,
    out: *mut t_dref,
) -> c_int {
    if vm.is_null() || utf8.is_null() || out.is_null() {
        return E_INVAL;
    }
    // SAFETY: utf8 est une chaîne C, out non nul.
    unsafe {
        *out = DVM_NULL;
        let n = rt_utf8_to16(utf8, core::ptr::null_mut());
        if n < 0 {
            return n;
        }
        let r = rt_str_alloc(vm, n as u32, out);
        if r != 0 {
            return r;
        }
        rt_utf8_to16(utf8, (*rt_obj(vm, *out, DOK_STRING)).data.cast());
        0
    }
}

pub(crate) unsafe fn dvm_string_get_inner(vm: *mut t_dvm, r: t_dref, out: *mut t_dstr16) -> c_int {
    // SAFETY: rt_obj vérifie vm.
    unsafe {
        let o = rt_obj(vm, r, DOK_STRING);
        if o.is_null() || out.is_null() {
            return E_INVAL;
        }
        *out = t_dstr16 {
            p: (*o).data.cast(),
            n: (*o).len,
        };
        0
    }
}

pub(crate) unsafe fn dvm_string_utf8_inner(vm: *mut t_dvm, r: t_dref, out: t_text) -> c_int {
    let mut s = t_dstr16 {
        p: core::ptr::null(),
        n: 0,
    };
    // SAFETY: même contrat que dvm_string_get.
    if unsafe { dvm_string_get_inner(vm, r, &mut s) } != 0 {
        return E_INVAL;
    }
    // SAFETY: s vient de dvm_string_get_inner.
    unsafe { rt_utf16_to8(s, out) }
}

pub(crate) unsafe fn rt_exc_new(
    vm: *mut t_dvm,
    c: *mut t_dclass,
    msg: *const c_char,
    out: *mut t_dref,
) -> c_int {
    // SAFETY: vm et c non nuls pour un new. msg est nul ou une chaîne C.
    unsafe {
        let mut s = DVM_NULL;
        let mut r = 0;
        if !msg.is_null() {
            r = dvm_string_utf8_new_inner(vm, msg, &mut s);
        }
        if r == E_INVAL {
            r = 0;
        }
        if r != 0 {
            return r;
        }
        dvm_pin_inner(vm, s);
        r = dvm_new_inner(vm, c, out);
        dvm_unpin_inner(vm, s);
        if r != 0 {
            return r;
        }
        *rt_words(rt_obj(vm, *out, DOK_OBJECT)) = s;
        0
    }
}

unsafe fn rt_join(buf: *mut c_char, desc: *const c_char, msg: *const c_char) {
    // SAFETY: buf vise DJOIN_MAX octets. desc et msg sont nuls ou des chaînes C.
    unsafe {
        let mut n = 0usize;
        while !desc.is_null() && *desc.add(n) != 0 && n < DDESC_MAX {
            *buf.add(n) = *desc.add(n);
            n += 1;
        }
        let mut k = 0usize;
        while !msg.is_null() && *msg.add(k) != 0 && k < DMSG_MAX {
            k += 1;
        }
        while k > 0 && (*msg.add(k) as u8 & 0xc0) == 0x80 {
            k -= 1;
        }
        if k > 0 {
            *buf.add(n) = b':' as c_char;
            n += 1;
            *buf.add(n) = b' ' as c_char;
            n += 1;
        }
        let mut j = 0usize;
        while j < k {
            *buf.add(n) = *msg.add(j);
            n += 1;
            j += 1;
        }
        *buf.add(n) = 0;
    }
}

pub(crate) unsafe fn dvm_throw_inner(
    vm: *mut t_dvm,
    desc: *const c_char,
    mut msg: *const c_char,
) -> c_int {
    if vm.is_null() {
        return E_INVAL;
    }
    // SAFETY: vm non nul. desc est le descripteur demandé, possiblement inconnu.
    unsafe {
        let mut buf = [0 as c_char; DJOIN_MAX];
        let mut c = rt_find(vm, desc);
        if c.is_null()
            || ((*c).access & (ACC_ABSTRACT | ACC_INTERFACE)) != 0
            || rt_assignable(c, (*(*vm).classes).core[DCORE_THROWABLE]) == 0
        {
            c = (*(*vm).classes).core[DCORE_THROWABLE];
            rt_join(buf.as_mut_ptr(), desc, msg);
            msg = buf.as_ptr();
        }
        let mut exc = DVM_NULL;
        let r = rt_exc_new(vm, c, msg, &mut exc);
        if r != 0 {
            return r;
        }
        (*vm).pending = exc;
        DVM_THROWN
    }
}

pub(crate) unsafe fn dvm_throwable_message_inner(
    vm: *mut t_dvm,
    exc: t_dref,
    msg: *mut t_dref,
) -> c_int {
    // SAFETY: rt_obj vérifie vm.
    unsafe {
        let o = rt_obj(vm, exc, DOK_OBJECT);
        if o.is_null() || msg.is_null() {
            return E_INVAL;
        }
        if rt_assignable((*o).cls, (*(*vm).classes).core[DCORE_THROWABLE]) == 0 {
            return E_INVAL;
        }
        *msg = *rt_words(o);
        0
    }
}

pub(crate) unsafe fn dvm_throwable_set_message_inner(
    vm: *mut t_dvm,
    exc: t_dref,
    msg: t_dref,
) -> c_int {
    // SAFETY: rt_obj vérifie vm.
    unsafe {
        let o = rt_obj(vm, exc, DOK_OBJECT);
        if o.is_null() || (*o).len < 1 {
            return E_INVAL;
        }
        if rt_assignable((*o).cls, (*(*vm).classes).core[DCORE_THROWABLE]) == 0 {
            return E_INVAL;
        }
        if msg != DVM_NULL && rt_obj(vm, msg, DOK_STRING).is_null() {
            return E_INVAL;
        }
        *rt_words(o) = msg;
        0
    }
}

unsafe fn rt_core_one(vm: *mut t_dvm, slot: usize, desc: *const c_char, access: u32) -> c_int {
    // SAFETY: vm partiellement construit, slot < DCORE_N.
    unsafe {
        let b = t_dbuiltin {
            desc,
            super_desc: if slot == DCORE_OBJECT || slot == DCORE_CHARSEQ {
                core::ptr::null()
            } else {
                cstr(b"Ljava/lang/Object;\0")
            },
            iface_desc: if slot == DCORE_STRING {
                cstr(b"Ljava/lang/CharSequence;\0")
            } else {
                core::ptr::null()
            },
            access,
            payload_bytes: 0,
        };
        rt_define(vm, &b, &mut (*(*vm).classes).core[slot])
    }
}

unsafe fn rt_core(vm: *mut t_dvm) -> c_int {
    // SAFETY: vm a heap et classes.
    unsafe {
        let mut r = rt_core_one(vm, DCORE_OBJECT, cstr(b"Ljava/lang/Object;\0"), ACC_PUBLIC);
        if r == 0 {
            r = rt_core_one(
                vm,
                DCORE_CLASS,
                cstr(b"Ljava/lang/Class;\0"),
                ACC_PUBLIC | ACC_FINAL,
            );
        }
        if r == 0 {
            r = rt_core_one(
                vm,
                DCORE_CHARSEQ,
                cstr(b"Ljava/lang/CharSequence;\0"),
                ACC_PUBLIC | ACC_INTERFACE | ACC_ABSTRACT,
            );
        }
        if r == 0 {
            r = rt_core_one(
                vm,
                DCORE_STRING,
                cstr(b"Ljava/lang/String;\0"),
                ACC_PUBLIC | ACC_FINAL,
            );
        }
        if r == 0 {
            r = rt_core_one(
                vm,
                DCORE_THROWABLE,
                cstr(b"Ljava/lang/Throwable;\0"),
                ACC_PUBLIC,
            );
        }
        if r != 0 {
            return r;
        }
        (*(*(*vm).classes).core[DCORE_THROWABLE]).words = 1;
        (*(*(*vm).classes).core[DCORE_CLASS]).pay_end = 8;
        rt_exc_new(
            vm,
            (*(*vm).classes).core[DCORE_THROWABLE],
            cstr(b"Ljava/lang/OutOfMemoryError;\0"),
            &mut (*(*vm).heap).oom,
        )
    }
}

unsafe fn rt_limits(lim: *mut t_dlimits) -> c_int {
    // SAFETY: lim pointe la copie dans la machine.
    unsafe {
        if (*lim).heap_bytes == 0 {
            (*lim).heap_bytes = DVM_HEAP_DEFAULT;
        }
        if (*lim).stack_words == 0 {
            (*lim).stack_words = DVM_STACK_WORDS_DEFAULT;
        }
        if (*lim).depth_max == 0 {
            (*lim).depth_max = DVM_DEPTH_DEFAULT;
        }
        if (*lim).classes_max == 0 {
            (*lim).classes_max = DVM_CLASSES_DEFAULT;
        }
        if (*lim).classes_max < DCORE_N as u32 || (*lim).classes_max > DCLASSES_HARD {
            return E_INVAL;
        }
        if (*lim).stack_words > DSTACK_HARD {
            return E_INVAL;
        }
        0
    }
}

unsafe fn rt_parts(vm: *mut t_dvm) -> c_int {
    // SAFETY: vm vient de calloc.
    unsafe {
        (*vm).heap = c_calloc(1, core::mem::size_of::<crate::s_dheap>()).cast();
        (*vm).classes = c_calloc(1, core::mem::size_of::<crate::s_dclasses>()).cast();
        (*vm).stack = c_calloc((*vm).lim.stack_words as usize, core::mem::size_of::<u32>()).cast();
        if (*vm).heap.is_null() || (*vm).classes.is_null() || (*vm).stack.is_null() {
            return E_NOMEM;
        }
        (*vm).stack_words = (*vm).lim.stack_words;
        (*(*vm).heap).limit = (*vm).lim.heap_bytes;
        (*(*vm).classes).max = (*vm).lim.classes_max;
        (*(*vm).classes).tab = c_calloc(
            (*vm).lim.classes_max as usize,
            core::mem::size_of::<*mut t_dclass>(),
        )
        .cast();
        if (*(*vm).classes).tab.is_null() {
            return E_NOMEM;
        }
        rt_core(vm)
    }
}

pub(crate) unsafe fn dvm_create_inner(out: *mut *mut t_dvm, lim: *const t_dlimits) -> c_int {
    if out.is_null() {
        return E_INVAL;
    }
    // SAFETY: out non nul. lim est nul ou une structure vivante.
    unsafe {
        *out = core::ptr::null_mut();
        let vm = c_calloc(1, core::mem::size_of::<t_dvm>()).cast::<t_dvm>();
        if vm.is_null() {
            return E_NOMEM;
        }
        if !lim.is_null() {
            (*vm).lim = *lim;
        }
        let mut r = rt_limits(&mut (*vm).lim);
        if r == 0 {
            r = rt_parts(vm);
        }
        if r != 0 {
            crate::heap::dvm_destroy_inner(vm);
            return r;
        }
        *out = vm;
        0
    }
}
