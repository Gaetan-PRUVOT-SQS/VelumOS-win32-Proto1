use core::ffi::{c_char, c_int, c_void};

#[allow(dead_code)]
pub(crate) const E_OK: c_int = 0;
pub(crate) const E_NOENT: c_int = -2;
pub(crate) const E_NOMEM: c_int = -12;
#[allow(dead_code)]
pub(crate) const E_FAULT: c_int = -14;
pub(crate) const E_EXIST: c_int = -17;
pub(crate) const E_INVAL: c_int = -22;
pub(crate) const E_RANGE: c_int = -34;
pub(crate) const E_OVERFLOW: c_int = -75;
pub(crate) const E_NOTSUP: c_int = -95;
pub(crate) const E_TIMEOUT: c_int = -110;

pub const DVM_THROWN: c_int = 1;
pub const DVM_NULL: u32 = 0;
pub const DVM_HEAP_DEFAULT: u32 = 16_777_216;
pub const DVM_STACK_WORDS_DEFAULT: u32 = 65_536;
pub const DVM_DEPTH_DEFAULT: u32 = 256;
pub const DVM_CLASSES_DEFAULT: u32 = 2048;
pub const DVM_LOCALS_MAX: u32 = 64;

pub const DIK_VIRTUAL: u8 = 0;
pub const DIK_SUPER: u8 = 1;
pub const DIK_DIRECT: u8 = 2;
pub const DIK_STATIC: u8 = 3;
pub const DIK_INTERFACE: u8 = 4;

pub const DAK_INT: u32 = 0;
pub const DAK_WIDE: u32 = 1;
pub const DAK_OBJECT: u32 = 2;
pub const DAK_BOOLEAN: u32 = 3;
pub const DAK_BYTE: u32 = 4;
pub const DAK_CHAR: u32 = 5;
pub const DAK_SHORT: u32 = 6;

pub const DEX_NO_INDEX: u32 = 0xffff_ffff;
pub const DEX_T_STRING: usize = 0;
pub const DEX_T_TYPE: usize = 1;
pub const DEX_T_PROTO: usize = 2;
pub const DEX_T_FIELD: usize = 3;
pub const DEX_T_METHOD: usize = 4;
pub const DEX_T_CLASS: usize = 5;
pub const DEX_M_DIRECT: u32 = 2;
pub const DEX_M_SFIELD: u32 = 0;
pub const DEX_V_STRING: u32 = 0x17;
pub const DEX_V_NULL: u32 = 0x1e;

pub const ACC_PUBLIC: u32 = 0x1;
pub const ACC_STATIC: u32 = 0x8;
pub const ACC_FINAL: u32 = 0x10;
pub const ACC_NATIVE: u32 = 0x100;
pub const ACC_INTERFACE: u32 = 0x200;
pub const ACC_ABSTRACT: u32 = 0x400;

pub const DOK_FREE: u8 = 0;
pub const DOK_OBJECT: u8 = 1;
pub const DOK_STRING: u8 = 2;
pub const DOK_ARRAY: u8 = 3;
pub const DOK_ANY: u8 = 255;
pub const DOBJ_COST: u32 = 16;
pub const DOBJ_FIRST: u32 = 64;
pub const DOBJ_MAX: u32 = 0x0800_0000;
pub const DSTR_MAX: u32 = 0x0fff_ffff;
pub const DDESC_MAX: usize = 255;
pub const DPAYLOAD_MAX: u32 = 1_048_576;
pub const DMSG_MAX: usize = 240;
pub const DJOIN_MAX: usize = 512;
pub const DCLASSES_HARD: u32 = 65_536;
pub const DSTACK_HARD: u32 = 16_777_216;
pub const DNAT_TABS: usize = 8;
pub const DLINK_DEPTH: u32 = 64;
pub const DIFACES_MAX: u32 = 64;
pub const DMEMBERS_MAX: u32 = 4096;
pub const DWORDS_MAX: u32 = 65_535;
pub const DCORE_OBJECT: usize = 0;
pub const DCORE_CLASS: usize = 1;
pub const DCORE_CHARSEQ: usize = 2;
pub const DCORE_STRING: usize = 3;
pub const DCORE_THROWABLE: usize = 4;
pub const DCORE_N: usize = 5;

pub const IN_HDR: u32 = 4;
pub const IN_INVOKE_LEN: u32 = 3;
pub const DF_3RC: u8 = 23;

