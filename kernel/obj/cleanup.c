#include "obj_int.h"

void	object_process_cleanup(t_process *p)
{
	t_htab	*ht;

	if (!p)
		return ;
	ht = ht_of(p);
	if (ht)
	{
		htab_close_all(ht);
		secmaps_cleanup(p);
	}
	obj_task_exited(p->obj);
}
