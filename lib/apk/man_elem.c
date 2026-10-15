#include "apk_int.h"

static int	on_root(t_manwalk *w)
{
	t_apkmanifest	*m;
	int				r;

	m = w->out;
	if (w->roots++ != 0 || strcmp(w->name, "manifest") != 0)
		return (E_INVAL);
	if (man_package(&w->x, (t_text){m->package, APK_PKG_MAX}) < 0)
	{
		m->reason = APKR_PAQUET;
		return (E_INVAL);
	}
	r = man_u32(&w->x, AXML_ATTR_VERSION_CODE, &m->version_code);
	if (r < 0)
		return (r);
	r = man_str(&w->x, AXML_ATTR_VERSION_NAME,
			(t_text){m->version_name, APK_VNAME_MAX});
	if (r < 0 && r != E_NOENT)
		return (r);
	return (0);
}

static int	on_sdk(t_manwalk *w)
{
	int	r;

	r = man_u32(&w->x, AXML_ATTR_MIN_SDK, &w->out->min_sdk);
	if (r == 0)
		r = man_u32(&w->x, AXML_ATTR_TARGET_SDK, &w->out->target_sdk);
	return (r);
}

static int	on_perm(t_manwalk *w)
{
	t_apkmanifest	*m;
	int				r;

	m = w->out;
	if (m->perm_count >= APK_PERM_MAX)
		return (E_RANGE);
	r = man_str(&w->x, AXML_ATTR_NAME,
			(t_text){m->perms[m->perm_count], APK_PERM_LEN});
	if (r < 0)
		return (r);
	m->perm_count++;
	return (0);
}

static int	on_app(t_manwalk *w)
{
	t_axmlattr	a;
	int			r;

	if (w->apps++ != 0)
		return (E_INVAL);
	w->in_app = 1;
	r = axml_attr_find(&w->x, AXML_ATTR_LABEL, &a);
	if (r == E_NOENT)
		return (0);
	if (r < 0)
		return (r);
	w->label_kind = a.type;
	w->label_ref = a.data;
	if (a.type == RES_T_REFERENCE)
		return (0);
	w->out->reason = APKR_LIBELLE;
	r = man_str(&w->x, AXML_ATTR_LABEL,
			(t_text){w->out->label, APK_LABEL_MAX});
	if (r == 0)
		w->out->reason = APKR_MANIFESTE;
	return (r);
}

int	man_on_start(t_manwalk *w)
{
	w->depth++;
	if (w->depth == 1)
		return (on_root(w));
	if (w->depth == 2 && strcmp(w->name, "uses-sdk") == 0)
		return (on_sdk(w));
	if (w->depth == 2 && strcmp(w->name, "uses-permission") == 0)
		return (on_perm(w));
	if (w->depth == 2 && strcmp(w->name, "application") == 0)
		return (on_app(w));
	return (man_on_child(w));
}