pub const OP_MOVE_WIDE: u8 = 0x04;
pub const OP_MOVE_WIDE_16: u8 = 0x06;
pub const OP_MOVE_RESULT_WIDE: u8 = 0x0b;
pub const OP_RETURN_VOID: u8 = 0x0e;
pub const OP_RETURN_WIDE: u8 = 0x10;
pub const OP_CONST_WIDE_16: u8 = 0x16;
pub const OP_CONST_CLASS: u8 = 0x1c;
pub const OP_NEW_INSTANCE: u8 = 0x22;
pub const OP_THROW: u8 = 0x27;
pub const OP_CMPG_FLOAT: u8 = 0x2e;
pub const OP_CMPL_DOUBLE: u8 = 0x2f;
pub const OP_CMPG_DOUBLE: u8 = 0x30;
pub const OP_CMP_LONG: u8 = 0x31;
pub const OP_IF_EQ: u8 = 0x32;
pub const OP_IF_LE: u8 = 0x37;
pub const OP_IF_EQZ: u8 = 0x38;
pub const OP_AGET: u8 = 0x44;
pub const OP_APUT: u8 = 0x4b;
pub const OP_IGET: u8 = 0x52;
pub const OP_INVOKE_VIRTUAL: u8 = 0x6e;
pub const OP_NEG_INT: u8 = 0x7b;
pub const OP_NOT_INT: u8 = 0x7c;
pub const OP_NEG_LONG: u8 = 0x7d;
pub const OP_NOT_LONG: u8 = 0x7e;
pub const OP_NEG_FLOAT: u8 = 0x7f;
pub const OP_NEG_DOUBLE: u8 = 0x80;
pub const OP_INT_TO_LONG: u8 = 0x81;
pub const OP_INT_TO_FLOAT: u8 = 0x82;
pub const OP_INT_TO_DOUBLE: u8 = 0x83;
pub const OP_LONG_TO_INT: u8 = 0x84;
pub const OP_LONG_TO_FLOAT: u8 = 0x85;
pub const OP_FLOAT_TO_INT: u8 = 0x87;
pub const OP_FLOAT_TO_LONG: u8 = 0x88;
pub const OP_DOUBLE_TO_INT: u8 = 0x8a;
pub const OP_DOUBLE_TO_LONG: u8 = 0x8b;
#[allow(dead_code)]
pub const OP_DOUBLE_TO_FLOAT: u8 = 0x8c;
pub const OP_INT_TO_BYTE: u8 = 0x8d;
pub const OP_INT_TO_CHAR: u8 = 0x8e;
pub const OP_ADD_INT: u8 = 0x90;
pub const OP_ADD_LONG: u8 = 0x9b;
pub const OP_ADD_FLOAT: u8 = 0xa6;
pub const OP_ADD_DOUBLE: u8 = 0xab;
pub const OP_ADD_INT_2ADDR: u8 = 0xb0;
pub const OP_ADD_LONG_2ADDR: u8 = 0xbb;
pub const OP_ADD_FLOAT_2ADDR: u8 = 0xc6;
pub const OP_ADD_DOUBLE_2ADDR: u8 = 0xcb;
pub const OP_ADD_INT_LIT16: u8 = 0xd0;
pub const OP_ADD_INT_LIT8: u8 = 0xd8;

