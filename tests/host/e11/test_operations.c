#include "harness.h"
#include "velum/err.h"
#include "velum/libk.h"
#include "fake_fs.h"

static t_fakefs	g_fs;
static t_ffops	g_ops;
static t_ffdir	g_dir;

static void	monter(void)
{
	fake_init(&g_fs);
	fake_add(&g_fs, "/Zeta.txt", 0, 10);
	fake_add(&g_fs, "/beta", 1, 0);
	fake_add(&g_fs, "/Alpha", 1, 0);
	fake_add(&g_fs, "/alpha.txt", 0, 2048);
	fake_add(&g_fs, "/beta/dedans", 0, 1);
	fake_ops(&g_ops, &g_fs);
}

static void	liste_triee(void)
{
	monter();
	h_eq_i64("chargement", ff_load(&g_ops, "/", &g_dir), 0);
	h_eq_u64("4 éléments à la racine", g_dir.count, 4);
	h_eq_str("dossier Alpha d'abord", g_dir.items[0].name, "Alpha");
	h_eq_str("dossier beta", g_dir.items[1].name, "beta");
	h_eq_str("fichier alpha.txt", g_dir.items[2].name, "alpha.txt");
	h_eq_str("fichier Zeta.txt en dernier", g_dir.items[3].name, "Zeta.txt");
}

static void	points_filtres(void)
{
	monter();
	fake_add(&g_fs, "/beta/.", 1, 0);
	fake_add(&g_fs, "/beta/..", 1, 0);
	h_eq_i64("chargement", ff_load(&g_ops, "/beta", &g_dir), 0);
	h_eq_u64(". et .. écartés", g_dir.count, 1);
	h_eq_str("seul reste dedans", g_dir.items[0].name, "dedans");
}

static void	liste_erreurs(void)
{
	monter();
	h_eq_i64("dossier absent", ff_load(&g_ops, "/nul", &g_dir), E_NOENT);
	h_eq_u64("liste vide après échec", g_dir.count, 0);
	h_eq_i64("chemin relatif", ff_load(&g_ops, "beta", &g_dir), E_INVAL);
	g_fs.fail_list = E_IO;
	h_eq_i64("erreur d'E/S propagée", ff_load(&g_ops, "/", &g_dir), E_IO);
}

static void	creation(void)
{
	monter();
	h_eq_i64("nouveau dossier", ff_make_dir(&g_ops, "/", "Projets"), 0);
	h_true(fake_has(&g_fs, "/Projets"), "dossier présent");
	h_eq_i64("nouveau fichier", ff_make_file(&g_ops, "/beta", "n.txt"), 0);
	h_true(fake_has(&g_fs, "/beta/n.txt"), "fichier présent");
	h_eq_i64("doublon refusé", ff_make_dir(&g_ops, "/", "Projets"), E_EXIST);
	h_eq_i64("doublon fichier refusé", ff_make_file(&g_ops, "/", "alpha.txt"),
		E_EXIST);
	h_eq_i64("nom invalide", ff_make_dir(&g_ops, "/", "a/b"), E_INVAL);
	h_eq_i64("nom vide", ff_make_file(&g_ops, "/", ""), E_INVAL);
	h_eq_u64("seules 2 écritures effectuées", g_fs.writes, 2);
	g_fs.fail_write = E_NOSPC;
	h_eq_i64("disque plein propagé", ff_make_file(&g_ops, "/", "z"), E_NOSPC);
	h_true(!fake_has(&g_fs, "/z"), "rien créé");
}

static void	renommage(void)
{
	monter();
	h_eq_i64("renommer", ff_rename(&g_ops, "/", "alpha.txt", "gamma.txt"), 0);
	h_true(fake_has(&g_fs, "/gamma.txt") && !fake_has(&g_fs, "/alpha.txt"),
		"ancien nom disparu, nouveau présent");
	h_eq_i64("cible existante", ff_rename(&g_ops, "/", "gamma.txt", "Zeta.txt"),
		E_EXIST);
	h_eq_i64("source absente", ff_rename(&g_ops, "/", "nul", "x"), E_NOENT);
	h_eq_i64("nouveau nom invalide", ff_rename(&g_ops, "/", "gamma.txt", ".."),
		E_INVAL);
	g_fs.writes = 0;
	h_eq_i64("même nom", ff_rename(&g_ops, "/", "gamma.txt", "gamma.txt"), 0);
	h_eq_u64("même nom : aucune écriture", g_fs.writes, 0);
}

