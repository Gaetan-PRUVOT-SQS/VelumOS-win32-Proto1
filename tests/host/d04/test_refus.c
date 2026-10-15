#include "harness.h"
#include "fake.h"

static const uint16_t	g_rows[51][17] = {
{2, 1, 0, 0x1012, 0x000f},
{2, 1, 1, 0x003e, 0x000e},
{1, 1, 1, 0x00e3},
{2, 1, 1, 0x0014, 0},
{2, 1, 2, 0x1112, 0x000e},
{3, 1, 2, 0x0016, 1, 0x000e},
{3, 2, 0, 0x0016, 1, 0x000e},
{3, 1, 2, 0x001a, 4, 0x000e},
{3, 1, 2, 0x0022, 4, 0x000e},
{3, 1, 2, 0x0060, 4, 0x000e},
{4, 1, 2, 0x0071, 4, 0, 0x000e},
{8, 1, 0, 0x001a, 3, 0x0060, 3, 0x0071, 3, 0, 0x000e},
{2, 1, 1, 0x0528, 0x000e},
{1, 1, 1, 0xff28},
{4, 1, 1, 0x0014, 0, 0, 0xfe28},
{1, 1, 1, 0x0028},
{2, 1, 0, 0, 0xff28},
{3, 1, 0, 0x002a, 0, 0},
{9, 1, 1, 0x002b, 5, 0, 0x000e, 0x000e, 0x0100, 0, 0, 0},
{8, 1, 1, 0x002b, 4, 0, 0, 0x0100, 0, 0, 0},
{8, 1, 1, 0x002b, 4, 0, 0x000e, 0x0100, 9, 0, 0},
{8, 1, 1, 0x0026, 4, 0, 0x000e, 0x0300, 3, 0, 0},
{0x000e, 1, 1, 0x002c, 4, 0, 0x000e, 0x0200, 2, 5, 0, 5, 0, 3, 0, 3, 0},
{1, 1, 1, 0x0400},
{0x000a, 1, 1, 0x002b, 4, 0, 0x000e, 0x0100, 1, 0, 0, 0x0063, 0},
{0x000a, 1, 1, 0x002b, 4, 0, 0x000e, 0x0100, 1, 0, 0, 1, 0},
{6, 1, 1, 0x002b, 4, 0, 0x000e, 0x0200, 0},
{8, 1, 1, 0x0026, 4, 0, 0x000e, 0x0100, 0, 0, 0},
{4, 1, 1, 0x002b, 3, 0, 0x000e},
{1, 1, 1, 0x1012},
{4, 1, 2, 0x3071, 0, 0, 0x000e},
{5, 1, 3, 0x00fa, 0, 0, 0, 0x000e},
{5, 1, 3, 0x00fb, 0, 0, 0, 0x000e},
{4, 1, 3, 0x00fc, 0, 0, 0x000e},
{4, 1, 3, 0x00fd, 0, 0, 0x000e},
{3, 1, 3, 0x00fe, 0, 0x000e},
{3, 1, 3, 0x00ff, 0, 0x000e},
{0x000a, 1, 1, 0x002b, 6, 0, 0x000e, 0x0128, 0, 0x0100, 0, 0, 0},
{8, 1, 1, 0x002b, 4, 0, 0x0328, 0x0100, 0, 0, 0},
{4, 1, 2, 0x0277, 0, 0, 0x000e},
{4, 2, 0, 0x0277, 0, 0, 0x000e},
{4, 1, 2, 0x1071, 0, 1, 0x000e},
{4, 1, 1, 0x6071, 0, 0, 0x000e},
{2, 1, 1, 0x000e, 0},
{4, 1, 1, 0x0100, 0, 0, 0},
{8, 1, 0, 0x002b, 4, 0, 0x000e, 0x0100, 0, 0, 0},
{0x000c, 1, 1, 0x002b, 4, 0, 0x000e, 0x0100, 2, 0xffff, 0x7fff, 3, 0, 3, 0},
{0x000a, 1, 0, 0x0026, 4, 0, 0x000e, 0x0300, 1, 3, 0, 0x0201, 3},
{9, 1, 1, 0x0026, 4, 0, 0x000e, 0x0300, 1, 3, 0, 0x0201},
{4, 1, 0, 0x0038, 2, 0x000e, 0x000e},
{2, 1, 1, 0x0038, 0xffff}
};
static const char		*g_names[51] = {
	"ok const puis return",
	"code inutilise 3e",
	"code inutilise e3",
	"instruction tronquee",
	"registre hors bornes",
	"deuxieme moitie de paire hors bornes",
	"paire dans les bornes",
	"chaine hors table",
	"type hors table",
	"champ hors table",
	"methode hors table",
	"indices maximaux acceptes",
	"saut apres la fin",
	"saut avant le debut",
	"saut au milieu d une instruction",
	"saut nul",
	"saut arriere valide",
	"goto/32 nul accepte",
	"charge desalignee",
	"charge atteinte en sequence",
	"charge tronquee",
	"largeur de tableau invalide",
	"cles sparse non triees",
	"identifiant de charge inconnu",
	"cible de switch hors du code",
	"cible de switch au milieu",
	"packed-switch vers charge sparse",
	"fill-array-data vers charge packed",
	"switch vers une instruction",
	"fin du code atteinte",
	"arguments au dela de outs",
	"invoke-polymorphic",
	"invoke-polymorphic/range",
	"invoke-custom",
	"invoke-custom/range",
	"const-method-handle",
	"const-method-type",
	"saut vers le nop de bourrage",
	"saut dans une charge",
	"plage hors registres",
	"plage au dernier registre",
	"argument 35c hors registres",
	"compte 35c superieur a 5",
	"nop mort en fin de code",
	"charge au debut du code",
	"switch vide accepte",
	"premiere cle packed qui deborde",
	"tableau largeur 1 compte impair",
	"tableau tronque",
	"if vers cible valide",
	"if sans suite"
};

static void	refus_table(void)
{
	int		i;
	int		code[4];

	code[0] = 0;
	code[1] = E_INVAL;
	code[2] = E_RANGE;
	code[3] = E_NOTSUP;
	i = 0;
	while (i < 51)
	{
		fake_us(&g_rows[i][3], g_rows[i][0]);
		h_eq_i64(g_names[i], fake_v(g_rows[i][1]), code[g_rows[i][2]]);
		i++;
	}
}

static void	refus_arguments(void)
{
	t_dcodelimits	l;
	t_span			s;
	uint8_t			st[4];

	fake_sample(0);
	l = fake_lim(3);
	s.p = g_c.b;
	s.len = 12;
	h_eq_i64("temoin", dexcode_verify(s, &l, st), 0);
	h_eq_i64("limites nulles", dexcode_verify(s, NULL, st), E_INVAL);
	h_eq_i64("debuts nuls", dexcode_verify(s, &l, NULL), E_INVAL);
	s.len = 11;
	h_eq_i64("longueur impaire", dexcode_verify(s, &l, st), E_INVAL);
	s.len = 0;
	h_eq_i64("code vide", dexcode_verify(s, &l, st), E_INVAL);
	s.len = 12;
	s.p = NULL;
	h_eq_i64("code nul", dexcode_verify(s, &l, st), E_INVAL);
	s.p = g_c.b;
	l.ins = 4;
	h_eq_i64("ins au dela de registers", dexcode_verify(s, &l, st), E_INVAL);
}

int	main(void)
{
	h_begin("d04 refus de dexcode_verify");
	h_run("table de decision des refus", refus_table);
	h_run("arguments invalides", refus_arguments);
	return (h_end());
}
