#ifndef CRT_INT_H
# define CRT_INT_H

# include <stdint.h>

# define CRT_EXIT_FAIL 127
# define CRT_EXIT_ABI 126
# define CRT_EXIT_SMASH 134
# define CRT_GUARD_LEN 8

int				main(int argc, char **argv);
uint64_t		v_relro_start(void);
uint64_t		v_relro_end(void);
void			v_guard_set(uint64_t value);
_Noreturn void	__velum_start(void *sp) __attribute__((no_stack_protector));
_Noreturn void	crt_die(const char *msg, int code);

#endif
