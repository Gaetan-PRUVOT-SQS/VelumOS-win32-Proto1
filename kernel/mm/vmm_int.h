#ifndef VMM_INT_H
# define VMM_INT_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "velum/boot.h"
# include "velum/pmm.h"
# include "velum/util.h"
# include "velum/vmm.h"

# ifdef VELUM_DEBUG
#  define VMM_DEBUG 1
# else
#  define VMM_DEBUG 0
# endif

# define PTE_P 0x1ull
# define PTE_W 0x2ull
# define PTE_U 0x4ull
# define PTE_PWT 0x8ull
# define PTE_PCD 0x10ull
# define PTE_PS 0x80ull
# define PTE_PAT4K 0x80ull
# define PTE_G 0x100ull
# define PTE_OWNED 0x200ull
# define PTE_SHARED 0x400ull
# define PTE_PAT2M 0x1000ull
# define PTE_NX 0x8000000000000000ull
# define PTE_ADDR 0x000ffffffffff000ull
# define PT_ENTRIES 512
# define PT_LEAF 1
# define PT_PD 2
# define PT_PDPT 3
# define PT_PML4 4
# define PAGE_2M 0x200000ull
# define PML4_SPAN 0x8000000000ull
# define WIN_SIZE 0x10000000000ull
# define WIN_STRIDE 0x100000000000ull
# define USER_HALF_END 0x0000800000000000ull
# define HHDM_MAX 0x0000400000000000ull
# define PHYS_LIMIT 0x0010000000000000ull
# define BIOS_AREA_LO 0xe0000ull
# define BIOS_AREA_HI 0x100000ull
# define HSPAN_MAX 130
# define VM_PROT 0x07
# define VM_KNOWN 0xff
# define VM_RESERVED 0x100
# define VM_HELD 0x200
# define VM_KEPT 0x300
# define VM_IO_OK 0x33
# define KSTACK_MAX 256
# define POOL_SLOTS 128
# define EFER_NXE 0x800ull
# define CPUID_PAT 0x10000u
# define PAT_UC 0x00
# define PAT_WC 0x01
# define ASLR_LO 0x10000000ull
# define ASLR_HI 0x00007e0000000000ull
# define ALLOC_GRAN 0x10000ull

typedef struct s_vmregion
{
	uintptr_t			start;
	uintptr_t			end;
	struct s_vmregion	*next;
	uint32_t			flags;
	uint32_t			maxprot;
}	t_vmregion;

typedef struct s_ptlook
{
	uint64_t	pte;
	uint64_t	pa;
	uint64_t	size;
}	t_ptlook;

struct s_aspace
{
	uint64_t	pml4;
	t_vmregion	*regions;
	t_vmregion	*free_desc;
	uint64_t	pool_next;
	uint64_t	self_phys;
	uint64_t	owned;
	uint64_t	tables;
	uint64_t	pool_pages;
	uintptr_t	lo;
	uintptr_t	hi;
	uint32_t	nfree;
	uint32_t	ticket;
	uint32_t	serving;
	bool		kernel;
};

typedef struct s_hspan
{
	uint64_t	base;
	uint64_t	end;
	uint64_t	bits;
}	t_hspan;

typedef struct s_vmm
{
	t_aspace	kas;
	t_aspace	*current;
	uint64_t	nx;
	uint64_t	uc_bits;
	uint64_t	wc_bits;
	uint64_t	cursor;
	uint32_t	nspan;
	bool		ready;
	t_hspan		hspan[HSPAN_MAX];
}	t_vmm;

typedef struct s_ucopy
{
	uint8_t		*kbuf;
	t_uptr		uva;
	size_t		n;
	bool		to_user;
}	t_ucopy;

typedef struct s_vquery
{
	uint32_t	prot;
	uint32_t	reserved;
	uint64_t	size;
}	t_vquery;

typedef struct s_vareq
{
	uint64_t	hint;
	uint64_t	len;
	uint64_t	limit;
	uint32_t	fl;
}	t_vareq;

extern t_vmm	g_vmm;

