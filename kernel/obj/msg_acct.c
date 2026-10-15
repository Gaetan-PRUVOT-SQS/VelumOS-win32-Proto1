#include "obj_int.h"
#include "velum/util.h"

int	msg_charge(t_process *p, t_ipcmsg *m)
{
	t_htab		*ht;
	uint64_t	used;
	int			rc;

	ht = ht_of(p);
	if (!ht)
		return (E_INVAL);
	used = 0;
	if (p->aspace)
		used = vmm_pages_used(p->aspace) * PAGE_SIZE;
	rc = acct_charge(ht->acct, used, p->mem_limit, sizeof(t_ipcmsg) + m->len);
	if (rc < 0)
		return (rc);
	acct_ref(ht->acct);
	m->acct = ht->acct;
	return (0);
}

void	msg_uncharge(t_ipcmsg *m)
{
	if (!m->acct)
		return ;
	acct_uncharge(m->acct, sizeof(t_ipcmsg) + m->len);
	acct_unref(m->acct);
	m->acct = NULL;
}
