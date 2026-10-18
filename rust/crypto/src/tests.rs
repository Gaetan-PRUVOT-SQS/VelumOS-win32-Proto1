use core::ffi::{c_int, c_void};

use super::{
    E_INVAL, E_OK, PBKDF2_ITERS_MAX, TChacha20, THmac, TPbkdf2, TSha256, chacha20_block,
    chacha20_init, ct_equal, hmac_final, hmac_prepare, hmac_sha256, hmac_update, pbkdf2_sha256,
    pbkdf2_validate, secure_zero, sha256, sha256_final, sha256_init, sha256_update,
};

const SHA_VIDE: &str = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
const SHA_ABC: &str = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
const CHACHA_232: &str = "10f1e7e4d13b5915500fdd1fa32071c4c7d1f4c733c068030422aa9ac3d46c4ed2826446079faa0914c2d705d98b02a2b5129cd1de164eb9cbd083e8a2503c4e";

fn unhex(s: &str) -> Vec<u8> {
    if s == "-" {
        return Vec::new();
    }
    assert!(s.len().is_multiple_of(2), "hex impair");
    let b = s.as_bytes();
    let mut out = Vec::with_capacity(s.len() / 2);
    let mut i = 0usize;
    while i < b.len() {
        out.push((nibble(b[i]) << 4) | nibble(b[i + 1]));
        i += 2;
    }
    out
}

fn nibble(c: u8) -> u8 {
    match c {
        b'0'..=b'9' => c - b'0',
        b'a'..=b'f' => c - b'a' + 10,
        _ => panic!("nibble {c}"),
    }
}

fn hash(data: &[u8]) -> [u8; 32] {
    let mut out = [0u8; 32];
    // SAFETY: data est un slice, out a 32 octets. Longueur nulle : data n'est pas lu.
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        sha256(data.as_ptr().cast(), data.len(), out.as_mut_ptr())
    };
    out
}

fn hash_ptr(data: *const u8, n: usize) -> [u8; 32] {
    let mut out = [0u8; 32];
    // SAFETY: si n vaut 0, data peut être nul. Sinon l'appelant garantit n octets. out a 32 octets.
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        sha256(data.cast(), n, out.as_mut_ptr())
    };
    out
}

fn hash_chunks(data: &[u8], chunk: usize) -> [u8; 32] {
    let mut c = core::mem::MaybeUninit::<TSha256>::uninit();
    // SAFETY: tampon non initialisé de la taille d'un t_sha256. sha256_init ne lit pas l'ancien contenu.
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        sha256_init(c.as_mut_ptr())
    };
    let mut off = 0usize;
    while off < data.len() {
        let n = chunk.min(data.len() - off);
        // SAFETY: le contexte est initialisé, le sous-slice couvre n octets.
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            sha256_update(c.as_mut_ptr(), data[off..].as_ptr().cast(), n)
        };
        off += n;
    }
    let mut out = [0u8; 32];
    // SAFETY: contexte initialisé, out a 32 octets.
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        sha256_final(c.as_mut_ptr(), out.as_mut_ptr())
    };
    out
}

fn ctx_bytes(c: &TSha256) -> &[u8] {
    // SAFETY: c est initialisé, on lit les 112 octets y compris le remplissage.
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        core::slice::from_raw_parts((c as *const TSha256).cast::<u8>(), 112)
    }
}

fn hmac(key: &[u8], msg: &[u8]) -> [u8; 32] {
    let k = TPbkdf2 {
        pw: key.as_ptr(),
        pwlen: key.len(),
        salt: core::ptr::null(),
        saltlen: 0,
        iters: 1,
    };
    let mut out = [0u8; 32];
    // SAFETY: k est un t_pbkdf2 local, msg un slice, out a 32 octets.
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.

        hmac_sha256(
            &raw const k,
            msg.as_ptr().cast(),
            msg.len(),
            out.as_mut_ptr(),
        )
    };
    out
}

fn derive(
    pw: *const u8,
    pwlen: usize,
    salt: *const u8,
    saltlen: usize,
    iters: u32,
    out: &mut [u8],
) -> c_int {
    let p = TPbkdf2 {
        pw,
        pwlen,
        salt,
        saltlen,
        iters,
    };
    // SAFETY: p est local. out couvre out.len() octets, ou le pointeur n'est pas déréférencé si le code refuse.
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        pbkdf2_sha256(&raw const p, out.as_mut_ptr(), out.len())
    }
}

