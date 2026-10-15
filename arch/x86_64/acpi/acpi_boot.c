#include "acpi_int.h"
#include "velum/boot.h"
#include "velum/err.h"
#include "velum/klog.h"
#include "velum/libk.h"
#include "velum/util.h"
#include "velum/vmm.h"

static t_acpi_state	g_acpi;

t_acpi_state	*acpi_state(void)
{
	return (&g_acpi);
}

static const void	*acpi_kmap(uint64_t phys, uint64_t len, void *ctx)
{
	uint64_t	base;
	uint64_t	end;
	uint8_t		*v;

	(void)ctx;
	if (!len || phys >= ACPI_PHYS_LIMIT || len > ACPI_PHYS_LIMIT - phys)
		return (NULL);
	base = align_down(phys, PAGE_SIZE);
	end = align_up(phys + len, PAGE_SIZE);
	v = vmm_io_map(base, end - base, VM_R);
	if (!v)
		return (NULL);
	return (v + (phys - base));
}

static void	acpi_kunmap(const void *virt, uint64_t len, void *ctx)
{
	uintptr_t	v;
	uintptr_t	base;

	(void)ctx;
	if (!virt || !len)
		return ;
	v = (uintptr_t)virt;
	base = align_down(v, PAGE_SIZE);
	vmm_io_unmap((void *)base, align_up(v + len, PAGE_SIZE) - base);
}

static uint64_t	acpi_rsdp_phys(const t_bootinfo *bi)
{
	uint64_t	raw;

	raw = bi->rsdp_phys + bi->hhdm;
	if (bi->rsdp_phys && raw < bi->hhdm)
		return (raw);
	return (bi->rsdp_phys);
}

int	acpi_boot_init(void)
{
	t_acpi_src	src;
	uint64_t	rsdp;
	int			rc;

	memset(&g_acpi, 0, sizeof(g_acpi));
	src.map = acpi_kmap;
	src.unmap = acpi_kunmap;
	src.ctx = NULL;
	rsdp = acpi_rsdp_phys(boot_info());
	if (!rsdp)
	{
		klog_warn("acpi: pas de RSDP, machine sans tables ACPI");
		return (E_OK);
	}
	rc = acpi_rsdp_parse(&src, rsdp, &g_acpi.root);
	if (rc == E_OK)
		rc = acpi_collect(&src, &g_acpi.root, &g_acpi.set);
	if (rc < 0)
	{
		klog_err("acpi: RSDP @%#llx ou racine illisible (%d)", rsdp, rc);
		return (E_OK);
	}
	acpi_parse_all(&g_acpi.set, &g_acpi.info, &g_acpi.x);
	acpi_log(&g_acpi);
	return (E_OK);
}
