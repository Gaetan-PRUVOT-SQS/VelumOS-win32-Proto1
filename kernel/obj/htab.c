#include "obj_int.h"
#include "velum/heap.h"
#include "velum/klog.h"
#include "velum/libk.h"

void	*handle_table_create(void)
{
	t_htab	*ht;

	ht = kmalloc_tag(sizeof(t_htab), HEAP_OBJECT);
	if (!ht)
		return (NULL);
	memset(ht, 0, sizeof(t_htab));
	ht->acct = acct_new();
	if (!ht->acct)
	{
		kfree(ht);
		return (NULL);
	}
	spin_init(&ht->lock, "handles");
	mutex_init(&ht->maplock, "secmaps");
	return (ht);
}

void	handle_table_destroy(void *ptr)
{
	t_htab	*ht;

	ht = ptr;
	if (!ht)
		return ;
	htab_close_all(ht);
	if (ht->maps)
	{
		klog_warn("objets: table detruite avec des sections mappees");
		secmaps_abandon(ht);
	}
	acct_unref(ht->acct);
	kfree(ht->ents);
	kfree(ht);
}

static void	ht_swap(t_htab *ht, t_hentry **arr, uint32_t oldcap, uint32_t ncap)
{
	t_hentry	*old;
	uint64_t	fl;

	fl = spin_lock_irqsave(&ht->lock);
	if (ht->cap == oldcap)
	{
		if (oldcap)
			memcpy(*arr, ht->ents, oldcap * sizeof(t_hentry));
		old = ht->ents;
		ht->ents = *arr;
		ht->cap = ncap;
		*arr = old;
	}
	spin_unlock_irqrestore(&ht->lock, fl);
}

int	ht_grow(t_htab *ht, uint32_t oldcap)
{
	t_hentry	*arr;
	uint32_t	ncap;

	ncap = HT_INIT_CAP;
	if (oldcap)
		ncap = oldcap * 2;
	if (ncap > HANDLE_MAX)
		return (E_MFILE);
	arr = kmalloc_tag(ncap * sizeof(t_hentry), HEAP_OBJECT);
	if (!arr)
		return (E_NOMEM);
	memset(arr, 0, ncap * sizeof(t_hentry));
	ht_swap(ht, &arr, oldcap, ncap);
	kfree(arr);
	return (0);
}

int	ht_reserve_one(t_htab *ht)
{
	uint64_t	fl;
	uint32_t	cap;
	int			idx;

	while (1)
	{
		fl = spin_lock_irqsave(&ht->lock);
		cap = ht->cap;
		idx = E_CANCELED;
		if (!ht->closed)
			idx = ht_find_free(ht);
		spin_unlock_irqrestore(&ht->lock, fl);
		if (idx != E_NOSPC)
			return (idx);
		if (cap >= HANDLE_MAX)
			return (E_MFILE);
		idx = ht_grow(ht, cap);
		if (idx < 0)
			return (idx);
	}
}
