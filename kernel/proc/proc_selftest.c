#include "proc_int.h"
#include "velum/err.h"
#include "velum/klog.h"
#include "velum/libk.h"

#define ST_IMG_SIZE 0x100
#define ST_ENTRY 0x78
#define ST_SPAWNS 1000
#define ST_WARMUP 16

static const uint8_t	g_st_code[8] = {0x31, 0xff, 0x31, 0xc0, 0x0f, 0x05,
	0x0f, 0x0b};

static void	st_image(uint8_t *img)
{
	t_elf64_ehdr	eh;
	t_elf64_phdr	ph;

	memset(img, 0, ST_IMG_SIZE);
	memset(&eh, 0, sizeof(eh));
	memset(&ph, 0, sizeof(ph));
	memcpy(eh.ident, "\177ELF\2\1\1", 7);
	eh.type = ELF_ET_DYN;
	eh.machine = ELF_EM_X86_64;
	eh.version = 1;
	eh.entry = ST_ENTRY;
	eh.phoff = sizeof(eh);
	eh.ehsize = sizeof(eh);
	eh.phentsize = sizeof(ph);
	eh.phnum = 1;
	ph.type = ELF_PT_LOAD;
	ph.flags = ELF_PF_R | ELF_PF_X;
	ph.filesz = ST_ENTRY + sizeof(g_st_code);
	ph.memsz = ph.filesz;
	ph.align = ELF_PAGE;
	memcpy(img, &eh, sizeof(eh));
	memcpy(img + sizeof(eh), &ph, sizeof(ph));
	memcpy(img + ST_ENTRY, g_st_code, sizeof(g_st_code));
}

static int	st_spawn_once(const uint8_t *img)
{
	t_spawnreq	rq;
	t_process	*p;
	uint32_t	waited;
	int			rc;

	memset(&rq, 0, sizeof(rq));
	rq.path = "/selftest";
	rq.image = img;
	rq.image_size = ST_IMG_SIZE;
	p = NULL;
	rc = proc_spawn(&rq, &p);
	if (rc < 0)
		return (rc);
	waited = 0;
	while (__atomic_load_n(&p->state, __ATOMIC_ACQUIRE) != PS_ZOMBIE
		&& waited++ < 10000)
		sched_sleep_ns(200000);
	if (p->state != PS_ZOMBIE)
		rc = E_TIMEOUT;
	else if (p->exit_code != 0)
		rc = E_IO;
	proc_unref(p);
	return (rc);
}

static uint64_t	st_free_pages(void)
{
	t_pmm_stats	st;

	if (!pmm_get_stats)
		return (0);
	pmm_get_stats(&st);
	return (st.free_pages);
}

static int	st_spawn_loop(const uint8_t *img, uint32_t n)
{
	uint64_t	t0;
	uint32_t	i;
	int			rc;

	t0 = 0;
	if (time_now_ns)
		t0 = time_now_ns();
	i = 0;
	while (i < n)
	{
		rc = st_spawn_once(img);
		if (rc < 0)
		{
			klog_err("proc: autotest, spawn %u échoue (%d)", i, rc);
			return (rc);
		}
		i++;
	}
	if (time_now_ns)
		klog_info("proc: %u lancements et fins en %llu ms", n,
			(unsigned long long)((time_now_ns() - t0) / 1000000));
	return (0);
}

int	proc_spawn_selftest(void)
{
	uint8_t		*img;
	uint64_t	before;
	uint32_t	waited;
	int			rc;

	if (!__atomic_load_n(&proc_tab()->ready, __ATOMIC_ACQUIRE))
		return (0);
	img = kmalloc_tag(ST_IMG_SIZE, HEAP_PROC);
	if (!img)
		return (1);
	st_image(img);
	rc = st_spawn_loop(img, ST_WARMUP);
	before = st_free_pages();
	if (rc == 0)
		rc = st_spawn_loop(img, ST_SPAWNS);
	waited = 0;
	while (rc == 0 && st_free_pages() != before && waited++ < 500)
		sched_sleep_ns(2000000);
	klog_info("proc: pages libres %llu avant, %llu apres",
		(unsigned long long)before, (unsigned long long)st_free_pages());
	if (rc == 0 && st_free_pages() != before)
		rc = 2;
	kfree(img);
	return (rc != 0);
}
