#ifndef VSTART_H
# define VSTART_H

# include <stddef.h>
# include <stdint.h>

# define AT_NULL 0
# define AT_PHDR 3
# define AT_PHNUM 5
# define AT_PAGESZ 6
# define AT_ENTRY 9
# define AT_RANDOM 25
# define AT_VELUM_ABI 0x7001
# define AT_VELUM_FLAGS 0x7002

# define START_ARGC_MAX 256
# define START_ENVC_MAX 256
# define START_AUX_MAX 64
# define START_WORDS_MAX 1024
# define START_RANDOM_LEN 16
# define START_OK 0
# define START_TRUNCATED 1

typedef struct s_startinfo
{
	uint64_t		argc;
	char			**argv;
	char			**envp;
	uint64_t		envc;
	uint64_t		pagesz;
	const uint8_t	*random;
	const void		*phdr;
	uint64_t		phnum;
	uint64_t		entry;
	uint64_t		abi;
	uint64_t		flags;
}	t_startinfo;

int	start_parse(const uint64_t *sp, size_t nwords, t_startinfo *out);
int	relro_lock(uint64_t start, uint64_t end);

#endif
