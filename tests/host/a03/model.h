#ifndef MODEL_H
# define MODEL_H

# include <stdint.h>
# include "fake.h"

# define MODEL_PAGES 64
# define MODEL_OPS 4000
# define MODEL_SEED 0x5eed03u

typedef struct s_model
{
	uint8_t		mapped[MODEL_PAGES];
	uint8_t		prot[MODEL_PAGES];
	uint64_t	rng;
	uint32_t	next_prot;
	int			mismatches;
}	t_model;

uint32_t	model_rand(t_model *m);
int			model_expect(const t_model *m, uint32_t op, uint32_t a, uint32_t n);
int			model_call(uint32_t op, uint32_t a, uint32_t n, uint32_t prot);
void		model_update(t_model *m, uint32_t op, uint32_t a, uint32_t n);
void		model_compare(t_model *m);
void		model_fill(t_aspace *as, int reverse);
int			model_diff(t_aspace *a, t_aspace *b);

#endif
