use core::ffi::{c_char, c_int, c_void};

use crate::mem::{c_calloc, c_free};
use crate::{
    ACC_ABSTRACT, ACC_INTERFACE, DOBJ_COST, DOBJ_FIRST, DOBJ_MAX, DOK_ANY, DOK_ARRAY, DOK_FREE,
    DOK_OBJECT, DVM_LOCALS_MAX, DVM_NULL, E_INVAL, E_NOMEM, E_RANGE, s_dheap, t_dclass,
    t_dheapstats, t_dnewobj, t_dobj, t_dref, t_dvm,
};

pub(crate) unsafe fn rt_obj(vm: *const t_dvm, r: t_dref, kind: u8) -> *mut t_dobj {
    if vm.is_null() {
        return core::ptr::null_mut();
    }
    // SAFETY: vm non nul. heap et tab sont nuls ou alloués par cette crate.
    unsafe {
        let heap = (*vm).heap;
        if heap.is_null() || r == DVM_NULL || r >= (*heap).cap {
            return core::ptr::null_mut();
        }
        let o = (*heap).tab.add(r as usize);
        if (*o).kind == DOK_FREE || (kind != DOK_ANY && (*o).kind != kind) {
            return core::ptr::null_mut();
        }
        o
    }
}

pub(crate) unsafe fn rt_words(o: *const t_dobj) -> *mut u32 {
    // SAFETY: o est un objet vivant. data et cls sont posés par rt_alloc.
    unsafe {
        let base = (*o).data.cast::<u8>();
        let off = (*(*o).cls).pay_end as usize;
        base.add(off).cast()
    }
}

unsafe fn rt_chain(h: *mut s_dheap, tab: *mut t_dobj, cap: u32) {
    // SAFETY: h et tab viennent de calloc. cap est la nouvelle capacité.
    unsafe {
        let mut i = 0u32;
        while i < (*h).cap {
            *tab.add(i as usize) = *(*h).tab.add(i as usize);
            i += 1;
        }
        while i < cap {
            if i != 0 {
                (*tab.add(i as usize)).next = (*h).free_head;
                (*h).free_head = i;
            }
            i += 1;
        }
    }
}

pub(crate) unsafe fn rt_grow(h: *mut s_dheap) -> c_int {
    // SAFETY: h est le tas de la machine, non nul.
    unsafe {
        let mut cap = (*h).cap.wrapping_mul(2);
        if (*h).cap == 0 {
            cap = DOBJ_FIRST;
        }
        if cap > DOBJ_MAX {
            return E_NOMEM;
        }
        let tab = c_calloc(cap as usize, core::mem::size_of::<t_dobj>()).cast::<t_dobj>();
        if tab.is_null() {
            return E_NOMEM;
        }
        rt_chain(h, tab, cap);
        c_free((*h).tab.cast());
        (*h).tab = tab;
        (*h).cap = cap;
        0
    }
}

pub(crate) unsafe fn rt_release(h: *mut s_dheap, i: u32) {
    // SAFETY: i est un indice d'objet du tas h.
    unsafe {
        let o = (*h).tab.add(i as usize);
        c_free((*o).data);
        (*h).st.bytes = (*h).st.bytes.wrapping_sub((*o).bytes);
        (*h).st.objects = (*h).st.objects.wrapping_sub(1);
        (*o).data = core::ptr::null_mut();
        (*o).cls = core::ptr::null_mut();
        (*o).len = 0;
        (*o).bytes = 0;
        (*o).pins = 0;
        (*o).mark = 0;
        (*o).kind = DOK_FREE;
        (*o).next = (*h).free_head;
        (*h).free_head = i;
    }
}

fn rt_fits(h: *const s_dheap, bytes: u64) -> bool {
    // SAFETY: h non nul, appelant de rt_room.
    unsafe {
        bytes <= u64::from((*h).limit) && u64::from((*h).st.bytes) <= u64::from((*h).limit) - bytes
    }
}

unsafe fn rt_room(vm: *mut t_dvm, bytes: u64) -> c_int {
    // SAFETY: vm et son tas sont non nuls, vérifiés par rt_alloc.
    unsafe {
        if rt_fits((*vm).heap, bytes) {
            return 0;
        }
        dvm_gc_inner(vm);
        if rt_fits((*vm).heap, bytes) {
            return 0;
        }
        if rt_obj(vm, (*(*vm).heap).oom, DOK_OBJECT).is_null() {
            return E_NOMEM;
        }
        (*vm).pending = (*(*vm).heap).oom;
        crate::DVM_THROWN
    }
}

