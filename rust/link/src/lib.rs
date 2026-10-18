#![cfg_attr(not(test), no_std)]
#![deny(unsafe_op_in_unsafe_fn)]

extern crate velum_panic;

#[cfg(feature = "apk")]
extern crate velum_apk;
#[cfg(feature = "axml")]
extern crate velum_axml;
#[cfg(feature = "crypto")]
extern crate velum_crypto;
#[cfg(feature = "dex")]
extern crate velum_dex;
#[cfg(feature = "dexcode")]
extern crate velum_dexcode;
#[cfg(feature = "font")]
extern crate velum_font;
#[cfg(feature = "gfx")]
extern crate velum_gfx;
#[cfg(feature = "libk")]
extern crate velum_libk;
#[cfg(feature = "notif")]
extern crate velum_notif;
#[cfg(feature = "pm")]
extern crate velum_pm;
#[cfg(feature = "rsa")]
extern crate velum_rsa;
#[cfg(feature = "settings")]
extern crate velum_settings;
#[cfg(feature = "wm")]
extern crate velum_wm;
#[cfg(feature = "zip")]
extern crate velum_zip;

#[cfg(feature = "apk")]
mod keep_apk;
#[cfg(feature = "axml")]
mod keep_axml;
#[cfg(feature = "crypto")]
mod keep_crypto;
#[cfg(feature = "dex")]
mod keep_dex;
#[cfg(feature = "dexcode")]
mod keep_dexcode;
#[cfg(feature = "font")]
mod keep_font;
#[cfg(feature = "gfx")]
mod keep_gfx;
#[cfg(feature = "libk")]
mod keep_libk;
#[cfg(feature = "notif")]
mod keep_notif;
#[cfg(feature = "pm")]
mod keep_pm;
#[cfg(feature = "rsa")]
mod keep_rsa;
#[cfg(feature = "settings")]
mod keep_settings;
#[cfg(feature = "wm")]
mod keep_wm;
#[cfg(feature = "zip")]
mod keep_zip;