fn chacha_block(key: &[u8; 32], nonce: &[u8; 12], counter: u32) -> ([u8; 64], u32) {
    let mut c = core::mem::MaybeUninit::<TChacha20>::uninit();
    // SAFETY: état non initialisé, clé 32 octets, nonce 12. chacha20_init écrit sans lire l'ancien état.
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        chacha20_init(c.as_mut_ptr(), key.as_ptr(), nonce.as_ptr(), counter)
    };
    let mut out = [0u8; 64];
    // SAFETY: l'état vient d'être écrit, out a 64 octets.
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        chacha20_block(c.as_mut_ptr(), out.as_mut_ptr())
    };
    // SAFETY: init puis un bloc ont écrit les 16 mots.
    let ctx = unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        c.assume_init()
    };
    (out, ctx.state[12])
}

fn vec_path(name: &str) -> String {
    format!(
        "{}/../../tests/host/a09/vectors/{name}",
        env!("CARGO_MANIFEST_DIR")
    )
}

fn vec_lines(name: &str) -> Vec<Vec<String>> {
    let path = vec_path(name);
    let text = match std::fs::read_to_string(&path) {
        Ok(s) => s,
        Err(e) => panic!("lecture {path}: {e}"),
    };
    let mut rows = Vec::new();
    for line in text.lines() {
        let line = line.trim();
        if line.is_empty() || line.starts_with('#') {
            continue;
        }
        rows.push(line.split_whitespace().map(str::to_string).collect());
    }
    rows
}

#[test]
fn tailles_abi() {
    assert_eq!(core::mem::size_of::<usize>(), 8);
    assert_eq!(core::mem::size_of::<TSha256>(), 112);
    assert_eq!(core::mem::align_of::<TSha256>(), 8);
    assert_eq!(core::mem::size_of::<THmac>(), 224);
    assert_eq!(core::mem::align_of::<THmac>(), 8);
    assert_eq!(core::mem::size_of::<TPbkdf2>(), 40);
    assert_eq!(core::mem::align_of::<TPbkdf2>(), 8);
    assert_eq!(core::mem::size_of::<TChacha20>(), 64);
    assert_eq!(core::mem::align_of::<TChacha20>(), 4);
    assert_eq!(core::mem::offset_of!(TSha256, total), 32);
    assert_eq!(core::mem::offset_of!(TSha256, buf), 40);
    assert_eq!(core::mem::offset_of!(TSha256, fill), 104);
    assert_eq!(core::mem::offset_of!(THmac, outer), 112);
    assert_eq!(core::mem::offset_of!(TPbkdf2, iters), 32);
}

#[test]
fn sha256_message_vide() {
    assert_eq!(hash(b""), unhex(SHA_VIDE).as_slice());
}

#[test]
fn sha256_abc() {
    assert_eq!(hash(b"abc"), unhex(SHA_ABC).as_slice());
}

#[test]
fn sha256_pointeur_nul_longueur_zero() {
    assert_eq!(hash_ptr(core::ptr::null(), 0), unhex(SHA_VIDE).as_slice());
}

#[test]
fn sha256_pointeur_nul_longueur_positive_n_ecrit_rien() {
    let mut out = [0x11u8; 32];
    // SAFETY: data nul et n > 0 : le contrat n'écrit pas out.
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        sha256(core::ptr::null(), 4, out.as_mut_ptr())
    };
    assert_eq!(out, [0x11u8; 32]);
}

#[test]
fn sha256_update_vide_et_nul() {
    assert_eq!(hash_chunks(b"abc", 3), unhex(SHA_ABC).as_slice());
    let mut c = core::mem::MaybeUninit::<TSha256>::uninit();
    // SAFETY: contexte non initialisé, init l'écrit entièrement.
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        sha256_init(c.as_mut_ptr())
    };
    // SAFETY: n vaut 0, data nul ou non n'est pas lu.
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        sha256_update(c.as_mut_ptr(), core::ptr::null(), 0)
    };
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        sha256_update(c.as_mut_ptr(), b"abc".as_ptr().cast(), 0)
    };
    // SAFETY: data nul et n > 0 : le contexte n'est pas modifié.
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        sha256_update(c.as_mut_ptr(), core::ptr::null(), 5)
    };
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        sha256_update(c.as_mut_ptr(), b"abc".as_ptr().cast(), 3)
    };
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        sha256_update(c.as_mut_ptr(), core::ptr::null(), 0)
    };
    let mut out = [0u8; 32];
    // SAFETY: contexte initialisé, out a 32 octets.
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        sha256_final(c.as_mut_ptr(), out.as_mut_ptr())
    };
    assert_eq!(out, unhex(SHA_ABC).as_slice());
}

