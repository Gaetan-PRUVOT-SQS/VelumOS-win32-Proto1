#ifndef FAKE_KERN_H
# define FAKE_KERN_H

# include <stdint.h>
# include "velum/abi/abi_input.h"
# include "velum/abi/abi_ipc.h"
# include "velum/abi/abi_types.h"

# define FK_OBJ_MAX 512
# define FK_HANDLE_MAX 1024
# define FK_LISTEN_Q 16
# define FK_INPUT_MAX 256
# define FK_FREE 0
# define FK_CHAN 1
# define FK_LISTEN 2
# define FK_SECTION 3
# define FK_INPUT 4

typedef struct s_fkmsg
{
	uint8_t		*data;
	uint32_t	len;
	int32_t		obj;
}	t_fkmsg;

typedef struct s_fkobj
{
	int32_t		type;
	int32_t		refs;
	int32_t		maps;
	int32_t		peer;
	uint32_t	head;
	uint32_t	n;
	t_fkmsg		q[IPC_QUEUE_MAX];
	int32_t		pend[FK_LISTEN_Q];
	uint32_t	npend;
	uint8_t		*mem;
	uint64_t	size;
}	t_fkobj;

typedef struct s_fk
{
	t_fkobj		o[FK_OBJ_MAX];
	int32_t		h[FK_HANDLE_MAX];
	uint64_t	now;
	int32_t		listener;
	uint32_t	*fb;
	uint32_t	fb_w;
	uint32_t	fb_h;
	uint32_t	fb_pitch;
	t_inpevent	in[FK_INPUT_MAX];
	uint32_t	in_head;
	uint32_t	in_n;
	int64_t		fail_section;
	int64_t		fail_map;
	int64_t		fail_alloc;
	uint32_t	logs;
	uint32_t	kcon;
	void		(*pump)(void *arg);
	void		*pump_arg;
	uint32_t	pumping;
	const char	*last_log;
}	t_fk;

extern t_fk		g_fk;

void		fk_reset(void);
void		fk_set_display(uint32_t w, uint32_t h);
int			fk_obj_new(int32_t type);
int64_t		fk_handle_new(int obj);
int			fk_obj_of(t_handle h, int32_t type);
void		fk_unref(int obj);
int			fk_close(t_handle h);
int			fk_send(t_handle ch, const void *msg, uint32_t len, t_handle hx);
int			fk_recv(t_handle ch, void *buf, uint32_t max, t_handle *hx);
int			fk_signaled(int obj);
int			fk_fail(int64_t *counter);
void		*fk_map(t_handle h, uint64_t len);
void		fk_unmap(void *va);
uint32_t	fk_live(void);
void		fk_push_input(uint32_t type, uint32_t code, int32_t x, int32_t y);
void		fk_push_key(uint32_t type, uint32_t code, uint32_t mods);
uint32_t	fk_queued(t_handle ch);
int64_t		fk_connect(void);

# define FK_ALLOC_MAX 32

typedef struct s_fkmem
{
	uint32_t	live;
	uint32_t	bad;
	uint32_t	calls;
	void		*p[FK_ALLOC_MAX];
	uint64_t	size[FK_ALLOC_MAX];
}	t_fkmem;

extern t_fkmem	g_fkmem;

void		fk_alloc_reset(void);

#endif
