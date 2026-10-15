#ifndef VMM_H
# define VMM_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "uptr.h"

# define KERNEL_BASE 0xffffffff80000000ull
# define HHDM_DEFAULT 0xffff800000000000ull
# define KHEAP_BASE 0xffffc00000000000ull
# define KIO_BASE 0xffffd00000000000ull
# define KSTACK_BASE 0xffffe00000000000ull
# define USER_MIN 0x0000000000010000ull
# define USER_TOP 0x00007ffffffff000ull
# define USER_STACK_TOP 0x00007fffffff0000ull
# define VM_R 0x01
# define VM_W 0x02
# define VM_X 0x04
# define VM_USER 0x08
# define VM_NOCACHE 0x10
# define VM_WC 0x20
# define VM_GLOBAL 0x40
# define VM_SHARED 0x80

typedef struct s_aspace	t_aspace;

typedef struct s_vmreq
{
	uintptr_t	va;
	uint64_t	pa;
	size_t		len;
	uint32_t	flags;
}	t_vmreq;

typedef struct s_vminfo
{
	uint64_t	pa;
	uint32_t	flags;
}	t_vminfo;

int			vmm_boot_init(void);
void		*phys_to_virt(uint64_t phys);
uint64_t	virt_to_phys(const void *kvirt);
t_aspace	*vmm_kernel_aspace(void);
t_aspace	*vmm_aspace_create(void);
void		vmm_aspace_destroy(t_aspace *as);
void		vmm_switch(t_aspace *as);
int			vmm_map(t_aspace *as, const t_vmreq *rq);
int			vmm_alloc(t_aspace *as, uintptr_t va, size_t len, uint32_t fl);
int			vmm_unmap(t_aspace *as, uintptr_t va, size_t len);
int			vmm_protect(t_aspace *as, uintptr_t va, size_t len, uint32_t fl);
bool		vmm_query(t_aspace *as, uintptr_t va, t_vminfo *out);
uintptr_t	vmm_find_free(t_aspace *as, size_t len, uintptr_t lo, uintptr_t hi);
void		*vmm_io_map(uint64_t phys, size_t len, uint32_t flags);
void		vmm_io_unmap(void *virt, size_t len);
void		*vmm_kstack_alloc(size_t pages);
void		vmm_kstack_free(void *base, size_t pages);
uint64_t	vmm_pages_used(const t_aspace *as);
int			copy_from_user(void *dst, t_uptr src, size_t n);
int			copy_to_user(t_uptr dst, const void *src, size_t n);
int			strncpy_from_user(char *dst, t_uptr src, size_t max);
bool		user_range_ok(t_uptr addr, size_t n);
int			vmm_audit_wx(void);
bool		vmm_hhdm_covers(uint64_t phys, uint64_t len);
int			vmm_reserve(t_aspace *as, uintptr_t va, size_t len);
uint64_t	vmm_region_count(t_aspace *as);
int			vmm_unreserve(t_aspace *as, uintptr_t va, size_t len);
int			vmm_stack_map(t_aspace *as, uintptr_t *va, size_t len);
int			vmm_stack_unmap(t_aspace *as, uintptr_t va, size_t len);

#endif
