#include "apk_int.h"

int	man_str(const t_axml *x, uint32_t res_id, t_text out)
{
	t_axmlattr	a;
	int			r;

	r = axml_attr_find(x, res_id, &a);
	if (r < 0)
		return (r);
	if (a.type != RES_T_STRING)
		return (E_NOTSUP);
	r = axml_string(x, a.data, out);
	if (r < 0)
		return (r);
	return (apk_text_clean(out.p));
}

int	man_u32(const t_axml *x, uint32_t res_id, uint32_t *out)
{
	t_axmlattr	a;
	int			r;

	r = axml_attr_find(x, res_id, &a);
	if (r == E_NOENT)
		return (0);
	if (r < 0)
		return (r);
	if (a.type != RES_T_INT_DEC && a.type != RES_T_INT_HEX)
		return (E_NOTSUP);
	*out = a.data;
	return (0);
}

int	man_package(const t_axml *x, t_text out)
{
	t_axmlattr	a;
	char		name[APK_ELEM_MAX];
	uint32_t	i;

	i = 0;
	while (i < axml_attr_count(x))
	{
		if (axml_attr(x, i, &a) < 0)
			return (E_INVAL);
		if (a.res_id == 0 && a.type == RES_T_STRING
			&& axml_string(x, a.name, (t_text){name, sizeof(name)}) >= 0
			&& strcmp(name, "package") == 0)
		{
			if (axml_string(x, a.data, out) < 0)
				return (E_INVAL);
			return (0);
		}
		i++;
	}
	return (E_NOENT);
}
