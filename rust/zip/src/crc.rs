const CRC4: [u32; 16] = [
    0x0000_0000,
    0x1db7_1064,
    0x3b6e_20c8,
    0x26d9_30ac,
    0x76dc_4190,
    0x6b6b_51f4,
    0x4db2_6158,
    0x5005_713c,
    0xedb8_8320,
    0xf00f_9344,
    0xd6d6_a3e8,
    0xcb61_b38c,
    0x9b64_c2b0,
    0x86d3_d2d4,
    0xa00a_e278,
    0xbdbd_f21c,
];

pub(crate) fn crc32(crc: u32, data: &[u8]) -> u32 {
    let mut c = !crc;
    for &b in data {
        c ^= u32::from(b);
        c = (c >> 4) ^ CRC4[(c & 15) as usize];
        c = (c >> 4) ^ CRC4[(c & 15) as usize];
    }
    !c
}
