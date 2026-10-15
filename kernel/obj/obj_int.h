#ifndef OBJ_INT_H
# define OBJ_INT_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "velum/err.h"
# include "velum/object.h"
# include "velum/abi/abi_ipc.h"
# include "velum/abi/abi_syscall.h"
# include "velum/ksyscall.h"
# include "velum/timer.h"

# define HT_INIT_CAP 16
# define HT_IDX_BITS 12
# define HT_IDX_MASK 0xfff
# define HT_GEN_MAX 0xfffff
# define HE_FREE 0
# define HE_USED 1
# define HE_RESERVED 2
# define HE_BUSY 3
# define HE_RETIRED 4
# define OBJ_REFS_MAX 0x7fffffff
# define WAIT_MAX 64
# define WREG_BUCKETS 64
# define WREG_SHIFT 58
# define SEC_SIZE_MAX 0x4000000
# define SEC_MAPS_MAX 1024
# define SEC_MAP_TRIES 3
# define PROT_ALL 0x7
# define TMR_SLOTS 512
# define TMR_SLOT_BITS 9
# define TMR_SLOT_MASK 0x1ff
# define TMR_PERIOD_MIN 1000000
# define RIGHTS_CHAN 0x18d
# define RIGHTS_LISTEN 0x185
# define RIGHTS_SIGOBJ 0x183
# define RIGHTS_SECTION 0x180
# define ST_TIMEOUT 5000000000ull
# define ST_ROUNDS 50000

typedef struct s_hentry
{
	t_object	*obj;
	uint32_t	rights;
	uint32_t	gen;
	uint32_t	state;
	uint32_t	pad;
}	t_hentry;

typedef struct s_memacct
{
	uint64_t	bytes;
	uint32_t	refs;
	uint32_t	pad;
}	t_memacct;

typedef struct s_secmap
{
	struct s_secmap	*next;
	uintptr_t		va;
	uint64_t		len;
	uint64_t		offset;
	t_object		*sec;
}	t_secmap;

typedef struct s_handle_table
{
	t_spinlock	lock;
	t_hentry	*ents;
	uint32_t	cap;
	uint32_t	used;
	uint32_t	hint;
	uint32_t	closed;
	uint32_t	nmaps;
	uint32_t	pad;
	t_secmap	*maps;
	t_memacct	*acct;
	t_mutex		maplock;
}	t_htab;

typedef struct s_wblock
{
	t_spinlock	lock;
	t_waitq		wq;
	uint32_t	fired;
	uint32_t	pad;
}	t_wblock;

typedef struct s_wlink
{
	struct s_wlink	*next;
	struct s_wlink	*prev;
	t_object		*obj;
	t_wblock		*wb;
	uint32_t		pulsed;
	uint32_t		pad;
}	t_wlink;

typedef struct s_waitset
{
	t_object	*objs[WAIT_MAX];
	t_wlink		links[WAIT_MAX];
	t_wblock	wb;
	uint32_t	n;
	uint32_t	mode;
}	t_waitset;

typedef struct s_wreg
{
	t_spinlock	lock;
	t_wlink		*heads[WREG_BUCKETS];
}	t_wreg;

typedef struct s_sigstate
{
	uint32_t	state;
	uint32_t	manual;
}	t_sigstate;

typedef struct s_tmrimpl
{
	t_sigstate	sig;
	uint32_t	slot;
	uint32_t	pad;
}	t_tmrimpl;

typedef struct s_tmrslot
{
	t_object	*obj;
	uint64_t	armgen;
	int64_t		tid;
	uint64_t	deadline;
	uint64_t	period;
}	t_tmrslot;

typedef struct s_tmrtab
{
	t_spinlock	lock;
	t_tmrslot	slots[TMR_SLOTS];
}	t_tmrtab;

typedef struct s_taskobj
{
	t_process	*proc;
	t_thread	*thread;
	uint32_t	dead;
	uint32_t	pad;
}	t_taskobj;

typedef struct s_ipcmsg
{
	struct s_ipcmsg	*next;
	t_memacct		*acct;
	uint32_t		len;
	uint32_t		nh;
	t_object		*objs[IPC_HANDLES_MAX];
	uint32_t		rights[IPC_HANDLES_MAX];
	uint8_t			data[];
}	t_ipcmsg;

