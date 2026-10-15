#include "obj_int.h"

static const t_sysent	g_obj_sys[] = {
{SYS_CLOSE, sys_close, "close"}, {SYS_DUP, sys_dup, "dup"},
{SYS_WAIT, sys_wait, "wait"}, {SYS_WAIT_MANY, sys_wait_many, "wait_many"},
{SYS_EVENT_CREATE, sys_event_create, "event_create"},
{SYS_EVENT_OP, sys_event_op, "event_op"},
{SYS_SECTION_CREATE, sys_section_create, "section_create"},
{SYS_SECTION_MAP, sys_section_map, "section_map"},
{SYS_PORT_LISTEN, sys_port_listen, "port_listen"},
{SYS_PORT_CONNECT, sys_port_connect, "port_connect"},
{SYS_PORT_ACCEPT, sys_port_accept, "port_accept"},
{SYS_CHAN_SEND, sys_chan_send, "chan_send"},
{SYS_CHAN_RECV, sys_chan_recv, "chan_recv"},
{SYS_TIMER_CREATE, sys_timer_create, "timer_create"},
{SYS_TIMER_SET, sys_timer_set, "timer_set"}, {0, NULL, NULL}
};

int	object_boot_init(void)
{
	int	i;
	int	rc;

	wreg_init();
	sig_init();
	tmr_init();
	port_init();
	ipc_grave_init();
	i = 0;
	while (g_obj_sys[i].fn)
	{
		rc = syscall_register(g_obj_sys[i].num, g_obj_sys[i].fn,
				g_obj_sys[i].name);
		if (rc < 0)
			return (rc);
		i++;
	}
	return (0);
}