void		vmm_flush(t_aspace *as, uintptr_t va);
uint64_t	vmm_lock(t_aspace *as);
void		vmm_unlock(t_aspace *as, uint64_t flags);
uint64_t	vmm_pte_bits(uint32_t fl);
uint64_t	vmm_pte_large(uint64_t bits);
uint32_t	vmm_pte_flags(uint64_t pte, uint64_t size);
t_pmm_owner	vmm_owner(const t_aspace *as, uintptr_t va, uint64_t pte);
int			vmm_check_range(const t_aspace *as, uintptr_t va, size_t len);
int			vmm_check_flags(const t_aspace *as, uint32_t fl, bool anon);
uint64_t	*pt_table(uint64_t phys);
uint32_t	pt_index(uintptr_t va, int level);
uint64_t	pt_span(int level);
uint64_t	*vmm_entry(t_aspace *as, uintptr_t va, int level, bool create);
uint64_t	vmm_reach(t_aspace *as, uintptr_t va, int level, uint64_t **out);
bool		vmm_lookup(t_aspace *as, uintptr_t va, t_ptlook *out);
void		vmm_prune(t_aspace *as, uintptr_t lo, uintptr_t hi);
void		vmm_pool_carve(t_aspace *as, void *page, uint32_t first);
int			vmm_pool_reserve(t_aspace *as, uint32_t n);
t_vmregion	*vmm_desc_get(t_aspace *as);
void		vmm_desc_put(t_aspace *as, t_vmregion *d);
bool		vmm_reg_overlaps(t_aspace *as, uintptr_t lo, uintptr_t hi);
void		vmm_reg_insert(t_aspace *as, const t_vmregion *tpl);
t_vmregion	*vmm_reg_from(t_aspace *as, uintptr_t va);
void		vmm_reg_split(t_aspace *as, uintptr_t at);
void		vmm_reg_cut(t_aspace *as, uintptr_t lo, uintptr_t hi);
void		vmm_reg_reflag(t_aspace *as, uintptr_t a, uintptr_t b, uint32_t f);
bool		vmm_reg_allows(t_aspace *as, uintptr_t a, uintptr_t b, uint32_t f);
int			vmm_map_one(t_aspace *as, uintptr_t va, uint64_t pa, uint64_t bits);
int			vmm_alloc_locked(t_aspace *as, uintptr_t va, size_t n, uint32_t f);
int			vmm_map_locked(t_aspace *as, const t_vmreq *rq);
void		vmm_unmap_pages(t_aspace *as, uintptr_t va, size_t len);
int			vmm_unmap_locked(t_aspace *as, uintptr_t va, size_t len);
uintptr_t	vmm_find_locked(t_aspace *as, size_t n, uintptr_t lo, uintptr_t hi);
void		vmm_free_lower(t_aspace *as);
bool		vmm_user_page(t_aspace *as, uintptr_t va, bool wr, t_ptlook *lk);
int			vmm_user_check(t_aspace *as, t_uptr a, size_t n, bool wr);
void		vmm_user_copy(t_aspace *as, const t_ucopy *c);
int			vmm_user_xfer(const t_ucopy *c);
int			vmm_kwin_place(t_vmreq *rq, uintptr_t win, bool anon);
int			vmm_map_span(t_aspace *as, const t_vmreq *rq, uint64_t bits);
int			vmm_hhdm_build(const t_bootinfo *bi);
int			vmm_hhdm_map(t_aspace *as, uint64_t hhdm);
uint64_t	vmm_hhdm_top(void);
t_hspan		*vmm_hhdm_find(uint64_t phys, uint64_t len);
void		vmm_cpu_detect(void);
void		vmm_boot_finish(const t_bootinfo *bi);
void		mmu_invlpg(uint64_t va);
void		mmu_write_cr3(uint64_t pml4);
uint64_t	mmu_read_cr2(void);
void		mmu_flush_global(void);
uint64_t	mmu_gdt_base(void);
uint32_t	mmu_cpuid_edx(uint32_t leaf);
void		mmu_image_layout(uint64_t out[6]);
bool		vmm_reg_has(t_aspace *as, uintptr_t lo, uintptr_t hi, uint32_t m);
int			vmm_reg_drop(t_aspace *as, uintptr_t lo, uintptr_t hi, uint32_t k);

#endif
