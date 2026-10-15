#ifndef FAKE_F4_H
# define FAKE_F4_H

# include <stddef.h>
# include <stdint.h>
# include "velum/vstart.h"
# include "velum/vtls.h"

# define F4_WORDS 1200
# define F4_NR_GETPID 39
# define F4_NR_WRITE 1
# define F4_NR_CLOSE 3
# define F4_NR_MMAP 9
# define F4_NR_MUNMAP 11
# define F4_NR_ARCH_PRCTL 158
# define F4_ARCH_GET_FS 0x1003
# define F4_PTR_BASE 0x10000

typedef struct s_f4_stack
{
	uint64_t	w[F4_WORDS];
	size_t		n;
}	t_f4_stack;

void			f4_reset(t_f4_stack *s);
void			f4_push(t_f4_stack *s, uint64_t v);
void			f4_head(t_f4_stack *s, uint64_t argc, uint64_t envc);
void			f4_aux_full(t_f4_stack *s);
void			f4_pairs(t_f4_stack *s, uint64_t count, uint64_t type);
const uint64_t	*f4_open(const t_f4_stack *s, size_t nwords);
void			f4_close(const uint64_t *buf, size_t nwords);
int				f4_parse(const t_f4_stack *s, size_t nwords, t_startinfo *info);
int				f4_model(const uint64_t *w, size_t n);
uint64_t		f4_rng(uint64_t *state);
uint64_t		f4_u(const void *p);
void			f4_fill_random(t_f4_stack *s, uint64_t *rng, int structural);
void			f4_fill_mutated(t_f4_stack *s, uint64_t *rng);
int				f4_fuzz_one(const t_f4_stack *s);
int				f4_hist(int slot);
int64_t			v_syscall6_real(uint64_t num, const uint64_t *args);
int				stub_preserve_check(void);
void			tls_probe(const t_vtcb *tcb, uint64_t *out);

#endif
