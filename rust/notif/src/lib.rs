#![cfg_attr(not(test), no_std)]
#![deny(unsafe_op_in_unsafe_fn)]

mod abi;
#[cfg(any(test, velum_user))]
mod client;
mod store;
mod text;
mod wire;

pub use abi::*;
#[cfg(any(test, velum_user))]
pub use client::{
    notif_client_clear, notif_client_close, notif_client_count, notif_client_dismiss,
    notif_client_toggle, notif_post,
};
pub use store::{
    notif_store_clear, notif_store_count, notif_store_dismiss, notif_store_get, notif_store_init,
    notif_store_post,
};
pub use text::{notif_age_text, notif_badge_text};
pub use wire::{notif_fields_valid, notif_msg_decode, notif_msg_init, notif_msg_post};

#[cfg(test)]
mod tests;
