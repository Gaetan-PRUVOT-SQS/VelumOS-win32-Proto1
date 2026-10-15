#ifndef WS_SYS_H
# define WS_SYS_H

# include <stdint.h>
# include "velum/abi/abi_input.h"
# include "velum/abi/abi_ipc.h"
# include "velum/abi/abi_types.h"

typedef struct s_wsrx
{
	uint64_t	buf[IPC_MSG_MAX / 8];
	uint32_t	len;
	uint32_t	nhandles;
	t_handle	handles[IPC_HANDLES_MAX];
}	t_wsrx;

int64_t		ws_sys_listen(const char *name);
int64_t		ws_sys_accept(t_handle listener);
int			ws_sys_send(t_handle ch, const void *msg, uint32_t len, t_handle h);
int			ws_sys_recv(t_handle ch, t_wsrx *rx);
int			ws_sys_wait(const t_handle *hs, uint32_t n, uint64_t timeout_ns);
int			ws_sys_close(t_handle h);
int64_t		ws_sys_section(uint64_t size);
void		*ws_sys_map(t_handle section, uint64_t len);
void		ws_sys_unmap(void *va, uint64_t len);
int			ws_sys_display(t_dispinfo *di, void **fb);
int			ws_sys_kcon(uint32_t on);
int64_t		ws_sys_input_open(void);
int			ws_sys_input_read(t_handle h, t_inpevent *ev, uint32_t max);
uint64_t	ws_sys_now(void);
void		*ws_sys_alloc(uint64_t size);
void		ws_sys_free(void *p, uint64_t size);
void		ws_sys_log(const char *msg);

#endif
