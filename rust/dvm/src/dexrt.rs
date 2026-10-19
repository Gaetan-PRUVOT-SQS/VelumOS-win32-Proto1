use core::ffi::{c_char, c_int};

use crate::bridge::{go_call, go_throw};
use crate::cls::{dvm_array_new_inner, dvm_class_inner, dvm_string_utf8_new_inner};
use crate::heap::{cstr, dvm_new_inner, rt_class_free, rt_obj, rt_words};
use crate::mem::{c_calloc, c_free, c_strcmp, c_strlen};
use crate::{
    ACC_ABSTRACT, ACC_FINAL, ACC_INTERFACE, ACC_NATIVE, ACC_STATIC, DAK_BOOLEAN, DAK_BYTE,
    DAK_CHAR, DAK_INT, DAK_OBJECT, DAK_SHORT, DAK_WIDE, DCORE_CLASS, DCORE_STRING, DEX_M_DIRECT,
    DEX_M_SFIELD, DEX_NO_INDEX, DEX_T_CLASS, DEX_T_FIELD, DEX_T_METHOD, DEX_T_PROTO, DEX_T_STRING,
    DEX_T_TYPE, DEX_V_NULL, DEX_V_STRING, DIFACES_MAX, DIK_DIRECT, DIK_INTERFACE, DIK_STATIC,
    DIK_SUPER, DIK_VIRTUAL, DJOIN_MAX, DLINK_DEPTH, DMEMBERS_MAX, DNAT_TABS, DOK_ANY, DOK_ARRAY,
    DOK_OBJECT, DOK_STRING, DVM_NULL, DVM_THROWN, DWORDS_MAX, DX_AIOOBE, DX_ASE, DX_CCE,
    DX_NOCLASS, DX_NPE, DX_VERIFY, E_INVAL, E_NOENT, E_NOMEM, E_NOTSUP, E_RANGE, t_daacc, t_darray,
    t_dclass, t_dcodelimits, t_dex, t_dexcatch, t_dexcdata, t_dexclass, t_dexcode, t_dexfield,
    t_dexhit, t_dexmember, t_dexmethod, t_dexproto, t_dexstr, t_dexvalue, t_dfield, t_dinvoke,
    t_dmethod, t_dname, t_dnative, t_dnewarr, t_dobj, t_dref, t_dvm, t_span, t_text,
};

macro_rules! dex {
    ($name:ident($($a:expr),* $(,)?)) => {{
        #[allow(unused_unsafe)]
        // SAFETY: symbole C dex ou dexcode, même signature que le contrat, non défini ici.
        unsafe {
            #[cfg(not(test))]
            {
                crate::ext::$name($($a),*)
            }
            #[cfg(test)]
            {
                crate::dexstub::$name($($a),*)
            }
        }
    }};
}

fn zstr() -> t_dexstr {
    t_dexstr {
        p: core::ptr::null(),
        size: 0,
        utf16: 0,
    }
}

pub(crate) unsafe fn rt_dstr(d: *const t_dex, idx: u32) -> *const c_char {
    let mut s = zstr();
    if dex!(dex_string(d, idx, &mut s)) != 0 {
        return core::ptr::null();
    }
    s.p
}

pub(crate) unsafe fn rt_dtype(d: *const t_dex, idx: u32) -> *const c_char {
    let mut s = zstr();
    if dex!(dex_type(d, idx, &mut s)) != 0 {
        return core::ptr::null();
    }
    s.p
}

pub(crate) unsafe fn dvm_load_dex(vm: *mut t_dvm, dex: t_span) -> c_int {
    if vm.is_null() || dex.p.is_null() {
        return E_INVAL;
    }
    // SAFETY: vm non nul. classes vient de dvm_create.
    unsafe {
        let cs = (*vm).classes;
        if (*cs).has_dex != 0 {
            return E_NOTSUP;
        }
        let r = dex!(dex_open(core::ptr::addr_of_mut!((*cs).dex), dex));
        if r != 0 {
            return r;
        }
        let nclass = (*cs).dex.n[DEX_T_CLASS] as usize;
        let nstr = (*cs).dex.n[DEX_T_STRING] as usize;
        (*cs).def_state = c_calloc(nclass.wrapping_add(1), 1).cast();
        (*cs).interned = c_calloc(nstr.wrapping_add(1), core::mem::size_of::<t_dref>()).cast();
        if !(*cs).def_state.is_null() && !(*cs).interned.is_null() {
            (*cs).has_dex = 1;
        }
        if (*cs).has_dex != 0 {
            return 0;
        }
        c_free((*cs).def_state.cast());
        c_free((*cs).interned.cast());
        (*cs).def_state = core::ptr::null_mut();
        (*cs).interned = core::ptr::null_mut();
        E_NOMEM
    }
}

unsafe fn rt_native_fill(vm: *const t_dvm, t: *const t_dnative, m: *mut t_dmethod) -> c_int {
    // SAFETY: t et m non nuls, vm a ses classes.
    unsafe {
        if (*t).cls.is_null() || (*t).name.is_null() || (*t).sig.is_null() || (*t).fn_.is_none() {
            return E_INVAL;
        }
        (*m).cls = crate::cls::rt_find(vm, (*t).cls);
        if (*m).cls.is_null() {
            return E_NOENT;
        }
        (*m).name = (*t).name;
        (*m).sig = (*t).sig;
        (*m).native = (*t).fn_;
        (*m).access = (*t).access | ACC_NATIVE;
        (*m).ins = rt_sig_words((*t).sig).wrapping_add(u16::from(((*t).access & ACC_STATIC) == 0));
        0
    }
}

pub(crate) unsafe fn dvm_natives(vm: *mut t_dvm, tab: *const t_dnative, n: u32) -> c_int {
    if vm.is_null() || tab.is_null() || n == 0 {
        return E_INVAL;
    }
    // SAFETY: vm non nul, tab vise n natives.
    unsafe {
        let cs = (*vm).classes;
        if (*cs).nnat >= DNAT_TABS as u32 {
            return E_RANGE;
        }
        let m = c_calloc(n as usize, core::mem::size_of::<t_dmethod>()).cast::<t_dmethod>();
        if m.is_null() {
            return E_NOMEM;
        }
        let mut i = 0u32;
        while i < n && rt_native_fill(vm, tab.add(i as usize), m.add(i as usize)) == 0 {
            i += 1;
        }
        if i < n {
            c_free(m.cast());
            return E_NOENT;
        }
        let slot = (*cs).nnat as usize;
        (*cs).natm[slot] = m;
        (*cs).natn[slot] = n;
        (*cs).nnat += 1;
        0
    }
}

pub(crate) unsafe fn rt_sig_words(mut s: *const c_char) -> u16 {
    // SAFETY: s est une signature C non nulle.
    unsafe {
        let mut n = 0u16;
        if *s == b'(' as c_char {
            s = s.add(1);
        }
        while *s != 0 && *s != b')' as c_char {
            let wide = u16::from(*s == b'J' as c_char || *s == b'D' as c_char);
            while *s == b'[' as c_char {
                s = s.add(1);
            }
            if *s == b'L' as c_char {
                while *s != 0 && *s != b';' as c_char {
                    s = s.add(1);
                }
            }
            if *s != 0 {
                s = s.add(1);
            }
            n = n.wrapping_add(1 + wide);
        }
        n
    }
}

