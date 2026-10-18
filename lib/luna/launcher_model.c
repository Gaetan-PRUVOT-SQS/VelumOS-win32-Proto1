#include "velum/libk.h"
#include "launcher_int.h"

void	luna_lm_init(t_lmodel *m)
{
	if (m == NULL)
		return ;
	memset(m, 0, sizeof(*m));
	m->sel = LUNA_LM_NONE;
	m->hot = LUNA_LM_NONE;
}

int	luna_lm_set_apps(t_lmodel *m, const t_lmapp *apps, uint32_t n)
{
	uint32_t	i;

	if (m == NULL || n > LUNA_LM_APPS_MAX || (apps == NULL && n > 0))
		return (-1);
	i = 0;
	while (i < n)
	{
		m->apps[i] = apps[i];
		m->apps[i].label[LUNA_LM_LABEL_MAX - 1] = '\0';
		i++;
	}
	m->napps = n;
	lp_lm_refresh(m);
	return (0);
}

static uint32_t	utf8_encode(uint32_t c, char *out)
{
	if (c < 0x80)
		out[0] = (char)c;
	if (c < 0x80)
		return (1);
	if (c < 0x800)
	{
		out[0] = (char)(0xc0 | (c >> 6));
		out[1] = (char)(0x80 | (c & 0x3f));
		return (2);
	}
	if (c < 0x10000)
	{
		out[0] = (char)(0xe0 | (c >> 12));
		out[1] = (char)(0x80 | ((c >> 6) & 0x3f));
		out[2] = (char)(0x80 | (c & 0x3f));
		return (3);
	}
	out[0] = (char)(0xf0 | (c >> 18));
	out[1] = (char)(0x80 | ((c >> 12) & 0x3f));
	out[2] = (char)(0x80 | ((c >> 6) & 0x3f));
	out[3] = (char)(0x80 | (c & 0x3f));
	return (4);
}

static int	printable(uint32_t c)
{
	if (c < 0x20 || (c >= 0x7f && c < 0xa0))
		return (0);
	return (c <= 0x10ffff && (c < 0xd800 || c > 0xdfff));
}

int	luna_lm_type(t_lmodel *m, uint32_t codepoint)
{
	char		enc[4];
	uint32_t	n;

	if (m == NULL || !printable(codepoint))
		return (0);
	n = utf8_encode(codepoint, enc);
	if (m->qlen + n >= LUNA_LM_QUERY_MAX)
		return (0);
	memcpy(m->query + m->qlen, enc, n);
	m->qlen += n;
	m->query[m->qlen] = '\0';
	lp_lm_refresh(m);
	return (1);
}

int	luna_lm_backspace(t_lmodel *m)
{
	if (m == NULL || m->qlen == 0)
		return (0);
	m->qlen--;
	while (m->qlen > 0 && ((uint8_t)m->query[m->qlen] & 0xc0) == 0x80)
		m->qlen--;
	m->query[m->qlen] = '\0';
	lp_lm_refresh(m);
	return (1);
}

void	luna_lm_clear_query(t_lmodel *m)
{
	if (m == NULL)
		return ;
	m->qlen = 0;
	m->query[0] = '\0';
	lp_lm_refresh(m);
}

void	luna_lm_touch(t_lmodel *m, uint32_t id)
{
	uint32_t	i;

	if (m == NULL)
		return ;
	i = 0;
	while (i < m->nrecent && m->recent[i] != id)
		i++;
	if (i == m->nrecent && m->nrecent < LUNA_LM_RECENTS)
		m->nrecent++;
	if (i == m->nrecent)
		i--;
	while (i > 0)
	{
		m->recent[i] = m->recent[i - 1];
		i--;
	}
	m->recent[0] = id;
	lp_lm_refresh(m);
}
