#![cfg_attr(not(test), no_std)]
#![deny(unsafe_op_in_unsafe_fn)]
#![cfg_attr(all(not(test), velum_host), allow(dead_code))]

mod abi;
mod bridge;
mod cls;
mod dexrt;
mod ext;
mod heap;
mod ins;
mod mem;

#[cfg(test)]
mod dexstub;

mod export;

#[cfg(not(test))]
mod anchor;
#[cfg(all(not(test), feature = "interp", not(feature = "runtime")))]
pub use anchor::anchor_interp;
#[cfg(all(not(test), feature = "runtime", not(feature = "interp")))]
pub use anchor::anchor_runtime;

#[cfg(test)]
mod tests;

use abi::*;
