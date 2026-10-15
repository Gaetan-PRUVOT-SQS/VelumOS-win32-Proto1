#include "harness.h"
#include "th.h"
#include "velum/err.h"

static int	open_all_slots(void)
{
	int	n;

	n = 0;
	while (th_open(INPUT_KIND_ALL, NULL) != 0 && n < 32)
		n++;
	return (n);
}

static void	slots_exhausted(void)
{
	t_handle	h;
	t_handle	extra;

	fake_all_reset();
	h_eq_i64("8 lecteurs possibles", open_all_slots(), INPQ_READERS);
	fproc_set(PF_INPUT);
	h_eq_i64("9e refuse", input_open_kind(proc_current(), 0, &extra), E_NFILE);
	h = 1;
	h_eq_i64("fermeture libere un emplacement", handle_close(proc_current(),
			h), 0);
	h_true(th_open(INPUT_KIND_ALL, NULL) != 0, "reouverture");
}

static void	allocation_failures(void)
{
	t_handle	h;

	fake_all_reset();
	fproc_set(PF_INPUT);
	g_fobj.fail_create = 1;
	h_eq_i64("obj_create echoue", input_open_kind(proc_current(), 0, &h),
		E_NOMEM);
	g_fobj.fail_create = 0;
	g_fobj.fail_handle = 1;
	h_eq_i64("handle_alloc echoue", input_open_kind(proc_current(), 0, &h),
		E_NOMEM);
	g_fobj.fail_handle = 0;
	h_eq_i64("objet detruit apres echec", g_fobj.live, 0);
	h_eq_i64("emplacements tous rendus", open_all_slots(), INPQ_READERS);
}

static void	invalid_arguments(void)
{
	t_handle	h;

	fake_all_reset();
	fproc_set(PF_INPUT);
	h_eq_i64("genre 3", input_open_kind(proc_current(), 3, &h), E_INVAL);
	h_eq_i64("genre enorme", input_open_kind(proc_current(), 0xffffffffu,
			&h), E_INVAL);
	h_eq_i64("processus nul", input_open_kind(NULL, 0, &h), E_INVAL);
	h_eq_i64("sortie nulle", input_open_kind(proc_current(), 0, NULL),
		E_INVAL);
	h_eq_i64("input_open : genre tout", input_open(proc_current(), &h), 0);
	h_eq_i64("aucun emplacement perdu", open_all_slots(), INPQ_READERS - 1);
}

int	main(void)
{
	h_begin("a10/reader_fail");
	h_run("emplacements epuises", slots_exhausted);
	h_run("echecs d'allocation", allocation_failures);
	h_run("arguments invalides", invalid_arguments);
	return (h_end());
}
