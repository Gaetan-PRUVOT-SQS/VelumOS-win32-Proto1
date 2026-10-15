#ifndef VOBJ_H
# define VOBJ_H

# include <stddef.h>
# include <stdint.h>
# include "velum/abi/abi_ipc.h"
# include "velum/abi/abi_syscall.h"
# include "velum/abi/abi_types.h"

# ifndef HR_ALL
#  define HR_WAIT 0x001
#  define HR_SIGNAL 0x002
#  define HR_READ 0x004
#  define HR_WRITE 0x008
#  define HR_MAP_R 0x010
#  define HR_MAP_W 0x020
#  define HR_MAP_X 0x040
#  define HR_DUP 0x080
#  define HR_TRANSFER 0x100
#  define HR_ALL 0x1ff
# endif

# define WAIT_MANY_MAX 64
# define V_NO_HANDLE 0

typedef struct s_vmaprange
{
	uint64_t	offset;
	uint64_t	len;
	uint32_t	prot;
	uint32_t	reserved;
}	t_vmaprange;

int		v_close(t_handle h);
int64_t	v_dup(t_handle h, uint32_t rights);
int		v_wait(t_handle h, uint64_t timeout_ns);
int		v_wait_many(const t_handle *handles, uint32_t n, uint32_t flags,
			uint64_t timeout_ns);
int64_t	v_event_create(int manual, int initial);
int		v_event_op(t_handle event, uint32_t op);
int64_t	v_section_create(uint64_t size, uint32_t prot);
int64_t	v_section_map(t_handle section, uint64_t offset, uint64_t len,
			uint32_t prot);
int64_t	v_section_map_at(t_handle section, uint64_t hint_va,
			const t_vmaprange *range);
int64_t	v_port_listen(const char *name, uint32_t flags);
int64_t	v_port_connect(const char *name, uint32_t flags);
int64_t	v_port_accept(t_handle listener, uint64_t timeout_ns);
int		v_chan_send(t_handle chan, const t_chansend *msg);
int		v_chan_recv(t_handle chan, t_chanrecv *msg);
int64_t	v_timer_create(int manual);
int		v_timer_set(t_handle timer, uint64_t deadline_ns, uint64_t period_ns);

#endif
