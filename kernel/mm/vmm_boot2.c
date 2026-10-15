#include "velum/klog.h"
#include "vmm_int.h"
#include "vmm_weak.h"

static void	reclaim(const t_bootinfo *bi)
{
	uint64_t	l[6];
	uint64_t	gdt;
	uint64_t	total;
	uint32_t	i;

	mmu_image_layout(l);
	gdt = mmu_gdt_base();
	if (gdt < l[0] || gdt >= l[5])
	{
		klog_warn("vmm: GDT du chargeur active, memoire de demarrage gardee");
		return ;
	}
	total = 0;
	i = 0;
	while (i < bi->nranges && i < BOOT_MAX_RANGES)
	{
		if (bi->ranges[i].type == MEM_BOOT_RECLAIM
			&& pmm_add_region(bi->ranges[i].base, bi->ranges[i].length) >= 0)
			total += bi->ranges[i].length;
		i++;
	}
	klog_info("vmm: %llu Kio rendus par le chargeur", total / 1024);
}

static void	register_calls(void)
{
	int	rc;

	if (!syscall_register)
		return ;
	rc = syscall_register(SYS_VALLOC, vmm_sys_valloc, "valloc");
	if (rc >= 0)
		rc = syscall_register(SYS_VFREE, vmm_sys_vfree, "vfree");
	if (rc >= 0)
		rc = syscall_register(SYS_VPROTECT, vmm_sys_vprotect, "vprotect");
	if (rc >= 0)
		rc = syscall_register(SYS_VQUERY, vmm_sys_vquery, "vquery");
	if (rc < 0)
		klog_warn("vmm: appels memoire non enregistres (%d)", rc);
}

static void	check_nx(void)
{
	if (cpu_features && cpu_features()->nx && !g_vmm.nx)
		klog_warn("vmm: le CPU annonce NX mais EFER.NXE est coupe");
	if (!g_vmm.nx)
		klog_warn("vmm: NX absent, W^X limite a l'ecriture");
}

static const char	*yes_no(bool v)
{
	if (v)
		return ("oui");
	return ("non");
}

void	vmm_boot_finish(const t_bootinfo *bi)
{
	check_nx();
	reclaim(bi);
	if (!idt_set_handler)
		klog_warn("vmm: pas d'IDT, faute de page non geree");
	else if (idt_set_handler(VEC_PAGE_FAULT, vmm_page_fault, NULL) < 0)
		klog_warn("vmm: gestionnaire de faute de page refuse");
	register_calls();
	klog_info("vmm: tables %llu Kio, hhdm %u plages jusqu'a %llu Mio, NX %s,"
		" WC %s", g_vmm.kas.tables * 4, g_vmm.nspan, vmm_hhdm_top() >> 20,
		yes_no(g_vmm.nx != 0), yes_no(g_vmm.wc_bits != g_vmm.uc_bits));
}
