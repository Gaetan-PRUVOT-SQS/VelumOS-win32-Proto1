#include "dr_int.h"

int	dr_tv_settext(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	t_droidview	*v;
	t_dstr16	s;
	t_dref		str;
	int			rc;

	(void)ret;
	rc = dr_view_get(vm, args[0], &v);
	if (rc == 0 && v->kind != DV_TEXT && v->kind != DV_BUTTON)
		rc = dvm_throw(vm, DR_CCE, "TextView attendu");
	if (rc == 0)
		rc = dr_text_of(vm, args[1], &str);
	if (rc != 0)
		return (rc);
	v->text[0] = '\0';
	if (str != DVM_NULL && dvm_string_get(vm, str, &s) == 0)
		dr_utf8(s, v->text, DROID_TEXT_MAX);
	dr_host(vm)->dirty = 1;
	return (0);
}

int	dr_tv_gettext(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	t_droidview	*v;
	int			rc;

	rc = dr_view_get(vm, args[0], &v);
	if (rc == 0 && v->kind != DV_TEXT && v->kind != DV_BUTTON)
		rc = dvm_throw(vm, DR_CCE, "TextView attendu");
	if (rc != 0)
		return (rc);
	return (dr_new8(vm, v->text, ret));
}

static int	dr_ancestor(const t_droid *d, uint32_t id, uint32_t of)
{
	uint32_t	n;

	n = 0;
	while (of != 0 && of <= d->nviews && n++ <= DROID_VIEWS_MAX)
	{
		if (of == id)
			return (1);
		of = d->views[of - 1].parent;
	}
	return (0);
}

int	dr_vg_addview(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	t_droid		*d;
	t_droidview	*g;
	t_droidview	*k;
	int			rc;

	(void)ret;
	d = dr_host(vm);
	rc = dr_view_get(vm, args[0], &g);
	if (rc == 0)
		rc = dr_view_get(vm, args[1], &k);
	if (rc != 0)
		return (rc);
	if (!dr_is(vm, args[0], DR_VG))
		return (dvm_throw(vm, DR_CCE, "ViewGroup attendu"));
	if (k->parent != 0 || k->id == d->root || dr_ancestor(d, k->id, g->id))
		return (dvm_throw(vm, DR_ISE, "vue déjà placée ou cycle"));
	if (g->nkids >= DROID_KIDS_MAX)
		return (dvm_throw(vm, DR_ISE, "trop d'enfants"));
	g->kids[g->nkids++] = k->id;
	k->parent = g->id;
	d->dirty = 1;
	return (0);
}

int	dr_ll_orient(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	t_droidview	*v;
	int			rc;

	(void)ret;
	rc = dr_view_get(vm, args[0], &v);
	if (rc != 0)
		return (rc);
	if (v->kind != DV_LINEAR)
		return (dvm_throw(vm, DR_CCE, "LinearLayout attendu"));
	if (args[1] > DROID_VERTICAL)
		return (dvm_throw(vm, DR_IAE, "orientation inconnue"));
	v->orientation = args[1];
	dr_host(vm)->dirty = 1;
	return (0);
}
