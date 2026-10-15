#include "dr_int.h"

static int	dr_is_exc(const char *desc)
{
	size_t	n;

	n = strlen(desc);
	return ((n > 10 && strcmp(desc + n - 10, "Exception;") == 0)
		|| (n > 6 && strcmp(desc + n - 6, "Error;") == 0));
}

static void	dr_ctor(t_dnative *nat, const char *cls, int with_msg)
{
	nat->cls = cls;
	nat->name = "<init>";
	nat->sig = "()V";
	nat->access = ACC_PUBLIC | ACC_CONSTRUCTOR;
	nat->fn = dr_nop;
	if (!with_msg)
		return ;
	nat->sig = "(Ljava/lang/String;)V";
	nat->fn = dr_thr_init_msg;
}

static int	dr_ctors(t_dvm *vm, const t_dbuiltin *tab, uint32_t n)
{
	t_dnative	*nat;
	uint32_t	i;
	uint32_t	k;
	int			rc;

	nat = malloc(sizeof(*nat) * 2 * n);
	if (!nat)
		return (E_NOMEM);
	i = 0;
	k = 0;
	while (i < n)
	{
		if (dr_is_exc(tab[i].desc))
		{
			dr_ctor(&nat[k++], tab[i].desc, 0);
			dr_ctor(&nat[k++], tab[i].desc, 1);
		}
		i++;
	}
	rc = dvm_natives(vm, nat, k);
	free(nat);
	return (rc);
}

static void	dr_reset(t_droid *d, t_dvm *vm)
{
	t_droidlog		log;
	t_droidclock	clock;
	void			*user;

	log = d->log;
	clock = d->clock;
	user = d->user;
	memset(d, 0, sizeof(*d));
	d->log = log;
	d->clock = clock;
	d->user = user;
	d->vm = vm;
	vm->host = d;
}

int	droid_install(t_droid *d, t_dvm *vm)
{
	const t_dbuiltin	*cls;
	const t_dnative		*nat;
	uint32_t			n;
	int					rc;

	if (!d || !vm)
		return (E_INVAL);
	dr_reset(d, vm);
	n = dr_classes(&cls);
	rc = dvm_builtins(vm, cls, n);
	if (rc == 0)
		rc = dr_ctors(vm, cls, n);
	n = dr_tab_lang(&nat);
	if (rc == 0)
		rc = dvm_natives(vm, nat, n);
	n = dr_tab_android(&nat);
	if (rc == 0)
		rc = dvm_natives(vm, nat, n);
	return (rc);
}
