#include "../../kernel/kcon/kcon_int.h"
#include "display_int.h"
#include "velum/err.h"
#include "velum/klog.h"
#include "velum/libk.h"
#include "velum/panic.h"

static void	start(void)
{
	kcon_attach(&g_display.surf);
	g_display.hold_on_panic = display_fault_requested();
	panic_set_screen(display_panic_screen);
	if (boot_cmdline_has("verbose"))
		kcon_enable(true);
	if (kcon_enabled())
		return ;
	g_display.state = DSP_SPLASH;
	display_splash(0);
}

static void	log_ready(void)
{
	const char	*mem;

	mem = "sans cache";
	if (g_display.info.flags & DISP_FLAG_WC)
		mem = "écriture combinée";
	klog_info("display: %ux%ux%u pitch %u, mémoire vidéo en %s",
		g_display.info.width, g_display.info.height, g_display.info.bpp,
		g_display.info.pitch, mem);
}

int	display_boot_init(void)
{
	int	rc;

	memset(&g_display, 0, sizeof(g_display));
	mutex_init(&g_display.lock, "display");
	display_register_syscalls();
	rc = display_open();
	if (rc == E_NODEV)
		klog_info("display: aucun framebuffer, console série seule");
	else if (rc < 0)
		klog_warn("display: framebuffer refusé (%d)", rc);
	else
	{
		start();
		log_ready();
	}
	return (E_OK);
}
