#include "inp_selftest.h"
#include "velum/kinput.h"
#include "velum/libk.h"

static int	selftest_queue(void)
{
	t_inpevent	in;
	t_inpevent	out;
	uint32_t	i;

	while (input_pop(&out) > 0)
		continue ;
	memset(&in, 0, sizeof(in));
	in.type = INP_KEY_DOWN;
	i = 0;
	while (i < 3)
	{
		in.code = 'A' + i;
		input_push(&in);
		i++;
	}
	i = 0;
	while (i < 3)
	{
		if (input_pop(&out) != 1 || out.code != 'A' + i)
			return (-1);
		i++;
	}
	return (-(input_pop(&out) != 0));
}

static int	selftest_overflow(void)
{
	t_inpevent	ev;
	uint64_t	before;
	uint32_t	i;
	uint32_t	kept;

	before = input_lost();
	memset(&ev, 0, sizeof(ev));
	ev.type = INP_WHEEL;
	i = 0;
	while (i < INPUT_QUEUE_LEN + 44)
	{
		ev.code = i;
		input_push(&ev);
		i++;
	}
	kept = 0;
	while (input_pop(&ev) > 0)
	{
		if (kept == 0 && ev.code != 44)
			return (-1);
		kept++;
	}
	if (kept != INPUT_QUEUE_LEN || input_lost() - before != 44)
		return (-2);
	return (0);
}

int	input_selftest(void)
{
	int	rc;

	rc = selftest_queue();
	if (rc == 0)
		rc = selftest_overflow();
	if (rc == 0)
		rc = selftest_scan();
	if (rc == 0)
		rc = selftest_layout();
	if (rc == 0)
		rc = selftest_mouse();
	return (rc);
}
