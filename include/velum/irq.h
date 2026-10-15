#ifndef IRQ_H
# define IRQ_H

# include <stdint.h>

# define IRQF_SHARED 0x1
# define IRQF_LEVEL 0x2
# define IRQF_LOW 0x4

typedef void	(*t_irqfn)(void *ctx);

int			irq_boot_init(void);
int			irq_request(uint32_t gsi, t_irqfn fn, void *ctx, uint32_t flags);
void		irq_free(uint32_t gsi, t_irqfn fn);
uint32_t	irq_isa_to_gsi(uint8_t isa);
int			irq_vector_alloc(t_irqfn fn, void *ctx);
void		irq_vector_free(int vec);
void		irq_mask(uint32_t gsi);
void		irq_unmask(uint32_t gsi);
void		apic_eoi(void);
uint32_t	apic_id(void);

#endif