pub const DX_NPE: &[u8] = b"Ljava/lang/NullPointerException;\0";
pub const DX_CCE: &[u8] = b"Ljava/lang/ClassCastException;\0";
pub const DX_AIOOBE: &[u8] = b"Ljava/lang/ArrayIndexOutOfBoundsException;\0";
pub const DX_ASE: &[u8] = b"Ljava/lang/ArrayStoreException;\0";
pub const DX_VERIFY: &[u8] = b"Ljava/lang/VerifyError;\0";
pub const DX_NOCLASS: &[u8] = b"Ljava/lang/NoClassDefFoundError;\0";
pub const IN_VERIFY: &[u8] = DX_VERIFY;
pub const IN_SOE: &[u8] = b"Ljava/lang/StackOverflowError;\0";
pub const IN_NPE: &[u8] = DX_NPE;
pub const IN_ARITH: &[u8] = b"Ljava/lang/ArithmeticException;\0";
pub const IN_CAST: &[u8] = b"Ljava/lang/ClassCastException;\0";
pub const IN_ABSTRACT: &[u8] = b"Ljava/lang/AbstractMethodError;\0";

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_span {
    pub p: *const u8,
    pub len: usize,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_text {
    pub p: *mut c_char,
    pub cap: usize,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dex {
    pub p: *const u8,
    pub len: usize,
    pub n: [u32; 6],
    pub off: [u32; 6],
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexstr {
    pub p: *const c_char,
    pub size: u32,
    pub utf16: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexlist {
    pub p: *const u8,
    pub n: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexproto {
    pub shorty_idx: u32,
    pub return_idx: u32,
    pub params: t_dexlist,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexfield {
    pub class_idx: u32,
    pub type_idx: u32,
    pub name_idx: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexmethod {
    pub class_idx: u32,
    pub proto_idx: u32,
    pub name_idx: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexclass {
    pub class_idx: u32,
    pub access: u32,
    pub superclass_idx: u32,
    pub interfaces: t_dexlist,
    pub source_file_idx: u32,
    pub class_data_off: u32,
    pub static_values_off: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexcdata {
    pub d: *const t_dex,
    pub pos: u32,
    pub left: [u32; 4],
    pub kind: u32,
    pub last: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexmember {
    pub kind: u32,
    pub idx: u32,
    pub access: u32,
    pub code_off: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexcode {
    pub insns: *const u8,
    pub registers: u32,
    pub ins: u32,
    pub outs: u32,
    pub tries: u32,
    pub insns_size: u32,
    pub off: u32,
    pub tries_off: u32,
    pub handlers_off: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexhit {
    pub d: *const t_dex,
    pub pos: u32,
    pub left: u32,
    pub catch_all: u32,
    pub insns_size: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexcatch {
    pub type_idx: u32,
    pub addr: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dexvalue {
    pub kind: u32,
    pub i: i64,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dinsn {
    pub op: u8,
    pub fmt: u8,
    pub len: u8,
    pub argc: u8,
    pub a: u32,
    pub b: u32,
    pub c: u32,
    pub lit: i64,
    pub idx: u32,
    pub idx2: u32,
    pub args: [u16; 5],
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dcodelimits {
    pub registers: u32,
    pub ins: u32,
    pub outs: u32,
    pub strings: u32,
    pub types: u32,
    pub fields: u32,
    pub methods: u32,
    pub protos: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dswitch {
    pub sparse: u8,
    pub count: u32,
    pub first_key: i32,
    pub keys: *const u8,
    pub targets: *const u8,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_darray {
    pub width: u32,
    pub count: u32,
    pub data: *const u8,
}

#[allow(non_camel_case_types)]
pub type t_dref = u32;

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dlimits {
    pub heap_bytes: u32,
    pub stack_words: u32,
    pub depth_max: u32,
    pub classes_max: u32,
    pub budget: u64,
}

#[allow(non_camel_case_types)]
pub type t_dnativefn = Option<unsafe extern "C" fn(*mut t_dvm, *const u32, *mut u64) -> c_int>;

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dmethod {
    pub cls: *mut t_dclass,
    pub name: *const c_char,
    pub sig: *const c_char,
    pub shorty: *const c_char,
    pub dex: *const t_dex,
    pub native: t_dnativefn,
    pub insns: t_span,
    pub code_off: u32,
    pub access: u32,
    pub slot: u32,
    pub registers: u16,
    pub ins: u16,
    pub outs: u16,
    pub tries: u16,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dheapstats {
    pub bytes: u32,
    pub objects: u32,
    pub collections: u32,
    pub peak_bytes: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dobj {
    pub cls: *mut t_dclass,
    pub data: *mut c_void,
    pub len: u32,
    pub bytes: u32,
    pub next: u32,
    pub pins: u16,
    pub kind: u8,
    pub mark: u8,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dnewobj {
    pub cls: *mut t_dclass,
    pub bytes: u64,
    pub len: u32,
    pub kind: u8,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct s_dheap {
    pub tab: *mut t_dobj,
    pub cap: u32,
    pub free_head: u32,
    pub limit: u32,
    pub oom: t_dref,
    pub st: t_dheapstats,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dfield {
    pub name: *const c_char,
    pub r#type: *const c_char,
    pub slot: u32,
    pub is_static: u8,
    pub wide: u8,
    pub is_ref: u8,
}

#[repr(C)]
pub struct t_dclass {
    pub desc: *mut c_char,
    pub super_: *mut t_dclass,
    pub iface: *mut t_dclass,
    pub elem: *mut t_dclass,
    pub ifaces: *mut *mut t_dclass,
    pub methods: *mut t_dmethod,
    pub starts: *mut *mut u8,
    pub sigs: *mut *mut c_char,
    pub fields: *mut t_dfield,
    pub statics: *mut u32,
    pub nifaces: u32,
    pub nmethods: u32,
    pub nfields: u32,
    pub nstatics: u32,
    pub mirror: t_dref,
    pub init: u32,
    pub access: u32,
    pub words: u32,
    pub pay_off: u32,
    pub pay_end: u32,
    pub width: u8,
    pub is_ref: u8,
    pub is_array: u8,
}

#[repr(C)]
pub struct s_dclasses {
    pub tab: *mut *mut t_dclass,
    pub n: u32,
    pub max: u32,
    pub core: [*mut t_dclass; DCORE_N],
    pub dex: t_dex,
    pub def_state: *mut u8,
    pub interned: *mut t_dref,
    pub natm: [*mut t_dmethod; DNAT_TABS],
    pub natn: [u32; DNAT_TABS],
    pub nnat: u32,
    pub has_dex: u32,
    pub depth: u32,
}

#[repr(C)]
pub struct t_dvm {
    pub heap: *mut s_dheap,
    pub classes: *mut s_dclasses,
    pub host: *mut c_void,
    pub stack: *mut u32,
    pub stack_words: u32,
    pub sp: u32,
    pub depth: u32,
    pub pending: t_dref,
    pub result: u64,
    pub spent: u64,
    pub lim: t_dlimits,
    pub locals: [t_dref; DVM_LOCALS_MAX as usize],
    pub nlocals: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dbuiltin {
    pub desc: *const c_char,
    pub super_desc: *const c_char,
    pub iface_desc: *const c_char,
    pub access: u32,
    pub payload_bytes: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dnative {
    pub cls: *const c_char,
    pub name: *const c_char,
    pub sig: *const c_char,
    pub access: u32,
    pub fn_: t_dnativefn,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dname {
    pub name: *const c_char,
    pub sig: *const c_char,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dstr16 {
    pub p: *const u16,
    pub n: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_darrview {
    pub data: *mut c_void,
    pub len: u32,
    pub width: u8,
    pub is_ref: u8,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dfacc {
    pub field_idx: u32,
    pub obj: t_dref,
    pub val: u64,
    pub wide: u8,
    pub is_ref: u8,
    pub put: u8,
    pub is_static: u8,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_daacc {
    pub arr: t_dref,
    pub index: i32,
    pub val: u64,
    pub kind: u8,
    pub put: u8,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dinvoke {
    pub method_idx: u32,
    pub self_: t_dref,
    pub target: *const t_dmethod,
    pub kind: u8,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct t_dnewarr {
    pub type_idx: u32,
    pub len: i32,
    pub out: t_dref,
}

#[allow(non_camel_case_types)]
pub type t_inop = unsafe extern "C" fn(*mut t_in) -> c_int;

#[repr(C)]
pub struct t_in {
    pub vm: *mut t_dvm,
    pub m: *const t_dmethod,
    pub r: *mut u32,
    pub nreg: u32,
    pub pc: u32,
    pub next: u32,
    pub fp: u32,
    pub fp0: u32,
    pub done: u32,
    pub bad: u32,
    pub i: t_dinsn,
    pub argv: [u32; 5],
}

pub(crate) fn lit_desc(s: &[u8]) -> *const c_char {
    s.as_ptr().cast()
}

const _: () = {
    use core::mem::{align_of, offset_of, size_of};
    assert!(size_of::<t_dlimits>() == 24 && align_of::<t_dlimits>() == 8);
    assert!(size_of::<t_dmethod>() == 88 && align_of::<t_dmethod>() == 8);
    assert!(size_of::<t_dvm>() == 352 && align_of::<t_dvm>() == 8);
    assert!(size_of::<t_dbuiltin>() == 32 && align_of::<t_dbuiltin>() == 8);
    assert!(size_of::<t_dnative>() == 40 && align_of::<t_dnative>() == 8);
    assert!(size_of::<t_dname>() == 16 && align_of::<t_dname>() == 8);
    assert!(size_of::<t_dstr16>() == 16 && align_of::<t_dstr16>() == 8);
    assert!(size_of::<t_darrview>() == 16 && align_of::<t_darrview>() == 8);
    assert!(size_of::<t_dheapstats>() == 16 && align_of::<t_dheapstats>() == 4);
    assert!(size_of::<t_dfacc>() == 24 && align_of::<t_dfacc>() == 8);
    assert!(size_of::<t_daacc>() == 24 && align_of::<t_daacc>() == 8);
    assert!(size_of::<t_dinvoke>() == 24 && align_of::<t_dinvoke>() == 8);
    assert!(size_of::<t_dnewarr>() == 12 && align_of::<t_dnewarr>() == 4);
    assert!(size_of::<t_dobj>() == 32 && align_of::<t_dobj>() == 8);
    assert!(size_of::<t_dnewobj>() == 24 && align_of::<t_dnewobj>() == 8);
    assert!(size_of::<s_dheap>() == 40 && align_of::<s_dheap>() == 8);
    assert!(size_of::<t_dfield>() == 24 && align_of::<t_dfield>() == 8);
    assert!(size_of::<t_dclass>() == 128 && align_of::<t_dclass>() == 8);
    assert!(size_of::<s_dclasses>() == 248 && align_of::<s_dclasses>() == 8);
    assert!(size_of::<t_in>() == 128 && align_of::<t_in>() == 8);
    assert!(size_of::<t_dinsn>() == 48 && align_of::<t_dinsn>() == 8);
    assert!(size_of::<t_span>() == 16 && align_of::<t_span>() == 8);
    assert!(size_of::<t_text>() == 16 && align_of::<t_text>() == 8);
    assert!(size_of::<t_dex>() == 64 && align_of::<t_dex>() == 8);
    assert!(size_of::<t_dexstr>() == 16 && align_of::<t_dexstr>() == 8);
    assert!(size_of::<t_dexlist>() == 16 && align_of::<t_dexlist>() == 8);
    assert!(size_of::<t_dexproto>() == 24 && align_of::<t_dexproto>() == 8);
    assert!(size_of::<t_dexfield>() == 12 && align_of::<t_dexfield>() == 4);
    assert!(size_of::<t_dexmethod>() == 12 && align_of::<t_dexmethod>() == 4);
    assert!(size_of::<t_dexclass>() == 48 && align_of::<t_dexclass>() == 8);
    assert!(size_of::<t_dexcdata>() == 40 && align_of::<t_dexcdata>() == 8);
    assert!(size_of::<t_dexmember>() == 16 && align_of::<t_dexmember>() == 4);
    assert!(size_of::<t_dexcode>() == 40 && align_of::<t_dexcode>() == 8);
    assert!(size_of::<t_dexhit>() == 24 && align_of::<t_dexhit>() == 8);
    assert!(size_of::<t_dexcatch>() == 8 && align_of::<t_dexcatch>() == 4);
    assert!(size_of::<t_dexvalue>() == 16 && align_of::<t_dexvalue>() == 8);
    assert!(size_of::<t_dswitch>() == 32 && align_of::<t_dswitch>() == 8);
    assert!(size_of::<t_darray>() == 16 && align_of::<t_darray>() == 8);
    assert!(size_of::<t_dcodelimits>() == 32 && align_of::<t_dcodelimits>() == 4);
    assert!(offset_of!(t_dvm, heap) == 0);
    assert!(offset_of!(t_dvm, stack) == 24);
    assert!(offset_of!(t_dvm, pending) == 44);
    assert!(offset_of!(t_dvm, result) == 48);
    assert!(offset_of!(t_dvm, lim) == 64);
    assert!(offset_of!(t_dvm, locals) == 88);
    assert!(offset_of!(t_dvm, nlocals) == 344);
    assert!(offset_of!(t_dmethod, native) == 40);
    assert!(offset_of!(t_dmethod, registers) == 76);
    assert!(offset_of!(t_dmethod, tries) == 82);
    assert!(offset_of!(t_dclass, super_) == 8);
    assert!(offset_of!(t_dclass, statics) == 72);
    assert!(offset_of!(t_dclass, width) == 120);
    assert!(offset_of!(s_dclasses, dex) == 56);
    assert!(offset_of!(s_dclasses, natm) == 136);
    assert!(offset_of!(s_dclasses, depth) == 240);
    assert!(offset_of!(s_dheap, st) == 24);
    assert!(offset_of!(t_dobj, pins) == 28);
    assert!(offset_of!(t_dobj, mark) == 31);
    assert!(offset_of!(t_dfield, r#type) == 8);
    assert!(offset_of!(t_dfield, is_ref) == 22);
    assert!(offset_of!(t_in, i) == 56);
    assert!(offset_of!(t_in, argv) == 104);
    assert!(offset_of!(t_dinsn, lit) == 16);
    assert!(offset_of!(t_dinsn, args) == 32);
    assert!(offset_of!(t_dswitch, count) == 4);
    assert!(offset_of!(t_dswitch, targets) == 24);
    assert!(offset_of!(t_dex, n) == 16);
    assert!(offset_of!(t_dex, off) == 40);
    assert!(offset_of!(t_dfacc, val) == 8);
    assert!(offset_of!(t_dfacc, is_static) == 19);
    assert!(offset_of!(t_daacc, kind) == 16);
    assert!(offset_of!(t_dinvoke, kind) == 16);
    assert!(offset_of!(t_dnewarr, out) == 8);
    assert!(offset_of!(t_dlimits, budget) == 16);
    assert!(offset_of!(t_dnewobj, kind) == 20);
};
