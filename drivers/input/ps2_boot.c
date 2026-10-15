#include "ps2.h"
#include "ps2_api.h"
#include "velum/err.h"
#include "velum/klog.h"
#include "velum/libk.h"

static void	boot_devices(t_ps2 *g)
{
	if (g->kbd_ok && ps2_ctl_write(g->ops, PS2_C_EN1) < 0)
		g->kbd_ok = 0;
	if (g->mouse_ok && ps2_ctl_write(g->ops, PS2_C_EN2) < 0)
		g->mouse_ok = 0;
	if (g->kbd_ok && ps2_kbd_init(g) < 0)
		g->kbd_ok = 0;
	if (g->mouse_ok && ps2_mouse_init(g) < 0)
		g->mouse_ok = 0;
}

static int	boot_finish(t_ps2 *g)
{
	uint8_t	cfg;
	int		rc;

	g->kbd_ok = g->kbd_ok && ps2_irq_hook(g, 1);
	g->mouse_ok = g->mouse_ok && ps2_irq_hook(g, 12);
	rc = i8042_cfg_read(g, &cfg);
	if (rc < 0)
		return (rc);
	return (i8042_cfg_write(g, ps2_final_cfg(g, cfg)));
}

int	ps2_boot(void)
{
	t_ps2		*g;
	uint64_t	flags;
	int			rc;

	g = &g_ps2;
	memset(g, 0, sizeof(*g));
	spin_init(&g->lock, "ps2");
	g->ops = ps2_hw_ops();
	rc = i8042_init(g);
	if (rc == E_NODEV)
	{
		klog_info("input: pas de contrôleur PS/2 utilisable");
		return (0);
	}
	if (rc < 0)
		return (rc);
	boot_devices(g);
	rc = boot_finish(g);
	flags = spin_lock_irqsave(&g->lock);
	ps2_drain(g);
	spin_unlock_irqrestore(&g->lock, flags);
	ps2_led_sync();
	return (rc);
}
