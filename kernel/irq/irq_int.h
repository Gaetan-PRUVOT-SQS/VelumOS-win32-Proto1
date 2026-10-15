#ifndef IRQ_INT_H
# define IRQ_INT_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "velum/acpi.h"
# include "velum/cpu.h"
# include "velum/irq.h"
# include "a05_lock.h"

# define IRQ_NVEC 192
# define IRQ_ACTIONS_MAX 128
# define IRQ_SHARE_MAX 8
# define IRQ_NO_GSI 0xffffffffu
# define IRQ_NONE -1
# define IRQF_ALL 0x7
# define IRQF_TRIG_MASK 0x6
# define IRQ_ISA_LINES 16

typedef struct s_irq_action
{
	t_irqfn		fn;
	void		*ctx;
	int16_t		next;
	uint8_t		used;
	uint8_t		reserved;
}	t_irq_action;

typedef struct s_irq_slot
{
	uint32_t	gsi;
	uint32_t	trig;
	int16_t		head;
	uint8_t		used;
	uint8_t		shared;
	uint8_t		foreign;
	uint8_t		nact;
	uint8_t		reserved[2];
}	t_irq_slot;

typedef struct s_irq_core
{
	t_irq_slot		slot[IRQ_NVEC];
	t_irq_action	act[IRQ_ACTIONS_MAX];
}	t_irq_core;

typedef struct s_irq_req
{
	uint32_t	gsi;
	uint32_t	trig;
	t_irqfn		fn;
	void		*ctx;
	bool		shared;
}	t_irq_req;

typedef struct s_irq_state
{
	t_irq_core	core;
	t_a05lock	lock;
	bool		ready;
}	t_irq_state;

void		irqc_init(t_irq_core *c);
int			irqc_vec_alloc(t_irq_core *c);
void		irqc_vec_release(t_irq_core *c, int idx);
int			irqc_find_gsi(const t_irq_core *c, uint32_t gsi);
int			irqc_attach(t_irq_core *c, int idx, t_irqfn fn, void *ctx);
int			irqc_detach(t_irq_core *c, int idx, t_irqfn fn);
int			irqc_request(t_irq_core *c, const t_irq_req *rq, bool *is_new);
uint32_t	irqc_snapshot(const t_irq_core *c, int idx, t_irq_action *out,
				uint32_t max);
uint32_t	irq_resolve_trig(const t_acpi_info *ai, uint32_t gsi,
				uint32_t flags);
uint32_t	irq_isa_lookup(const t_acpi_info *ai, uint8_t isa);
bool		irq_nmi_reserved(uint32_t gsi);
int			irq_selftest(void);
int			irq_test_self_ipi(void);
t_irq_state	*irq_state(void);

#endif