static void	suppression(void)
{
	monter();
	h_eq_i64("dossier non vide", ff_remove(&g_ops, "/", "beta"), E_NOTEMPTY);
	h_true(fake_has(&g_fs, "/beta"), "dossier conservé");
	h_eq_i64("fichier", ff_remove(&g_ops, "/", "Zeta.txt"), 0);
	h_true(!fake_has(&g_fs, "/Zeta.txt"), "fichier supprimé");
	h_eq_i64("absent", ff_remove(&g_ops, "/", "Zeta.txt"), E_NOENT);
	h_eq_i64("nom invalide", ff_remove(&g_ops, "/", "../x"), E_INVAL);
}

static void	proprietes(void)
{
	char	out[FF_PROPS_MAX];

	monter();
	h_eq_i64("fichier", ff_props(&g_ops, "/", "alpha.txt", out, sizeof(out)),
		(int64_t)sizeof("alpha.txt : fichier, 2,0 Ko") - 1);
	h_eq_str("fichier : texte", out, "alpha.txt : fichier, 2,0 Ko");
	ff_props(&g_ops, "/", "beta", out, sizeof(out));
	h_eq_str("dossier : texte", out, "beta : dossier");
	g_fs.nodes[4].ro = 1;
	ff_props(&g_ops, "/", "alpha.txt", out, sizeof(out));
	h_eq_str("lecture seule", out, "alpha.txt : fichier, 2,0 Ko, lecture seule");
	h_eq_i64("absent", ff_props(&g_ops, "/", "nul", out, sizeof(out)), E_NOENT);
	h_eq_i64("tampon trop petit", ff_props(&g_ops, "/", "alpha.txt", out, 5),
		E_RANGE);
}

static void	noms_refuses(void)
{
	char	ctrl[2];
	char	del[2];

	monter();
	ctrl[0] = 0x01;
	ctrl[1] = '\0';
	del[0] = 0x7f;
	del[1] = '\0';
	h_eq_i64("point", ff_make_dir(&g_ops, "/", "."), E_INVAL);
	h_eq_i64("deux points", ff_make_dir(&g_ops, "/", ".."), E_INVAL);
	h_eq_i64("barre", ff_make_file(&g_ops, "/", "/secret"), E_INVAL);
	h_eq_i64("contrôle", ff_make_file(&g_ops, "/", ctrl), E_INVAL);
	h_eq_i64("DEL", ff_make_dir(&g_ops, "/", del), E_INVAL);
	h_eq_i64("renommage vers ..", ff_rename(&g_ops, "/", "alpha.txt", ".."),
		E_INVAL);
	h_eq_i64("suppression de .", ff_remove(&g_ops, "/", "."), E_INVAL);
	h_eq_u64("aucune écriture", g_fs.writes, 0);
	h_true(fake_has(&g_fs, "/alpha.txt"), "source intacte");
}

static void	nom_index(char *out, uint32_t i)
{
	out[0] = 'n';
	out[1] = (char)('0' + (i / 1000) % 10);
	out[2] = (char)('0' + (i / 100) % 10);
	out[3] = (char)('0' + (i / 10) % 10);
	out[4] = (char)('0' + i % 10);
	out[5] = '\0';
}

static uint32_t	pousser(uint32_t n)
{
	char		name[8];
	uint32_t	i;
	uint32_t	ok;

	ok = 0;
	i = 0;
	while (i < n)
	{
		nom_index(name, i);
		if (fake_vfs_push(name, 0) == 0)
			ok++;
		i++;
	}
	return (ok);
}

static void	charger_vfs(void)
{
	t_ffops	ops;

	ff_ops_vfs(&ops);
	h_eq_i64("chargement vfs", ff_load(&ops, "/", &g_dir), 0);
}

static void	plafond_exact(void)
{
	fake_vfs_reset();
	h_eq_u64("512 dirents", pousser(FF_ENTRIES_MAX), FF_ENTRIES_MAX);
	charger_vfs();
	h_eq_u64("toutes gardées", g_dir.count, FF_ENTRIES_MAX);
	h_eq_u64("pas tronquée", g_dir.truncated, 0);
	h_eq_str("première", g_dir.items[0].name, "n0000");
	h_eq_str("dernière", g_dir.items[FF_ENTRIES_MAX - 1].name, "n0511");
}

static void	plafond_depasse(void)
{
	fake_vfs_reset();
	h_eq_u64("513 dirents", pousser(FF_ENTRIES_MAX + 1), FF_ENTRIES_MAX + 1);
	charger_vfs();
	h_eq_u64("plafond", g_dir.count, FF_ENTRIES_MAX);
	h_eq_u64("drapeau", g_dir.truncated, 1);
	h_eq_str("première gardée", g_dir.items[0].name, "n0000");
	h_eq_str("dernière gardée", g_dir.items[FF_ENTRIES_MAX - 1].name,
		"n0511");
	h_true(strcmp(g_dir.items[FF_ENTRIES_MAX - 1].name, "n0512") != 0,
		"n0512 hors du tableau");
}

