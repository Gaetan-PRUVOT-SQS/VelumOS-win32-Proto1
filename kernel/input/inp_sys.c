#include "inp_queue.h"
#include "inp_sys.h"
#include "velum/err.h"
#include "velum/kinput.h"
#include "velum/vmm.h"

int64_t	sys_input_open(const t_sysargs *args)
{
	t_process	*p;
	t_handle	h;
	int			rc;

	p = proc_current();
	if (!p || !(p->flags & PF_INPUT))
		return (E_PERM);
	if (args->a[0] > INPUT_KIND_MOUSE)
		return (E_INVAL);
	rc = input_open_kind(p, (uint32_t)args->a[0], &h);
	if (rc < 0)
		return (rc);
	return ((int64_t)h);
}

static int64_t	read_chunks(t_reader *r, t_uptr out, uint64_t max)
{
	t_inpevent	buf[INPQ_TAKE_CHUNK];
	uint64_t	total;
	uint32_t	want;
	uint32_t	n;
	int			rc;

	total = 0;
	while (total < max)
	{
		want = INPQ_TAKE_CHUNK;
		if (max - total < want)
			want = (uint32_t)(max - total);
		n = reader_take(r, buf, want);
		if (n == 0)
			break ;
		rc = copy_to_user(out + total * sizeof(t_inpevent), buf,
				n * sizeof(t_inpevent));
		if (rc < 0 && total == 0)
			return (rc);
		if (rc < 0)
			break ;
		total += n;
	}
	return ((int64_t)total);
}

int64_t	sys_input_read(const t_sysargs *args)
{
	t_process	*p;
	t_hget		g;
	int64_t		rc;
	uint64_t	max;

	max = args->a[2];
	if (args->a[0] > 0xffffffffull)
		return (E_BADF);
	if (max == 0)
		return (0);
	if (max > INPQ_LEN)
		max = INPQ_LEN;
	if (!user_range_ok(args->a[1], max * sizeof(t_inpevent)))
		return (E_FAULT);
	p = proc_current();
	if (!p)
		return (E_PERM);
	rc = handle_get(p, (t_handle)args->a[0], OBJ_INPUT, &g);
	if (rc < 0)
		return (rc);
	rc = E_PERM;
	if (g.rights & HR_READ)
		rc = read_chunks((t_reader *)g.obj->impl, args->a[1], max);
	obj_unref(g.obj);
	return (rc);
}

int	inp_sys_register(void)
{
	int	fails;

	fails = 0;
	fails += syscall_register(SYS_INPUT_OPEN, sys_input_open, "input_open") < 0;
	fails += syscall_register(SYS_INPUT_READ, sys_input_read, "input_read") < 0;
	fails += syscall_register(SYS_INPUT_LAYOUT, sys_input_layout,
			"input_layout") < 0;
	fails += syscall_register(SYS_INPUT_LEDS, sys_input_leds, "input_leds") < 0;
	fails += syscall_register(SYS_INPUT_MOUSE_CFG, sys_input_mouse_cfg,
			"input_mouse_cfg") < 0;
	return (fails);
}