unsafe fn rt_fill(h: *mut s_dheap, i: u32, rq: *const t_dnewobj, data: *mut c_void) {
    // SAFETY: i est free_head, data vient de calloc, rq est vivant.
    unsafe {
        let o = (*h).tab.add(i as usize);
        (*h).free_head = (*o).next;
        (*o).cls = (*rq).cls;
        (*o).data = data;
        (*o).len = (*rq).len;
        (*o).bytes = ((*rq).bytes as u32).wrapping_add(DOBJ_COST);
        (*o).kind = (*rq).kind;
        (*o).pins = 0;
        (*o).mark = 0;
        (*h).st.bytes = (*h).st.bytes.wrapping_add((*o).bytes);
        (*h).st.objects = (*h).st.objects.wrapping_add(1);
        if (*h).st.bytes > (*h).st.peak_bytes {
            (*h).st.peak_bytes = (*h).st.bytes;
        }
    }
}

pub(crate) unsafe fn rt_alloc(vm: *mut t_dvm, rq: *const t_dnewobj, out: *mut t_dref) -> c_int {
    // SAFETY: vm, rq et out sont non nuls. Le contrat de rt_alloc l'exige.
    unsafe {
        let h = (*vm).heap;
        *out = DVM_NULL;
        let mut r = rt_room(vm, (*rq).bytes.wrapping_add(u64::from(DOBJ_COST)));
        if r == 0 && (*h).free_head == 0 {
            r = rt_grow(h);
        }
        if r != 0 {
            return r;
        }
        let data = c_calloc(1, ((*rq).bytes as usize).wrapping_add(8));
        if data.is_null() {
            return E_NOMEM;
        }
        *out = (*h).free_head;
        rt_fill(h, (*h).free_head, rq, data);
        0
    }
}

pub(crate) unsafe fn rt_mark(vm: *const t_dvm, value: u32) {
    // SAFETY: vm est la machine du passage GC, ou un appelant qui a le même contrat.
    unsafe {
        let o = rt_obj(vm, value, DOK_ANY);
        if !o.is_null() && (*o).mark == 0 {
            (*o).mark = 1;
        }
    }
}

unsafe fn rt_scan(vm: *const t_dvm, o: *mut t_dobj) {
    // SAFETY: o est un objet marqué 1. data et cls sont cohérents avec kind.
    unsafe {
        (*o).mark = 2;
        let mut w: *const u32 = core::ptr::null();
        if (*o).kind == DOK_OBJECT {
            w = rt_words(o);
        }
        if (*o).kind == DOK_ARRAY && (*(*o).cls).is_ref != 0 {
            w = (*o).data.cast();
        }
        let mut i = 0u32;
        while !w.is_null() && i < (*o).len {
            rt_mark(vm, *w.add(i as usize));
            i += 1;
        }
    }
}

pub(crate) unsafe fn rt_roots_classes(vm: *const t_dvm) {
    // SAFETY: vm non nul, classes initialisées.
    unsafe {
        let cs = (*vm).classes;
        let mut i = 0u32;
        while i < (*cs).n {
            let c = *(*cs).tab.add(i as usize);
            rt_mark(vm, (*c).mirror);
            let mut k = 0u32;
            while !(*c).statics.is_null() && k < (*c).nstatics {
                rt_mark(vm, *(*c).statics.add(k as usize));
                k += 1;
            }
            i += 1;
        }
        i = 0;
        while !(*cs).interned.is_null() && i < (*cs).dex.n[crate::DEX_T_STRING] {
            rt_mark(vm, *(*cs).interned.add(i as usize));
            i += 1;
        }
    }
}