unsafe fn rt_sig_one(d: *const t_dex, ty: u32, out: *mut c_char, mut n: usize) -> usize {
    // SAFETY: d est le dex ouvert. out est nul ou assez grand.
    unsafe {
        let mut s = zstr();
        if dex!(dex_type(d, ty, &mut s)) != 0 {
            return n;
        }
        let mut k = 0usize;
        while *s.p.add(k) != 0 {
            if !out.is_null() {
                *out.add(n) = *s.p.add(k);
            }
            n = n.wrapping_add(1);
            k += 1;
        }
        n
    }
}

unsafe fn rt_sig_put(d: *const t_dex, p: *const t_dexproto, out: *mut c_char) -> usize {
    // SAFETY: p est un prototype rempli par dex_proto.
    unsafe {
        let mut n = 1usize;
        if !out.is_null() {
            *out = b'(' as c_char;
        }
        let mut i = 0u32;
        while i < (*p).params.n {
            let ty = dex!(dex_list_at(core::ptr::addr_of!((*p).params), i));
            i += 1;
            n = rt_sig_one(d, ty, out, n);
        }
        if !out.is_null() {
            *out.add(n) = b')' as c_char;
        }
        n = rt_sig_one(d, (*p).return_idx, out, n.wrapping_add(1));
        if !out.is_null() {
            *out.add(n) = 0;
        }
        n
    }
}

pub(crate) unsafe fn rt_sig_build(d: *const t_dex, p: *const t_dexproto) -> *mut c_char {
    // SAFETY: d et p non nuls.
    unsafe {
        let n = rt_sig_put(d, p, core::ptr::null_mut());
        let sig = c_calloc(1, n.wrapping_add(1)).cast::<c_char>();
        if !sig.is_null() {
            rt_sig_put(d, p, sig);
        }
        sig
    }
}

pub(crate) unsafe fn rt_verr(vm: *mut t_dvm, c: *const t_dclass) -> c_int {
    // SAFETY: c non nul, desc est une chaîne de la classe.
    unsafe { go_throw(vm, cstr(DX_VERIFY), (*c).desc) }
}

unsafe fn rt_same(m: *const t_dmethod, nm: *const t_dname) -> bool {
    // SAFETY: m et nm non nuls.
    unsafe {
        if (*m).name.is_null() || (*m).sig.is_null() {
            return false;
        }
        c_strcmp((*m).name, (*nm).name) == 0 && c_strcmp((*m).sig, (*nm).sig) == 0
    }
}

pub(crate) unsafe fn rt_method_own(
    vm: *const t_dvm,
    c: *const t_dclass,
    nm: *const t_dname,
) -> *mut t_dmethod {
    // SAFETY: vm, c et nm non nuls.
    unsafe {
        let mut i = 0u32;
        while i < (*c).nmethods {
            let m = (*c).methods.add(i as usize);
            if rt_same(m, nm) {
                return m;
            }
            i += 1;
        }
        let cs = (*vm).classes;
        let mut k = 0u32;
        while k < (*cs).nnat {
            i = 0;
            while i < (*cs).natn[k as usize] {
                let m = (*cs).natm[k as usize].add(i as usize);
                if core::ptr::eq((*m).cls, c) && rt_same(m, nm) {
                    return m;
                }
                i += 1;
            }
            k += 1;
        }
        core::ptr::null_mut()
    }
}

pub(crate) unsafe fn rt_method_find(
    vm: *const t_dvm,
    c: *const t_dclass,
    nm: *const t_dname,
) -> *mut t_dmethod {
    // SAFETY: vm, c et nm non nuls.
    unsafe {
        let mut k = c;
        let mut m: *mut t_dmethod = core::ptr::null_mut();
        while !k.is_null() && m.is_null() {
            m = rt_method_own(vm, k, nm);
            k = (*k).super_;
        }
        k = c;
        while !k.is_null() && m.is_null() {
            let mut i = 0u32;
            while i < (*k).nifaces && m.is_null() {
                m = rt_method_find(vm, *(*k).ifaces.add(i as usize), nm);
                i += 1;
            }
            k = (*k).super_;
        }
        m
    }
}

pub(crate) unsafe fn dvm_method(
    vm: *mut t_dvm,
    c: *mut t_dclass,
    name: *const t_dname,
    out: *mut *const t_dmethod,
) -> c_int {
    if vm.is_null()
        || c.is_null()
        || name.is_null()
        || out.is_null()
        // SAFETY: name non nul, les tests précédents ont court-circuité.
        || unsafe { (*name).name.is_null() || (*name).sig.is_null() }
    {
        return E_INVAL;
    }
    // SAFETY: les pointeurs exigés sont non nuls.
    unsafe {
        *out = rt_method_find(vm, c, name);
        if (*out).is_null() {
            return E_NOENT;
        }
        0
    }
}

pub(crate) unsafe fn rt_clinit(vm: *mut t_dvm, c: *mut t_dclass) -> c_int {
    // SAFETY: vm et c non nuls.
    unsafe {
        let nm = t_dname {
            name: cstr(b"<clinit>\0"),
            sig: cstr(b"()V\0"),
        };
        let m = rt_method_own(vm, c, &nm);
        if m.is_null() || (*c).init != 0 {
            return 0;
        }
        (*c).init = 1;
        let mut ret = 0u64;
        go_call(vm, m, core::ptr::null(), &mut ret)
    }
}

unsafe fn rt_method_code(d: *const t_dex, m: *mut t_dmethod) -> c_int {
    // SAFETY: d et m non nuls.
    unsafe {
        if (*m).code_off == 0 {
            return 0;
        }
        let mut code = t_dexcode {
            insns: core::ptr::null(),
            registers: 0,
            ins: 0,
            outs: 0,
            tries: 0,
            insns_size: 0,
            off: 0,
            tries_off: 0,
            handlers_off: 0,
        };
        if dex!(dex_code(d, (*m).code_off, &mut code)) != 0 {
            return E_INVAL;
        }
        if code.ins != u32::from((*m).ins)
            || code.ins > code.registers
            || code.registers > 0xffff
            || code.outs > 0xffff
            || code.tries > 0xffff
        {
            return E_INVAL;
        }
        (*m).registers = code.registers as u16;
        (*m).outs = code.outs as u16;
        (*m).tries = code.tries as u16;
        (*m).insns.p = code.insns;
        (*m).insns.len = (code.insns_size as usize).wrapping_mul(2);
        0
    }
}

