#include "inp_queue.h"
#include "inp_sys.h"
#include "xlate.h"
#include "../../drivers/input/ps2_api.h"
#include "velum/boot.h"
#include "velum/kinput.h"
#include "velum/klog.h"

#ifdef VELUM_DEBUG
# define INPUTLOG_ALLOWED 1
#else
# define INPUTLOG_ALLOWED 0
#endif

uint32_t	input_leds(void)
{
	return (xlate_locks());
}

static void	input_log_setup(void)
{
	if (INPUTLOG_ALLOWED && boot_cmdline_has("inputlog"))
	{
		g_inputq.logging = 1;
		klog_info("input: journal des événements actif");
	}
}

int	input_boot_init(void)
{
	int	fails;
	int	rc;

	inpq_init();
	xlate_init();
	fails = inp_sys_register();
	if (fails)
		klog_err("input: %d appel(s) système non enregistré(s)", fails);
	input_log_setup();
	rc = ps2_boot();
	if (rc < 0)
		klog_warn("input: contrôleur PS/2 indisponible (%d)", rc);
	return (0);
}
