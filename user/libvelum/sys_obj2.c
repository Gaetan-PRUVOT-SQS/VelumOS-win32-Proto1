#include "sys_int.h"
#include "velum/vobj.h"

int	v_event_op(t_handle event, uint32_t op)
{
	return ((int)sys2(SYS_EVENT_OP, event, op));
}

int64_t	v_section_create(uint64_t size, uint32_t prot)
{
	return (sys2(SYS_SECTION_CREATE, size, prot));
}

int64_t	v_section_map(t_handle section, uint64_t offset, uint64_t len,
		uint32_t prot)
{
	t_vmaprange	range;

	range.offset = offset;
	range.len = len;
	range.prot = prot;
	range.reserved = 0;
	return (v_section_map_at(section, 0, &range));
}

int64_t	v_section_map_at(t_handle section, uint64_t hint_va,
		const t_vmaprange *range)
{
	uint64_t	a[V_SYS_ARGS];

	if (!range)
		return (E_FAULT);
	a[0] = section;
	a[1] = hint_va;
	a[2] = range->prot;
	a[3] = range->offset;
	a[4] = range->len;
	a[5] = 0;
	return (v_syscall6(SYS_SECTION_MAP, a));
}

int64_t	v_port_listen(const char *name, uint32_t flags)
{
	int64_t	len;

	len = sys_cstrlen(name, IPC_NAME_MAX);
	if (len < 0)
		return (len);
	return (sys3(SYS_PORT_LISTEN, sys_ptr(name), (uint64_t)len, flags));
}
