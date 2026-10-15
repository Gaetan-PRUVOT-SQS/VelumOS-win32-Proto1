#include "dr_int.h"

const t_droidview	*droid_view(const t_droid *d, uint32_t view_id)
{
	if (!d || view_id == 0 || view_id > d->nviews
		|| view_id > DROID_VIEWS_MAX)
		return (NULL);
	return (&d->views[view_id - 1]);
}

void	dr_release(t_droid *d)
{
	uint32_t	i;

	i = 0;
	while (i < d->nviews && i < DROID_VIEWS_MAX)
	{
		dvm_unpin(d->vm, d->views[i].self);
		dvm_unpin(d->vm, d->views[i].listener);
		i++;
	}
	dvm_unpin(d->vm, d->activity);
	d->activity = DVM_NULL;
	d->nviews = 0;
	d->root = 0;
	d->dirty = 1;
}
