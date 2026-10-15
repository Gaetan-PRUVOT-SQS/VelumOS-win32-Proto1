#ifndef PROC_DEPS_H
# define PROC_DEPS_H

# include <stddef.h>
# include <stdint.h>
# include "velum/cpu.h"
# include "velum/heap.h"
# include "velum/object.h"
# include "velum/pmm.h"
# include "velum/proc.h"
# include "velum/random.h"
# include "velum/timer.h"
# include "velum/vfs.h"

extern void			cpu_set_user_fault(t_userfaultfn fn) __attribute__((weak));
extern void			pmm_get_stats(t_pmm_stats *out) __attribute__((weak));
extern void			heap_get_stats(t_heap_stats *out) __attribute__((weak));
extern uint64_t		time_now_ns(void) __attribute__((weak));
extern void			krandom(void *buf, size_t n) __attribute__((weak));
extern uint64_t		krandom_below(uint64_t bound) __attribute__((weak));
extern int			vfs_read_all(const char *path, void **buf, size_t *size)
					__attribute__((weak));
extern t_object		*obj_process_new(t_process *p) __attribute__((weak));
extern t_object		*obj_thread_new(t_thread *t) __attribute__((weak));
extern void			obj_task_exited(t_object *o) __attribute__((weak));
extern t_process	*obj_process_of(t_object *o) __attribute__((weak));
extern t_thread		*obj_thread_of(t_object *o) __attribute__((weak));
extern void			thread_cancel(t_thread *t) __attribute__((weak));
extern void			obj_unref(t_object *obj) __attribute__((weak));
extern void			obj_signal(t_object *obj) __attribute__((weak));
extern void			*handle_table_create(void) __attribute__((weak));
extern void			handle_table_destroy(void *ht) __attribute__((weak));
extern int			handle_alloc(t_process *p, t_object *o, uint32_t rt,
						t_handle *out) __attribute__((weak));
extern int			handle_get(t_process *p, t_handle h, uint32_t type,
						t_hget *out) __attribute__((weak));
extern int			handle_close(t_process *p, t_handle h)
					__attribute__((weak));
extern void			object_process_cleanup(t_process *p)
					__attribute__((weak));

#endif
