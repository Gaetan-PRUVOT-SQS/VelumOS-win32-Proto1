#include "obj_int.h"
#include "velum/heap.h"
#include "velum/libk.h"
#include "velum/pmm.h"
#include "velum/util.h"

static const t_objops	g_sec_ops = {"section", section_destroy, NULL};

static void	sec_frames_free(t_section *s, uint64_t n)
{
	while (n > 0)
	{
		n--;
		pmm_free(s->frames[n], PMM_USER);
	}
}

static int	sec_frames_alloc(t_section *s)
{
	uint64_t	i;

	s->frames = kmalloc_tag(s->npages * sizeof(uint64_t), HEAP_OBJECT);
	if (!s->frames)
		return (E_NOMEM);
	i = 0;
	while (i < s->npages)
	{
		s->frames[i] = pmm_alloc_zero(PMM_USER);
		if (!s->frames[i])
		{
			sec_frames_free(s, i);
			kfree(s->frames);
			s->frames = NULL;
			return (E_NOMEM);
		}
		i++;
	}
	return (0);
}

static void	sec_release(t_section *s)
{
	if (s->frames)
		sec_frames_free(s, s->npages);
	kfree(s->frames);
	acct_uncharge(s->acct, s->npages * PAGE_SIZE);
	acct_unref(s->acct);
	kfree(s);
}

void	section_destroy(t_object *o)
{
	sec_release(o->impl);
}

int	section_create(t_process *p, uint64_t z, uint32_t r, t_object **o)
{
	t_section	*s;
	int			rc;

	if (!ht_of(p) || !o || !sec_args_ok(z, r))
		return (E_INVAL);
	if (z > SEC_SIZE_MAX)
		return (E_NOMEM);
	s = kmalloc_tag(sizeof(t_section), HEAP_OBJECT);
	if (!s)
		return (E_NOMEM);
	memset(s, 0, sizeof(t_section));
	s->npages = align_up(z, PAGE_SIZE) >> PAGE_SHIFT;
	s->maxprot = r;
	rc = sec_charge(p, s);
	if (rc == 0)
		rc = sec_frames_alloc(s);
	*o = NULL;
	if (rc == 0)
		*o = obj_create(OBJ_SECTION, &g_sec_ops, s);
	if (*o)
		return (0);
	sec_release(s);
	if (rc == 0)
		rc = E_NOMEM;
	return (rc);
}
