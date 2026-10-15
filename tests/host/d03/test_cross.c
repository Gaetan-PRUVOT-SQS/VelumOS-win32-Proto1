#include "harness.h"
#include "fake.h"
#include "dex_int.h"

static t_dex	g_d;

static void	rej(const char *what, uint32_t off, uint32_t v, uint32_t width)
{
	h_eq_i64(what, fake_patched(off, v, width), E_INVAL);
}

static void	ids(void)
{
	uint32_t	end;
	uint32_t	str;
	uint32_t	list;

	end = (uint32_t)g_d.len & ~3u;
	str = dex_u32(g_d.p + g_d.off[DEX_T_STRING]);
	list = dex_u32(g_d.p + g_d.off[DEX_T_PROTO] + 8);
	rej("chaine hors fichier", g_d.off[DEX_T_STRING], (uint32_t)g_d.len, 4);
	rej("octet MUTF-8 interdit", str + 1, 0xff, 1);
	rej("longueur UTF-16 fausse", str, g_d.p[str] + 1, 1);
	rej("type -> chaine", g_d.off[DEX_T_TYPE], g_d.n[DEX_T_STRING], 4);
	rej("proto -> shorty", g_d.off[DEX_T_PROTO], g_d.n[DEX_T_STRING], 4);
	rej("proto -> retour", g_d.off[DEX_T_PROTO] + 4, g_d.n[DEX_T_TYPE], 4);
	rej("proto -> liste", g_d.off[DEX_T_PROTO] + 8, end, 4);
	rej("liste non alignee", g_d.off[DEX_T_PROTO] + 8, list + 2, 4);
	rej("liste -> type", list + 4, g_d.n[DEX_T_TYPE], 2);
	rej("liste trop longue", list, 0x7fffffff, 4);
}

static void	members(void)
{
	uint32_t	f;
	uint32_t	m;

	f = g_d.off[DEX_T_FIELD];
	m = g_d.off[DEX_T_METHOD];
	rej("champ -> classe", f, g_d.n[DEX_T_TYPE], 2);
	rej("champ -> type", f + 2, g_d.n[DEX_T_TYPE], 2);
	rej("champ -> nom", f + 4, g_d.n[DEX_T_STRING], 4);
	rej("methode -> classe", m, g_d.n[DEX_T_TYPE], 2);
	rej("methode -> proto", m + 2, g_d.n[DEX_T_PROTO], 2);
	rej("methode -> nom", m + 4, g_d.n[DEX_T_STRING], 4);
}

static void	classes(void)
{
	uint32_t	c;
	uint32_t	end;

	c = g_d.off[DEX_T_CLASS];
	end = (uint32_t)g_d.len;
	rej("classe -> type", c, g_d.n[DEX_T_TYPE], 4);
	rej("classe -> parent", c + 8, g_d.n[DEX_T_TYPE], 4);
	h_eq_i64("sans parent", fake_patched(c + 8, DEX_NO_INDEX, 4), E_OK);
	rej("classe -> interfaces", c + 12, end & ~3u, 4);
	rej("classe -> source", c + 16, g_d.n[DEX_T_STRING], 4);
	h_eq_i64("sans source", fake_patched(c + 16, DEX_NO_INDEX, 4), E_OK);
	rej("classe -> annotations", c + 20, end, 4);
	rej("classe -> class_data", c + 24, end, 4);
	rej("classe -> valeurs", c + 28, end, 4);
	rej("seconde classe -> type", c + 32, g_d.n[DEX_T_TYPE], 4);
}

int	main(void)
{
	h_begin("d03/cross");
	h_true(dex_open(&g_d, fake_fixture()) == E_OK, "fixture ouverte");
	h_run("chaines, types, prototypes", ids);
	h_run("champs et methodes", members);
	h_run("classes", classes);
	return (h_end());
}