pub(crate) unsafe fn rt_method_fill(
    vm: *mut t_dvm,
    mb: *const t_dexmember,
    c: *mut t_dclass,
    i: u32,
) -> c_int {
    // SAFETY: vm, mb et c non nuls. i < nmethods.
    unsafe {
        let d = core::ptr::addr_of!((*(*vm).classes).dex);
        let m = (*c).methods.add(i as usize);
        let mut dm = t_dexmethod {
            class_idx: 0,
            proto_idx: 0,
            name_idx: 0,
        };
        let mut p = t_dexproto {
            shorty_idx: 0,
            return_idx: 0,
            params: crate::t_dexlist {
                p: core::ptr::null(),
                n: 0,
            },
        };
        if dex!(dex_method(d, (*mb).idx, &mut dm)) != 0
            || dex!(dex_proto(d, dm.proto_idx, &mut p)) != 0
        {
            return E_INVAL;
        }
        let sig = rt_sig_build(d, &p);
        *(*c).sigs.add(i as usize) = sig;
        if sig.is_null() {
            return E_NOMEM;
        }
        (*m).cls = c;
        (*m).name = rt_dstr(d, dm.name_idx);
        (*m).shorty = rt_dstr(d, p.shorty_idx);
        (*m).sig = sig;
        (*m).dex = d;
        (*m).access = (*mb).access;
        (*m).code_off = (*mb).code_off;
        (*m).slot = i;
        (*m).ins = rt_sig_words((*m).sig).wrapping_add(u16::from(((*mb).access & ACC_STATIC) == 0));
        if (*m).name.is_null() {
            return E_INVAL;
        }
        rt_method_code(d, m)
    }
}

pub(crate) unsafe fn dvm_method_start(m: *const t_dmethod, pc: u32) -> c_int {
    if m.is_null() {
        return 0;
    }
    // SAFETY: m non nul.
    unsafe {
        if (*m).cls.is_null()
            || (*(*m).cls).starts.is_null()
            || (*m).native.is_some()
            || (*m).slot >= (*(*m).cls).nmethods
            || pc >= ((*m).insns.len / 2) as u32
        {
            return 0;
        }
        let starts = *(*(*m).cls).starts.add((*m).slot as usize);
        if starts.is_null() {
            return 0;
        }
        c_int::from(((*starts.add((pc >> 3) as usize) >> (pc & 7)) & 1) != 0)
    }
}

unsafe fn rt_count(d: *const t_dex, dc: *const t_dexclass, c: *mut t_dclass) -> c_int {
    // SAFETY: d, dc et c non nuls.
    unsafe {
        let mut it = t_dexcdata {
            d: core::ptr::null(),
            pos: 0,
            left: [0; 4],
            kind: 0,
            last: 0,
        };
        let mut nf = 0u32;
        let mut nm = 0u32;
        if (*dc).class_data_off != 0 && dex!(dex_cdata_open(d, dc, &mut it)) != 0 {
            return E_INVAL;
        }
        let mut member = t_dexmember {
            kind: 0,
            idx: 0,
            access: 0,
            code_off: 0,
        };
        while (*dc).class_data_off != 0 && dex!(dex_cdata_next(&mut it, &mut member)) == 0 {
            nf += u32::from(member.kind < DEX_M_DIRECT);
            nm += u32::from(member.kind >= DEX_M_DIRECT);
        }
        if nf > DMEMBERS_MAX || nm > DMEMBERS_MAX {
            return E_INVAL;
        }
        (*c).nfields = nf;
        (*c).nmethods = nm;
        0
    }
}

unsafe fn rt_members_alloc(c: *mut t_dclass) -> c_int {
    // SAFETY: c non nul.
    unsafe {
        (*c).fields = c_calloc((*c).nfields as usize + 1, core::mem::size_of::<t_dfield>()).cast();
        (*c).statics = c_calloc(
            (2 * (*c).nfields as usize).wrapping_add(1),
            core::mem::size_of::<u32>(),
        )
        .cast();
        (*c).methods = c_calloc(
            (*c).nmethods as usize + 1,
            core::mem::size_of::<t_dmethod>(),
        )
        .cast();
        (*c).sigs = c_calloc(
            (*c).nmethods as usize + 1,
            core::mem::size_of::<*mut c_char>(),
        )
        .cast();
        (*c).starts = c_calloc((*c).nmethods as usize + 1, core::mem::size_of::<*mut u8>()).cast();
        if (*c).fields.is_null()
            || (*c).methods.is_null()
            || (*c).sigs.is_null()
            || (*c).starts.is_null()
            || (*c).statics.is_null()
        {
            return E_NOMEM;
        }
        0
    }
}

