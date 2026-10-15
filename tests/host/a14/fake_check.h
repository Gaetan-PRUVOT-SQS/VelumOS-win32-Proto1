#ifndef FAKE_CHECK_H
# define FAKE_CHECK_H

# include <stdint.h>

uint64_t	sys_u(const void *p);
void		sys_is(uint64_t num, uint64_t a0, uint64_t a1, uint64_t a2);
void		sys_is_hi(uint64_t a3, uint64_t a4, uint64_t a5);
void		sys_none(void);

#endif
