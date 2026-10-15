#ifndef LIMINE_REQ_H
# define LIMINE_REQ_H

int limine_base_ok(void);
const void *lim_memmap(void);
const void *lim_hhdm(void);
const void *lim_fb(void);
const void *lim_rsdp(void);
const void *lim_module(void);
const void *lim_addr(void);
const void *lim_cmdline(void);
const void *lim_date(void);
const void *lim_efi(void);

#endif
