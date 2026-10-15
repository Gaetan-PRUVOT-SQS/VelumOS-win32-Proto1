#ifndef BOOT_H
# define BOOT_H

# include <stdint.h>

# define BOOT_MAX_RANGES 128
# define BOOT_CMDLINE_MAX 256
# define BOOT_INFO_VERSION 1

typedef enum e_memtype
{
	MEM_USABLE = 0,
	MEM_RESERVED = 1,
	MEM_ACPI_RECLAIM = 2,
	MEM_ACPI_NVS = 3,
	MEM_BAD = 4,
	MEM_BOOT_RECLAIM = 5,
	MEM_KERNEL = 6,
	MEM_FRAMEBUFFER = 7
}	t_memtype;

typedef struct s_memrange
{
	uint64_t	base;
	uint64_t	length;
	uint32_t	type;
	uint32_t	reserved;
}	t_memrange;

typedef struct s_fbinfo
{
	uint64_t	phys;
	uint64_t	virt;
	uint32_t	width;
	uint32_t	height;
	uint32_t	pitch;
	uint32_t	bpp;
	uint8_t		red_shift;
	uint8_t		red_size;
	uint8_t		green_shift;
	uint8_t		green_size;
	uint8_t		blue_shift;
	uint8_t		blue_size;
	uint8_t		reserved[2];
}	t_fbinfo;

typedef struct s_bootinfo
{
	uint32_t	version;
	uint32_t	size;
	uint64_t	hhdm;
	uint64_t	kernel_phys;
	uint64_t	kernel_virt;
	uint64_t	rsdp_phys;
	uint64_t	efi_systab_phys;
	uint64_t	initrd_phys;
	uint64_t	initrd_size;
	int64_t		boot_unix_s;
	uint32_t	efi;
	uint32_t	nranges;
	t_fbinfo	fb;
	t_memrange	ranges[BOOT_MAX_RANGES];
	char		cmdline[BOOT_CMDLINE_MAX];
}	t_bootinfo;

int					boot_collect(void);
const t_bootinfo	*boot_info(void);
t_bootinfo			*boot_info_rw(void);
int					boot_cmdline_has(const char *word);
const char			*boot_cmdline_get(const char *key);
const char			*boot_build_id(void);

#endif
