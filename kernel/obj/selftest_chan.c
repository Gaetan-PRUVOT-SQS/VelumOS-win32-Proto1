#include "obj_int.h"
#include "velum/libk.h"

static void	st_echo(void *arg)
{
	t_stping	*st;
	t_chanrd	rd;
	uint32_t	i;
	int			rc;

	st = arg;
	i = 0;
	rc = 0;
	while (rc == 0 && i < st->count)
	{
		memset(&rd, 0, sizeof(rd));
		rd.buf_len = sizeof(uint32_t);
		rc = chan_read_wait(st->end, &rd, wait_deadline(ST_TIMEOUT));
		if (rc == 0)
			rc = chan_write(st->end, rd.msg);
		if (rc < 0)
			msg_free(rd.msg);
		i++;
	}
	st->result = rc;
	thread_exit(0);
}

static int	st_round(t_object *a, uint32_t i)
{
	t_ipcmsg	*m;
	t_chanrd	rd;
	uint32_t	got;
	int			rc;

	m = msg_alloc(sizeof(uint32_t));
	if (!m)
		return (E_NOMEM);
	memcpy(m->data, &i, sizeof(i));
	rc = chan_write(a, m);
	if (rc < 0)
	{
		msg_free(m);
		return (rc);
	}
	memset(&rd, 0, sizeof(rd));
	rd.buf_len = sizeof(uint32_t);
	rc = chan_read_wait(a, &rd, wait_deadline(ST_TIMEOUT));
	if (rc < 0)
		return (rc);
	memcpy(&got, rd.msg->data, sizeof(got));
	msg_free(rd.msg);
	if (got != i)
		return (E_PROTO);
	return (0);
}

static int	st_loop(t_object *a, uint32_t n)
{
	uint32_t	i;
	int			rc;

	i = 0;
	rc = 0;
	while (rc == 0 && i < n)
	{
		rc = st_round(a, i);
		i++;
	}
	return (rc);
}

static t_thread	*st_spawn(t_stping *st)
{
	t_threadreq	rq;

	memset(&rq, 0, sizeof(rq));
	rq.name = "a08-echo";
	rq.fn = st_echo;
	rq.arg = st;
	rq.prio = PRIO_NORMAL;
	return (thread_create(&rq));
}

int	st_chan(void)
{
	t_object	*a;
	t_object	*b;
	t_stping	st;
	t_thread	*t;
	int			rc;

	if (chan_create(&a, &b) < 0)
		return (E_NOMEM);
	st.end = b;
	st.count = ST_ROUNDS;
	st.result = E_TIMEOUT;
	t = st_spawn(&st);
	rc = E_NOMEM;
	if (t)
		rc = st_loop(a, ST_ROUNDS);
	obj_unref(a);
	if (t && thread_join(t, ST_TIMEOUT) < 0 && rc == 0)
		rc = E_TIMEOUT;
	if (t)
		thread_unref(t);
	obj_unref(b);
	if (rc == 0)
		rc = st.result;
	return (rc);
}
