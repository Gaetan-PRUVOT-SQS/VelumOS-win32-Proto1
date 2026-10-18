#![cfg_attr(not(test), no_std)]
#![deny(unsafe_op_in_unsafe_fn)]

#[cfg(all(not(test), feature = "handler"))]
#[panic_handler]
fn rust_panic(_info: &core::panic::PanicInfo) -> ! {
    loop {
        core::hint::spin_loop();
    }
}

#[cfg(all(not(test), feature = "handler"))]
#[unsafe(no_mangle)]
/// # Safety
/// Le déroulement est absent. Le symbole existe pour le lien, personne ne l'appelle.
unsafe extern "C" fn rust_eh_personality() {}
