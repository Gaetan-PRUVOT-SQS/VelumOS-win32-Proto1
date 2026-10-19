#![cfg_attr(not(test), no_std)]
#![deny(unsafe_op_in_unsafe_fn)]

#[macro_use]
mod exp;

mod abi;
mod ctrl;
mod decor;
mod dock;
mod frame;
mod glass;
mod hostgfx;
mod ink;
mod launch;
mod layout;
mod mark;
mod num;
mod ops;
mod pal;
mod shape;
mod text;
mod wall;

#[cfg(not(test))]
mod anchor;
#[cfg(not(test))]
pub use anchor::anchor_syms;

#[cfg(test)]
mod tests;

pub use abi::*;
