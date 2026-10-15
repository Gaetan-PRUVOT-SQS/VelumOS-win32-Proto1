#ifndef WMC_INT_H
# define WMC_INT_H

# include <stdbool.h>
# include <stdint.h>
# include "velum/wm.h"

# define WMC_STASH 32
# define WMC_PAGE 4096ull
# define WMC_DIM_MAX 8192

typedef struct s_wmcmsg
{
	uint64_t	buf[WM_MSG_MAX / 8];
	uint32_t	len;
	t_handle	handle;
}	t_wmcmsg;

typedef struct s_wmcstate
{
	t_handle	chan;
	uint32_t	seq;
	bool		lost;
	t_wmhello	info;
	uint32_t	head;
	uint32_t	n;
	t_wmcmsg	stash[WMC_STASH];
	t_wmcmsg	rx;
	t_wmwin		*wins[WM_WIN_PER_CLIENT];
}	t_wmcstate;

extern t_wmcstate	g_wmc;

int64_t		wmsys_connect(const char *name);
int			wmsys_send(t_handle ch, const void *msg, uint32_t len);
int			wmsys_recv(t_handle ch, t_wmcmsg *m, uint64_t timeout_ns);
void		*wmsys_map(t_handle section, uint64_t len);
void		wmsys_unmap(void *va, uint64_t len);
void		wmsys_close(t_handle h);
uint64_t	wmsys_now(void);
void		wmc_hdr(t_wmhdr *h, uint32_t type, uint32_t size, uint32_t window);
int			wmc_send(void *msg);
bool		wmc_valid(const t_wmcmsg *m);
int			wmc_recv(t_wmcmsg *m, uint64_t timeout_ns);
int			wmc_wait(uint32_t seq, uint32_t type, t_wmcmsg *out);
void		wmc_stash_push(const t_wmcmsg *m);
bool		wmc_stash_pop(t_wmcmsg *m);
int			wmc_slot(uint32_t id);
void		wmc_on_message(t_wmcmsg *m);
int			wmc_attach(t_wmwin *w, const t_wmcmsg *m);
void		wmc_detach(t_wmwin *w);
void		wmc_title_copy(char *dst, const char *src);
int			wmc_simple(uint32_t type, uint32_t window, uint32_t value);

#endif
