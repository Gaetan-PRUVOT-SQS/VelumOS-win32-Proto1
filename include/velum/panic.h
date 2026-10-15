#ifndef PANIC_H
# define PANIC_H

# include <stdint.h>

struct	s_regs;

_Noreturn void	panic(const char *fmt, ...);
_Noreturn void	panic_regs(const struct s_regs *regs, const char *msg);
void			kassert_check(int cond, const char *msg);
void			panic_set_screen(void (*fn)(const char *t, const char *m));

#endif
