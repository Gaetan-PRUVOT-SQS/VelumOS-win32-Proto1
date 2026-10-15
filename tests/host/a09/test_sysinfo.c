#include <string.h>
#include "a09_test.h"
#include "fake.h"
#include "harness.h"
#include "rng_int.h"

static void	build_champs_et_troncature(void)
{
	t_sysinfo_in	in;
	t_sysinfo		out;
	char			longtext[100];

	memset(&out, 0xaa, sizeof(out));
	memset(longtext, 'x', sizeof(longtext) - 1);
	longtext[sizeof(longtext) - 1] = '\0';
	memset(&in, 0, sizeof(in));
	in.ncpus = 4;
	in.nprocs = 9;
	in.cpu_brand = longtext;
	in.build_id = "abc";
	sysinfo_build(&out, &in);
	h_eq_u64("version d'ABI", out.abi_version, 1);
	h_eq_u64("ncpus", out.ncpus, 4);
	h_eq_u64("champ reserve a zero", out.reserved, 0);
	h_eq_u64("marque tronquee a 47 + NUL", strlen(out.cpu_brand), 47);
	h_true(tail_is_zero(out.cpu_brand, 48), "pas d'octet residuel (marque)");
	h_eq_str("build id", out.build_id, "abc");
	h_true(tail_is_zero(out.build_id, 48), "pas d'octet residuel (build)");
	h_eq_str("nom du systeme", out.os_name, "VelumOS");
	h_true(tail_is_zero(out.os_name, 16), "pas d'octet residuel (os)");
}

static void	build_chaines_absentes(void)
{
	t_sysinfo_in	in;
	t_sysinfo		out;

	memset(&out, 0xaa, sizeof(out));
	memset(&in, 0, sizeof(in));
	sysinfo_build(&out, &in);
	h_eq_str("marque absente", out.cpu_brand, "");
	h_eq_str("build absent", out.build_id, "");
	h_true(tail_is_zero(out.cpu_brand, 48), "marque entierement a zero");
}

static void	collect_valeurs_et_echecs(void)
{
	t_sysinfo	si;

	fake_reset();
	g_fake.ncpus = 4;
	g_fake.nprocs = 7;
	g_fake.now_ns = 123456789;
	g_fake.pmm.total_pages = 1000;
	g_fake.pmm.free_pages = 250;
	g_fake.heap.bytes_live[0] = 100;
	g_fake.heap.bytes_live[HEAP_TAGS - 1] = 23;
	sysinfo_collect(&si);
	h_eq_u64("ncpus", si.ncpus, 4);
	h_eq_u64("nprocs", si.nprocs, 7);
	h_eq_u64("uptime", si.uptime_ns, 123456789);
	h_eq_u64("memoire totale", si.mem_total, 1000 * 4096);
	h_eq_u64("memoire libre", si.mem_free, 250 * 4096);
	h_eq_u64("tas vivant (somme des etiquettes)", si.heap_live, 123);
	h_eq_str("marque du processeur", si.cpu_brand, "Fake CPU @ 1 GHz");
}

static void	collect_nprocs_cas_limites(void)
{
	t_sysinfo	si;

	fake_reset();
	g_fake.nprocs = 7;
	g_fake.alloc_fail = 1;
	sysinfo_collect(&si);
	h_eq_u64("tas refuse : compte juste, aucun tampon", si.nprocs, 7);
	g_fake.alloc_fail = 0;
	g_fake.nprocs = 0;
	sysinfo_collect(&si);
	h_eq_u64("aucun processus", si.nprocs, 0);
	g_fake.nprocs = 256;
	sysinfo_collect(&si);
	h_eq_u64("table pleine", si.nprocs, 256);
	g_fake.nprocs = 0xffffffffu;
	sysinfo_collect(&si);
	h_eq_u64("compte negatif : 0", si.nprocs, 0);
}

int	main(void)
{
	h_begin("a09/sysinfo");
	h_run("sysinfo/build-champs-et-troncature", build_champs_et_troncature);
	h_run("sysinfo/build-chaines-absentes", build_chaines_absentes);
	h_run("sysinfo/collect-valeurs", collect_valeurs_et_echecs);
	h_run("sysinfo/collect-nprocs-limites", collect_nprocs_cas_limites);
	return (h_end());
}
