#ifndef SMP_H
# define SMP_H

# include <stdbool.h>
# include <stdint.h>
# include "velum/cpu.h"

# define SMP_VEC_MIN 0x20

int			smp_init(uint32_t max);
void		smp_boot_cmdline(void);
uint32_t	smp_cpu_count(void);
uint32_t	smp_cpu_id(void);
uint32_t	smp_cpu_apic_id(uint32_t cpu);
int			smp_send_ipi(uint32_t cpu, uint8_t vecteur);
uint64_t	smp_ipi_count(uint32_t cpu);

#endif
