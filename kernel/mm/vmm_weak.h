#ifndef VMM_WEAK_H
# define VMM_WEAK_H

# include <stdint.h>
# include "velum/cpu.h"
# include "velum/ksyscall.h"
# include "velum/proc.h"
# include "velum/random.h"

int				idt_set_handler(uint8_t v, t_trapfn f, void *c)
				__attribute__((weak));
const t_cpufeat	*cpu_features(void) __attribute__((weak));
void			proc_fault(t_regs *r, uint64_t v, uint64_t a)
				__attribute__((weak));
t_process		*proc_current(void) __attribute__((weak));
int				syscall_register(uint32_t n, t_sysfn f, const char *s)
				__attribute__((weak));
uint64_t		krandom_below(uint64_t bound) __attribute__((weak));
void			secmaps_forget(t_process *p, uintptr_t va, uint64_t len)
				__attribute__((weak));
void			vmm_page_fault(t_regs *regs, void *ctx);
t_aspace		*vmm_sys_caller(t_process **out);
int				vmm_sys_prot(uint64_t prot, uint32_t *fl);
int64_t			vmm_sys_valloc(const t_sysargs *a);
int64_t			vmm_sys_vfree(const t_sysargs *a);
int64_t			vmm_sys_vprotect(const t_sysargs *a);
int64_t			vmm_sys_vquery(const t_sysargs *a);
int				vmm_selftest(void);
int				vmm_st_user(void);

#endif
