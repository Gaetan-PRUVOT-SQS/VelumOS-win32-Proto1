#ifndef RNG_INT_H
# define RNG_INT_H

# include <stddef.h>
# include <stdint.h>
# include "../../lib/crypto/chacha20.h"
# include "velum/abi/abi_types.h"
# include "velum/crypto.h"
# include "velum/ksyscall.h"
# include "velum/sync.h"

# define RNG_SEED_LEN 32
# define RNG_CHUNK 32
# define RNG_ADD_CHUNK 128
# define RNG_RESEED_BYTES 1048576ull
# define RNG_RESEED_NS 300000000000ull
# define RNG_RESEED_EARLY_NS 1000000000ull
# define RNG_POOL_TRIGGER 64
# define RNG_JITTER_SAMPLES 128
# define RNG_HW_WORDS 4
# define RNG_HW_RETRIES 10
# define RNG_SRC_RDSEED 0x1
# define RNG_SRC_RDRAND 0x2
# define RNG_SRC_JITTER 0x4
# define RNG_SRC_TIME 0x8
# define RANDOM_SYS_MAX 256
# define SYSINFO_PROC_MAX 256

typedef struct s_rng_env
{
	uint64_t	(*now_ns)(void);
	uint64_t	(*cycles)(void);
	int			(*rdseed)(uint64_t *out);
	int			(*rdrand)(uint64_t *out);
}	t_rng_env;

typedef struct s_rng
{
	t_spinlock		lock;
	const t_rng_env	*env;
	t_sha256		pool;
	uint64_t		pool_bytes;
	uint64_t		since_reseed;
	uint64_t		last_reseed_ns;
	uint64_t		reseeds;
	uint32_t		ready;
	uint32_t		seeded;
	uint8_t			key[CHACHA20_KEY_LEN];
}	t_rng;

typedef struct s_sysinfo_in
{
	uint32_t	ncpus;
	uint32_t	nprocs;
	uint64_t	uptime_ns;
	uint64_t	mem_total;
	uint64_t	mem_free;
	uint64_t	heap_live;
	const char	*cpu_brand;
	const char	*build_id;
}	t_sysinfo_in;

extern const t_rng_env	g_rng_env;

uint64_t	rng_x86_rdtsc(void);
int			rng_x86_rdrand(uint64_t *out);
int			rng_x86_rdseed(uint64_t *out);
uint32_t	rng_gather(const t_rng_env *env, uint8_t out[RNG_SEED_LEN]);
void		rng_init(t_rng *r, const t_rng_env *env);
uint64_t	rng_now(const t_rng *r);
uint32_t	rng_reseed(t_rng *r);
void		rng_read(t_rng *r, void *buf, size_t n);
void		rng_add(t_rng *r, const void *data, size_t n);
t_rng		*rng_global(void);
void		sysinfo_build(t_sysinfo *out, const t_sysinfo_in *in);
void		sysinfo_collect(t_sysinfo *out);
int64_t		sys_getrandom(const t_sysargs *a);
int64_t		sys_sysinfo(const t_sysargs *a);
int			crypto_selftest(void);
int			rng_selftest(void);
int			sys_selftest(void);

#endif