static void	plafond_avec_points(void)
{
	fake_vfs_reset();
	h_eq_i64("point", fake_vfs_push(".", 1), 0);
	h_eq_i64("parent", fake_vfs_push("..", 1), 0);
	h_eq_u64("512 fichiers", pousser(FF_ENTRIES_MAX), FF_ENTRIES_MAX);
	charger_vfs();
	h_eq_u64("points écartés du plafond", g_dir.count, FF_ENTRIES_MAX - 2);
	h_eq_u64("drapeau", g_dir.truncated, 1);
	h_eq_str("première visible", g_dir.items[0].name, "n0000");
	h_eq_str("dernière visible", g_dir.items[FF_ENTRIES_MAX - 3].name,
		"n0509");
}

static void	dirents_hostiles(void)
{
	char	limite[VFS_NAME_MAX];
	char	trop[VFS_NAME_MAX + 1];
	t_ffops	ops;

	memset(limite, 'a', sizeof(limite));
	memset(trop, 'z', sizeof(trop));
	fake_vfs_reset();
	h_eq_i64("nom vide", fake_vfs_push_raw(NULL, 0, S_TYPE_REG), 0);
	h_eq_i64("256 octets", fake_vfs_push_raw(trop, VFS_NAME_MAX + 1,
			S_TYPE_REG), 0);
	h_eq_i64("255 octets", fake_vfs_push_raw(limite, VFS_NAME_MAX,
			S_TYPE_REG), 0);
	h_eq_i64("court", fake_vfs_push("ok", 0), 0);
	ff_ops_vfs(&ops);
	h_eq_i64("chargement", ff_load(&ops, "/", &g_dir), 0);
	h_eq_u64("deux noms tenus", g_dir.count, 2);
	h_eq_u64("pas tronquée", g_dir.truncated, 0);
	h_eq_u64("longueur tenue", strlen(g_dir.items[0].name), VFS_NAME_MAX);
	h_eq_str("puis ok", g_dir.items[1].name, "ok");
}

static void	ouverture_refusee(void)
{
	t_ffops	ops;

	fake_vfs_reset();
	h_eq_u64("une entrée", pousser(1), 1);
	fake_vfs_open_rc(E_IO);
	ff_ops_vfs(&ops);
	h_eq_i64("E/S à l'ouverture", ff_load(&ops, "/", &g_dir), E_IO);
	h_eq_u64("liste vide", g_dir.count, 0);
	h_eq_u64("drapeau clair", g_dir.truncated, 0);
	h_eq_i64("relatif avant le vfs", ff_load(&ops, "beta", &g_dir), E_INVAL);
}

static void	troncature(void)
{
	char		name[16];
	uint32_t	i;

	fake_init(&g_fs);
	fake_ops(&g_ops, &g_fs);
	i = 0;
	while (i < 60)
	{
		name[0] = '/';
		name[1] = (char)('a' + i / 10);
		name[2] = (char)('0' + i % 10);
		name[3] = '\0';
		fake_add(&g_fs, name, 0, i);
		i++;
	}
	h_eq_i64("60 entrées", ff_load(&g_ops, "/", &g_dir), 0);
	h_eq_u64("toutes présentes", g_dir.count, 60);
	h_eq_str("première", g_dir.items[0].name, "a0");
	h_eq_str("dernière", g_dir.items[59].name, "f9");
}

int	main(void)
{
	h_begin("e11/operations");
	h_run("liste: tri dossiers puis noms", liste_triee);
	h_run("liste: points écartés", points_filtres);
	h_run("liste: erreurs", liste_erreurs);
	h_run("création: cas et erreurs", creation);
	h_run("renommage: cas et erreurs", renommage);
	h_run("suppression: cas et erreurs", suppression);
	h_run("propriétés: texte", proprietes);
	h_run("liste: 60 entrées triées", troncature);
	h_run("noms: refusés à l'écriture", noms_refuses);
	h_run("liste vfs: 512 exact", plafond_exact);
	h_run("liste vfs: 513e refusée", plafond_depasse);
	h_run("liste vfs: points dans le plafond", plafond_avec_points);
	h_run("liste vfs: dirents hors contrat", dirents_hostiles);
	h_run("liste vfs: ouverture en erreur", ouverture_refusee);
	return (h_end());
}
