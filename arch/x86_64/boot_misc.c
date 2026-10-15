#include "limine.h"
#include "limine_req.h"
#include "velum/boot.h"
#include "velum/libk.h"

static uint64_t	to_phys(uint64_t addr, uint64_t hhdm)
{
	if (addr >= hhdm)
		return (addr - hhdm);
	return (addr);
}

static void	fill_fb(t_bootinfo *bi)
{
	const struct limine_framebuffer_response	*r;
	const struct limine_framebuffer				*f;

	r = lim_fb();
	if (!r || !r->framebuffer_count)
		return ;
	f = r->framebuffers[0];
	bi->fb.virt = (uint64_t)f->address;
	bi->fb.phys = to_phys(bi->fb.virt, bi->hhdm);
	bi->fb.width = (uint32_t)f->width;
	bi->fb.height = (uint32_t)f->height;
	bi->fb.pitch = (uint32_t)f->pitch;
	bi->fb.bpp = f->bpp;
	bi->fb.red_shift = f->red_mask_shift;
	bi->fb.red_size = f->red_mask_size;
	bi->fb.green_shift = f->green_mask_shift;
	bi->fb.green_size = f->green_mask_size;
	bi->fb.blue_shift = f->blue_mask_shift;
	bi->fb.blue_size = f->blue_mask_size;
}

static void	fill_module(t_bootinfo *bi)
{
	const struct limine_module_response	*m;

	m = lim_module();
	if (!m || !m->module_count)
		return ;
	bi->initrd_phys = to_phys((uint64_t)m->modules[0]->address, bi->hhdm);
	bi->initrd_size = m->modules[0]->size;
}

static void	fill_firmware(t_bootinfo *bi)
{
	const struct limine_rsdp_response				*rsdp;
	const struct limine_efi_system_table_response	*efi;
	const struct limine_date_at_boot_response		*date;

	rsdp = lim_rsdp();
	if (rsdp)
		bi->rsdp_phys = to_phys((uint64_t)rsdp->address, bi->hhdm);
	efi = lim_efi();
	if (efi)
	{
		bi->efi = 1;
		bi->efi_systab_phys = to_phys((uint64_t)efi->address, bi->hhdm);
	}
	date = lim_date();
	if (date)
		bi->boot_unix_s = date->timestamp;
}

void	boot_fill_misc(t_bootinfo *bi)
{
	const struct limine_executable_address_response	*a;
	const struct limine_executable_cmdline_response	*c;

	a = lim_addr();
	if (a)
	{
		bi->kernel_phys = a->physical_base;
		bi->kernel_virt = a->virtual_base;
	}
	c = lim_cmdline();
	if (c && c->cmdline)
		strlcpy(bi->cmdline, c->cmdline, BOOT_CMDLINE_MAX);
	fill_fb(bi);
	fill_module(bi);
	fill_firmware(bi);
}
