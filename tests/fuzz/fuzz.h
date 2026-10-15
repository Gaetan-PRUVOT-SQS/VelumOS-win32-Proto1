#ifndef FUZZ_H
# define FUZZ_H

# include <stddef.h>
# include <stdint.h>

# define FUZZ_REQUIRE(cond) \
	((cond) ? (void)0 : fuzz_violation(#cond, __FILE__, __LINE__))

int		LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);
void	fuzz_violation(const char *what, const char *file, int line)
		__attribute__((noreturn));
void	fuzz_heap_fail_after(int64_t n);
int64_t	fuzz_heap_live(void);

#endif
