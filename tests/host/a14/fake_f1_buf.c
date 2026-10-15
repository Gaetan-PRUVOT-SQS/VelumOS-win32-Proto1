#include <string.h>
#include "fake_f1_buf.h"
#include "fake_sys.h"

char	*f1_unterminated(size_t n)
{
	char	*p;

	p = fake_guarded(n);
	if (p)
		memset(p, 'a', n);
	return (p);
}

char	*f1_cstr(size_t len)
{
	char	*p;

	p = f1_unterminated(len + 1);
	if (p)
		p[len] = '\0';
	return (p);
}

void	f1_release(char *p, size_t n)
{
	fake_guarded_free(p, n);
}

void	f1_fill_argv(const char **argv, int n, const char *s)
{
	int	i;

	i = 0;
	while (i < n)
	{
		argv[i] = s;
		i++;
	}
	argv[n] = NULL;
}
