#include "ps2.h"
#include "../../kernel/input/xlate.h"
#include "velum/irq.h"
#include "velum/klog.h"
#include "velum/timer.h"

uint8_t	ps2_irq_hook(t_ps2 *g, uint8_t isa)
{
	int	rc;

	rc = irq_request(irq_isa_to_gsi(isa), ps2_irq_handler, g, 0);
	if (rc < 0)
		klog_warn("input: IRQ ISA %u refusée (%d)", (unsigned)isa, rc);
	return (rc >= 0);
}

uint8_t	ps2_final_cfg(const t_ps2 *g, uint8_t cfg)
{
	cfg &= ~(PS2_CFG_IRQ1 | PS2_CFG_IRQ2 | PS2_CFG_XLAT);
	cfg &= ~(PS2_CFG_CLK1_OFF | PS2_CFG_CLK2_OFF);
	if (g->kbd_ok)
		cfg |= PS2_CFG_IRQ1;
	else
		cfg |= PS2_CFG_CLK1_OFF;
	if (g->mouse_ok)
		cfg |= PS2_CFG_IRQ2;
	else
		cfg |= PS2_CFG_CLK2_OFF;
	return (cfg);
}

void	ps2_led_sync(void)
{
	t_ps2		*g;
	uint64_t	flags;

	g = &g_ps2;
	if (!g->kbd_ok)
		return ;
	flags = spin_lock_irqsave(&g->lock);
	led_request(g, xlate_locks(), time_now_ns());
	spin_unlock_irqrestore(&g->lock, flags);
}