unsafe fn rt_roots(vm: *const t_dvm) {
    // SAFETY: vm non nul, tas présent. Appelé par dvm_gc.
    unsafe {
        let mut i = 0u32;
        while !(*vm).stack.is_null() && i < (*vm).sp && i < (*vm).stack_words {
            rt_mark(vm, *(*vm).stack.add(i as usize));
            i += 1;
        }
        i = 0;
        while i < (*vm).nlocals && i < DVM_LOCALS_MAX {
            rt_mark(vm, (*vm).locals[i as usize]);
            i += 1;
        }
        rt_mark(vm, (*vm).pending);
        rt_mark(vm, (*vm).result as t_dref);
        rt_mark(vm, (*(*vm).heap).oom);
        rt_roots_classes(vm);
        i = 1;
        let heap = (*vm).heap;
        while i < (*heap).cap {
            let o = (*heap).tab.add(i as usize);
            if (*o).kind != DOK_FREE && (*o).pins != 0 {
                (*o).mark = 1;
            }
            i += 1;
        }
    }
}

unsafe fn rt_pass(vm: *const t_dvm) -> u32 {
    // SAFETY: tas présent.
    unsafe {
        let heap = (*vm).heap;
        let mut i = 1u32;
        let mut n = 0u32;
        while i < (*heap).cap {
            let o = (*heap).tab.add(i as usize);
            if (*o).kind != DOK_FREE && (*o).mark == 1 {
                rt_scan(vm, o);
                n += 1;
            }
            i += 1;
        }
        n
    }
}

pub(crate) unsafe fn dvm_gc_inner(vm: *mut t_dvm) {
    if vm.is_null() {
        return;
    }
    // SAFETY: vm non nul. heap nul : on sort.
    unsafe {
        if (*vm).heap.is_null() {
            return;
        }
        rt_roots(vm);
        let mut i = 1u32;
        while i != 0 {
            i = rt_pass(vm);
        }
        let heap = (*vm).heap;
        i = 1;
        while i < (*heap).cap {
            let o = (*heap).tab.add(i as usize);
            if (*o).kind != DOK_FREE && (*o).mark == 0 {
                rt_release(heap, i);
            } else {
                (*o).mark = 0;
            }
            i += 1;
        }
        (*heap).st.collections = (*heap).st.collections.wrapping_add(1);
    }
}

pub(crate) unsafe fn rt_class_free(c: *mut t_dclass) {
    if c.is_null() {
        return;
    }
    // SAFETY: c vient de calloc. Les tableaux sont nuls ou alloués ici.
    unsafe {
        let mut i = 0u32;
        while i < (*c).nmethods {
            if !(*c).starts.is_null() {
                c_free(*(*c).starts.add(i as usize).cast());
            }
            if !(*c).sigs.is_null() {
                c_free(*(*c).sigs.add(i as usize).cast());
            }
            i += 1;
        }
        let iface_slot = core::ptr::addr_of_mut!((*c).iface);
        if (*c).ifaces != iface_slot {
            c_free((*c).ifaces.cast());
        }
        c_free((*c).methods.cast());
        c_free((*c).starts.cast());
        c_free((*c).sigs.cast());
        c_free((*c).fields.cast());
        c_free((*c).statics.cast());
        c_free((*c).desc.cast());
        c_free(c.cast());
    }
}

unsafe fn rt_classes_free(cs: *mut crate::s_dclasses) {
    // SAFETY: cs vient de calloc.
    unsafe {
        let mut i = 0u32;
        while !(*cs).tab.is_null() && i < (*cs).n {
            rt_class_free(*(*cs).tab.add(i as usize));
            i += 1;
        }
        i = 0;
        while i < (*cs).nnat {
            c_free((*cs).natm[i as usize].cast());
            i += 1;
        }
        c_free((*cs).def_state.cast());
        c_free((*cs).interned.cast());
        c_free((*cs).tab.cast());
        c_free(cs.cast());
    }
}

unsafe fn rt_heap_free(h: *mut s_dheap) {
    // SAFETY: h vient de calloc.
    unsafe {
        let mut i = 1u32;
        while !(*h).tab.is_null() && i < (*h).cap {
            c_free((*(*h).tab.add(i as usize)).data);
            i += 1;
        }
        c_free((*h).tab.cast());
        c_free(h.cast());
    }
}

pub(crate) unsafe fn dvm_destroy_inner(vm: *mut t_dvm) {
    if vm.is_null() {
        return;
    }
    // SAFETY: vm vient de calloc. Les parties sont nulles ou à nous.
    unsafe {
        if !(*vm).heap.is_null() {
            rt_heap_free((*vm).heap);
        }
        if !(*vm).classes.is_null() {
            rt_classes_free((*vm).classes);
        }
        c_free((*vm).stack.cast());
        c_free(vm.cast());
    }
}

