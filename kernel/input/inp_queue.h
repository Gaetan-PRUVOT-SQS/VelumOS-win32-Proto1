#ifndef INP_QUEUE_H
# define INP_QUEUE_H

# include <stdint.h>
# include "velum/abi/abi_input.h"
# include "velum/object.h"
# include "velum/sync.h"

# define INPQ_LEN 256
# define INPQ_MASK 255
# define INPQ_READERS 8
# define INPQ_TAKE_CHUNK 16

typedef struct s_reader
{
	uint64_t	cur;
	uint64_t	lost;
	uint32_t	kind;
	uint32_t	used;
	t_object	*obj;
}	t_reader;

typedef struct s_inputq
{
	t_spinlock	lock;
	t_inpevent	ring[INPQ_LEN];
	uint64_t	head;
	uint64_t	kcur;
	uint64_t	lost;
	uint32_t	kuse;
	uint32_t	logging;
	t_reader	readers[INPQ_READERS];
}	t_inputq;

extern t_inputq	g_inputq;

void		inpq_init(void);
void		inpq_push_all(const t_inpevent *ev, int n);
uint32_t	inp_kind_of(uint32_t type);
int			inp_kind_match(uint32_t kind, uint32_t type);
t_reader	*reader_acquire(uint32_t kind);
void		reader_attach(t_reader *r, t_object *obj);
void		reader_release(t_reader *r);
uint32_t	reader_take(t_reader *r, t_inpevent *dst, uint32_t max);
void		reader_notify(t_inputq *q, t_reader *r, uint32_t type,
				uint32_t victim);
int			reader_ready(const t_reader *r);
void		inplog_event(const t_inpevent *ev);

#endif
