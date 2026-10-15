#include <stdio.h>
#include <string.h>
#include "velum/err.h"
#include "harness.h"
#include "fake.h"

#define BIGVA 0x1000000ull
#define BIGLEN (UPTR_COPY_MAX + 4096)

typedef struct s_demande
{
	uint32_t	size;
	uint32_t	a;
	uint64_t	b;
}	t_demande;

static uint8_t	g_big[BIGLEN];

static void	pose_demande(t_uptr va, uint32_t size, uint32_t a, uint64_t b)
{
	t_demande	d;

	d.size = size;
	d.a = a;
	d.b = b;
	copy_to_user(va, &d, sizeof(d));
}

static void	plafond_de_copie(void)
{
	t_aspace	*as;

	as = fake_user_env();
	h_eq_i64("grande zone", vmm_alloc(as, BIGVA, BIGLEN,
			VM_R | VM_W | VM_USER), 0);
	h_eq_i64("lecture au plafond", copy_from_user(g_big, BIGVA,
			UPTR_COPY_MAX), 0);
	h_eq_i64("ecriture au plafond", copy_to_user(BIGVA, g_big,
			UPTR_COPY_MAX), 0);
	h_eq_i64("lecture plafond+1", copy_from_user(g_big, BIGVA,
			UPTR_COPY_MAX + 1), UPTR_E2BIG);
	h_eq_i64("ecriture plafond+1", copy_to_user(BIGVA, g_big,
			UPTR_COPY_MAX + 1), UPTR_E2BIG);
	h_eq_i64("longueur SIZE_MAX", copy_from_user(g_big, BIGVA, SIZE_MAX),
		UPTR_E2BIG);
	h_true(user_range_ok(BIGVA, UPTR_COPY_MAX), "plage au plafond");
	h_true(!user_range_ok(BIGVA, UPTR_COPY_MAX + 1), "plage plafond+1");
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	plafond_et_limites_voisines(void)
{
	t_aspace	*as;
	char		buf[16];

	as = fake_user_env();
	h_eq_i64("len 0 adresse noyau", copy_from_user(buf, KERNEL_BASE, 0), 0);
	h_eq_i64("len 0 debordante", copy_from_user(buf,
			0xffffffffffffffffull, 0), 0);
	h_eq_i64("ptr+len deborde", copy_from_user(buf, 0xfffffffffffffff8ull,
			16), E_FAULT);
	h_eq_i64("adresse noyau", copy_to_user(KERNEL_BASE, buf, 8), E_FAULT);
	h_eq_i64("derniere page user", copy_from_user(buf, USER_TOP - 8, 16),
		E_FAULT);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	structure_versionnee_acceptee(void)
{
	t_aspace	*as;
	t_demande	d;

	as = fake_user_env();
	pose_demande(UVA, 16, 7, 0x1122334455667788ull);
	memset(&d, 0xaa, sizeof(d));
	h_eq_i64("taille exacte", copy_struct_from_user(&d, sizeof(d), 8, UVA),
		0);
	h_eq_u64("champ a", d.a, 7);
	h_eq_u64("champ b", d.b, 0x1122334455667788ull);
	h_eq_u64("taille retenue", d.size, 16);
	pose_demande(UVA, 8, 9, 0x5555);
	memset(&d, 0xaa, sizeof(d));
	h_eq_i64("ancienne appli", copy_struct_from_user(&d, sizeof(d), 8, UVA),
		0);
	h_eq_u64("a recopie", d.a, 9);
	h_eq_u64("fin mise a zero", d.b, 0);
	h_eq_u64("taille ancienne", d.size, 8);
	pose_demande(UVA, 24, 3, 4);
	memset(&d, 0xaa, sizeof(d));
	h_eq_i64("appli recente queue nulle", copy_struct_from_user(&d,
			sizeof(d), 8, UVA), 0);
	h_eq_u64("taille tronquee", d.size, 16);
	h_eq_u64("b conserve", d.b, 4);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	structure_versionnee_refusee(void)
{
	t_aspace	*as;
	t_demande	d;
	uint8_t		un;

	as = fake_user_env();
	un = 1;
	pose_demande(UVA, 24, 3, 4);
	copy_to_user(UVA + 20, &un, 1);
	memset(&d, 0xaa, sizeof(d));
	h_eq_i64("queue non nulle", copy_struct_from_user(&d, sizeof(d), 8, UVA),
		UPTR_E2BIG);
	h_eq_u64("destination effacee", d.a, 0);
	pose_demande(UVA, 4096, 1, 1);
	copy_to_user(UVA + 4095, &un, 1);
	h_eq_i64("dernier octet de queue", copy_struct_from_user(&d, sizeof(d),
			8, UVA), UPTR_E2BIG);
	un = 0;
	copy_to_user(UVA + 4095, &un, 1);
	copy_to_user(UVA + 20, &un, 1);
	h_eq_i64("taille au plafond", copy_struct_from_user(&d, sizeof(d), 8,
			UVA), 0);
	pose_demande(UVA, UPTR_STRUCT_MAX + 1, 1, 1);
	h_eq_i64("plafond+1", copy_struct_from_user(&d, sizeof(d), 8, UVA),
		UPTR_E2BIG);
	pose_demande(UVA, 0xffffffffu, 1, 1);
	h_eq_i64("taille enorme", copy_struct_from_user(&d, sizeof(d), 8, UVA),
		UPTR_E2BIG);
	pose_demande(UVA, 7, 1, 1);
	h_eq_i64("sous le minimum", copy_struct_from_user(&d, sizeof(d), 8, UVA),
		E_INVAL);
	pose_demande(UVA, 0, 1, 1);
	h_eq_i64("taille 0", copy_struct_from_user(&d, sizeof(d), 8, UVA),
		E_INVAL);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	structure_adresses_et_appelant(void)
{
	t_aspace	*as;
	t_demande	d;

	as = fake_user_env();
	h_eq_i64("ptr deborde", copy_struct_from_user(&d, sizeof(d), 8,
			0xfffffffffffffff0ull), E_FAULT);
	h_eq_i64("adresse noyau", copy_struct_from_user(&d, sizeof(d), 8,
			KERNEL_BASE), E_FAULT);
	h_eq_i64("adresse 0", copy_struct_from_user(&d, sizeof(d), 8, 0),
		E_FAULT);
	d.size = 16;
	d.a = 1;
	d.b = 1;
	memcpy(fake_ptr(as, UVA + 3 * 4096 - 8), &d, 16);
	h_eq_i64("a cheval sur non mappee", copy_struct_from_user(&d, sizeof(d),
			8, UVA + 3 * 4096 - 8), E_FAULT);
	h_eq_i64("destination nulle", copy_struct_from_user(NULL, 16, 8, UVA),
		E_FAULT);
	h_eq_i64("minimum > destination", copy_struct_from_user(&d, 8, 16, UVA),
		E_INVAL);
	h_eq_i64("destination 0", copy_struct_from_user(&d, 0, 0, UVA),
		E_INVAL);
	h_eq_i64("minimum sous le champ taille", copy_struct_from_user(&d,
			sizeof(d), 2, UVA), E_INVAL);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

#define PAGE 4096ull
#define IMAGE_LEN 4200u
#define BAS_USER USER_MIN
#define HAUT_USER (USER_TOP - PAGE)

typedef struct s_acces
{
	uint64_t	a;
	uint64_t	n;
}	t_acces;

typedef struct s_place
{
	uint64_t	va;
	uint64_t	dispo;
}	t_place;

static bool	modele_page_lisible(uint64_t p)
{
	if (p == BAS_USER || p == HAUT_USER)
		return (true);
	return (p >= UVA && p < UVA + 3 * PAGE);
}

static bool	modele_page_ecrivable(uint64_t p)
{
	if (p >= UVA + 2 * PAGE && p < UVA + 3 * PAGE)
		return (false);
	return (modele_page_lisible(p));
}

static int	modele_acces(uint64_t a, uint64_t n, bool ecriture)
{
	unsigned __int128	fin;
	uint64_t			p;

	if (n == 0)
		return (0);
	if (n > UPTR_COPY_MAX)
		return (UPTR_E2BIG);
	fin = (unsigned __int128)a + n;
	if (a < USER_MIN || fin > USER_TOP)
		return (E_FAULT);
	p = a - a % PAGE;
	while (p < fin)
	{
		if (ecriture && !modele_page_ecrivable(p))
			return (E_FAULT);
		if (!ecriture && !modele_page_lisible(p))
			return (E_FAULT);
		p += PAGE;
	}
	return (0);
}

static t_aspace	*env_bornes(void)
{
	t_aspace	*as;

	as = fake_user_env();
	vmm_alloc(as, BAS_USER, PAGE, VM_R | VM_W | VM_USER);
	vmm_alloc(as, HAUT_USER, PAGE, VM_R | VM_W | VM_USER);
	return (as);
}

static void	acces_contre_modele(void)
{
	static const uint64_t	adresses[] = {0, 1, BAS_USER - 1, BAS_USER,
		BAS_USER + PAGE - 1, BAS_USER + PAGE, UVA - 1, UVA, UVA + PAGE - 1,
		UVA + 2 * PAGE - 1, UVA + 2 * PAGE, UVA + 3 * PAGE - 1,
		UVA + 3 * PAGE, UVA + 4 * PAGE, HAUT_USER - 1, HAUT_USER,
		USER_TOP - 8, USER_TOP - 1, USER_TOP, USER_TOP + 1, KERNEL_BASE,
		0x8000000000000000ull, 0xfffffffffffffffcull, 0xffffffffffffffffull};
	static const uint64_t	longueurs[] = {0, 1, 2, 8, PAGE - 1, PAGE,
		PAGE + 1, 2 * PAGE, 3 * PAGE, UPTR_COPY_MAX, UPTR_COPY_MAX + 1,
		0x8000000000000000ull, 0xfffffffffffffff0ull, 0xffffffffffffffffull};
	t_aspace	*as;
	char		nom[96];
	size_t		i;
	size_t		j;

	as = env_bornes();
	for (i = 0; i < sizeof(adresses) / sizeof(adresses[0]); i++)
	{
		for (j = 0; j < sizeof(longueurs) / sizeof(longueurs[0]); j++)
		{
			snprintf(nom, sizeof(nom), "lecture a=%#llx n=%#llx",
				(unsigned long long)adresses[i],
				(unsigned long long)longueurs[j]);
			h_eq_i64(nom, copy_from_user(g_big, adresses[i], longueurs[j]),
				modele_acces(adresses[i], longueurs[j], false));
			snprintf(nom, sizeof(nom), "ecriture a=%#llx n=%#llx",
				(unsigned long long)adresses[i],
				(unsigned long long)longueurs[j]);
			h_eq_i64(nom, copy_to_user(adresses[i], g_big, longueurs[j]),
				modele_acces(adresses[i], longueurs[j], true));
			snprintf(nom, sizeof(nom), "plage a=%#llx n=%#llx",
				(unsigned long long)adresses[i],
				(unsigned long long)longueurs[j]);
			h_true(user_range_ok(adresses[i], longueurs[j])
				== (modele_acces(adresses[i], longueurs[j], false) == 0), nom);
		}
	}
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	frontiere_exacte_du_haut(void)
{
	t_aspace	*as;
	uint8_t		octet;

	as = env_bornes();
	octet = 0x5a;
	h_eq_i64("dernier octet user ecrit", copy_to_user(USER_TOP - 1, &octet,
			1), 0);
	octet = 0;
	h_eq_i64("dernier octet user lu", copy_from_user(&octet, USER_TOP - 1,
			1), 0);
	h_eq_u64("dernier octet relu", octet, 0x5a);
	h_eq_i64("un octet au dela", copy_from_user(&octet, USER_TOP, 1),
		E_FAULT);
	h_eq_i64("deux octets sur la limite", copy_from_user(g_big,
			USER_TOP - 1, 2), E_FAULT);
	h_eq_i64("premier octet user", copy_from_user(&octet, USER_MIN, 1), 0);
	h_eq_i64("juste sous le premier octet", copy_from_user(&octet,
			USER_MIN - 1, 1), E_FAULT);
	h_eq_i64("a cheval sous USER_MIN", copy_from_user(g_big, USER_MIN - 1,
			2), E_FAULT);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static uint8_t	modele_octet(size_t i)
{
	return ((uint8_t)(0x40u + i * 7u));
}

static uint32_t	modele_lit32(const uint8_t *m)
{
	return ((uint32_t)m[0] | (uint32_t)m[1] << 8 | (uint32_t)m[2] << 16
		| (uint32_t)m[3] << 24);
}

static int	modele_struct(const uint8_t *mem, size_t dispo, size_t dst_size,
		size_t min_size, uint8_t *attendu)
{
	size_t		claimed;
	size_t		known;
	size_t		i;

	memset(attendu, 0, dst_size);
	if (dst_size < 4 || dst_size > 4096 || min_size < 4 || min_size > dst_size)
		return (E_INVAL);
	if (dispo < 4)
		return (E_FAULT);
	claimed = modele_lit32(mem);
	if (claimed > 4096)
		return (UPTR_E2BIG);
	if (claimed < min_size)
		return (E_INVAL);
	if (claimed > dispo)
		return (E_FAULT);
	for (i = dst_size; i < claimed; i++)
	{
		if (mem[i])
			return (UPTR_E2BIG);
	}
	known = claimed < dst_size ? claimed : dst_size;
	memcpy(attendu, mem, known);
	attendu[0] = (uint8_t)known;
	attendu[1] = (uint8_t)(known >> 8);
	attendu[2] = 0;
	attendu[3] = 0;
	return (0);
}

static void	image_struct(uint8_t *img, uint32_t claimed, int queue)
{
	size_t	i;

	memset(img, 0xaa, IMAGE_LEN);
	for (i = 0; i < claimed && i < IMAGE_LEN; i++)
		img[i] = modele_octet(i);
	img[0] = (uint8_t)claimed;
	img[1] = (uint8_t)(claimed >> 8);
	img[2] = (uint8_t)(claimed >> 16);
	img[3] = (uint8_t)(claimed >> 24);
	if (claimed > 4096 || claimed < 4)
		return ;
	if (queue == 1)
		img[claimed - 1] = 0x01;
	if (queue == 2 && claimed > 16)
		img[16] = 0x80;
}

static void	pose_image(t_aspace *as, const t_place *pl, const uint8_t *img)
{
	uint64_t	n;

	n = pl->dispo < IMAGE_LEN ? pl->dispo : IMAGE_LEN;
	if (pl->va == UVA + 3 * PAGE - 16)
		memcpy(fake_ptr(as, pl->va), img, n);
	else
		copy_to_user(pl->va, img, n);
}

static void	structure_contre_modele(void)
{
	static const uint32_t	tailles[] = {0, 3, 4, 7, 8, 15, 16, 17, 24, 32,
		4095, 4096, 4097, 0xffffffffu};
	static const size_t		dsts[] = {3, 4, 8, 16, 32, 4096, 4097};
	static const size_t		mins[] = {0, 4, 8, 16, 32};
	static const t_place	places[] = {{UVA, 3 * PAGE},
		{UVA + 3 * PAGE - 16, 16}, {USER_TOP - 8, 8}};
	static uint8_t			img[IMAGE_LEN];
	static uint8_t			dst[4200];
	static uint8_t			attendu[4200];
	t_aspace				*as;
	char					nom[96];
	size_t					c;
	size_t					d;
	size_t					m;
	size_t					p;
	int						queue;
	int						rc;

	as = env_bornes();
	for (c = 0; c < sizeof(tailles) / sizeof(tailles[0]); c++)
	{
		for (queue = 0; queue < 3; queue++)
		{
			image_struct(img, tailles[c], queue);
			for (p = 0; p < 3; p++)
			{
				pose_image(as, &places[p], img);
				if (queue && tailles[c] > places[p].dispo)
					continue ;
				for (d = 0; d < sizeof(dsts) / sizeof(dsts[0]); d++)
				{
					for (m = 0; m < sizeof(mins) / sizeof(mins[0]); m++)
					{
						memset(dst, 0xcc, sizeof(dst));
						rc = modele_struct(img, places[p].dispo, dsts[d],
								mins[m], attendu);
						snprintf(nom, sizeof(nom),
							"struct taille=%u dst=%zu min=%zu queue=%d place=%zu",
							tailles[c], dsts[d], mins[m], queue, p);
						h_eq_i64(nom, copy_struct_from_user(dst, dsts[d],
								mins[m], places[p].va), rc);
						if (dsts[d] <= 4096)
							h_true(memcmp(dst, attendu, dsts[d]) == 0, nom);
					}
				}
			}
		}
	}
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	structure_plus_grande_octet_par_octet(void)
{
	t_aspace	*as;
	uint8_t		img[IMAGE_LEN];
	uint8_t		dst[16];
	char		nom[64];
	size_t		i;

	as = env_bornes();
	for (i = 16; i < 40; i++)
	{
		image_struct(img, 40, 0);
		memset(img + 16, 0, 24);
		img[i] = 0x01;
		copy_to_user(UVA, img, 40);
		snprintf(nom, sizeof(nom), "octet non nul en %zu", i);
		memset(dst, 0xcc, sizeof(dst));
		h_eq_i64(nom, copy_struct_from_user(dst, sizeof(dst), 8, UVA),
			UPTR_E2BIG);
		h_true(modele_lit32(dst) == 0 && dst[15] == 0, "destination effacee");
	}
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

int	main(void)
{
	h_begin("a03/uptr");
	h_run("plafond_de_copie", plafond_de_copie);
	h_run("plafond_et_limites_voisines", plafond_et_limites_voisines);
	h_run("structure_versionnee_acceptee", structure_versionnee_acceptee);
	h_run("structure_versionnee_refusee", structure_versionnee_refusee);
	h_run("structure_adresses_et_appelant", structure_adresses_et_appelant);
	h_run("acces_contre_modele", acces_contre_modele);
	h_run("frontiere_exacte_du_haut", frontiere_exacte_du_haut);
	h_run("structure_contre_modele", structure_contre_modele);
	h_run("structure_plus_grande_octet_par_octet",
		structure_plus_grande_octet_par_octet);
	return (h_end());
}
