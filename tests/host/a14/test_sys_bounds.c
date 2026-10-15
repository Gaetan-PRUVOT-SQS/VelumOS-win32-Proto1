#include "fake_check.h"
#include "fake_f1_buf.h"
#include "fake_sys.h"
#include "harness.h"
#include "velum/velum.h"

static void	bounds_paths(void)
{
	char	*p;
	char	*q;
	t_vstat	st;

	p = f1_unterminated(300);
	q = f1_unterminated(300);
	fake_reset();
	v_open(p, 1, 2);
	sys_is(SYS_OPEN, sys_u(p), VFS_PATH_MAX + 1, 1);
	v_stat(p, &st);
	sys_is(SYS_STAT, sys_u(p), VFS_PATH_MAX + 1, sys_u(&st));
	v_mkdir(p, 0x1ed);
	sys_is(SYS_MKDIR, sys_u(p), VFS_PATH_MAX + 1, 0x1ed);
	v_unlink(p);
	sys_is(SYS_UNLINK, sys_u(p), VFS_PATH_MAX + 1, 0);
	v_rename(p, q);
	sys_is(SYS_RENAME, sys_u(p), VFS_PATH_MAX + 1, sys_u(q));
	sys_is_hi(VFS_PATH_MAX + 1, 0, 0);
	f1_release(p, 300);
	f1_release(q, 300);
}

static void	bounds_names(void)
{
	char	*p;

	p = f1_unterminated(300);
	fake_reset();
	v_spawn(p, NULL, 0, 0);
	sys_is(SYS_PROC_SPAWN, sys_u(p), VFS_PATH_MAX + 1, 0);
	v_port_listen(p, 1);
	sys_is(SYS_PORT_LISTEN, sys_u(p), IPC_NAME_MAX + 1, 1);
	v_port_connect(p, 2);
	sys_is(SYS_PORT_CONNECT, sys_u(p), IPC_NAME_MAX + 1, 2);
	v_log(V_LOG_INFO, p);
	sys_is(SYS_LOG, V_LOG_INFO, sys_u(p), V_LOG_MAX);
	f1_release(p, 300);
}

static void	bounds_exact(void)
{
	char	*path;
	char	*name;
	char	*msg;

	path = f1_unterminated(VFS_PATH_MAX + 1);
	name = f1_unterminated(IPC_NAME_MAX + 1);
	msg = f1_unterminated(V_LOG_MAX);
	fake_reset();
	v_open(path, 0, 0);
	sys_is(SYS_OPEN, sys_u(path), VFS_PATH_MAX + 1, 0);
	v_port_connect(name, 0);
	sys_is(SYS_PORT_CONNECT, sys_u(name), IPC_NAME_MAX + 1, 0);
	v_log(V_LOG_INFO, msg);
	sys_is(SYS_LOG, V_LOG_INFO, sys_u(msg), V_LOG_MAX);
	f1_release(path, VFS_PATH_MAX + 1);
	f1_release(name, IPC_NAME_MAX + 1);
	f1_release(msg, V_LOG_MAX);
}

static void	bounds_limits(void)
{
	char	*path;
	char	*name;
	char	*msg;

	path = f1_cstr(VFS_PATH_MAX);
	name = f1_cstr(IPC_NAME_MAX - 1);
	msg = f1_cstr(V_LOG_MAX - 1);
	fake_reset();
	v_open(path, 0, 0);
	sys_is(SYS_OPEN, sys_u(path), VFS_PATH_MAX, 0);
	v_port_listen(name, 0);
	sys_is(SYS_PORT_LISTEN, sys_u(name), IPC_NAME_MAX - 1, 0);
	v_log(V_LOG_INFO, msg);
	sys_is(SYS_LOG, V_LOG_INFO, sys_u(msg), V_LOG_MAX - 1);
	f1_release(path, VFS_PATH_MAX + 1);
	f1_release(name, IPC_NAME_MAX);
	f1_release(msg, V_LOG_MAX);
}

int	main(void)
{
	h_begin("a14/sys_bounds");
	h_run("chemins/valeur limite : 300 octets non termines bornes a 257",
		bounds_paths);
	h_run("noms, spawn, log/valeur limite : 300 octets bornes a 65, 257, 200",
		bounds_names);
	h_run("bornes/valeur limite : tampon exact sans lecture au-dela",
		bounds_exact);
	h_run("bornes/valeur limite : longueurs 63, 199, 256 inchangees",
		bounds_limits);
	return (h_end());
}
