#include <string.h>
#include "harness.h"
#include "fake.h"
#include "velum/err.h"

#define MIO 0x100000ull
#define UB FK_BASE

static int	motif_ok(uint64_t from, uint64_t n)
{
	uint64_t	i;

	i = 0;
	while (i < n)
	{
		if (g_fk.umem[from + i] != fk_pattern(from + i))
			return (0);
		i++;
	}
	return (1);
}

static void	lecture_au_dela_de_1_mio(void)
{
	fk_reset(UB + FK_SIZE);
	h_eq_i64("read 2 Mio", fsys_rw(fk_args(1, UB, 2 * MIO, 0), 0, 1), 2 * MIO);
	h_true(motif_ok(0, 2 * MIO), "contenu lu identique au motif");
	h_eq_u64("position du fichier", g_fk.pos, 2 * MIO);
}

static void	ecriture_au_dela_de_1_mio(void)
{
	uint64_t	i;

	fk_reset(UB + FK_SIZE);
	i = 0;
	while (i < 2 * MIO + 12345)
	{
		g_fk.umem[i] = fk_pattern(i);
		i++;
	}
	h_eq_i64("write 2 Mio + 12345", fsys_rw(fk_args(1, UB, 2 * MIO + 12345,
				0), 1, 1), 2 * MIO + 12345);
	h_eq_u64("octets recus par le fichier", g_fk.written, 2 * MIO + 12345);
}

static void	lecture_vers_zone_non_inscriptible(void)
{
	fk_reset(UB);
	h_eq_i64("read vers zone en lecture seule", fsys_rw(fk_args(1, UB, 4096,
				0), 0, 1), E_FAULT);
	h_eq_u64("aucun octet consomme dans le fichier", g_fk.pos, 0);
}

static void	lecture_partielle_sans_perte(void)
{
	uint64_t	frontiere;

	frontiere = 3 * MIO / 2;
	fk_reset(UB + frontiere);
	h_eq_i64("read a cheval sur une zone en lecture seule",
		fsys_rw(fk_args(1, UB, 2 * MIO, 0), 0, 1), frontiere);
	h_eq_u64("le fichier n'a rien perdu", g_fk.pos, frontiere);
	h_true(motif_ok(0, frontiere), "octets livres corrects");
}

static void	adresse_invalide_avant_toute_lecture(void)
{
	fk_reset(UB + FK_SIZE);
	h_eq_i64("tampon hors espace", fsys_rw(fk_args(1, 0x1000, 2 * MIO, 0), 0,
			1), E_FAULT);
	h_eq_u64("fichier intact", g_fk.pos, 0);
	h_eq_i64("longueur nulle", fsys_rw(fk_args(1, 0, 0, 0), 0, 1), 0);
	fk_reset(UB + MIO);
	h_eq_i64("taille geante, tampon de 1 Mio inscriptible",
		fsys_rw(fk_args(1, UB, 0x7fffffffffffull, 0), 0, 1), MIO);
	h_eq_u64("rien consomme au dela", g_fk.pos, MIO);
}

int	main(void)
{
	h_begin("e05/io_grand");
	h_run("lecture au dela de 1 Mio", lecture_au_dela_de_1_mio);
	h_run("ecriture au dela de 1 Mio", ecriture_au_dela_de_1_mio);
	h_run("lecture vers une zone non inscriptible",
		lecture_vers_zone_non_inscriptible);
	h_run("lecture partielle sans perte", lecture_partielle_sans_perte);
	h_run("adresse invalide avant toute lecture",
		adresse_invalide_avant_toute_lecture);
	return (h_end());
}