unsafe fn rt_field_fill(
    d: *const t_dex,
    m: *const t_dexmember,
    c: *mut t_dclass,
    f: *mut t_dfield,
) {
    // SAFETY: d, m, c et f non nuls.
    unsafe {
        let mut df = t_dexfield {
            class_idx: 0,
            type_idx: 0,
            name_idx: 0,
        };
        if dex!(dex_field(d, (*m).idx, &mut df)) != 0 {
            return;
        }
        (*f).r#type = rt_dtype(d, df.type_idx);
        if (*f).r#type.is_null() {
            return;
        }
        (*f).name = rt_dstr(d, df.name_idx);
        (*f).wide = u8::from(*(*f).r#type == b'J' as c_char || *(*f).r#type == b'D' as c_char);
        (*f).is_ref = u8::from(*(*f).r#type == b'L' as c_char || *(*f).r#type == b'[' as c_char);
        (*f).is_static = u8::from((*m).kind == DEX_M_SFIELD);
        if (*f).is_static != 0 {
            (*f).slot = (*c).nstatics;
            (*c).nstatics = (*c).nstatics.wrapping_add(1 + u32::from((*f).wide));
            return;
        }
        (*f).slot = (*c).words;
        (*c).words = (*c).words.wrapping_add(1 + u32::from((*f).wide));
    }
}

unsafe fn rt_members_fill(vm: *mut t_dvm, dc: *const t_dexclass, c: *mut t_dclass) -> c_int {
    // SAFETY: vm, dc et c non nuls.
    unsafe {
        let mut it = t_dexcdata {
            d: core::ptr::null(),
            pos: 0,
            left: [0; 4],
            kind: 0,
            last: 0,
        };
        let mut member = t_dexmember {
            kind: 0,
            idx: 0,
            access: 0,
            code_off: 0,
        };
        let mut fi = 0u32;
        let mut mi = 0u32;
        let mut r = 0;
        let d = core::ptr::addr_of!((*(*vm).classes).dex);
        if (*dc).class_data_off == 0 || dex!(dex_cdata_open(d, dc, &mut it)) != 0 {
            return 0;
        }
        while r == 0 && dex!(dex_cdata_next(&mut it, &mut member)) == 0 {
            if member.kind < DEX_M_DIRECT && fi < (*c).nfields {
                rt_field_fill(d, &member, c, (*c).fields.add(fi as usize));
                fi += 1;
            } else if member.kind >= DEX_M_DIRECT && mi < (*c).nmethods {
                r = rt_method_fill(vm, &member, c, mi);
                mi += 1;
            }
        }
        r
    }
}

pub(crate) unsafe fn rt_members(vm: *mut t_dvm, dc: *const t_dexclass, c: *mut t_dclass) -> c_int {
    // SAFETY: vm, dc et c non nuls.
    unsafe {
        let d = core::ptr::addr_of!((*(*vm).classes).dex);
        let mut r = rt_count(d, dc, c);
        if r == 0 {
            r = rt_members_alloc(c);
        }
        if r == 0 {
            r = rt_members_fill(vm, dc, c);
        }
        if r == 0 && (*c).words > DWORDS_MAX {
            r = E_INVAL;
        }
        if r == 0 {
            r = rt_statics_init(vm, dc, c);
        }
        if r == E_INVAL {
            r = rt_verr(vm, c);
        }
        r
    }
}

unsafe fn rt_start(starts: *const u8, pc: u32) -> bool {
    // SAFETY: starts couvre pc.
    unsafe { ((*starts.add((pc >> 3) as usize) >> (pc & 7)) & 1) != 0 }
}

unsafe fn rt_try_ok(
    m: *const t_dmethod,
    code: *const t_dexcode,
    i: u32,
    starts: *const u8,
) -> bool {
    // SAFETY: m a un dex, code et starts viennent de dex_code et calloc.
    unsafe {
        let end = u64::from((*code).tries_off)
            .wrapping_add(8u64.wrapping_mul(u64::from(i).wrapping_add(1)));
        if end > (*(*m).dex).len as u64 {
            return false;
        }
        let t = (*(*m).dex)
            .p
            .add((*code).tries_off as usize + 8 * i as usize);
        let start = dex!(dexcode_rd32(t)) as u32;
        let span = u32::from(*t.add(4)) | (u32::from(*t.add(5)) << 8);
        if start >= (*code).insns_size
            || !rt_start(starts, start)
            || span > (*code).insns_size - start
        {
            return false;
        }
        let mut it = t_dexhit {
            d: core::ptr::null(),
            pos: 0,
            left: 0,
            catch_all: 0,
            insns_size: 0,
        };
        if dex!(dex_handler_open((*m).dex, code, i, &mut it)) != 0 {
            return false;
        }
        let mut ca = t_dexcatch {
            type_idx: 0,
            addr: 0,
        };
        while dex!(dex_handler_next(&mut it, &mut ca)) == 0 {
            if ca.addr >= (*code).insns_size || !rt_start(starts, ca.addr) {
                return false;
            }
        }
        true
    }
}

unsafe fn rt_code_limits(d: *const t_dex, code: *const t_dexcode, lim: *mut t_dcodelimits) {
    // SAFETY: les trois pointeurs sont non nuls.
    unsafe {
        (*lim).registers = (*code).registers;
        (*lim).ins = (*code).ins;
        (*lim).outs = (*code).outs;
        (*lim).strings = (*d).n[DEX_T_STRING];
        (*lim).types = (*d).n[DEX_T_TYPE];
        (*lim).fields = (*d).n[DEX_T_FIELD];
        (*lim).methods = (*d).n[DEX_T_METHOD];
        (*lim).protos = (*d).n[DEX_T_PROTO];
    }
}

unsafe fn rt_verify_one(vm: *mut t_dvm, c: *mut t_dclass, i: u32) -> c_int {
    // SAFETY: i < nmethods.
    unsafe {
        let d = core::ptr::addr_of!((*(*vm).classes).dex);
        let m = (*c).methods.add(i as usize);
        if (*m).code_off == 0 {
            return 0;
        }
        let mut code = t_dexcode {
            insns: core::ptr::null(),
            registers: 0,
            ins: 0,
            outs: 0,
            tries: 0,
            insns_size: 0,
            off: 0,
            tries_off: 0,
            handlers_off: 0,
        };
        if dex!(dex_code(d, (*m).code_off, &mut code)) != 0 {
            return E_INVAL;
        }
        let mut lim = t_dcodelimits {
            registers: 0,
            ins: 0,
            outs: 0,
            strings: 0,
            types: 0,
            fields: 0,
            methods: 0,
            protos: 0,
        };
        rt_code_limits(d, &code, &mut lim);
        let starts = c_calloc(1, (code.insns_size as usize / 8).wrapping_add(1)).cast::<u8>();
        *(*c).starts.add(i as usize) = starts;
        if starts.is_null() {
            return E_NOMEM;
        }
        if dex!(dexcode_verify((*m).insns, &lim, starts)) != 0 {
            return E_INVAL;
        }
        let mut t = 0u32;
        while t < code.tries && rt_try_ok(m, &code, t, starts) {
            t += 1;
        }
        if t < code.tries {
            return E_INVAL;
        }
        0
    }
}

pub(crate) unsafe fn rt_verify(vm: *mut t_dvm, c: *mut t_dclass) -> c_int {
    // SAFETY: vm et c non nuls.
    unsafe {
        let mut i = 0u32;
        let mut r = 0;
        while r == 0 && i < (*c).nmethods {
            r = rt_verify_one(vm, c, i);
            i += 1;
        }
        if r == E_INVAL {
            r = rt_verr(vm, c);
        }
        r
    }
}

unsafe fn rt_intern_new(vm: *mut t_dvm, s: *const c_char, out: *mut t_dref) -> c_int {
    // SAFETY: s est une chaîne C du dex.
    unsafe {
        let cap = c_strlen(s).wrapping_add(1);
        let p = c_calloc(1, cap).cast::<c_char>();
        if p.is_null() {
            return E_NOMEM;
        }
        let text = t_text { p, cap };
        let mut r = dex!(mutf8_to_utf8(s, text));
        if r >= 0 {
            r = dvm_string_utf8_new_inner(vm, text.p, out);
        }
        c_free(p.cast());
        r
    }
}

pub(crate) unsafe fn rt_intern(vm: *mut t_dvm, idx: u32, out: *mut t_dref) -> c_int {
    // SAFETY: vm et out non nuls.
    unsafe {
        let cs = (*vm).classes;
        *out = DVM_NULL;
        let mut s = core::ptr::null();
        if (*cs).has_dex != 0 {
            s = rt_dstr(core::ptr::addr_of!((*cs).dex), idx);
        }
        if s.is_null() {
            return E_INVAL;
        }
        *out = *(*cs).interned.add(idx as usize);
        if !rt_obj(vm, *out, DOK_STRING).is_null() {
            return 0;
        }
        let r = rt_intern_new(vm, s, out);
        if r == 0 {
            *(*cs).interned.add(idx as usize) = *out;
        }
        r
    }
}

unsafe fn rt_static_set(
    vm: *mut t_dvm,
    c: *mut t_dclass,
    f: *const t_dfield,
    v: *const t_dexvalue,
) -> c_int {
    // SAFETY: les pointeurs sont non nuls, statics est alloué.
    unsafe {
        if (*v).kind == DEX_V_NULL {
            return 0;
        }
        if (*v).kind == DEX_V_STRING {
            if (*f).is_ref == 0 {
                return E_INVAL;
            }
            let mut sref = DVM_NULL;
            let r = rt_intern(vm, (*v).i as u32, &mut sref);
            *(*c).statics.add((*f).slot as usize) = sref;
            return r;
        }
        if (*f).is_ref != 0 {
            return E_INVAL;
        }
        *(*c).statics.add((*f).slot as usize) = (*v).i as u32;
        if (*f).wide != 0 {
            *(*c).statics.add((*f).slot as usize + 1) = (((*v).i as u64) >> 32) as u32;
        }
        0
    }
}

pub(crate) unsafe fn rt_statics_init(
    vm: *mut t_dvm,
    dc: *const t_dexclass,
    c: *mut t_dclass,
) -> c_int {
    // SAFETY: vm, dc et c non nuls.
    unsafe {
        let mut v = t_dexvalue { kind: 0, i: 0 };
        let mut i = 0u32;
        let mut ord = 0u32;
        let mut r = 0;
        let d = core::ptr::addr_of!((*(*vm).classes).dex);
        while r == 0 && i < (*c).nfields {
            let f = (*c).fields.add(i as usize);
            if !(*f).r#type.is_null() && (*f).is_static != 0 {
                if dex!(dex_static_value(d, dc, ord, &mut v)) == 0 {
                    r = rt_static_set(vm, c, f, &v);
                }
                ord += 1;
            }
            i += 1;
        }
        r
    }
}

