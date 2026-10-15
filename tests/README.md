# Tests de VelumOS

Ce dossier contient tous les tests du projet, du test unitaire d'une fonction du noyau jusqu'au scénario
qui démarre une machine virtuelle et clique dans le bureau. La stratégie, les résultats et le registre des défauts
sont dans [`../TESTS.md`](../TESTS.md). Ce fichier explique comment les tests sont organisés, comment les lire
et comment en écrire un.

## Organisation

```
tests/
  host/            tests unitaires, exécutés sur l'hôte (ASan et UBSan)
    harness.c/h      harnais commun : h_begin, h_run, h_true, h_eq_*, h_end
    skel/            tests du squelette (printf du noyau)
    aNN/             un dossier par sous-système : test_<élément>.c et fake_*.c
  qemu/            scénarios système, chacun démarre une machine QEMU
    aNN_<sujet>.py   scénarios d'un sous-système
    int_*.py         scénarios d'intégration (session, préemption, captures, intégrité de l'image)
    kentry_ac.py     entrée noyau : AC effacé, SMAP effectif (ring3test entryaudit)
    matrice_*.py     matrice de configurations QEMU (démarrage, horloges, protections)
    skel_boot.py     démarrage du noyau complet et autotests
  tools/           tests des outils Python (image disque, ABI, dépendances, matrice QEMU, reproductibilité, scan)
  abi/             liste figée des numéros d'appels système (syscalls.golden), relue par tests/tools/test_abi_syscalls.py
```

Les sous-systèmes `aNN` sont ceux de la table du README (a01 processeur, a02 mémoire physique, et ainsi de suite).
Un fragment `mk/lot-aNN.mk` déclare pour chacun ses sources compilées sur l'hôte (`HSRC_aNN`), ses tests
(`HT_aNN`) et ses applis.

## Les quatre niveaux

| Niveau | Ce qu'on vérifie | Oracle | Lancement |
|--------|------------------|--------|-----------|
| Unitaire (hôte) | la logique pure d'un module : tables de pages, allocateurs, analyseurs, décodeurs, rendu | valeur attendue, modèle de référence, outil de l'hôte | `make host-aNN` |
| Autotest noyau | le module dans le vrai noyau : verrous, interruptions, matériel émulé, mesures de temps | invariants, compteurs avant et après, tolérances | option `selftest` |
| Scénario système (QEMU) | le système entier : démarrage BIOS et UEFI, fautes volontaires, session graphique | journal série, captures d'écran, code de sortie QEMU | `make test-qemu` |
| Statique | le code et les outils | zéro erreur | `make hdrcheck`, `make scan`, `ruff` |

## Matrice de couverture et tests manuels

| Fichier | Rôle |
|---------|------|
| `matrice.toml` | source des données : fonctionnalités vérifiables (mode A, AM ou M, tests qui les couvrent, et pour les plus récentes exigence, technique, élément de couverture et noms de cas) et cas manuels |
| `MATRICE.md` | matrice complète, tableau de traçabilité et taux de couverture, généré |
| `MANUEL.md` | plan de tests manuels (étapes, résultat attendu, statut), généré |

Les cas manuels se jouent avec deux outils du dépôt. `tools/manuel.py` garde une machine QEMU vivante entre deux
commandes et permet d'envoyer des touches, de cliquer, de glisser, de capturer l'écran (avec zoom) et de lire le journal
série ; tout est écrit dans `build/manuel/<nom>/`. `tools/contraste.py` mesure le rapport de contraste WCAG d'un texte
dans une capture. Exemple :

```
python3 tools/manuel.py demarrer s1
python3 tools/manuel.py touche s1 ret
python3 tools/manuel.py capture s1 bureau
python3 tools/contraste.py build/manuel/s1/bureau.png "horloge:955,743,1015,764"
python3 tools/manuel.py arreter s1
```

`make matrice` régénère les deux rapports et échoue si un test cité n'existe pas ; `make matrice-check` échoue si
les rapports ne sont pas à jour. Pour ajouter une fonctionnalité : une entrée `[[fonction]]` avec son mode et ses
tests, puis `make matrice`. Une fonctionnalité en mode AM ou M doit citer au moins un cas manuel.

Champs de traçabilité d'une entrée (les trois premiers vont ensemble, le générateur refuse une entrée incomplète) :

