#ifndef A09_TEST_H
# define A09_TEST_H

# include <stddef.h>
# include <stdint.h>
# include "velum/pci.h"

# define VEC_MAX_FIELDS 8
# define VEC_LINE_MAX 8192
# define VEC_DIR "tests/host/a09/vectors/"

typedef struct s_vecline
{
	const char	*f[VEC_MAX_FIELDS];
	int			n;
	int			lineno;
}	t_vecline;

size_t	hex_decode(const char *hex, uint8_t *out, size_t max);
void	hex_encode(const uint8_t *in, size_t n, char *out);
void	h_eq_hex(const char *what, const uint8_t *got, size_t n,
			const char *want_hex);
int		vec_foreach(const char *path, void (*cb)(const t_vecline *));
void	fill_pattern(uint8_t *buf, size_t n, uint32_t seed);
void	fill_random(uint8_t *buf, size_t n);
void	fake_user_map(uint64_t base, size_t len, int fill);
void	fake_user_unmap(void);
void	*fake_mmio(uint64_t phys);
void	fake_mmio_reset(void);
void	fake_ecam_window(uint8_t bus_start, uint8_t bus_end);
void	fake_ecam_backend(void);
void	*fab_bars_device(void);
void	*fab_msi_device(uint16_t ctrl);
void	*fab_plain_device(uint8_t line, uint8_t pin);
void	fake_nop_isr(void *ctx);
uint8_t	*fake_ecam_open(void);
void	fake_ecam_poke(uint8_t bus, uint8_t dev, uint32_t vendor_device);
int		bytes_are(const uint8_t *mem, size_t from, size_t to, uint8_t value);
int		tail_is_zero(const char *field, size_t size);

#endif