#[test]
fn sha256_final_nul_ne_consomme_pas() {
    let mut c = core::mem::MaybeUninit::<TSha256>::uninit();
    // SAFETY: init écrit le contexte, update lit « abc », final nul n'écrit rien.
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        sha256_init(c.as_mut_ptr())
    };
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        sha256_update(c.as_mut_ptr(), b"abc".as_ptr().cast(), 3)
    };
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        sha256_final(c.as_mut_ptr(), core::ptr::null_mut())
    };
    let mut out = [0u8; 32];
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        sha256_final(c.as_mut_ptr(), out.as_mut_ptr())
    };
    assert_eq!(out, unhex(SHA_ABC).as_slice());
}

#[test]
fn sha256_final_efface_le_contexte() {
    let mut c = core::mem::MaybeUninit::<TSha256>::uninit();
    // SAFETY: init, update de « secret », final écrit out et efface le contexte.
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        sha256_init(c.as_mut_ptr())
    };
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        sha256_update(c.as_mut_ptr(), b"secret".as_ptr().cast(), 6)
    };
    let mut out = [0u8; 32];
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        sha256_final(c.as_mut_ptr(), out.as_mut_ptr())
    };
    // SAFETY: final a écrit les 112 octets.
    let ctx = unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        c.assume_init()
    };
    assert_eq!(ctx_bytes(&ctx), &[0u8; 112]);
    assert_ne!(out, [0u8; 32]);
}

#[test]
fn sha256_reutilisation_apres_init() {
    let a = hash_chunks(b"premier message", 15);
    let b = hash_chunks(b"premier message", 4);
    assert_eq!(a, b);
    assert_eq!(a, hash(b"premier message"));
}

#[test]
fn sha256_abc_octet_par_octet() {
    assert_eq!(hash_chunks(b"abc", 1), unhex(SHA_ABC).as_slice());
}

#[test]
fn sha256_vecteurs_fichier() {
    let rows = vec_lines("sha256.txt");
    assert!(rows.len() >= 25, "lignes {}", rows.len());
    for row in &rows {
        assert_eq!(row.len(), 4, "{row:?}");
        let rep: usize = row[1].parse().expect("repetitions");
        let unit = unhex(&row[2]);
        let mut msg = Vec::new();
        let mut i = 0usize;
        while i < rep {
            msg.extend_from_slice(&unit);
            i += 1;
        }
        let got = hash(&msg);
        assert_eq!(got, unhex(&row[3]).as_slice(), "{}", row[0]);
    }
}

#[test]
fn sha256_million_de_a_par_octet() {
    let mut c = core::mem::MaybeUninit::<TSha256>::uninit();
    // SAFETY: init puis un million de mises à jour d'un octet, final vers 32 octets.
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        sha256_init(c.as_mut_ptr())
    };
    let mut i = 0u32;
    while i < 1_000_000 {
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            sha256_update(c.as_mut_ptr(), b"a".as_ptr().cast(), 1)
        };
        i += 1;
    }
    let mut out = [0u8; 32];
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        sha256_final(c.as_mut_ptr(), out.as_mut_ptr())
    };
    assert_eq!(
        out,
        unhex("cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0").as_slice()
    );
}

#[test]
fn sha256_decoupage_equivalent() {
    let msg = fill_pattern(300, 20_261_005);
    for len in 0..=300 {
        let whole = hash(&msg[..len]);
        for chunk in [1usize, 2, 3, 55, 56, 63, 64, 65, 127, 128, 200] {
            assert_eq!(
                hash_chunks(&msg[..len], chunk),
                whole,
                "len {len} morceau {chunk}"
            );
        }
    }
}

fn fill_pattern(n: usize, mut seed: u32) -> Vec<u8> {
    let mut buf = vec![0u8; n];
    for b in &mut buf {
        seed = seed.wrapping_mul(1_664_525).wrapping_add(1_013_904_223);
        *b = (seed >> 24) as u8;
    }
    buf
}

