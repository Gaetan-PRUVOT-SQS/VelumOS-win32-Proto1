#ifndef UPTR_H
# define UPTR_H

# include <stddef.h>
# include <stdint.h>

# define UPTR_E2BIG (-7)
# define UPTR_COPY_MAX 0x100000ull
# define UPTR_STRUCT_MAX 4096u
# define UPTR_SIZE_FIELD 4u

typedef uint64_t	t_uptr;

int	copy_struct_from_user(void *dst, size_t dst_size, size_t min_size,
		t_uptr src);

#endif
