#ifndef ABI_TYPES_H
# define ABI_TYPES_H

# include <stdint.h>

# define VFS_NAME_MAX 255
# define VFS_PATH_MAX 256
# define S_TYPE_MASK 0xf000
# define S_TYPE_REG 0x8000
# define S_TYPE_DIR 0x4000

typedef uint32_t	t_handle;

typedef struct s_vstat
{
	uint64_t	size;
	uint64_t	mtime_ns;
	uint64_t	ctime_ns;
	uint64_t	inode;
	uint32_t	mode;
	uint32_t	flags;
}	t_vstat;

typedef struct s_dirent
{
	uint64_t	inode;
	uint32_t	type;
	uint32_t	namelen;
	char		name[VFS_NAME_MAX + 1];
}	t_dirent;

typedef struct s_procinfo
{
	uint32_t	pid;
	uint32_t	ppid;
	uint32_t	state;
	uint32_t	flags;
	uint32_t	nthreads;
	uint32_t	reserved;
	uint64_t	mem_bytes;
	uint64_t	cpu_time_ns;
	uint64_t	started_ns;
	char		name[32];
}	t_procinfo;

typedef struct s_sysinfo
{
	uint32_t	abi_version;
	uint32_t	ncpus;
	uint32_t	nprocs;
	uint32_t	reserved;
	uint64_t	uptime_ns;
	uint64_t	mem_total;
	uint64_t	mem_free;
	uint64_t	heap_live;
	char		cpu_brand[48];
	char		build_id[48];
	char		os_name[16];
}	t_sysinfo;

typedef struct s_dispinfo
{
	uint32_t	width;
	uint32_t	height;
	uint32_t	pitch;
	uint32_t	bpp;
	uint32_t	format;
	uint32_t	flags;
	uint64_t	fb_size;
}	t_dispinfo;

typedef struct s_dispmode
{
	uint32_t	width;
	uint32_t	height;
	uint32_t	bpp;
	uint32_t	flags;
}	t_dispmode;

#endif