#[test]
fn hmac_rfc4231_et_fichier() {
    let rows = vec_lines("hmac_sha256.txt");
    assert!(rows.len() >= 17, "lignes {}", rows.len());
    for row in &rows {
        assert_eq!(row.len(), 4, "{row:?}");
        let key = unhex(&row[1]);
        let msg = unhex(&row[2]);
        let want = unhex(&row[3]);
        let got = hmac(&key, &msg);
        assert_eq!(&got[..want.len()], want.as_slice(), "{}", row[0]);
    }
}

#[test]
fn hmac_rfc4231_cas5_tronque() {
    let key = unhex("0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c");
    let msg = b"Test With Truncation";
    let want = unhex("a3b6167473100ee06e0c796c2955552b");
    let got = hmac(&key, msg);
    assert_eq!(want.len(), 16);
    assert_eq!(&got[..16], want.as_slice());
}

#[test]
fn hmac_cle_nulle_equivaut_vide() {
    let mut vide = [0u8; 32];
    let mut absente = [0u8; 32];
    let mut incoherent = [0u8; 32];
    let k_vide = TPbkdf2 {
        pw: core::ptr::null(),
        pwlen: 0,
        salt: core::ptr::null(),
        saltlen: 0,
        iters: 0,
    };
    let k_long = TPbkdf2 {
        pw: core::ptr::null(),
        pwlen: 5,
        salt: core::ptr::null(),
        saltlen: 0,
        iters: 0,
    };
    let msg = b"message";
    // SAFETY: clé vide, clé absente, ou pw nul avec pwlen incohérent. out a 32 octets.
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.

        hmac_sha256(
            &raw const k_vide,
            msg.as_ptr().cast(),
            msg.len(),
            vide.as_mut_ptr(),
        )
    };
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.

        hmac_sha256(
            core::ptr::null(),
            msg.as_ptr().cast(),
            msg.len(),
            absente.as_mut_ptr(),
        )
    };
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.

        hmac_sha256(
            &raw const k_long,
            msg.as_ptr().cast(),
            msg.len(),
            incoherent.as_mut_ptr(),
        )
    };
    assert_eq!(vide, absente);
    assert_eq!(vide, incoherent);
    assert_eq!(vide, hmac(&[], msg));
}

#[test]
fn hmac_message_vide_et_sortie_nulle() {
    let out = [0x5au8; 32];
    let k = TPbkdf2 {
        pw: b"k".as_ptr(),
        pwlen: 1,
        salt: core::ptr::null(),
        saltlen: 0,
        iters: 1,
    };
    // SAFETY: o nul, rien n'est écrit dans out (qui n'est pas passé).
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        hmac_sha256(&raw const k, core::ptr::null(), 0, core::ptr::null_mut())
    };
    assert_eq!(out, [0x5au8; 32]);
    assert_eq!(
        hmac(b"k", b""),
        unhex("8bb990c40a7d61cb97597a942125025be50ac8beb74436e3735b98893a7f6620").as_slice()
    );
}

#[test]
fn hmac_decoupage_et_modele() {
    let key = fill_pattern(70, 4231);
    let msg = fill_pattern(400, 8018);
    let whole = hmac(&key, &msg);
    for chunk in 1..=200 {
        let mut h = core::mem::MaybeUninit::<THmac>::uninit();
        // SAFETY: prepare écrit le modèle, update lit des morceaux, final écrit 32 octets.
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            hmac_prepare(h.as_mut_ptr(), key.as_ptr(), key.len())
        };
        let mut off = 0usize;
        while off < msg.len() {
            let n = chunk.min(msg.len() - off);
            unsafe {
                // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
                hmac_update(h.as_mut_ptr(), msg[off..].as_ptr().cast(), n)
            };
            off += n;
        }
        let mut parts = [0u8; 32];
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            hmac_final(h.as_mut_ptr(), parts.as_mut_ptr())
        };
        assert_eq!(parts, whole, "morceau {chunk}");
    }
    let mut tmpl = core::mem::MaybeUninit::<THmac>::uninit();
    // SAFETY: prepare écrit tmpl. Les copies sont des HMAC préparés. final écrit 32 octets.
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        hmac_prepare(tmpl.as_mut_ptr(), b"cle".as_ptr(), 3)
    };
    let src = unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        tmpl.assume_init()
    };
    let mut a = src;
    let mut b = src;
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        hmac_update(&raw mut a, b"un".as_ptr().cast(), 2)
    };
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        hmac_update(&raw mut b, b"un".as_ptr().cast(), 2)
    };
    let mut out_a = [0u8; 32];
    let mut out_b = [0u8; 32];
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        hmac_final(&raw mut a, out_a.as_mut_ptr())
    };
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        hmac_final(&raw mut b, out_b.as_mut_ptr())
    };
    assert_eq!(out_a, out_b);
    assert_eq!(out_a, hmac(b"cle", b"un"));
}