unsafe fn rt_link_parents(vm: *mut t_dvm, dc: *const t_dexclass, c: *mut t_dclass) -> c_int {
    // SAFETY: vm, dc et c non nuls.
    unsafe {
        let d = core::ptr::addr_of!((*(*vm).classes).dex);
        if (*dc).superclass_idx == DEX_NO_INDEX {
            return rt_verr(vm, c);
        }
        let r = dvm_class_inner(vm, rt_dtype(d, (*dc).superclass_idx), &mut (*c).super_);
        if r != 0 {
            return r;
        }
        if (*(*c).super_).is_array != 0
            || ((*(*c).super_).access & (ACC_FINAL | ACC_INTERFACE)) != 0
        {
            return rt_verr(vm, c);
        }
        (*c).words = (*(*c).super_).words;
        (*c).pay_off = (*(*c).super_).pay_end;
        (*c).pay_end = (*(*c).super_).pay_end;
        0
    }
}

unsafe fn rt_link_ifaces(vm: *mut t_dvm, dc: *const t_dexclass, c: *mut t_dclass) -> c_int {
    // SAFETY: vm, dc et c non nuls.
    unsafe {
        if (*dc).interfaces.n > DIFACES_MAX {
            return rt_verr(vm, c);
        }
        (*c).ifaces = c_calloc(
            (*dc).interfaces.n as usize + 1,
            core::mem::size_of::<*mut t_dclass>(),
        )
        .cast();
        if (*c).ifaces.is_null() {
            return E_NOMEM;
        }
        let d = core::ptr::addr_of!((*(*vm).classes).dex);
        let mut i = 0u32;
        while i < (*dc).interfaces.n {
            let ty = dex!(dex_list_at(core::ptr::addr_of!((*dc).interfaces), i));
            let r = dvm_class_inner(vm, rt_dtype(d, ty), (*c).ifaces.add(i as usize));
            if r != 0 {
                return r;
            }
            if (*(*(*c).ifaces.add(i as usize))).access & ACC_INTERFACE == 0 {
                return rt_verr(vm, c);
            }
            i += 1;
            (*c).nifaces = i;
        }
        0
    }
}

unsafe fn rt_link_steps(vm: *mut t_dvm, dc: *const t_dexclass, c: *mut t_dclass) -> c_int {
    // SAFETY: pointeurs non nuls.
    unsafe {
        (*c).access = (*dc).access;
        let mut r = rt_link_parents(vm, dc, c);
        if r == 0 {
            r = rt_link_ifaces(vm, dc, c);
        }
        if r == 0 {
            r = rt_members(vm, dc, c);
        }
        if r == 0 {
            r = rt_verify(vm, c);
        }
        r
    }
}

unsafe fn rt_link_def(vm: *mut t_dvm, def: u32, out: *mut *mut t_dclass) -> c_int {
    // SAFETY: vm et out non nuls.
    unsafe {
        *out = core::ptr::null_mut();
        let d = core::ptr::addr_of!((*(*vm).classes).dex);
        let mut dc = t_dexclass {
            class_idx: 0,
            access: 0,
            superclass_idx: 0,
            interfaces: crate::t_dexlist {
                p: core::ptr::null(),
                n: 0,
            },
            source_file_idx: 0,
            class_data_off: 0,
            static_values_off: 0,
        };
        if dex!(dex_class(d, def, &mut dc)) != 0 {
            return E_INVAL;
        }
        let c = c_calloc(1, core::mem::size_of::<t_dclass>()).cast::<t_dclass>();
        if !c.is_null() {
            (*c).desc = crate::cls::rt_dup(rt_dtype(d, dc.class_idx));
        }
        if c.is_null() || (*c).desc.is_null() {
            rt_class_free(c);
            return E_NOMEM;
        }
        let mut r = rt_link_steps(vm, &dc, c);
        if r == 0 && (*(*vm).classes).n >= (*(*vm).classes).max {
            r = go_throw(vm, cstr(DX_NOCLASS), (*c).desc);
        }
        if r != 0 {
            rt_class_free(c);
        }
        if r == 0 {
            *out = c;
        }
        r
    }
}

pub(crate) unsafe fn rt_link(
    vm: *mut t_dvm,
    desc: *const c_char,
    out: *mut *mut t_dclass,
) -> c_int {
    // SAFETY: vm, desc et out non nuls. Appelé par dvm_class.
    unsafe {
        let cs = (*vm).classes;
        let mut def = E_NOENT;
        if (*cs).has_dex != 0 {
            def = dex!(dex_find_class(core::ptr::addr_of!((*cs).dex), desc));
        }
        if def < 0 || *(*cs).def_state.add(def as usize) != 0 || (*cs).depth >= DLINK_DEPTH {
            return go_throw(vm, cstr(DX_NOCLASS), desc);
        }
        *(*cs).def_state.add(def as usize) = 1;
        (*cs).depth = (*cs).depth.wrapping_add(1);
        let r = rt_link_def(vm, def as u32, out);
        (*cs).depth = (*cs).depth.wrapping_sub(1);
        *(*cs).def_state.add(def as usize) = 2 * u8::from(r != E_NOMEM);
        if r != 0 {
            return r;
        }
        let n = (*cs).n;
        *(*cs).tab.add(n as usize) = *out;
        (*cs).n = n + 1;
        rt_clinit(vm, *out)
    }
}

pub(crate) unsafe fn rt_cat(buf: *mut c_char, mut n: usize, mut s: *const c_char) -> usize {
    // SAFETY: buf vise DJOIN_MAX octets. s est nul ou une chaîne C.
    unsafe {
        while !s.is_null() && *s != 0 && n.wrapping_add(1) < DJOIN_MAX {
            *buf.add(n) = *s;
            n += 1;
            s = s.add(1);
        }
        *buf.add(n) = 0;
        n
    }
}

pub(crate) unsafe fn rt_class_idx(vm: *mut t_dvm, type_idx: u32, out: *mut *mut t_dclass) -> c_int {
    // SAFETY: out non nul quand vm l'est. Le corps vérifie vm.
    unsafe {
        *out = core::ptr::null_mut();
        if vm.is_null() || (*(*vm).classes).has_dex == 0 {
            return E_INVAL;
        }
        let desc = rt_dtype(core::ptr::addr_of!((*(*vm).classes).dex), type_idx);
        if desc.is_null() {
            return E_INVAL;
        }
        dvm_class_inner(vm, desc, out)
    }
}