typedef struct s_msgq
{
	t_ipcmsg	*head;
	t_ipcmsg	*tail;
	uint32_t	count;
	uint32_t	pad;
}	t_msgq;

typedef struct s_grave
{
	t_spinlock	lock;
	t_msgq		q;
	uint32_t	active;
	uint32_t	pad;
}	t_grave;

typedef struct s_chanend
{
	struct s_chanpair	*pair;
	uint32_t			side;
	uint32_t			pad;
}	t_chanend;

typedef struct s_chanpair
{
	t_spinlock	lock;
	t_msgq		q[2];
	t_object	*ends[2];
	t_chanend	impl[2];
	uint32_t	closed[2];
	uint32_t	refs;
	uint32_t	pad;
}	t_chanpair;

typedef struct s_chanrd
{
	t_ipcmsg	*msg;
	uint64_t	buf_len;
	uint64_t	need_len;
	uint32_t	hmax;
	uint32_t	need_nh;
}	t_chanrd;

typedef struct s_listener
{
	struct s_listener	*next;
	t_object			*obj;
	t_msgq				pending;
	uint32_t			namelen;
	uint32_t			registered;
	char				name[IPC_NAME_MAX + 1];
}	t_listener;

typedef struct s_ports
{
	t_spinlock	lock;
	t_listener	*head;
}	t_ports;

typedef struct s_section
{
	uint64_t	*frames;
	uint64_t	npages;
	t_memacct	*acct;
	uint32_t	maxprot;
	uint32_t	pad;
}	t_section;

typedef struct s_secreq
{
	uintptr_t	hint;
	uint64_t	offset;
	uint64_t	len;
	uint32_t	prot;
	uint32_t	pad;
}	t_secreq;

typedef struct s_xfer
{
	t_handle	hs[IPC_HANDLES_MAX];
	t_object	*objs[IPC_HANDLES_MAX];
	uint32_t	rights[IPC_HANDLES_MAX];
	uint32_t	idx[IPC_HANDLES_MAX];
	uint32_t	n;
	uint32_t	move;
}	t_xfer;

typedef struct s_sysent
{
	uint32_t	num;
	t_sysfn		fn;
	const char	*name;
}	t_sysent;

typedef struct s_stping
{
	t_object	*end;
	uint32_t	count;
	int32_t		result;
}	t_stping;

extern t_wreg	g_wreg;
extern t_tmrtab	g_tmr;
extern t_ports	g_ports;