pub(crate) unsafe fn dvm_new_inner(vm: *mut t_dvm, c: *mut t_dclass, out: *mut t_dref) -> c_int {
    if vm.is_null() || c.is_null() || out.is_null() {
        return E_INVAL;
    }
    // SAFETY: les trois pointeurs sont non nuls.
    unsafe {
        if (*c).is_array != 0
            || ((*c).access & (ACC_ABSTRACT | ACC_INTERFACE)) != 0
            || c == (*(*vm).classes).core[crate::DCORE_STRING]
        {
            return E_INVAL;
        }
        let rq = t_dnewobj {
            cls: c,
            len: (*c).words,
            bytes: u64::from((*c).pay_end) + u64::from((*c).words) * 4,
            kind: DOK_OBJECT,
        };
        rt_alloc(vm, &rq, out)
    }
}

pub(crate) unsafe fn dvm_class_of_inner(vm: *mut t_dvm, r: t_dref) -> *mut t_dclass {
    // SAFETY: vm peut être nul. rt_obj le refuse.
    unsafe {
        let o = rt_obj(vm, r, DOK_ANY);
        if o.is_null() {
            return core::ptr::null_mut();
        }
        (*o).cls
    }
}

pub(crate) unsafe fn dvm_payload_inner(
    vm: *mut t_dvm,
    r: t_dref,
    builtin: *const t_dclass,
) -> *mut c_void {
    // SAFETY: rt_obj et rt_assignable tolèrent les nuls qu'ils vérifient.
    unsafe {
        let o = rt_obj(vm, r, DOK_OBJECT);
        if o.is_null() || builtin.is_null() || (*builtin).pay_end == (*builtin).pay_off {
            return core::ptr::null_mut();
        }
        if crate::cls::rt_assignable((*o).cls, builtin) == 0 {
            return core::ptr::null_mut();
        }
        (*o).data
            .cast::<u8>()
            .add((*builtin).pay_off as usize)
            .cast()
    }
}

pub(crate) unsafe fn dvm_local_inner(vm: *mut t_dvm, r: t_dref) -> c_int {
    if vm.is_null() {
        return E_INVAL;
    }
    // SAFETY: vm non nul. rt_obj refuse une référence hors tas.
    unsafe {
        if r != DVM_NULL && rt_obj(vm, r, DOK_ANY).is_null() {
            return E_INVAL;
        }
        if (*vm).nlocals >= DVM_LOCALS_MAX {
            return E_RANGE;
        }
        let n = (*vm).nlocals;
        (*vm).locals[n as usize] = r;
        (*vm).nlocals = n + 1;
        0
    }
}

pub(crate) unsafe fn dvm_local_pop_inner(vm: *mut t_dvm, mut n: u32) {
    if vm.is_null() {
        return;
    }
    // SAFETY: vm non nul.
    unsafe {
        if n > (*vm).nlocals {
            n = (*vm).nlocals;
        }
        (*vm).nlocals -= n;
    }
}

pub(crate) unsafe fn dvm_pin_inner(vm: *mut t_dvm, r: t_dref) -> c_int {
    // SAFETY: rt_obj vérifie vm.
    unsafe {
        let o = rt_obj(vm, r, DOK_ANY);
        if o.is_null() {
            return E_INVAL;
        }
        if (*o).pins == u16::MAX {
            return E_RANGE;
        }
        (*o).pins += 1;
        0
    }
}

pub(crate) unsafe fn dvm_unpin_inner(vm: *mut t_dvm, r: t_dref) {
    // SAFETY: rt_obj vérifie vm.
    unsafe {
        let o = rt_obj(vm, r, DOK_ANY);
        if !o.is_null() && (*o).pins > 0 {
            (*o).pins -= 1;
        }
    }
}

pub(crate) unsafe fn dvm_heap_stats_inner(vm: *const t_dvm, out: *mut t_dheapstats) {
    if vm.is_null() || out.is_null() {
        return;
    }
    // SAFETY: vm et out non nuls.
    unsafe {
        if (*vm).heap.is_null() {
            return;
        }
        *out = (*(*vm).heap).st;
    }
}

pub(crate) fn cstr(s: &[u8]) -> *const c_char {
    s.as_ptr().cast()
}
