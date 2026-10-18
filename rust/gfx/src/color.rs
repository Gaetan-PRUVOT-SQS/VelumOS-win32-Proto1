use crate::Color;

pub(crate) fn argb(a: u8, r: u8, g: u8, b: u8) -> Color {
    (u32::from(a) << 24) | (u32::from(r) << 16) | (u32::from(g) << 8) | u32::from(b)
}

fn mix_lanes(s: u32, d: u32, a: u32) -> u32 {
    let t = s
        .wrapping_mul(a)
        .wrapping_add(d.wrapping_mul(255u32.wrapping_sub(a)))
        .wrapping_add(0x0080_0080);
    let t = t.wrapping_add((t >> 8) & 0x00ff_00ff);
    (t >> 8) & 0x00ff_00ff
}

pub(crate) fn blend(dst: Color, src: Color) -> Color {
    let a = src >> 24;
    if a == 255 {
        return src;
    }
    if a == 0 {
        return dst;
    }
    let src = src | 0xff00_0000;
    let ag = mix_lanes((src >> 8) & 0x00ff_00ff, (dst >> 8) & 0x00ff_00ff, a);
    let rb = mix_lanes(src & 0x00ff_00ff, dst & 0x00ff_00ff, a);
    (ag << 8) | rb
}

fn lerp_lanes(a: u32, b: u32, t: u32) -> u32 {
    let v = a
        .wrapping_mul(256u32.wrapping_sub(t))
        .wrapping_add(b.wrapping_mul(t))
        .wrapping_add(0x0080_0080);
    (v >> 8) & 0x00ff_00ff
}

pub(crate) fn lerp(a: Color, b: Color, mut t256: u32) -> Color {
    if t256 > 256 {
        t256 = 256;
    }
    let ag = lerp_lanes((a >> 8) & 0x00ff_00ff, (b >> 8) & 0x00ff_00ff, t256);
    let rb = lerp_lanes(a & 0x00ff_00ff, b & 0x00ff_00ff, t256);
    (ag << 8) | rb
}