#[test]
fn pbkdf2_vecteurs_fichier() {
    let rows = vec_lines("pbkdf2_sha256.txt");
    assert!(rows.len() >= 17, "lignes {}", rows.len());
    for row in &rows {
        assert_eq!(row.len(), 5, "{row:?}");
        let iters: u32 = row[1].parse().expect("iters");
        let pw = unhex(&row[2]);
        let salt = unhex(&row[3]);
        let want = unhex(&row[4]);
        let mut out = vec![0xa5u8; want.len() + 8];
        let rc = derive(
            pw.as_ptr(),
            pw.len(),
            salt.as_ptr(),
            salt.len(),
            iters,
            &mut out[..want.len()],
        );
        assert_eq!(rc, E_OK, "{}", row[0]);
        assert_eq!(&out[..want.len()], want.as_slice(), "{}", row[0]);
        assert_eq!(&out[want.len()..], &[0xa5u8; 8], "garde {}", row[0]);
    }
}

#[test]
fn pbkdf2_troncature_prefixe() {
    let pw = b"secret";
    let salt = b"sel";
    let mut longue = [0u8; 100];
    let rc = derive(
        pw.as_ptr(),
        pw.len(),
        salt.as_ptr(),
        salt.len(),
        3,
        &mut longue,
    );
    assert_eq!(rc, E_OK);
    let want100 = unhex(
        "a5566bd22965aa5fb49db42d7c9fdd5ee13da561f04fac68610fec8dcebfdb18d7dfe0a61c3ce442e6f8bc7055a69cd56e06f1beb447038022e0a8a4c902311fd524a5645305b568e2d2fb72148a17d99660edc8fb6123e80257776be474a5b03a76616d",
    );
    assert_eq!(longue, want100.as_slice());
    for len in [1usize, 31, 32, 33, 64, 65] {
        let mut courte = vec![0u8; len];
        let rc = derive(
            pw.as_ptr(),
            pw.len(),
            salt.as_ptr(),
            salt.len(),
            3,
            &mut courte,
        );
        assert_eq!(rc, E_OK, "len {len}");
        assert_eq!(courte.as_slice(), &longue[..len], "prefixe {len}");
    }
}

#[test]
fn pbkdf2_vide_et_pointeurs_nuls() {
    let salt = b"sel";
    let want_pw = unhex("8286c700c8fc418075b08ee1ce918a98025accab666204b3bc1baea8887bc5bc");
    let mut out = [0u8; 32];
    let rc = derive(core::ptr::null(), 0, salt.as_ptr(), salt.len(), 5, &mut out);
    assert_eq!(rc, E_OK);
    assert_eq!(out, want_pw.as_slice());
    let pw = b"secret";
    let want_salt = unhex("f5c8cffb84c47f18462fa2fc7b2f6aa84bf905a098f0105f2e9aca2a9998a1d9");
    let rc = derive(pw.as_ptr(), pw.len(), core::ptr::null(), 0, 5, &mut out);
    assert_eq!(rc, E_OK);
    assert_eq!(out, want_salt.as_slice());
    let p = TPbkdf2 {
        pw: b"pw".as_ptr(),
        pwlen: 2,
        salt: b"sel".as_ptr(),
        saltlen: 3,
        iters: 2,
    };
    // SAFETY: paramètres lus seulement. Les refus n'écrivent pas out.
    let mut sink = [0u8; 32];
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            pbkdf2_sha256(core::ptr::null(), sink.as_mut_ptr(), 32)
        },
        E_INVAL
    );
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            pbkdf2_sha256(&raw const p, core::ptr::null_mut(), 32)
        },
        E_INVAL
    );
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            pbkdf2_sha256(&raw const p, sink.as_mut_ptr(), 0)
        },
        E_INVAL
    );
    assert_eq!(sink, [0u8; 32]);
    let bad_pw = TPbkdf2 {
        pw: core::ptr::null(),
        pwlen: 2,
        salt: b"sel".as_ptr(),
        saltlen: 3,
        iters: 2,
    };
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            pbkdf2_sha256(&raw const bad_pw, sink.as_mut_ptr(), 32)
        },
        E_INVAL
    );
    let bad_salt = TPbkdf2 {
        pw: b"pw".as_ptr(),
        pwlen: 2,
        salt: core::ptr::null(),
        saltlen: 3,
        iters: 2,
    };
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            pbkdf2_sha256(&raw const bad_salt, sink.as_mut_ptr(), 32)
        },
        E_INVAL
    );
    assert_eq!(sink, [0u8; 32]);
}

