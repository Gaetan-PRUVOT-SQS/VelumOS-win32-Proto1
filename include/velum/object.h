#ifndef OBJECT_H
# define OBJECT_H

# include <stdbool.h>
# include <stdint.h>
# include "proc.h"
# include "sync.h"

# define HR_WAIT 0x001
# define HR_SIGNAL 0x002
# define HR_READ 0x004
# define HR_WRITE 0x008
# define HR_MAP_R 0x010
# define HR_MAP_W 0x020
# define HR_MAP_X 0x040
# define HR_DUP 0x080
# define HR_TRANSFER 0x100
# define HR_ALL 0x1ff
# define HANDLE_INVALID 0
# define HANDLE_MAX 4096

typedef enum e_objtype
{
	OBJ_NONE = 0,
	OBJ_PROCESS,
	OBJ_THREAD,
	OBJ_EVENT,
	OBJ_SEMAPHORE,
	OBJ_TIMER,
	OBJ_SECTION,
	OBJ_CHANNEL,
	OBJ_LISTENER,
	OBJ_FILE,
	OBJ_DIR,
	OBJ_INPUT,
	OBJ_TYPES
}	t_objtype;

typedef struct s_object
{
	uint32_t				type;
	uint32_t				refs;
	const struct s_objops	*ops;
	void					*impl;
	t_spinlock				lock;
	t_waitq					waiters;
}	t_object;

typedef struct s_objops
{
	const char	*name;
	void		(*destroy)(t_object *obj);
	bool		(*signaled)(t_object *obj);
}	t_objops;

typedef struct s_hget
{
	t_object	*obj;
	uint32_t	rights;
}	t_hget;

int			object_boot_init(void);
t_object	*obj_create(uint32_t type, const t_objops *ops, void *impl);
void		obj_ref(t_object *obj);
void		obj_unref(t_object *obj);
void		obj_signal(t_object *obj);
int			obj_wait(t_object *obj, uint64_t timeout_ns);
void		*handle_table_create(void);
void		handle_table_destroy(void *ht);
int			handle_alloc(t_process *p, t_object *o, uint32_t rt, t_handle *out);
int			handle_get(t_process *p, t_handle h, uint32_t type, t_hget *out);
int			handle_close(t_process *p, t_handle h);
int			handle_dup(t_process *s, t_handle h, t_process *d, t_handle *out);

# define CHAN_SEND_MOVE 0x1

void		object_process_cleanup(t_process *p);
int			object_selftest(void);
t_object	*obj_process_new(t_process *p);
t_object	*obj_thread_new(t_thread *t);
void		obj_task_exited(t_object *o);
t_process	*obj_process_of(t_object *o);
t_thread	*obj_thread_of(t_object *o);
int			handle_need(t_hget *g, uint32_t need);
int			handle_dup_as(t_process *p, t_handle h, uint32_t r, t_handle *o);

#endif
