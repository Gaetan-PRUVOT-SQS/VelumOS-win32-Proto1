#ifndef FAKE_F1_BUF_H
# define FAKE_F1_BUF_H

# include <stddef.h>

char	*f1_unterminated(size_t n);
char	*f1_cstr(size_t len);
void	f1_release(char *p, size_t n);
void	f1_fill_argv(const char **argv, int n, const char *s);

#endif
