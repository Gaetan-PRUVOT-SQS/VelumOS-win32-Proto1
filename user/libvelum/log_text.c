#include "stdio.h"
#include "string.h"
#include "velum/vmisc.h"

#define LOG_BUF 512

static void	log_chunks(uint32_t level, const char *s, size_t n)
{
	char	tmp[V_LOG_MAX + 1];
	size_t	take;

	while (n)
	{
		take = n;
		if (take > V_LOG_MAX)
			take = V_LOG_MAX;
		memcpy(tmp, s, take);
		tmp[take] = '\0';
		v_log(level, tmp);
		s += take;
		n -= take;
	}
}

static void	log_lines(uint32_t level, const char *s, size_t n)
{
	size_t	i;
	size_t	start;

	i = 0;
	start = 0;
	while (i <= n)
	{
		if (i == n || s[i] == '\n')
		{
			log_chunks(level, s + start, i - start);
			start = i + 1;
		}
		i++;
	}
}

int	v_vlogf(uint32_t level, const char *fmt, va_list ap)
{
	char	buf[LOG_BUF];
	int		n;
	size_t	len;

	n = vsnprintf(buf, sizeof(buf), fmt, ap);
	if (n < 0)
		return (n);
	len = (size_t)n;
	if (len >= sizeof(buf))
		len = sizeof(buf) - 1;
	log_lines(level, buf, len);
	return (n);
}

int	v_logf(uint32_t level, const char *fmt, ...)
{
	va_list	ap;
	int		n;

	va_start(ap, fmt);
	n = v_vlogf(level, fmt, ap);
	va_end(ap);
	return (n);
}
