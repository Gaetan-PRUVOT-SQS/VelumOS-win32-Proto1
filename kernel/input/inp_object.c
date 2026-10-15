#include "inp_queue.h"
#include "velum/err.h"
#include "velum/kinput.h"

int	reader_ready(const t_reader *r)
{
	uint64_t	cur;
	uint64_t	head;

	cur = __atomic_load_n(&r->cur, __ATOMIC_ACQUIRE);
	head = __atomic_load_n(&g_inputq.head, __ATOMIC_ACQUIRE);
	return (cur != head);
}

static void	input_destroy(t_object *obj)
{
	reader_release((t_reader *)obj->impl);
}

static bool	input_signaled(t_object *obj)
{
	return (reader_ready((const t_reader *)obj->impl) != 0);
}

static const t_objops	g_input_ops = {"input", input_destroy, input_signaled};

int	input_open_kind(t_process *p, uint32_t kind, t_handle *out)
{
	t_reader	*r;
	t_object	*obj;
	int			rc;

	if (!p || !out || kind > INPUT_KIND_MOUSE)
		return (E_INVAL);
	r = reader_acquire(kind);
	if (!r)
		return (E_NFILE);
	obj = obj_create(OBJ_INPUT, &g_input_ops, r);
	if (!obj)
	{
		reader_release(r);
		return (E_NOMEM);
	}
	reader_attach(r, obj);
	rc = handle_alloc(p, obj, HR_WAIT | HR_READ, out);
	obj_unref(obj);
	if (rc < 0)
		return (rc);
	return (0);
}

int	input_open(t_process *p, t_handle *out)
{
	return (input_open_kind(p, INPUT_KIND_ALL, out));
}