pub(crate) unsafe fn dvmrt_new_instance_inner(
    vm: *mut t_dvm,
    _from: *const t_dmethod,
    idx: u32,
    out: *mut t_dref,
) -> c_int {
    if out.is_null() {
        return E_INVAL;
    }
    // SAFETY: out non nul.
    unsafe {
        *out = DVM_NULL;
        let mut c = core::ptr::null_mut();
        let r = rt_class_idx(vm, idx, &mut c);
        if r != 0 {
            return r;
        }
        if (*c).is_array != 0
            || ((*c).access & (ACC_ABSTRACT | ACC_INTERFACE)) != 0
            || c == (*(*vm).classes).core[DCORE_STRING]
        {
            return go_throw(vm, cstr(b"Ljava/lang/InstantiationError;\0"), (*c).desc);
        }
        dvm_new_inner(vm, c, out)
    }
}

pub(crate) unsafe fn dvmrt_const_string_inner(
    vm: *mut t_dvm,
    _from: *const t_dmethod,
    idx: u32,
    out: *mut t_dref,
) -> c_int {
    if vm.is_null() || out.is_null() {
        return E_INVAL;
    }
    // SAFETY: vm et out non nuls.
    unsafe { rt_intern(vm, idx, out) }
}

pub(crate) unsafe fn dvmrt_const_class_inner(
    vm: *mut t_dvm,
    _from: *const t_dmethod,
    idx: u32,
    out: *mut t_dref,
) -> c_int {
    if out.is_null() {
        return E_INVAL;
    }
    // SAFETY: out non nul.
    unsafe {
        *out = DVM_NULL;
        let mut c = core::ptr::null_mut();
        let r = rt_class_idx(vm, idx, &mut c);
        if r != 0 {
            return r;
        }
        *out = (*c).mirror;
        if !rt_obj(vm, *out, DOK_OBJECT).is_null() {
            return 0;
        }
        let r = dvm_new_inner(vm, (*(*vm).classes).core[DCORE_CLASS], out);
        if r != 0 {
            return r;
        }
        let slot = (*rt_obj(vm, *out, DOK_OBJECT)).data.cast::<*mut t_dclass>();
        *slot = c;
        (*c).mirror = *out;
        0
    }
}

pub(crate) unsafe fn dvmrt_instance_of_inner(
    vm: *mut t_dvm,
    _from: *const t_dmethod,
    idx: u32,
    obj: t_dref,
) -> c_int {
    // SAFETY: rt_class_idx vérifie vm.
    unsafe {
        let mut c = core::ptr::null_mut();
        let r = rt_class_idx(vm, idx, &mut c);
        if r == DVM_THROWN {
            return E_NOENT;
        }
        if r != 0 {
            return r;
        }
        let o = rt_obj(vm, obj, DOK_ANY);
        if o.is_null() {
            return 0;
        }
        crate::cls::rt_assignable((*o).cls, c)
    }
}

pub(crate) unsafe fn dvmrt_catches_inner(
    vm: *mut t_dvm,
    from: *const t_dmethod,
    idx: u32,
    exc: t_dref,
) -> c_int {
    if vm.is_null() {
        return E_INVAL;
    }
    if idx == DEX_NO_INDEX {
        return 1;
    }
    // SAFETY: vm non nul.
    unsafe {
        let pending = (*vm).pending;
        let _ = crate::heap::dvm_pin_inner(vm, pending);
        let r = crate::bridge::go_instance_of(vm, from, idx, exc);
        crate::heap::dvm_unpin_inner(vm, pending);
        (*vm).pending = pending;
        if r == E_NOENT {
            return 0;
        }
        r
    }
}

pub(crate) unsafe fn dvmrt_new_array_inner(
    vm: *mut t_dvm,
    _from: *const t_dmethod,
    rq: *mut t_dnewarr,
) -> c_int {
    if vm.is_null() || rq.is_null()
        // SAFETY: vm non nul, le test précédent a court-circuité.
        || unsafe { (*(*vm).classes).has_dex == 0 }
    {
        return E_INVAL;
    }
    // SAFETY: vm et rq non nuls, dex chargé.
    unsafe {
        let outp = core::ptr::addr_of_mut!((*rq).out);
        *outp = DVM_NULL;
        let desc = rt_dtype(core::ptr::addr_of!((*(*vm).classes).dex), (*rq).type_idx);
        if desc.is_null() || *desc != b'[' as c_char {
            return E_INVAL;
        }
        dvm_array_new_inner(vm, desc, (*rq).len, outp)
    }
}

pub(crate) unsafe fn dvmrt_array_length_inner(vm: *mut t_dvm, arr: t_dref, len: *mut u32) -> c_int {
    if vm.is_null() || len.is_null() {
        return E_INVAL;
    }
    // SAFETY: vm et len non nuls.
    unsafe {
        if arr == DVM_NULL {
            return go_throw(vm, cstr(DX_NPE), core::ptr::null());
        }
        let o = rt_obj(vm, arr, DOK_ARRAY);
        if o.is_null() {
            return go_throw(vm, cstr(DX_CCE), core::ptr::null());
        }
        *len = (*o).len;
        0
    }
}

pub(crate) unsafe fn dvmrt_fill_array_inner(
    vm: *mut t_dvm,
    arr: t_dref,
    data: *const t_darray,
) -> c_int {
    if vm.is_null() || data.is_null()
        // SAFETY: data non nul, le test précédent a court-circuité.
        || unsafe { (*data).data.is_null() }
    {
        return E_INVAL;
    }
    // SAFETY: vm et data non nuls, data->data non nul.
    unsafe {
        if arr == DVM_NULL {
            return go_throw(vm, cstr(DX_NPE), core::ptr::null());
        }
        let o = rt_obj(vm, arr, DOK_ARRAY);
        if o.is_null() || (*(*o).cls).is_ref != 0 || u32::from((*(*o).cls).width) != (*data).width {
            return go_throw(vm, cstr(DX_CCE), core::ptr::null());
        }
        if (*data).count > (*o).len {
            return go_throw(vm, cstr(DX_AIOOBE), core::ptr::null());
        }
        let mut a = t_daacc {
            arr: 0,
            index: 0,
            val: 0,
            kind: 0,
            put: 0,
        };
        while (a.index as u32) < (*data).count {
            a.val = 0;
            let mut k = (*data).width;
            while k > 0 {
                k -= 1;
                let at = (a.index as usize)
                    .wrapping_mul((*data).width as usize)
                    .wrapping_add(k as usize);
                a.val = (a.val << 8) | u64::from(*(*data).data.add(at));
            }
            rt_aput(o, &a);
            a.index += 1;
        }
        0
    }
}

