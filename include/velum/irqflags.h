#ifndef IRQFLAGS_H
# define IRQFLAGS_H

# include <stdint.h>

uint64_t	irq_save(void);
void		irq_restore(uint64_t flags);
void		irq_disable(void);
void		irq_enable(void);
void		cpu_relax(void);

#endif
