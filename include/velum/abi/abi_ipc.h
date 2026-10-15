#ifndef ABI_IPC_H
# define ABI_IPC_H

# include <stdint.h>

# define IPC_MSG_MAX 4096
# define IPC_HANDLES_MAX 8
# define IPC_QUEUE_MAX 64
# define IPC_NAME_MAX 64

typedef struct s_chansend
{
	uint64_t	msg;
	uint64_t	len;
	uint64_t	handles;
	uint32_t	nhandles;
	uint32_t	reserved;
}	t_chansend;

typedef struct s_chanrecv
{
	uint64_t	buf;
	uint64_t	buf_len;
	uint64_t	handles;
	uint32_t	handles_max;
	uint32_t	reserved;
	uint64_t	timeout_ns;
	uint64_t	out_len;
	uint32_t	out_nhandles;
	uint32_t	reserved2;
}	t_chanrecv;

#endif
