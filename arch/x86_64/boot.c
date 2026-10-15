#include "limine.h"
#include "limine_req.h"
#include "boot_int.h"
#include "velum/boot.h"
#include "velum/err.h"
#include "velum/libk.h"

int	boot_collect(void)
{
	t_bootinfo							*bi;
	const struct limine_hhdm_response	*hhdm;

	if (!limine_base_ok())
		return (E_NOTSUP);
	hhdm = lim_hhdm();
	if (!hhdm)
		return (E_PROTO);
	bi = boot_info_rw();
	memset(bi, 0, sizeof(*bi));
	bi->version = BOOT_INFO_VERSION;
	bi->size = sizeof(*bi);
	bi->hhdm = hhdm->offset;
	if (boot_fill_memmap(bi) < 0)
		return (E_PROTO);
	boot_fill_misc(bi);
	return (E_OK);
}