#[test]
fn pbkdf2_iters_et_dklen_limites() {
    let p = TPbkdf2 {
        pw: b"pw".as_ptr(),
        pwlen: 2,
        salt: b"sel".as_ptr(),
        saltlen: 3,
        iters: 0,
    };
    // SAFETY: validate ne lit que les champs du t_pbkdf2, pas les octets du mot de passe.
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            pbkdf2_validate(&raw const p, 32)
        },
        E_INVAL
    );
    let mut ok = p;
    ok.iters = 1;
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            pbkdf2_validate(&raw const ok, 32)
        },
        E_OK
    );
    ok.iters = PBKDF2_ITERS_MAX;
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            pbkdf2_validate(&raw const ok, 32)
        },
        E_OK
    );
    ok.iters = PBKDF2_ITERS_MAX + 1;
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            pbkdf2_validate(&raw const ok, 32)
        },
        E_INVAL
    );
    ok.iters = u32::MAX;
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            pbkdf2_validate(&raw const ok, 32)
        },
        E_INVAL
    );
    ok.iters = 1;
    let max_len = 0xffff_ffffusize * 32;
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            pbkdf2_validate(&raw const ok, max_len)
        },
        E_OK
    );
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            pbkdf2_validate(&raw const ok, max_len + 1)
        },
        E_INVAL
    );
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            pbkdf2_validate(&raw const ok, usize::MAX)
        },
        E_INVAL
    );
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            pbkdf2_validate(&raw const ok, 1)
        },
        E_OK
    );
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            pbkdf2_validate(core::ptr::null(), 32)
        },
        E_INVAL
    );
}

#[test]
fn chacha20_rfc8439_2_3_2() {
    let key: [u8; 32] = unhex("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f")
        .try_into()
        .expect("cle");
    let nonce: [u8; 12] = unhex("000000090000004a00000000").try_into().expect("nonce");
    let (out, counter) = chacha_block(&key, &nonce, 1);
    assert_eq!(out, unhex(CHACHA_232).as_slice());
    assert_eq!(counter, 2);
}

#[test]
fn chacha20_vecteurs_fichier() {
    let rows = vec_lines("chacha20_block.txt");
    assert!(rows.len() >= 7, "lignes {}", rows.len());
    for row in &rows {
        assert_eq!(row.len(), 5, "{row:?}");
        let key: [u8; 32] = unhex(&row[1]).try_into().expect("cle");
        let counter: u32 = row[2].parse().expect("compteur");
        let nonce: [u8; 12] = unhex(&row[3]).try_into().expect("nonce");
        let (out, after) = chacha_block(&key, &nonce, counter);
        assert_eq!(out, unhex(&row[4]).as_slice(), "{}", row[0]);
        assert_eq!(after, counter.wrapping_add(1), "{}", row[0]);
    }
}

