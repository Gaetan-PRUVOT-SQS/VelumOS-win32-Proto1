use core::ffi::c_int;

use crate::{
    E_INVAL, E_OVERFLOW, E_RANGE, SET_FILE_MAX, SET_LAYOUT_QWERTY, SET_MODES, SET_THEME_AUTO,
    modes, t_settings,
};

const THEME: [&[u8]; 3] = [b"clair", b"sombre", b"auto"];
const TRANSP: [&[u8]; 2] = [b"normale", b"reduite"];
const LAYOUT: [&[u8]; 2] = [b"azerty", b"qwerty"];
const MODE_NAME: [&[u8]; 5] = [
    b"1024x768",
    b"1280x720",
    b"1366x768",
    b"1600x900",
    b"1920x1080",
];

fn lookup(names: &[&[u8]], val: &[u8]) -> Option<u32> {
    let mut i = 0u32;
    for name in names {
        if *name == val {
            return Some(i);
        }
        i = i.checked_add(1)?;
    }
    None
}

fn apply(s: &mut t_settings, key: &[u8], val: &[u8]) -> bool {
    let idx = if key == b"theme" {
        lookup(&THEME, val)
    } else if key == b"transparence" {
        lookup(&TRANSP, val)
    } else if key == b"clavier" {
        lookup(&LAYOUT, val)
    } else if key == b"resolution" {
        lookup(&MODE_NAME, val)
    } else {
        return false;
    };
    let Some(idx) = idx else {
        return false;
    };
    if key == b"theme" {
        s.theme = idx;
    } else if key == b"transparence" {
        s.reduced = idx;
    } else if key == b"clavier" {
        s.layout = idx;
    } else {
        s.mode = idx;
    }
    true
}

fn parse_line(s: &mut t_settings, line: &[u8]) -> bool {
    let line = match line.split_last() {
        Some((b'\r', head)) => head,
        _ => line,
    };
    let Some(eq) = line.iter().position(|&b| b == b'=') else {
        return false;
    };
    let key = &line[..eq];
    let raw = &line[eq + 1..];
    if raw.is_empty() || raw.len() >= 16 {
        return false;
    }
    let val = match raw.iter().position(|&b| b == 0) {
        Some(z) => &raw[..z],
        None => raw,
    };
    apply(s, key, val)
}

pub(crate) fn parse(text: &[u8], s: &mut t_settings) -> c_int {
    modes::apply_default(s);
    if text.len() > SET_FILE_MAX {
        return E_RANGE;
    }
    let mut pos = 0usize;
    let mut ok = 0u32;
    while pos < text.len() {
        let rest = &text[pos..];
        let n = rest.iter().position(|&b| b == b'\n').unwrap_or(rest.len());
        if parse_line(s, &rest[..n]) {
            ok = match ok.checked_add(1) {
                Some(v) => v,
                None => return E_OVERFLOW,
            };
        }
        pos = match pos.checked_add(n).and_then(|p| p.checked_add(1)) {
            Some(v) => v,
            None => return E_OVERFLOW,
        };
    }
    c_int::try_from(ok).unwrap_or(E_OVERFLOW)
}

fn add(out: &mut [u8], pos: &mut usize, part: &[u8]) -> bool {
    let Some(end) = pos.checked_add(part.len()) else {
        return false;
    };
    if end >= out.len() {
        return false;
    }
    out[*pos..end].copy_from_slice(part);
    *pos = end;
    out[*pos] = 0;
    true
}

pub(crate) fn format(s: &t_settings, out: &mut [u8]) -> c_int {
    if s.theme > SET_THEME_AUTO
        || s.reduced > 1
        || s.layout > SET_LAYOUT_QWERTY
        || s.mode >= SET_MODES
    {
        return E_INVAL;
    }
    if out.is_empty() {
        return E_INVAL;
    }
    let theme = THEME[s.theme as usize];
    let transp = TRANSP[s.reduced as usize];
    let layout = LAYOUT[s.layout as usize];
    let mode = MODE_NAME[s.mode as usize];
    out[0] = 0;
    let mut pos = 0usize;
    if !add(out, &mut pos, b"theme=")
        || !add(out, &mut pos, theme)
        || !add(out, &mut pos, b"\ntransparence=")
        || !add(out, &mut pos, transp)
        || !add(out, &mut pos, b"\nclavier=")
        || !add(out, &mut pos, layout)
        || !add(out, &mut pos, b"\nresolution=")
        || !add(out, &mut pos, mode)
        || !add(out, &mut pos, b"\n")
    {
        return E_RANGE;
    }
    c_int::try_from(pos).unwrap_or(E_OVERFLOW)
}
