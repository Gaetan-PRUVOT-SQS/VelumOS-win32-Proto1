#include "ctl_int.h"

static size_t	text_cost(const char *text)
{
	char	tmp[CTL_TEXT_MAX];

	if (!text)
		return (0);
	ctl_copy_text(tmp, text);
	return ((strlen(tmp) + 8) & ~(size_t)7);
}

size_t	ctl_mnu_pool(const t_ctlmenuitem *it, uint32_t n, uint32_t d)
{
	size_t		total;
	size_t		sub;
	uint32_t	i;

	if (!it || n == 0 || n > CTL_MENU_ITEMS_MAX || d >= CTL_MENU_LEVELS_MAX)
		return (0);
	total = n * sizeof(t_ctlmenuitem);
	i = 0;
	while (i < n)
	{
		total += text_cost(it[i].text);
		if (it[i].sub && it[i].nsub)
		{
			sub = ctl_mnu_pool(it[i].sub, it[i].nsub, d + 1);
			if (sub == 0)
				return (0);
			total += sub;
		}
		i++;
	}
	return (total);
}

static void	*take(t_pool *p, size_t n)
{
	void	*mem;

	if (n > p->left)
		return (NULL);
	mem = p->cur;
	p->cur += n;
	p->left -= n;
	return (mem);
}

static bool	copy_one(t_pool *p, const t_ctlmenuitem *src, t_ctlmenuitem *dst)
{
	char	tmp[CTL_TEXT_MAX];
	char	*txt;

	dst->id = src->id;
	dst->flags = src->flags & (CTL_MI_DISABLED | CTL_MI_CHECKED
			| CTL_MI_SEPARATOR);
	if (src->text)
	{
		ctl_copy_text(tmp, src->text);
		txt = take(p, (strlen(tmp) + 8) & ~(size_t)7);
		if (!txt)
			return (false);
		memcpy(txt, tmp, strlen(tmp) + 1);
		dst->text = txt;
	}
	if (!src->sub || !src->nsub)
		return (true);
	dst->sub = ctl_mnu_copy(p, src->sub, src->nsub);
	dst->nsub = src->nsub;
	return (dst->sub != NULL);
}

t_ctlmenuitem	*ctl_mnu_copy(t_pool *p, const t_ctlmenuitem *it, uint32_t n)
{
	t_ctlmenuitem	*arr;
	uint32_t		i;

	arr = take(p, n * sizeof(t_ctlmenuitem));
	if (!arr)
		return (NULL);
	i = 0;
	while (i < n)
	{
		if (!copy_one(p, &it[i], &arr[i]))
			return (NULL);
		i++;
	}
	return (arr);
}