| Champ | Contenu |
|-------|---------|
| `exigence` | ce que le système doit faire, en une phrase vérifiable |
| `technique` | technique de 29119-4 employée (partitions, valeurs limites, table de décision, transitions d'états, métamorphique, combinatoire, injection de panne, supposition d'erreur, aléatoire à graine) |
| `element` | élément de couverture visé (classes, bornes, états, rangs d'échec) |
| `cas` | noms de cas (texte d'un `h_run` ou nom d'une fonction `test_`) ; chacun doit apparaître dans un fichier cité par `tests` |

`python3 tools/matrice_couverture.py --orphelins` liste les fichiers de tests (`tests/host`, `tests/qemu`, `tests/tools`,
`tests/abi`) qu'aucune entrée ne cite : à lancer après avoir ajouté un test, pour le rattacher à une fonctionnalité.

## Tests unitaires hôte

Un fichier `test_<élément>.c` est un exécutable : il appelle `h_begin`, enchaîne des cas avec `h_run`, et se termine
par `h_end`, qui renvoie un code non nul au premier échec. Le harnais est volontairement minimal :

```c
int	main(void)
{
	h_begin("skel/fmt");
	h_run("decimal", fmt_decimal);
	h_run("hexadecimal", fmt_hex);
	h_run("chaines", fmt_strings);
	h_run("troncature", fmt_truncation);
	return (h_end());
}
```

Chaque vérification porte un message : `h_true(cond, "quoi")`, `h_eq_i64("quoi", obtenu, attendu)`,
`h_eq_u64`, `h_eq_str`. Un échec affiche `ECHEC <suite>/<cas> : <quoi>` avec la valeur obtenue et la valeur attendue.

Conventions :

- Le nom du cas dit la technique et l'élément couvert, par exemple `v_spawnv/exigence : chemin, blob, longueur,
  flags`, `v_spawnv/supposition d'erreur : argv nul, 257, chemin nul`, `v_spawnv/injection de panne : VALLOC refuse`.
  C'est ce qui permet de remonter d'un test à son modèle (voir [`../TESTS.md`](../TESTS.md)).
- Les dépendances du module sont remplacées par des faux (`fake_*.c`) : mémoire physique simulée, verrous, horloge,
  8042, virtio, disque en mémoire, bus PCI. Un faux sait aussi injecter des pannes (`pmm_fail_after`,
  `heap_fail_after`, refus de `VALLOC`).
- Compilation avec `-fsanitize=address,undefined -fno-sanitize-recover=undefined -Werror` : un dépassement de tampon
  ou un comportement indéfini fait échouer le test sur-le-champ. Les fonctions de `lib/libk` (`strlen`...) sont renommées
  `vk_*` à la compilation pour ne pas entrer en conflit avec la libc de l'hôte.
- Tout test aléatoire utilise un générateur à graine fixe et **affiche sa graine** (`a07/elf_fuzz : graine 0xa07c0de`,
  `modele de reference : graine = 0x...`). Un échec se rejoue donc à l'identique.

## Tests du thème verre (a16 à a20)

Le thème verre (voir le README, section « Thème verre ») s'appuie sur cinq lots. Leur règle commune : la valeur
attendue vient de la maquette de conception ou d'un calcul fait à la main dans le test, jamais d'une copie de la sortie
du code.

| Lot | Ce qu'on y vérifie | Fichiers typiques |
|-----|--------------------|-------------------|
| a16 | génération reproductible des bitmaps (Instrument Sans, JetBrains Mono), jeu de caractères, budget de 80 Kio | `test_genfont.py`, `test_font_charset.c`, `test_font_cover.c`, `test_font_budget.c` |
| a17 | verre (flou, repli opaque, bornes, coins, fuzz), palette clair et sombre, contrastes, contrôles, lanceur, écran de démarrage, fond voile, métriques du cadre | `test_glass*.c`, `test_glass_robust*.c`, `test_colors_ext.c`, `test_ctrl_*.c`, `test_launcher*.c`, `test_boot_*.c`, `test_theme_*.c`, `test_geo_*.c`, `test_hit_*.c` |
| a18 | serveur : verre (bandes sales, équivalence incrémental et complet, échec d'allocation du tampon de flou), propriété et quotas, clavier, souris, sections | `test_core_glass*.c`, `test_srv_glass*.c`, `test_srv_alloc*.c`, `test_srv_owner.c`, `test_srv_keys.c`, `test_srv_mouse.c` |
| a19 | contrôles : sur fond de verre (encre des flèches, anneau de focus, police), zones sales, menus, barres de défilement, arbre, contraste WCAG, arguments nuls | `test_glass*.c`, `test_ring*.c`, `test_dirty*.c`, `test_menu_*.c`, `test_scroll*.c`, `test_tree*.c`, `test_contrast.c`, `test_null.c` |
| a20 | connexion, bureau, dock, menu, dialogues, fenêtre Bonjour, comptes, contrastes sur fonds variables, rendus par zone | `test_logon_layout*.c`, `test_desktop*.c`, `test_taskbar*.c`, `test_hello_layout.c`, `test_contrast*.c`, `render/test_render_*.c`, `test_acc_*.c`, `test_utf8_fuzz.c` |

Particularités :

- Le banc de rendu de a17 (`lt_*.c`, `fake_gfx.c`) dessine sur une surface en mémoire. Les galeries (`test_gallery.c`,
  `test_glass_gallery.c`, `test_wallpaper.c`, `test_desktop.c`) écrivent des PNG dans `build/a17/` (chemin fixe,
  indépendant de `B=`) pour une relecture à l'œil face aux captures de la maquette.
- Le verre travaille dans un tampon fourni par l'appelant. Les tests le donnent trop petit, absent, ou activent la
  transparence réduite, et vérifient que le résultat est l'aplat opaque et non une erreur (`test_glass_edge.c`,
  `test_srv_glass.c`).
- a18 vérifie l'équivalence : une composition incrémentale par zones sales doit donner les mêmes pixels qu'une
  composition complète (`test_core_glass_eq.c`), y compris quand le verre élargit le rectangle à redessiner.
- Les palettes claire et sombre sont toutes deux testées, alors qu'aucune appli ne bascule le thème : les tests sont le
  seul chemin qui exerce le sombre (voir Limites connues du README).
- Les rendus par zone de a20 (`render/test_render_*.c`) rejouent le bureau, la connexion, le dock, le menu et les
  dialogues sur une surface en mémoire et comparent des pixels à des valeurs de la maquette, clair et sombre.
- `a17/mesure_verre.c` et `a17/bench_glass.c` sont des mesures, pas des tests : la matrice ne les cite pas.
- Lancer un lot : `make -j3 B=build/<nom> host-a17` (puis a18, a19, a20). Les scénarios QEMU `a20_desktop` et
  `a20_souris` couvrent la session, pas le rendu du thème.

## Scénarios système (QEMU)

Un scénario est un fichier Python qui déclare une machine et une fonction `run(vm)` :

```python
NAME = "intégration : préemption et mort d'un processus qui boucle en anneau 3"
CMDLINE = "init=/system/bin/spin"


def run(vm):
    vm.attendre(r"SPIN start", delai=60)
    vm.attendre(r"SPIN PASS", delai=30)
    vm.attendre(r"KILL PASS", delai=30)
```

Scénarios récents hors lots : `int_hash_image` (Limine refuse un noyau ou un initrd altéré d'un octet), `kentry_ac`
(AC effacé à l'entrée des exceptions et des IRQ, SMAP effectif), `a02_boot` (métadonnées de la mémoire physique
recalculées à la main), `matrice_demarrage` (un démarrage par configuration d'une matrice de combinaisons de processeur,
de protections et d'horloge, générée par `matrice_configurations.py` et testée par `tests/tools/test_matrice_qemu.py`).
Les scénarios `a01_fault_*` et `a02_fault_*` jouent une faute volontaire (`fault=`, `pmm_fault=`) et vérifient
l'exception, les registres et l'écran d'arrêt.

Valeurs optionnelles : `FIRMWARES` (par défaut `("bios", "uefi")`), `MEMOIRE`, `SMP`, `QEMU_EXTRA`.
Le lanceur `tools/run_scenarios.py` construit une image par ligne de commande distincte, joue chaque scénario dans chaque
firmware, en parallèle (`--jobs`), et affiche `OK` ou `ECHEC` avec la fin du journal série.

L'objet `vm` (`tools/vtest.py`) offre :

| Méthode | Usage |
|---------|-------|
| `attendre(motif, delai)` | attend une expression régulière dans le journal série, échoue sur `PANIC:`, sur l'arrêt de QEMU ou au délai |
| `serie()` | journal série complet |
| `capture(nom)` | capture d'écran, renvoyée comme `Image` (`pixel`, `compter`, `differences`, `enregistrer_png`) |
| `capture_quand(condition, delai)` | recapture jusqu'à ce que la condition sur l'image soit vraie, ou jusqu'au délai |
| `capture_stable(delai)` | recapture jusqu'à ce que deux images successives soient identiques (à 200 pixels près) |
| `touche(nom)`, `souris_deplacer(dx, dy)`, `souris_bouton(masque)` | entrées par le moniteur QEMU |
| `moniteur(commande)` | commande libre du moniteur QEMU |

Le disque est monté en `snapshot=on` : un scénario ne modifie jamais l'image.

Options de ligne de commande reconnues par le noyau, utilisées par les scénarios :

| Option | Effet |
|--------|-------|
| `selftest` | lance les autotests des modules et affiche `SELFTESTS PASS n` ou `SELFTESTS FAIL` |
| `exit` | quitte QEMU à la fin (code de sortie par `isa-debug-exit`) |
| `fault=div0\|gp\|ud\|pf\|df\|stack\|nmi\|guard\|panic` | provoque une faute noyau précise (profil debug) pour vérifier l'exception, les registres, la pile utilisée, la page de garde ou l'écran d'arrêt |
| `pmm_fault=double\|owner\|align\|range` | provoque une libération invalide de mémoire physique (double, mauvais propriétaire, adresse non alignée, hors plage) et attend la panique |
| `inputlog` | journalise chaque événement clavier et souris sur la série |
| `power=off\|reboot` | extinction ou redémarrage ACPI à la fin des autotests de temps |
| `blktest`, `pci.q35`, `pci.edu`, `dispmode=LxH` | variantes d'autotests des disques, du PCI (carte mère q35, appareil `edu`) et de l'affichage |
| `verbose`, `init=/chemin` | console noyau à la place de l'écran de démarrage ; programme lancé en premier (`init=/aucun` laisse l'écran ou la console visibles) |

## Autotests noyau

Chaque module exporte `int <module>_selftest(void)` (0 pour la réussite). Le noyau les enchaîne et imprime, pour
chacun, `[TEST] <module> ...` puis `[TEST] <module> ... OK` ou `FAIL`, et pour finir `SELFTESTS PASS <n>`.
Les mesures (équité de l'ordonnanceur, précision du sommeil, nombre de pages libres avant et après) sont écrites
dans le journal et vérifiées contre des tolérances.

## Intégration continue

`.github/workflows/ci.yml` installe les paquets sur Ubuntu 24.04 puis lance `tools/ci.sh`, le même script que `make ci`
en local. Il compile avec le `gcc` du système (`CROSS=` vide), passe les tests des outils et les contrôles statiques,
joue dix lots de tests hôte (`a02`, `a03`, `a04`, `a08`, `a09`, `a11`, `a13`, `d00`, `d07`, `d11`) puis six scénarios QEMU : `a01_boot`,
`skel_boot`, `int_session`, `a20_desktop`, `a12_fat32` et `int_apk_installation`. Les runners n'ont pas KVM : la variable `VTEST_SANS_KVM=1` force
l'émulation pure (les scénarios durent quelques dizaines de secondes de plus).

Ce n'est pas la suite complète : elle reste à lancer en local (`make test`). La chaîne croisée épinglée par
`tools/build-toolchain.sh` n'est pas utilisée en CI, ce qui est une différence avec la construction de référence.

## Instabilités : comment on les traite

Un test qui échoue une fois sur dix n'est pas toléré, il est analysé. Les causes rencontrées et leur traitement :

| Symptôme | Cause | Traitement |
|----------|-------|------------|
| faux échec sur `PANIC:` | le journal est lu pendant l'écriture de la ligne | on n'échoue plus si le motif attendu contient lui-même `PANIC` |
| curseur resté dans le coin | QEMU fusionne les déplacements de souris envoyés trop vite | pause de 0,12 s entre deux mouvements |
| résultats périmés | image mise en cache non reconstruite quand seul l'initrd change | la date de l'initrd compte dans la décision |
| `[TEST] x ... OK` introuvable | les journaux du test s'intercalent avant le `OK` | la ligne de résultat est écrite en entier sur sa propre ligne |
| seuil de convoi dépassé | le seuil dépendait du nombre de cœurs de l'hôte | seuil réexprimé indépendamment du parallélisme |
| `a20_desktop` échoue une fois sur deux | capture juste après la touche, référence prise pendant le premier dessin, captures du BIOS et de l'UEFI sous le même nom | `capture_quand`, `capture_stable`, noms préfixés par la machine |

## Lancer les tests

```
make test-tools                       outils Python (pytest)
make matrice-check                    matrice à jour et tests cités présents
make host-a02                         un sous-système
make -j8 -k test-host                 tous les tests unitaires
make test-qemu                        tous les scénarios, BIOS et UEFI
python3 tools/run_scenarios.py --kernel build/main/debug/kernel.elf \
    --root build/main/debug/root --out build/main/qemu --filter a01_ --jobs 4
make hdrcheck scan                    contrôles statiques
make ci                               sous-ensemble rejoué par l'intégration continue (tools/ci.sh)
```

Les journaux série et les captures de chaque scénario restent dans `build/main/qemu/<scénario>/`.