unsafe fn rt_field_in(c: *const t_dclass, name: *const c_char, ty: *const c_char) -> *mut t_dfield {
    // SAFETY: c non nul.
    unsafe {
        let mut i = 0u32;
        while !name.is_null() && !ty.is_null() && i < (*c).nfields {
            let f = (*c).fields.add(i as usize);
            if !(*f).name.is_null()
                && c_strcmp((*f).name, name) == 0
                && c_strcmp((*f).r#type, ty) == 0
            {
                return f;
            }
            i += 1;
        }
        core::ptr::null_mut()
    }
}

unsafe fn rt_field_find(
    vm: *mut t_dvm,
    acc: *const crate::t_dfacc,
    f: *mut *mut t_dfield,
    k: *mut *mut t_dclass,
) -> c_int {
    // SAFETY: vm, acc, f et k non nuls.
    unsafe {
        let d = core::ptr::addr_of!((*(*vm).classes).dex);
        let mut df = t_dexfield {
            class_idx: 0,
            type_idx: 0,
            name_idx: 0,
        };
        if (*(*vm).classes).has_dex == 0 || dex!(dex_field(d, (*acc).field_idx, &mut df)) != 0 {
            return E_INVAL;
        }
        let r = rt_class_idx(vm, df.class_idx, k);
        if r != 0 {
            return r;
        }
        *f = core::ptr::null_mut();
        while !(*k).is_null() && (*f).is_null() {
            *f = rt_field_in(*k, rt_dstr(d, df.name_idx), rt_dtype(d, df.type_idx));
            if (*f).is_null() {
                *k = (**k).super_;
            }
        }
        if (*f).is_null() {
            return go_throw(
                vm,
                cstr(b"Ljava/lang/NoSuchFieldError;\0"),
                rt_dstr(d, df.name_idx),
            );
        }
        0
    }
}

unsafe fn rt_field_ptr(
    vm: *mut t_dvm,
    acc: *const crate::t_dfacc,
    f: *const t_dfield,
    p: *mut *mut u32,
) -> c_int {
    // SAFETY: pointeurs non nuls.
    unsafe {
        if (*f).is_static != u8::from((*acc).is_static != 0) {
            return go_throw(
                vm,
                cstr(b"Ljava/lang/IncompatibleClassChangeError;\0"),
                (*f).name,
            );
        }
        if (*f).wide != u8::from((*acc).wide != 0) || (*f).is_ref != u8::from((*acc).is_ref != 0) {
            return go_throw(vm, cstr(DX_VERIFY), (*f).name);
        }
        if (*f).is_static == 0 && (*acc).obj == DVM_NULL {
            return go_throw(vm, cstr(DX_NPE), (*f).name);
        }
        if (*acc).put != 0
            && (*f).is_ref != 0
            && (*acc).val != u64::from(DVM_NULL)
            && ((*acc).val > 0xffff_ffff || rt_obj(vm, (*acc).val as t_dref, DOK_ANY).is_null())
        {
            return go_throw(vm, cstr(DX_VERIFY), (*f).name);
        }
        if (*f).is_static != 0 {
            return 0;
        }
        let o = rt_obj(vm, (*acc).obj, DOK_OBJECT);
        if o.is_null() || (*f).slot.wrapping_add(u32::from((*f).wide)) >= (*o).len {
            return go_throw(vm, cstr(DX_CCE), (*f).name);
        }
        *p = rt_words(o).add((*f).slot as usize);
        0
    }
}

unsafe fn rt_field_io(acc: *mut crate::t_dfacc, f: *const t_dfield, p: *mut u32) {
    // SAFETY: p vise 1 ou 2 mots selon wide.
    unsafe {
        if (*acc).put != 0 {
            *p = (*acc).val as u32;
        }
        if (*acc).put != 0 && (*f).wide != 0 {
            *p.add(1) = ((*acc).val >> 32) as u32;
        }
        if (*acc).put == 0 {
            (*acc).val = u64::from(*p);
        }
        if (*acc).put == 0 && (*f).wide != 0 {
            (*acc).val |= u64::from(*p.add(1)) << 32;
        }
    }
}

pub(crate) unsafe fn dvmrt_field_inner(
    vm: *mut t_dvm,
    _from: *const t_dmethod,
    acc: *mut crate::t_dfacc,
) -> c_int {
    if vm.is_null() || acc.is_null() {
        return E_INVAL;
    }
    // SAFETY: vm et acc non nuls.
    unsafe {
        let mut f = core::ptr::null_mut();
        let mut k = core::ptr::null_mut();
        let r = rt_field_find(vm, acc, &mut f, &mut k);
        if r != 0 {
            return r;
        }
        let mut p = (*k).statics.add((*f).slot as usize);
        let r = rt_field_ptr(vm, acc, f, &mut p);
        if r != 0 {
            return r;
        }
        rt_field_io(acc, f, p);
        0
    }
}

fn rt_akind_ok(c: *const t_dclass, kind: u8) -> bool {
    // SAFETY: c et desc non nuls pour un tableau.
    let e = unsafe { *(*c).desc.add(1) } as u8;
    if kind == DAK_INT as u8 {
        return e == b'I' || e == b'F';
    }
    if kind == DAK_WIDE as u8 {
        return e == b'J' || e == b'D';
    }
    if kind == DAK_OBJECT as u8 {
        // SAFETY: c non nul, desc a déjà été lu.
        return unsafe { (*c).is_ref != 0 };
    }
    if kind == DAK_BOOLEAN as u8 {
        return e == b'Z';
    }
    if kind == DAK_BYTE as u8 {
        return e == b'B';
    }
    if kind == DAK_CHAR as u8 {
        return e == b'C';
    }
    kind == DAK_SHORT as u8 && e == b'S'
}

unsafe fn rt_aget(o: *const t_dobj, a: *mut t_daacc) {
    // SAFETY: index est dans la longueur, data est aligné sur la largeur.
    unsafe {
        let i = (*a).index as usize;
        let w = (*(*o).cls).width;
        if w == 1 {
            let b = *(*o).data.cast::<i8>().add(i);
            (*a).val = u64::from(b as i32 as u32);
        }
        if (*a).kind == DAK_BOOLEAN as u8 {
            (*a).val = u64::from(*(*o).data.cast::<u8>().add(i));
        }
        if w == 2 {
            let h = *(*o).data.cast::<i16>().add(i);
            (*a).val = u64::from(h as i32 as u32);
        }
        if (*a).kind == DAK_CHAR as u8 {
            (*a).val = u64::from(*(*o).data.cast::<u16>().add(i));
        }
        if w == 4 {
            (*a).val = u64::from(*(*o).data.cast::<u32>().add(i));
        }
        if w == 8 {
            (*a).val = *(*o).data.cast::<u64>().add(i);
        }
    }
}

pub(crate) unsafe fn rt_aput(o: *const t_dobj, a: *const t_daacc) {
    // SAFETY: index est dans la longueur.
    unsafe {
        let i = (*a).index as usize;
        let w = (*(*o).cls).width;
        if w == 1 {
            *(*o).data.cast::<u8>().add(i) = (*a).val as u8;
        }
        if w == 2 {
            *(*o).data.cast::<u16>().add(i) = (*a).val as u16;
        }
        if w == 4 {
            *(*o).data.cast::<u32>().add(i) = (*a).val as u32;
        }
        if w == 8 {
            *(*o).data.cast::<u64>().add(i) = (*a).val;
        }
    }
}

unsafe fn rt_astore_ok(vm: *const t_dvm, o: *const t_dobj, a: *const t_daacc) -> bool {
    // SAFETY: o est un tableau.
    unsafe {
        if (*(*o).cls).is_ref == 0 || (*a).val == u64::from(DVM_NULL) {
            return true;
        }
        if (*a).val > 0xffff_ffff {
            return false;
        }
        let v = rt_obj(vm, (*a).val as t_dref, DOK_ANY);
        !v.is_null() && crate::cls::rt_assignable((*v).cls, (*(*o).cls).elem) != 0
    }
}

pub(crate) unsafe fn dvmrt_array_inner(vm: *mut t_dvm, acc: *mut t_daacc) -> c_int {
    if vm.is_null() || acc.is_null() {
        return E_INVAL;
    }
    // SAFETY: vm et acc non nuls.
    unsafe {
        if (*acc).arr == DVM_NULL {
            return go_throw(vm, cstr(DX_NPE), core::ptr::null());
        }
        let o = rt_obj(vm, (*acc).arr, DOK_ARRAY);
        if o.is_null() || !rt_akind_ok((*o).cls, (*acc).kind) {
            return go_throw(vm, cstr(DX_CCE), core::ptr::null());
        }
        if (*acc).index < 0 || (*acc).index as u32 >= (*o).len {
            return go_throw(vm, cstr(DX_AIOOBE), core::ptr::null());
        }
        if (*acc).put == 0 {
            rt_aget(o, acc);
        }
        if (*acc).put == 0 {
            return 0;
        }
        if !rt_astore_ok(vm, o, acc) {
            return go_throw(vm, cstr(DX_ASE), core::ptr::null());
        }
        rt_aput(o, acc);
        0
    }
}

unsafe fn rt_res_miss(vm: *mut t_dvm, c: *const t_dclass, nm: *const t_dname) -> c_int {
    // SAFETY: c et nm non nuls. Le tampon fait DJOIN_MAX.
    unsafe {
        let mut buf = [0 as c_char; DJOIN_MAX];
        let mut n = rt_cat(buf.as_mut_ptr(), 0, (*c).desc);
        n = rt_cat(buf.as_mut_ptr(), n, cstr(b"->\0"));
        n = rt_cat(buf.as_mut_ptr(), n, (*nm).name);
        rt_cat(buf.as_mut_ptr(), n, (*nm).sig);
        go_throw(vm, cstr(b"Ljava/lang/NoSuchMethodError;\0"), buf.as_ptr())
    }
}

unsafe fn rt_res_self(vm: *mut t_dvm, inv: *const t_dinvoke, c: *mut *mut t_dclass) -> c_int {
    // SAFETY: vm, inv et c non nuls.
    unsafe {
        if (*inv).kind == DIK_STATIC {
            return 0;
        }
        if (*inv).self_ == DVM_NULL {
            return go_throw(vm, cstr(DX_NPE), core::ptr::null());
        }
        let o = rt_obj(vm, (*inv).self_, DOK_ANY);
        if o.is_null() || crate::cls::rt_assignable((*o).cls, *c) == 0 {
            return go_throw(vm, cstr(DX_CCE), core::ptr::null());
        }
        if (*inv).kind == DIK_VIRTUAL || (*inv).kind == DIK_INTERFACE {
            *c = (*o).cls;
        }
        0
    }
}

unsafe fn rt_res_pick(
    vm: *mut t_dvm,
    inv: *mut t_dinvoke,
    start: *const t_dclass,
    nm: *const t_dname,
) -> c_int {
    // SAFETY: pointeurs non nuls.
    unsafe {
        let m = if (*inv).kind == DIK_DIRECT {
            rt_method_own(vm, start, nm)
        } else {
            rt_method_find(vm, start, nm)
        };
        if m.is_null() {
            return rt_res_miss(vm, start, nm);
        }
        if (((*m).access & ACC_STATIC) != 0) != ((*inv).kind == DIK_STATIC) {
            return go_throw(
                vm,
                cstr(b"Ljava/lang/IncompatibleClassChangeError;\0"),
                (*nm).name,
            );
        }
        if (*m).native.is_none() && (*m).code_off == 0 {
            return go_throw(vm, cstr(b"Ljava/lang/AbstractMethodError;\0"), (*nm).name);
        }
        (*inv).target = m;
        0
    }
}

unsafe fn rt_res_name(vm: *mut t_dvm, idx: u32, nm: *mut t_dname, cls: *mut u32) -> *mut c_char {
    // SAFETY: vm, nm et cls non nuls.
    unsafe {
        let d = core::ptr::addr_of!((*(*vm).classes).dex);
        (*nm).name = core::ptr::null();
        let mut dm = t_dexmethod {
            class_idx: 0,
            proto_idx: 0,
            name_idx: 0,
        };
        let mut p = t_dexproto {
            shorty_idx: 0,
            return_idx: 0,
            params: crate::t_dexlist {
                p: core::ptr::null(),
                n: 0,
            },
        };
        if (*(*vm).classes).has_dex == 0
            || dex!(dex_method(d, idx, &mut dm)) != 0
            || dex!(dex_proto(d, dm.proto_idx, &mut p)) != 0
        {
            return core::ptr::null_mut();
        }
        let sig = rt_sig_build(d, &p);
        (*nm).name = rt_dstr(d, dm.name_idx);
        (*nm).sig = sig;
        *cls = dm.class_idx;
        sig
    }
}

pub(crate) unsafe fn dvmrt_resolve_inner(
    vm: *mut t_dvm,
    from: *const t_dmethod,
    inv: *mut t_dinvoke,
) -> c_int {
    if vm.is_null() || inv.is_null()
        // SAFETY: inv non nul, le test précédent a court-circuité.
        || unsafe { (*inv).kind > DIK_INTERFACE }
    {
        return E_INVAL;
    }
    // SAFETY: vm et inv non nuls, kind est un DIK_.
    unsafe {
        (*inv).target = core::ptr::null();
        let mut nm = t_dname {
            name: core::ptr::null(),
            sig: core::ptr::null(),
        };
        let mut cls = 0u32;
        let sig = rt_res_name(vm, (*inv).method_idx, &mut nm, &mut cls);
        if sig.is_null() && !nm.name.is_null() {
            return E_NOMEM;
        }
        if sig.is_null() || nm.name.is_null() {
            c_free(sig.cast());
            return E_INVAL;
        }
        let mut c = core::ptr::null_mut();
        let mut r = rt_class_idx(vm, cls, &mut c);
        if r == 0 {
            r = rt_res_self(vm, inv, &mut c);
        }
        if r == 0
            && (*inv).kind == DIK_SUPER
            && !from.is_null()
            && !(*from).cls.is_null()
            && !(*(*from).cls).super_.is_null()
        {
            c = (*(*from).cls).super_;
        }
        if r == 0 {
            r = rt_res_pick(vm, inv, c, &nm);
        }
        c_free(sig.cast());
        r
    }
}