#[test]
fn chacha20_compteur_max_boucle() {
    let key: [u8; 32] = unhex("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f")
        .try_into()
        .expect("cle");
    let nonce: [u8; 12] = unhex("000000090000004a00000000").try_into().expect("nonce");
    let mut c = core::mem::MaybeUninit::<TChacha20>::uninit();
    // SAFETY: init au compteur max, deux blocs vers des tampons de 64 octets.
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        chacha20_init(c.as_mut_ptr(), key.as_ptr(), nonce.as_ptr(), u32::MAX)
    };
    let mut premier = [0u8; 64];
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        chacha20_block(c.as_mut_ptr(), premier.as_mut_ptr())
    };
    // SAFETY: un bloc a écrit l'état, le compteur a tourné.
    let ctx = unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        c.assume_init()
    };
    assert_eq!(ctx.state[12], 0);
    let mut second = [0u8; 64];
    let mut suivi = ctx;
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        chacha20_block(&raw mut suivi, second.as_mut_ptr())
    };
    let (frais, apres) = chacha_block(&key, &nonce, 0);
    assert_eq!(second, frais);
    assert_eq!(apres, 1);
    assert_ne!(premier, frais);
}

#[test]
fn chacha20_pointeur_nul_n_ecrit_rien() {
    let mut out = [0x3cu8; 64];
    let key = [0u8; 32];
    let nonce = [0u8; 12];
    // SAFETY: contexte nul, rien n'est lu ni écrit.
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        chacha20_init(core::ptr::null_mut(), key.as_ptr(), nonce.as_ptr(), 1)
    };
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        chacha20_block(core::ptr::null_mut(), out.as_mut_ptr())
    };
    assert_eq!(out, [0x3cu8; 64]);
    let mut c = core::mem::MaybeUninit::<TChacha20>::uninit();
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        chacha20_init(c.as_mut_ptr(), key.as_ptr(), nonce.as_ptr(), 1)
    };
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        chacha20_block(c.as_mut_ptr(), core::ptr::null_mut())
    };
    // SAFETY: le bloc nul n'a pas avancé le compteur, l'état reste celui de l'init.
    let ctx = unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        c.assume_init()
    };
    assert_eq!(ctx.state[12], 1);
}

#[test]
fn ct_equal_nul_vide_et_dernier_octet() {
    let a = [1u8, 2, 3, 4, 5, 6, 7, 8];
    let mut b = a;
    // SAFETY: n vaut 0, ou les deux tampons couvrent la longueur demandée, ou un pointeur est nul.
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            ct_equal(a.as_ptr().cast(), b.as_ptr().cast(), a.len())
        },
        1
    );
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            ct_equal(a.as_ptr().cast(), b.as_ptr().cast(), 0)
        },
        1
    );
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            ct_equal(core::ptr::null(), core::ptr::null(), 0)
        },
        1
    );
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            ct_equal(core::ptr::null(), b.as_ptr().cast(), 4)
        },
        0
    );
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            ct_equal(a.as_ptr().cast(), core::ptr::null(), 4)
        },
        0
    );
    b[7] ^= 0x80;
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            ct_equal(a.as_ptr().cast(), b.as_ptr().cast(), a.len())
        },
        0
    );
    b[7] ^= 0x80;
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            ct_equal(a.as_ptr().cast(), b.as_ptr().cast(), a.len())
        },
        1
    );
    let mut diff = [0xffu8; 8];
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            ct_equal(a.as_ptr().cast(), diff.as_ptr().cast(), diff.len())
        },
        0
    );
    diff = [0u8; 8];
    diff[0] = 0x01;
    let z = [0u8; 8];
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            ct_equal(z.as_ptr().cast(), diff.as_ptr().cast(), z.len())
        },
        0
    );
    assert_eq!(
        unsafe {
            // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
            ct_equal(a.as_ptr().cast(), b.as_ptr().cast(), 3)
        },
        1
    );
}

#[test]
fn secure_zero_bornes_et_nul() {
    let mut buf = [0xaau8; 16];
    // SAFETY: la zone [4, 12) est dans buf. Taille nulle et pointeur nul ne lisent rien.
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        secure_zero(buf[4..].as_mut_ptr().cast(), 8)
    };
    assert_eq!(buf[3], 0xaa);
    assert_eq!(buf[4], 0);
    assert_eq!(buf[11], 0);
    assert_eq!(buf[12], 0xaa);
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        secure_zero(buf.as_mut_ptr().cast(), 0)
    };
    assert_eq!(buf[0], 0xaa);
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        secure_zero(core::ptr::null_mut::<c_void>(), 8)
    };
    unsafe {
        // SAFETY: fixtures du test, ou pointeur nul prévu par le contrat.
        secure_zero(buf.as_mut_ptr().cast(), buf.len())
    };
    assert_eq!(buf, [0u8; 16]);
}
