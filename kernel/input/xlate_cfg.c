#include "xlate.h"
#include "../../drivers/input/ps2_api.h"
#include "velum/err.h"
#include "velum/kinput.h"

void	xlate_locks_set(uint8_t mask)
{
	uint64_t	flags;

	flags = spin_lock_irqsave(&g_xlate.lock);
	g_xlate.kbd.locks = mask & INPUT_LED_ALL;
	spin_unlock_irqrestore(&g_xlate.lock, flags);
}

int	input_set_layout(const char *name)
{
	const t_layout	*l;
	uint64_t		flags;

	if (!name)
		return (E_INVAL);
	l = layout_find(name);
	if (!l)
		return (E_NOENT);
	flags = spin_lock_irqsave(&g_xlate.lock);
	g_xlate.kbd.layout = l;
	g_xlate.kbd.dead = 0;
	spin_unlock_irqrestore(&g_xlate.lock, flags);
	return (0);
}

const char	*input_layout(void)
{
	uint64_t	flags;
	const char	*name;

	flags = spin_lock_irqsave(&g_xlate.lock);
	name = g_xlate.kbd.layout->name;
	spin_unlock_irqrestore(&g_xlate.lock, flags);
	return (name);
}

int	input_mouse_config(uint32_t speed, uint32_t accel)
{
	uint64_t	flags;
	int			rc;

	flags = spin_lock_irqsave(&g_xlate.lock);
	rc = accel_set(&g_xlate.mouse.accel, speed, accel);
	spin_unlock_irqrestore(&g_xlate.lock, flags);
	return (rc);
}

void	input_set_leds(uint32_t mask)
{
	xlate_locks_set((uint8_t)(mask & INPUT_LED_ALL));
	ps2_led_sync();
}
