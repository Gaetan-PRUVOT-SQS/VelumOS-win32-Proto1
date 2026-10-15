#include "../../drivers/display/display_int.h"
#include "kcon_int.h"
#include "velum/irqflags.h"
#include "velum/klog.h"

static bool	g_sink;

static void	sink_once(void)
{
	if (g_sink)
		return ;
	g_sink = true;
	klog_add_sink(kcon_write);
}

void	kcon_write(const char *s, size_t n)
{
	uint64_t	flags;

	if (!s || !n || g_display.state != DSP_CONSOLE)
		return ;
	flags = irq_save();
	if (display_try_lock())
	{
		if (g_display.state == DSP_CONSOLE && g_kcon.ready)
		{
			kcon_cursor_hide(&g_kcon);
			kcon_feed(&g_kcon, s, n);
			kcon_cursor_show(&g_kcon);
		}
		display_unlock();
	}
	irq_restore(flags);
}

void	kcon_enable(bool on)
{
	uint64_t	flags;

	if (!g_display.ready)
		return ;
	if (!on)
	{
		display_state_set(DSP_OFF);
		return ;
	}
	if (!g_kcon.ready)
		return ;
	flags = irq_save();
	if (display_lock_wait())
	{
		if (g_display.state != DSP_CONSOLE)
			kcon_clear_all(&g_kcon);
		g_display.state = DSP_CONSOLE;
		display_unlock();
	}
	irq_restore(flags);
	sink_once();
}

bool	kcon_enabled(void)
{
	return (g_display.ready && g_display.state == DSP_CONSOLE);
}

void	kcon_clear(void)
{
	uint64_t	flags;

	flags = irq_save();
	if (display_lock_wait())
	{
		if (g_display.state == DSP_CONSOLE && g_kcon.ready)
			kcon_clear_all(&g_kcon);
		display_unlock();
	}
	irq_restore(flags);
}
