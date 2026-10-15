#ifndef VFS_WEAK_H
# define VFS_WEAK_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "velum/block.h"
# include "velum/ksyscall.h"
# include "velum/object.h"
# include "velum/proc.h"
# include "velum/timer.h"
# include "velum/uptr.h"
# include "velum/vmm.h"

extern uint32_t		blk_count(void) __attribute__((weak));
extern t_blkdev		*blk_at(uint32_t i) __attribute__((weak));
extern int			blk_read(t_blkdev *d, uint64_t lba, uint32_t n,
						void *buf) __attribute__((weak));
extern int			blk_write(t_blkdev *d, uint64_t lba, uint32_t n,
						const void *buf) __attribute__((weak));
extern int			blk_flush(t_blkdev *d) __attribute__((weak));
extern int			syscall_register(uint32_t num, t_sysfn fn,
						const char *name) __attribute__((weak));
extern uint64_t		time_wall_ns(void) __attribute__((weak));
extern t_process	*proc_current(void) __attribute__((weak));
extern t_object		*obj_create(uint32_t type, const t_objops *ops,
						void *impl) __attribute__((weak));
extern void			obj_unref(t_object *obj) __attribute__((weak));
extern int			handle_alloc(t_process *p, t_object *o, uint32_t rt,
						t_handle *out) __attribute__((weak));
extern int			handle_get(t_process *p, t_handle h, uint32_t type,
						t_hget *out) __attribute__((weak));
extern int			copy_from_user(void *dst, t_uptr src,
						size_t n) __attribute__((weak));
extern int			copy_to_user(t_uptr dst, const void *src,
						size_t n) __attribute__((weak));
extern bool			user_range_ok(t_uptr addr, size_t n) __attribute__((weak));

#endif
