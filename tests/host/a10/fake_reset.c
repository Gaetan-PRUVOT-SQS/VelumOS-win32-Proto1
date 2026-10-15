#include "fake.h"
#include "../../../kernel/input/inp_queue.h"
#include "../../../kernel/input/xlate.h"

void	fake_all_reset(void)
{
	inpq_init();
	xlate_init();
	fobj_reset();
	fproc_clear();
	fuser_reset();
	firq_reset();
	fklog_reset();
	fboot_set("");
	fsys_reset();
	ftime_reset();
	fsync_reset();
}