bool		obj_tryref(t_object *obj);
t_htab		*ht_of(t_process *p);
int			ht_find_free(t_htab *ht);
t_hentry	*ht_lookup(t_htab *ht, t_handle h);
t_handle	ht_encode(t_htab *ht, uint32_t idx);
void		ht_free_slot(t_htab *ht, uint32_t idx);
int			ht_grow(t_htab *ht, uint32_t oldcap);
int			ht_reserve_one(t_htab *ht);
int			htab_take(t_htab *ht, t_xfer *x);
void		htab_take_end(t_htab *ht, t_xfer *x, bool commit);
void		xfer_drop(t_xfer *x);
int			htab_reserve(t_htab *ht, t_xfer *x);
void		htab_unreserve(t_htab *ht, t_xfer *x);
int			htab_commit(t_htab *ht, t_xfer *x);
void		htab_close_all(t_htab *ht);
t_memacct	*acct_new(void);
void		acct_ref(t_memacct *a);
void		acct_unref(t_memacct *a);
int			acct_charge(t_memacct *a, uint64_t used, uint64_t lim, uint64_t n);
void		acct_uncharge(t_memacct *a, uint64_t bytes);
void		wreg_init(void);
uint32_t	wreg_bucket(const t_object *o);
void		wreg_add(t_waitset *ws);
void		wreg_del(t_waitset *ws);
void		wreg_notify(t_object *o);
void		wreg_pulse(t_object *o, bool all);
void		wblock_fire(t_wblock *wb);
int			wait_objects(t_object **o, uint32_t n, uint32_t m, uint64_t to);
int			wset_try(t_waitset *ws);
uint64_t	wait_deadline(uint64_t timeout);
int			wait_for(t_object *obj, uint64_t deadline);
void		sig_init(void);
uint64_t	sig_lock(void);
void		sig_unlock(uint64_t flags);
bool		sig_signaled(t_object *o);
bool		sig_is(t_object *o);
bool		sig_take_locked(t_object *o);
void		sig_set(t_object *o, uint32_t v);
int			evt_create(uint32_t manual, uint32_t initial, t_object **out);
int			evt_op(t_object *o, uint32_t op);
void		tmr_init(void);
void		tmr_destroy(t_object *o);
int			tmr_create(uint32_t manual, t_object **out);
int			tmr_set(t_object *o, uint64_t deadline, uint64_t period);
void		tmr_fire(void *ctx);
t_ipcmsg	*msg_alloc(uint32_t len);
void		msg_free(t_ipcmsg *m);
int			msg_charge(t_process *p, t_ipcmsg *m);
void		msg_uncharge(t_ipcmsg *m);
int			msgq_push(t_msgq *q, t_ipcmsg *m, uint32_t max);
void		msgq_push_front(t_msgq *q, t_ipcmsg *m);
t_ipcmsg	*msgq_pop(t_msgq *q);
void		ipc_grave_init(void);
void		ipc_bury(t_msgq *q);
void		ipc_reap(void);
int			chan_create(t_object **a, t_object **b);
void		chan_destroy(t_object *o);
bool		chan_signaled(t_object *o);
int			chan_write(t_object *end, t_ipcmsg *m);
int			chan_read_try(t_object *end, t_chanrd *rd);
void		chan_unread(t_object *end, t_ipcmsg *m);
int			chan_read_wait(t_object *end, t_chanrd *rd, uint64_t deadline);
bool		chan_same_pair(t_object *end, t_object *o);
void		port_init(void);
bool		port_name_ok(const char *name, uint64_t len);
t_listener	*port_find(const char *name, uint32_t len);
int			port_listen(const char *name, uint32_t len, t_object **out);
int			port_connect(const char *name, uint32_t len, t_object **out);
int			port_accept(t_object *lobj, uint64_t deadline, t_object **out);
int			listener_new(const char *name, uint32_t len, t_object **out);
void		listener_destroy(t_object *o);
bool		listener_signaled(t_object *o);
int			section_create(t_process *p, uint64_t z, uint32_t r, t_object **o);
void		section_destroy(t_object *o);
int			sec_charge(t_process *p, t_section *s);
bool		sec_args_ok(uint64_t size, uint32_t prot);
uint64_t	section_frame(t_object *sec, uint64_t page);
int			section_map(t_process *p, t_object *s, t_secreq *r, uintptr_t *v);
int			secmap_insert(t_htab *ht, t_secmap *m);
void		secmaps_cleanup(t_process *p);
void		secmaps_abandon(t_htab *ht);
t_process	*sys_proc(void);
int			sys_handle(uint64_t raw, uint32_t type, uint32_t need, t_hget *out);
int64_t		sys_install(t_object *o, uint32_t rights);
int64_t		sys_close(const t_sysargs *a);
int64_t		sys_dup(const t_sysargs *a);
int64_t		sys_wait(const t_sysargs *a);
int64_t		sys_wait_many(const t_sysargs *a);
int64_t		sys_event_create(const t_sysargs *a);
int64_t		sys_event_op(const t_sysargs *a);
int64_t		sys_timer_create(const t_sysargs *a);
int64_t		sys_timer_set(const t_sysargs *a);
int64_t		sys_section_create(const t_sysargs *a);
int64_t		sys_section_map(const t_sysargs *a);
int64_t		sys_port_listen(const t_sysargs *a);
int64_t		sys_port_connect(const t_sysargs *a);
int64_t		sys_port_accept(const t_sysargs *a);
int64_t		sys_chan_send(const t_sysargs *a);
int64_t		sys_chan_recv(const t_sysargs *a);
int			st_chan(void);
int			st_section(void);
int			st_handles(void);
uint64_t	st_pmm_user(void);
void		secmap_drop(t_aspace *as, t_secmap *m);
void		secmaps_forget(t_process *p, uintptr_t va, uint64_t len);
bool		sec_pages_mapped(t_aspace *as, const t_secmap *m);
uint64_t	sec_pages_unmap(t_aspace *as, const t_secmap *m);
void		sec_pages_rollback(t_aspace *as, const t_secmap *part);
int			secmap_record(t_process *p, t_secmap *m);

#endif
