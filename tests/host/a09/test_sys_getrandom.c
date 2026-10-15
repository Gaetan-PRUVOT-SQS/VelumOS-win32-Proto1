#include <string.h>
#include "a09_test.h"
#include "fake.h"
#include "harness.h"
#include "rng_int.h"
#include "velum/err.h"

#define USER_BASE 0x10000ull
#define USER_LEN 4096

static int64_t	call_getrandom(uint64_t ptr, uint64_t len, uint64_t flags)
{
	t_sysargs	a;

	memset(&a, 0, sizeof(a));
	a.a[0] = ptr;
	a.a[1] = len;
	a.a[2] = flags;
	return (sys_getrandom(&a));
}

static void	table_de_decision_getrandom(void)
{
	uint64_t	u;

	fake_reset();
	fake_user_map(USER_BASE, USER_LEN, 0xcc);
	u = USER_BASE;
	h_eq_i64("len 16, ptr valide", call_getrandom(u, 16, 0), 16);
	h_true(bytes_are(g_fake.user_mem, 16, USER_LEN, 0xcc),
		"ecriture bornee a len");
	h_eq_i64("len 0, ptr invalide", call_getrandom(0xdead, 0, 0), 0);
	h_eq_i64("len 256", call_getrandom(u, 256, 0), 256);
	h_eq_i64("len 257", call_getrandom(u, 257, 0), E_INVAL);
	h_eq_i64("len 2^63", call_getrandom(u, 1ull << 63, 0), E_INVAL);
	h_eq_i64("drapeau 1", call_getrandom(u, 16, 1), E_INVAL);
	h_eq_i64("drapeaux max", call_getrandom(u, 16, ~0ull), E_INVAL);
	h_eq_i64("pointeur noyau", call_getrandom(0xffff800000000000ull, 16, 0),
		E_FAULT);
	h_eq_i64("pointeur nul", call_getrandom(0, 16, 0), E_FAULT);
	fake_user_unmap();
}

static void	getrandom_a_cheval_sur_la_fin_de_zone(void)
{
	uint64_t	end;

	fake_reset();
	fake_user_map(USER_BASE, USER_LEN, 0xcc);
	end = USER_BASE + USER_LEN;
	h_eq_i64("finit pile a la limite", call_getrandom(end - 16, 16, 0), 16);
	h_eq_i64("deborde d'un octet", call_getrandom(end - 15, 16, 0), E_FAULT);
	h_eq_i64("deborde 64 bits", call_getrandom(~0ull - 7, 16, 0), E_FAULT);
	h_true(bytes_are(g_fake.user_mem, 0, USER_LEN - 16, 0xcc),
		"rien avant la fin");
	fake_user_unmap();
}

static void	getrandom_donne_des_octets_differents(void)
{
	uint8_t	a[256];
	uint8_t	b[256];

	fake_reset();
	fake_user_map(USER_BASE, USER_LEN, 0);
	call_getrandom(USER_BASE, 256, 0);
	memcpy(a, g_fake.user_mem, sizeof(a));
	call_getrandom(USER_BASE, 256, 0);
	memcpy(b, g_fake.user_mem, sizeof(b));
	h_true(memcmp(a, b, sizeof(a)) != 0, "deux appels, deux tirages");
	h_eq_i64("verrous desequilibres", g_fake.lock_errors, 0);
	fake_user_unmap();
}

int	main(void)
{
	h_begin("a09/sys_getrandom");
	h_run("getrandom/table-flags-len-pointeur", table_de_decision_getrandom);
	h_run("getrandom/fin-de-zone-utilisateur",
		getrandom_a_cheval_sur_la_fin_de_zone);
	h_run("getrandom/tirages-distincts", getrandom_donne_des_octets_differents);
	return (h_end());
}
