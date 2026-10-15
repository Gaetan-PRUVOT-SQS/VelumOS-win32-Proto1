#ifndef HEAP_CFG_H
# define HEAP_CFG_H

# include <stddef.h>
# include <stdint.h>

# ifdef VELUM_DEBUG
#  define HEAP_DEBUG 1
# else
#  define HEAP_DEBUG 0
# endif

# define HEAP_CLASSES 14
# define HEAP_SLAB_ARENAS 3
# define HEAP_ARENAS 4
# define HEAP_LARGE_ARENA 3
# define HEAP_BY16 129
# define HEAP_SMALL_MAX 2048
# define HEAP_ALIGN_MIN 16
# define HEAP_ALIGN_SLAB 64
# define HEAP_ALIGN_MAX 4096
# define HEAP_KEEP_EMPTY 1
# define HEAP_SLAB_HDR 32
# define HEAP_LARGE_HDR 64
# define HEAP_SLAB_MIN_PCT 93
# define HEAP_SHIFT_MIN 14
# define HEAP_SHIFT_MAX 26
# define HEAP_LARGE_MIN 4
# define HEAP_LARGE_MAX 65536
# define HEAP_WORDS_S0 256
# define HEAP_WORDS_S1 128
# define HEAP_WORDS_S2 64
# define HEAP_WORDS_L 1024
# define HEAP_WORDS_ALL 1472
# define HEAP_FILL_ALLOC 0xa5
# define HEAP_FILL_FREE 0xdd
# define HEAP_FILL_ZONE 0xfd
# define HEAP_ZONE_FROM 64
# define HEAP_ZONE_TAIL 16
# define HEAP_SLAB_MAGIC 0x51ab0c11u
# define HEAP_LARGE_MAGIC 0x1a49e00dc0ffee11ull
# define HEAP_ZONE_MAGIC 0x7a0e5ca1ab1e5eedull
# define HEAP_SPIN_LIMIT 0x10000000ull
# define HEAP_SHIFT_KERNEL 26
# define HEAP_LARGE_KERNEL 65536

typedef enum e_heap_kind
{
	HEAP_KIND_NONE = 0,
	HEAP_KIND_SLAB,
	HEAP_KIND_LARGE
}	t_heap_kind;

typedef struct s_heap_layout
{
	uintptr_t	base;
	uint32_t	slab_shift;
	uint32_t	large_pages;
}	t_heap_layout;

typedef struct s_heap_req
{
	size_t		size;
	size_t		align;
	uint32_t	tag;
	const void	*caller;
}	t_heap_req;

typedef struct s_block
{
	size_t		cap;
	size_t		used;
	uint32_t	tag;
	uint32_t	kind;
	uint32_t	unit;
}	t_block;

#endif
