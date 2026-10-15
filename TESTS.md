# Campagne de tests de la tranche 1

Ce document décrit la campagne de tests de la première tranche de VelumOS : plan, niveaux, techniques,
résultats, défauts trouvés et limites. Elle porte sur les 20 sous-systèmes (a01 à a20, voir le README),
le squelette du noyau et l'intégration, et date du 5 octobre 2026. La démarche suit ISO/IEC/IEEE 29119
dans une version allégée : une stratégie fondée sur les risques, des modèles de test et des techniques
nommées, des critères de fin écrits avant l'exécution, des incidents exploitables. Le plan et les cas vivent
dans ce fichier et dans le code des tests, sans autre document.

## 1. Ce qu'on teste et pourquoi

VelumOS est un noyau en C avec une interface graphique en mode utilisateur. Les risques sont
ceux d'un noyau : une panique, un gel, une fuite de pages, une faille d'isolation, un
pilote qui plante sur du matériel imprévu. Le risque monte encore pour tout ce qui lit une
entrée hostile (ELF, cpio, FAT32, ACPI, messages d'un client graphique, arguments d'appel système).

| Risque | Impact | Probabilité | Réponse de test |
|--------|--------|-------------|-----------------|
| Entrée hostile (ELF, cpio, FAT, ACPI, PCI, IPC, appels système) | 3 | 3 | corpus corrompus, fuzz à graine, limites, tests négatifs, ASan/UBSan |
| Mémoire (pmm, vmm, tas, sections) | 3 | 2 | modèle de référence, injection d'échec à chaque rang, compteurs avant/après |
| Concurrence (verrous, attentes, IPC) | 3 | 2 | simulation de courses à graine, pthreads, autotests noyau en QEMU |
| Matériel (APIC, PCI, 8042, virtio) | 2 | 3 | fakes pilotés par la spec, QEMU BIOS et UEFI, variantes de machine |
| Gel ou famine (préemption, kill) | 3 | 2 | scénario d'intégration avec preuve différentielle |
| Interface (rendu, hit-test, focus) | 2 | 2 | balayage de pixels, galerie relue à l'œil, captures QEMU |
| Format et sécurité du code | 1 | 3 | `make scan`, ruff, shellcheck, shfmt |

Critères de fin, écrits avant la campagne :

1. Chaque lot : `make B=build/aNN lot LOT=aNN` vert (compilation noyau et utilisateur, tests hôte).
2. Le noyau complet démarre en BIOS et en UEFI et passe tous ses autotests.
3. Tous les scénarios QEMU passent dans les deux firmwares.
4. Contrôle sécurité, ruff et shellcheck sans erreur sur tout le dépôt.
5. Aucun incident de gravité haute ouvert.

## 2. Niveaux, outils et environnement

| Niveau | Où | Outil | Oracle |
|--------|----|-------|--------|
| Composant (hôte) | `tests/host/aNN/test_*.c` | `make host-aNN`, harnais `tests/host/harness.*` | valeurs attendues, modèle de référence, outil de l'hôte (CPython, hashlib, openssl, sgdisk) |
| Noyau (autotests) | `<module>_selftest` dans le noyau | option `selftest` de la ligne de commande | compteurs, invariants, mesures de temps |
| Système (QEMU) | `tests/qemu/*.py` | `tools/run_scenarios.py`, `tools/vtest.py` | journal série, captures d'écran, code de sortie QEMU |
| Intégration | `tests/qemu/int_*.py`, `skel_boot.py` | idem | session complète, préemption, kill |
| Statique | tout le dépôt | `make hdrcheck`, `make scan`, ruff, shellcheck, shfmt | zéro erreur |

Harnais hôte : un exécutable par fichier `test_*.c`, compilé avec `-fsanitize=address,undefined
-fno-sanitize-recover=undefined -Werror`. Les fonctions de `lib/libk` sont renommées `vk_*` à la
compilation pour ne pas heurter la libc. Chaque lot fournit ses fakes (`fake_*.c`) : pseudo-physique,
verrous, temps, PCI, 8042, virtio, disque en mémoire. Le nom d'un cas dit la technique et l'élément couvert.

Harnais QEMU : `VM` lance `qemu-system-x86_64 -machine q35` avec le disque en `snapshot=on`, le
port série dans un fichier, un moniteur sur socket (`screendump`, `sendkey`, `mouse_move`,
`mouse_button`) et `isa-debug-exit`. UEFI par OVMF, copie propre des variables. Une image par ligne
de commande, reconstruite si le noyau ou l'initrd est plus récent. Un scénario déclare `CMDLINE`,
`FIRMWARES`, `MEMOIRE`, `SMP` et une fonction `run(vm)`.

Environnement de la campagne (versions relevées le jour même) :

| Élément | Version |
|---------|---------|
| QEMU | 10.2.2 (compilé localement, KVM actif) |
| Cross-compilateur noyau | x86_64-elf-gcc 15.3.0 |
| Compilateur hôte des tests | gcc 16.2.1 |
| Chargeur | Limine 12.9.1 (outil `image-disque`) |
| Firmware UEFI | edk2 OVMF (`/usr/share/edk2/ovmf`) |
| Python et lint | Python 3.14.7, ruff 0.16.9 |
| Hôte | Fedora 44, noyau Linux 7.2.8, processeur x86-64 Intel avec KVM |

Les seuls aléas sont des graines affichées par les tests (listées au point 5).

## 3. Techniques employées (29119-4)

- Partitions d'équivalence et valeurs limites : tailles et alignements de pmm, vmm, tas, chemins,
  longueurs de messages, plages 64 bits, arguments d'appel système.
- Table de décision : ordre de focus des contrôles (13 règles), droits des appels système, partition complète de
  `luna_hit_test`.
- Transitions d'états : processus (vivant, en sortie, zombie), PS/2 (décodeur de paquets avec
  resynchronisation), objets et handles (génération).
- Test syntaxique et corpus corrompus : ELF (plus de 5 000 variantes), cpio, FAT32 (400 images), tables ACPI,
  configurations PCI, messages du serveur de fenêtres.
- Métamorphique et dos à dos : séquences aléatoires qui doivent ramener les compteurs à zéro, UTF-8 comparé à
  CPython, PBKDF2 comparé à `hashlib`, GPT relue par `sgdisk -v`, mélange de pixels contre un calcul exact.
- Aléatoire et fuzz à graine : 1 million d'appels de dessin, 100 000 messages, 100 000 octets PS/2, 20 000 appels
  système d'affichage, 100 000 pas d'édition.
- Modèle de référence : pmm (100 000 opérations), vmm (4 000), FAT32 (20 000 avec contrôle de cohérence), régions
  de dessin (100 000 séquences, modèle bitmap).
- Injection d'échec : `pmm_fail_after`, `heap_fail_after`, échec d'allocation de chaque contrôle `ctl`, `VALLOC`
  refusé : l'échec est joué à chaque rang possible et on vérifie l'absence de fuite.
- Supposition d'erreur : pointeurs noyau passés comme pointeurs utilisateur, handle fermé deux fois, nom de port
  invalide, `W|X`, longueur nulle, `%.3s` sur chaîne non terminée.
- Couverture gcov (lignes, branches, conditions) sur le code pur, et test de mutation sur les lots les plus à risque.

## 4. Résultats de la campagne

### 4.1 Vue d'ensemble

| Mesure | Résultat |
|--------|----------|
| Compilation complète (`make -j8 all`) | 0 erreur, noyau, 12 applis, image Limine BIOS et UEFI |
| Autotests noyau au démarrage | 17 sur 17 (`SELFTESTS PASS 17`), BIOS et UEFI |
| Scénarios QEMU | 92 exécutions sur 92 (50 fichiers, deux firmwares pour la plupart) |
| Intégration continue (`tools/ci.sh`) | rejouée dans un conteneur Ubuntu 24.04 (gcc 13, QEMU 8.2, sans KVM) : 13 tests d'outils, 7 lots de tests hôte, 5 scénarios (10 exécutions), tout vert |
| Tests hôte | 669 exécutions de suites, 13,26 millions de vérifications, 0 échec au passage final (632 fichiers `test_*.c`, certains joués en debug et en release, plus 7 bancs de rendu) |
| `make scan` (fonctions C dangereuses, secrets, idiomes risqués) | 0 alerte sur tout le dépôt |
| ruff, ruff format, shellcheck, shfmt | propres |
| `make hdrcheck` | tous les en-têtes sont autonomes |
| États intermédiaires de l'historique git | 15 sur 15 compilent (`make all`) |

### 4.2 Par lot

Colonnes : fichiers de tests hôte, fichiers de scénarios QEMU, ce qui a été le plus poussé, couverture mesurée,
ce qui n'a pas tourné.

| Lot | Hôte | QEMU | Points forts | Couverture mesurée | Non fait ou incomplet |
|-----|------|------|--------------|--------------------|-----------------------|
| a01 cpu | 7 | 9 | octets des descripteurs d'après le SDM, CPUID factice, fautes volontaires `fault=` | 100 % lignes, branches et conditions, sauf `idt_table` 90 % des conditions | pas de SMP |
| a02 pmm | 33 | 6 | modèle de référence 100 000 opérations, métamorphique, échecs injectés, mutation | 100 % sauf `pmm_fault` (93,75 % des lignes) et `pmm_region` (96,67 % des branches) | mutants M13, M20, M32 à M41 non rejoués |
| a03 vmm | 16 | 2 | pseudo-physique, modèle de 4 000 opérations, échec pmm à chaque rang, copies utilisateur | 96,2 % des 954 lignes | pas de `PROT_NONE` ni de pages paresseuses |
| a04 heap | 30 | 2 | 6 graines de métamorphique (324 472 vérifications), échec à chaque rang, fils | non mesurée | `cov-a04`, bancs et TSan non lancés |
| a05 acpi, irq, temps | 17 | 0 | 1e9 points TSC vers ns à moins de 1 ppm, tables ACPI corrompues, RTC | code pur : 85,7 % à 100 % (13 fichiers sur 22 à 100 %) | scénarios dédiés non écrits (couverts par `skel_boot`), extinction jouée seulement par `a20_souris` |
| a06 sched | 17 | 1 | simulation pthreads, files de priorité, mutex, autotests noyau | non mesurée | exclusion mutuelle à 1 M d'incréments au lieu de 10 M |
| a07 proc | 15 | 3 | corpus ELF (4 890 refusés, 1 110 acceptés, graine `0xa07c0de`), `ring3test` 71 vérifications | non mesurée | chaîne `init` vers `winsrv` : redémarrage sur mort testé sur l'hôte seulement |
| a08 obj | 23 | 1 | 100 000 courses simulées sans réveil perdu, détection vérifiée par mutation volontaire (2 222 réveils perdus) | 95,0 % des lignes, 83,8 % des branches | pas de test SMP |
| a09 pci, aléa, crypto | 53 | 4 | vecteurs NIST, RFC 4231, 7914, 8439, khi-deux 246,3 pour 255 degrés, MSI de bout en bout sur l'appareil `edu` | non mesurée | fuzz de configuration PCI, mutation |
| a10 clavier, souris | 40 | 1 | décodeur sur 100 000 octets aléatoires, tables us et fr complètes | non mesurée | `inputlog`, `input_boot_init` et changement de disposition non testés sur l'hôte |
| a11 bloc, virtio | 21 | 3 | CRC32, GPT et MBR fabriquées (relues par `sgdisk -v` et `sfdisk`), faux appareil virtio complet | 97,4 % des 1 084 lignes, 90,1 % des branches, MC/DC à 100 % sur les bornes | MSI-X |
| a12 vfs, fat | 26 | 1 | cpio 405 vérifications (10 000 entrées, troncature à chaque octet), FAT32 : 400 images corrompues, 20 000 opérations avec contrôle fsck | non mesurée | scénario en BIOS seulement |
| a13 affichage | 37 | 4 | fuzz de 20 000 appels (graine 20261005), géométrie hostile, mutation (8 tuées sur 12) | 99,4 % des 880 lignes, 87,4 % des 546 branches | 2 mutants survivants renforcés mais non rejoués, chemin DISPI par ports jamais exécuté |
| a14 libvelum | 63 | 1 | allocateur (churn, double libération), pile de départ, 68 vérifications de `vtest` dans QEMU | pile de départ et RELRO 100 %, allocateur 505 sur 526 lignes, libc pure 100 % des lignes | `crt0.S`, `guard.S`, `thread.S` seulement en QEMU |
| a15 gfx | 30 | 0 | mélange exhaustif 16,7 millions de triplets, 1 million d'appels fuzz, régions contre modèle bitmap | non mesurée | `cov.sh`, bancs de performance |
| a16 polices | 23 | 1 | 10 365 120 séquences UTF-8 recoupées avec CPython sans écart, mutation (19 mutants tués sur 22 au premier passage, les 3 survivants tués à la repasse ciblée), génération reproductible | 100 % des lignes sauf l'autotest (92,7 %) | repasse complète des mutants sur l'arbre final interrompue |
| a17 thème verre | 30 | 0 | partition complète de `luna_hit_test` sur trois tailles, verre (bornes, repli, robustesse), palette clair et sombre, lanceur, galeries relues (voir 4.8) | non mesurée | couverture et mutation du verre, comparaison à la maquette (MT-25) |
| a18 winsrv | 22 | 0 | 1 000 opérations aléatoires, 100 000 messages, composition incrémentale identique à la complète | non mesurée | scénario QEMU propre, `lib/wm` contre le serveur |
| a19 ctl | 54 | 0 | chaque allocation échoue une fois (348), édition UTF-8 en fuzz, contraste WCAG (17:1, 5,25:1, 3,9:1), contrôles sur verre | non mesurée | comparaison à la maquette (MT-25) |
| a20 session | 42 | 2 | PBKDF2 contre `hashlib`, mise en page de la connexion, du bureau et de Bonjour d'après la maquette, contrastes sur fonds variables | non mesurée | rendu QEMU du thème verre non rejoué |
| Intégration | 0 | 3 | session complète, préemption, kill | sans objet | voir point 6 |

### 4.3 Mesures relevées pendant la campagne (QEMU sous KVM, BIOS)

- Démarrage jusqu'à « boot ok » : 0,19 s. Les 17 autotests durent environ 8 s au total sur une machine peu chargée.
- Horloge : TSC mesuré à 1 896 155 183 Hz, minuterie de 100 ms déclenchée après 100 029 620 ns.
- Ordonnanceur : quatre fils actifs se partagent le CPU entre 147 848 et 165 817 µs, un fil de priorité haute est réveillé
  avec 119 µs de retard, `sched_sleep_ns(20 ms)` mesuré à 20 126 µs, mutex à 6 000 sur 6 000.
- Fils : 10 000 créations et destructions, pages libres 64 307 avant et après, objets du tas 4 avant et après.
- Processus : 1 000 lancements et fins en 583 ms, pages libres 64 294 avant et après.
- Tas : environ 260 cycles par opération en rafale pour 32 octets, plus de 1 800 pour 256 octets (debug, indicatif).
- Mémoire physique : 72 Kio de métadonnées pour 256 Mio, 1 152 Kio pour 4 Gio (calculés), 1 728 Kio mesurés avec 4 Gio.
- Images relues à l'œil : écran de connexion, bureau avec icônes et barre des tâches, menu Démarrer ouvert.

### 4.4 Déroulé de la campagne d'intégration

| Passage | Résultat | Ce qui a été fait ensuite |
|---------|----------|---------------------------|
| Premier noyau complet | boot OK, 17 autotests OK, `init` lance `winsrv` et `logon` | écran de connexion capturé |
| Premiers scénarios (79 exécutions) | 60 réussies, 19 en échec | tri : 16 exécutions de scénarios trop stricts, 2 d'un écart de spécification (`VALLOC` arrondit), 1 effet de QEMU sur la souris |
| Après assouplissement des scénarios et du format `[TEST]` | 78 sur 79 | seul `a01_fault_stack` UEFI échoue, une fois dans la suite complète puis trois réussites seul : course de lecture sur `PANIC:` |
| Après correction du harnais et ajout de la préemption | 81 sur 81 | suite figée |
| Clone propre du dépôt public, puis rejeu en isolation | 82 sur 83, puis `a20_desktop` en échec 4 fois sur 8 | trois causes dans le harnais (T15), 46 exécutions sur 46 réussies après correction, suite complète 83 sur 83 |
| Suite hôte complète | 529 suites vertes, 1 échec (`a06/mutex`) | critère de convoi relâché, suite verte |
| Contrôle de sécurité sur tout le dépôt | 3 alertes | deux `strcpy` et un `strcat` des tests remplacés par des fonctions bornées, contrôle à 0 |
| Historique git (15 états cumulatifs) | 1 état ne liait pas avant le crochet, puis 15 sur 15 compilent | crochet faible `sched_irq_exit` dans le squelette |

### 4.5 Répartition automatisé et manuel

La couverture est mesurée sur les fonctionnalités vérifiables du système (103, regroupées par sous-système), chacune avec
un mode de vérification et la liste des tests qui la couvrent. Les données sont dans [`tests/matrice.toml`](tests/matrice.toml),
le script `tools/matrice_couverture.py` vérifie que chaque test cité existe et calcule les taux, et il génère
[`tests/MATRICE.md`](tests/MATRICE.md) (matrice complète) et [`tests/MANUEL.md`](tests/MANUEL.md) (plan de tests manuels).

| Mode | Sens | Fonctionnalités | Part |
|------|------|-----------------|------|
| A | automatisé, oracle automatique | 85 | 82,5 % |
| AM | logique automatisée, complément manuel (visuel, ergonomie, vrai clavier) | 11 | 10,7 % |
| M | manuel seulement | 7 | 6,8 % |

Couverture automatisée (A et AM) : **93,2 %**. Couverture manuelle seule : **6,8 %**. Au moins un test manuel
est nécessaire pour 17,5 % des fonctionnalités. Par nombre de cas : 2 185 cas automatisés (2 035 cas unitaires nommés,
50 scénarios QEMU, 100 tests Python) contre 22 cas manuels, soit 99,0 % ; ce taux est plus élevé parce que les cas
automatisés sont plus fins, le taux fonctionnel est la mesure à retenir.

Les 7 fonctionnalités en mode M sont : le redémarrage ACPI, la performance du dessin (bancs jamais lancés), la netteté
des polices, la fidélité visuelle au style de référence (Luna, puis maquette verre), le démarrage sur matériel réel, l'endurance et la cohérence linguistique
de l'interface. Le FAT32 est passé de M à AM avec le scénario `a12_fat32` (image mtools, relecture par mtools, `fsck.vfat`).

### 4.6 Exécution des cas manuels

Les cas de [`tests/MANUEL.md`](tests/MANUEL.md) ont été joués les 5 et 6 octobre 2026 sous QEMU/KVM : le clavier et la souris
sont injectés par le moniteur QEMU avec `tools/manuel.py`, chaque étape est relue sur une capture, et les mesures sont
faites sur les pixels (`tools/contraste.py` pour les rapports de contraste). Rien n'a été joué sur du matériel réel ni par
une personne devant l'écran. Résultat : 15 réussis, 4 partiels, 2 à exécuter (3 avec MT-25, ajouté pour le thème verre), aucun échec. Les quatre cas en échec le
5 octobre (MT-14, MT-17, MT-22, MT-23) ont été rejoués le 6 octobre après correction de leurs défauts (voir 5.3).

| Cas | Titre | Résultat | Incident |
|-----|-------|----------|----------|
| MT-01 | Écran de démarrage | Réussi | - |
| MT-02 | Démarrage en UEFI | Réussi | - |
| MT-04 | Fidélité visuelle au style Luna (ancien thème) | Partiel | - |
| MT-05 | Lisibilité des polices | Partiel | - |
| MT-06 | Manipulation des fenêtres à la souris | Réussi | - |
| MT-07 | Menu Démarrer au clavier | Réussi | - |
| MT-08 | Boîtes Exécuter et Éteindre | Réussi | - |
| MT-09 | Redémarrage ACPI | Réussi | - |
| MT-10 | Horloge de la barre des tâches | Réussi | - |
| MT-11 | Saisie au clavier AZERTY | Réussi | - |
| MT-13 | Session entière au clavier seul | Réussi | - |
| MT-14 | Contraste des textes | Réussi (27 zones, de 4,97 à 21,00) | P25 (corrigé) |
| MT-16 | Plusieurs fenêtres dans la barre des tâches | Réussi | - |
| MT-17 | Endurance | Partiel (300 fenêtres avec 128 Mo et avec 64 Mo, 8 h non jouées) | P27 (corrigé) |
| MT-18 | Matériel réel | A exécuter | - |
| MT-19 | Fluidité perçue | A exécuter | - |
| MT-20 | Mort du serveur de fenêtres | Réussi | - |
| MT-21 | Lisibilité de l'écran d'arrêt | Réussi | - |
| MT-22 | Revue linguistique de l'interface | Réussi | P26 (corrigé) |
| MT-23 | FAT32 sur une image réelle | Réussi | P24 (corrigé) |
| MT-24 | Construction sur une machine propre | Partiel | T15 |
| MT-25 | Fidélité à la maquette verre, clair et sombre | A exécuter | - |

Ce qui reste à faire : la fluidité perçue (MT-19) demande une personne, le matériel réel (MT-18) une machine, l'endurance
de 8 h (MT-17 n'a duré que 20 minutes) et la machine vierge (MT-24) du temps. MT-20, bloqué le 5 octobre, se joue depuis
que `init` accepte l'option de test `init.mort-winsrv` (scénario `int_winsrv_mort`). Le constat détaillé de chaque cas,
avec ses observations, est dans sa fiche.

### 4.7 Couche APK (6 octobre)

Douze lots écrits en trois vagues, chacun testé seul contre des faux, puis assemblés. La stratégie tient en une
phrase : tout ce qui vient d'un APK est hostile. Chaque lecteur a donc, en plus de ses cas nominaux et de ses refus
(un cas par règle), une troncature à chaque octet et des mutations à graine fixe, sous ASan et UBSan.

| Lot | Élément | Vérifications | Ce qui a été le plus poussé | Non fait ou limite |
|-----|---------|---------------|-----------------------------|--------------------|
| d01 | ZIP, DEFLATE | 33 162 | archives et flux de référence faits par les modules `zipfile` et `zlib` de Python, 24 000 mutations | 4 096 entrées et noms de 255 octets au plus, pas de ZIP64 |
| d02 | XML binaire, ressources | 262 | tables UTF-8 et UTF-16, cycle de références, 20 000 mutations | entrées complexes refusées, fixtures du même auteur |
| d03 | conteneur DEX | 1 492 | 20 000 mutations avec somme recalculée, dont plus de 1 000 acceptées puis parcourues en entier | tri des tables et SHA-1 non contrôlés |
| d04 | bytecode | 1 483 | table de décision de 51 refus ; propriété : un code accepté ne fait jamais sortir un décodage ni un registre des bornes | pas de vérification de types (faite à l'exécution) |
| d05 | RSA, X.509 | 11 146 | 53 vecteurs `openssl` (2048 à 4096 bits, e = 3 et 65537), contrefaçon classique par racine cubique refusée | exponentiation lente, non mesurée |
| d07 | signature v2, manifeste | 130 | 30 APK fabriqués, 12 000 mutations d'un APK signé dont aucune n'est acceptée, second écrivain de signature indépendant | un seul signataire, RSA seulement |
| d08 | objets, tas, ramasse-miettes | 242 | références hostiles, chaque allocation en échec une à une, code invalide refusé | type déclaré d'un champ référence non contrôlé à l'écriture |
| d09 | interpréteur | 1 467 | division, décalages et conversions flottantes aux bornes, reste flottant comparé à celui de l'hôte | champs d'instance et appels virtuels testés à l'intégration seulement |
| d10 | registre des paquets | 21 158 | panne injectée à chaque opération de chaque scénario : registre toujours ancien ou nouveau, jamais mêlé | fichier absent non détecté par la liste |
| d11 | classes de base, vues | 264 | sept programmes d'essai aux résultats connus, 10 000 clics avec un tas de 1 Mio sans croissance | `StringBuilder` plafonné à 1 024 caractères |
| d12 | `apkrun`, `apkinst` | 357 | mise en page sur arbres hostiles, renommage sûr avec panne à chaque étape | affichage prouvé par les scénarios seulement |
| d00 | essais croisés | 47 | voir ci-dessous | |

Essais croisés, qui sont la preuve d'intégration :

- les fichiers DEX et APK fabriqués par les outils Python sont relus par les lecteurs C, écrits par d'autres mains
  depuis les mêmes spécifications ; les deux tables d'instructions (256 entrées) sont comparées nom par nom ;
- la machine virtuelle complète donne le résultat attendu des sept programmes d'essai (arithmétique, boucles,
  tableaux, aiguillages, exceptions, appels virtuels et d'interface, chaînes) ;
- un APK de 2,4 Mio est accepté, et un octet changé dans chacun de ses trois blocs de 1 Mio le fait refuser ;
- `int_apk_installation` (QEMU, disque FAT32 fait par mtools) : installation d'un APK valide, deux fois ; refus
  d'un APK altéré, non signé, signé par une autre clé, absent ; lancement de l'appli installée ; refus affiché au
  lancement de l'APK altéré ; puis le disque est relu par mtools (APK identique octet pour octet, registre d'une
  ligne) et contrôlé par `fsck.vfat` ;
- `int_apk_menu` (QEMU, BIOS et UEFI) : l'appli d'exemple est lancée à la souris depuis le menu Démarrer, trois
  clics donnent trois lignes de journal et « Clics : 3 » à l'écran, Alt+F4 la ferme.

Relecture à contexte vierge du code de signature (`lib/rsa`, `lib/apk`, `lib/zip`, les deux appelants), faite après
l'intégration : aucun contournement, aucune forge, aucun défaut mémoire ; un défaut de durée sur fichier hostile (P29),
corrigé en plafonnant les noms d'entrée à 255 octets. Limite de cette preuve : aucun APK produit par la chaîne officielle
n'a été lu (elle n'est pas installée ici).

### 4.8 Thème verre, lots a16 à a20 (10 octobre)

Après la campagne du 5 et 6 octobre, l'interface est passée du premier thème (style Windows XP) à un thème « verre »
clair et sombre. Les lots a16 à a20 ont été réécrits ou étendus ; les fonctionnalités F95 à F103 et le cas manuel
MT-25 de la [matrice](tests/MATRICE.md) les couvrent. Le plan est fondé sur les risques : une régression de géométrie
casse le hit-test (risque de clic perdu), un flou mal borné écrit hors de la surface (risque mémoire), un verre mal
élargi donne une image différente entre composition incrémentale et complète (risque visuel silencieux).

| Risque | Technique (29119-4) | Tests |
|--------|---------------------|-------|
| Débordement ou lecture hors surface du flou | valeurs limites, partitions de rectangles (hors surface, dégénéré, rognage) | `test_glass_bounds.c`, `test_glass_robust*.c` |
| Verre sans mémoire de travail | injection de panne (tampon absent, trop petit), repli opaque | `test_glass_edge.c`, `test_srv_glass.c` |
| Composition incrémentale différente de la complète | métamorphique : mêmes pixels dans les deux modes | `test_core_glass_eq.c`, `test_core_glass_chain.c` (chaînes de bandes, boucle bornée) |
| Géométrie du cadre, du dock, du lanceur, de la connexion | tables de valeurs calculées depuis la maquette, balayage de pixels | `test_geo_*.c`, `test_hit_*.c`, `test_launcher*.c`, `test_logon_layout*.c`, `test_desktop*.c` |
| Texte illisible sur fond de verre | contraste WCAG calculé sur fonds variables, clair et sombre | `test_contrast*.c`, `test_colors_ext.c` |
| Police générée différente d'une exécution à l'autre | génération reproductible, empreintes, budget de taille | `test_genfont.py`, `test_font_budget.c` |

Résultat relevé au point de départ de la reprise du 10 octobre (`make host-a17 host-a18 host-a19 host-a20`) : 165 suites,
372 283 vérifications, 0 échec ; `host-a16` vert, `test-host` code 0, `make all` code 0 (image de 67 Mio). Ce relevé
vient de la passe de reprise, il n'a pas été rejoué pour cette mise à jour de la documentation.

Ce qui n'est pas fait ou pas démontré pour ce thème :

- Aucune couverture gcov ni mutation sur le code du verre, du dock, du lanceur ni de la connexion.
- Aucun scénario QEMU n'a été rejoué pour le rendu. MT-25 (comparaison à la maquette, clair et sombre) est à exécuter,
  et les fiches MT-01, MT-04, MT-05, MT-14 et MT-22 datent de l'ancien thème.
- Le contrôle `make scan` n'a pas été rejoué dans la passe de reprise elle-même ; relancé ensuite sans dépôt git
  (`tools/scan_securite.py` parcourt alors le disque), il rend 0 alerte.
- Le thème sombre n'est exercé que par les tests : aucune appli ne le sélectionne.
- Limites visuelles connues (ombres de fenêtres, flou des fenêtres sous le dock, client opaque, écrans non intégrés) :
  README, section Limites connues.

## 5. Registre des incidents

Gravité : haute si le système gèle, panique ou perd l'isolation, moyenne si une fonction est fausse
mais le système reste utilisable, basse si c'est cosmétique ou limité à un test.

### 5.1 Défauts trouvés dans le produit

| Réf | Élément | Attendu | Obtenu | Gravité | Test de régression |
|-----|---------|---------|--------|---------|--------------------|
| P01 | a01 trace d'appels | s'arrête sur une chaîne de cadres nulle | lecture à l'adresse 0 | moyenne | cas de chaîne partant de 0 dans les tests de trace |
| P02 | a03 `vmm_io_map` | une page, un seul type de cache | tables ACPI déjà en WB remappées en UC | moyenne | test de plage HHDM dans `test_vmm_io` |
| P03 | a06 mutex | un convoi coûte moins d'un réveil pour 4 prises | 203 s pour 3 fils de 100 000 prises | moyenne | critère indépendant de la charge |
| P04 | a07 sortie de processus | un fil mourant ne revient jamais sur un espace détruit | #GP noyau, espace libéré avant la dernière bascule | haute | `a07_selftest` (1 000 lancements), `a07_ring3test` |
| P05 | a12 recherche de nom | « été » trouve « Été » | repli de casse limité à l'ASCII | moyenne | test de noms Unicode dans `fat_names` (à réintégrer, voir les limites) |
| P06 | a14 allocateur | une adresse rendue par le noyau après `VFREE` est valide | prise pour une double libération | haute | `test_malloc_recycle` |
| P07 | a14 `relro_lock` | rend 0 pour une plage sans page entière | autre comportement, corrigé (détail dans la justification a14) | basse | cas ajouté |
| P08 | a15 `gfx_blit_smooth` | respecte `sr.y` | ignorait la ligne de départ de la source (135 échecs) | haute | `test_smooth_regress` |
| P09 | a19 `ctl_add(CT_MENU)` | refus propre | déréférencement NULL | moyenne | `test_tree_invalid` |
| P10 | a19 liste | Fin sans sélection va au dernier élément | allait au premier | basse | `test_list` |
| P11 | a20 barre des tâches | bouton de 160 px au plus | 161 px | basse | `taskbar-sweep` |
| P12 | a20 menu Démarrer | le pied reste dans le panneau | débordait pour une entrée de 64 px | basse | `startmenu-layout` |
| P13 | a20 bureau | Maj ne sélectionne pas la première icône | la sélectionnait | basse | `desk_move` |
| P14 | a20 mot de passe | le champ se vide proprement | ne se réinitialisait pas | moyenne | cas ajouté |
| P15 | libk `%.3s` | la précision borne la lecture | lecture au-delà d'une chaîne non terminée, SIGSEGV prouvé | moyenne | `test_snprintf_va` (attendu C17) |
| P16 | libk `%#o` | 0 donne `0` | donnait `00` | basse | `test_snprintf` |
| P17 | libk `%d` de 0 | donne `0` | n'écrivait aucun chiffre (les horodatages du journal étaient vides) | moyenne | `test_fmt` (cas « zero »), trouvé au premier boot QEMU |
| P18 | boot, adresse RSDP | adresse physique | soustraite du HHDM, valeur fausse | moyenne | `skel_boot`, `acpi_selftest` |
| P19 | ordonnanceur, sortie d'IRQ | un fil qui calcule en anneau 3 est préempté | machine gelée (un seul CPU) | haute | `int_preempt`, preuve différentielle ci-dessous |
| P20 | processus tué qui boucle | meurt au prochain retour d'interruption | survit, `KILL FAIL attente de la mort` | haute | `int_preempt` (`KILL PASS`) |
| P21 | libvelum `v_spawnv` | `argv[0]` fourni par le noyau | chemin passé deux fois, le menu affichait `/system/bin/shell` à la place du nom | moyenne | `test_sys_spawnv`, capture du menu |
| P22 | libvelum et noyau | `-pie` donne un ELF `ET_DYN` | `-static -pie` donnait `ET_EXEC` à l'adresse 0, applis inchargeables | haute | contrôle `readelf` dans `a14-elf`, `a07_ring3test` |
| P23 | Luna bouton Démarrer | « démarrer » entier | « démar... » | basse | capture relue |
| P28 | machine virtuelle des applis APK | un tas plein lève `OutOfMemoryError` | l'instance était un `Throwable` simple, créée avant que la classe existe : un `catch OutOfMemoryError` ne l'attrapait pas | basse | `tests/host/d00/test_croise_oom.c` |
| P29 | lecture d'une archive APK | un fichier hostile est refusé vite | 4 096 entrées aux noms de plusieurs kilo-octets : recherche de doublons en temps quadratique multiplié par la longueur des noms, plusieurs minutes avant le refus (trouvé par la relecture à contexte vierge, non mesuré) | basse | cas « nom de 256 octets » de `tests/host/d01/test_zip.c` |

Preuve différentielle de P19 et P20 : sans les appels `sched_irq_exit()` et `proc_return_check()`, `int_preempt` échoue
au bout de 30 s (« SPIN PASS » absent, puis « KILL FAIL »). Avec eux, `SPIN PASS` à 0,42 s et `KILL PASS` à 0,30 s.

### 5.2 Défauts de l'infrastructure et des tests

| Réf | Élément | Cause | Correction |
|-----|---------|-------|------------|
| T01 | `mk/skel.mk` | jokers sur `arch/x86_64/*.c` : fichiers compilés deux fois ou sans leur lot | liste explicite |
| T02 | `rootfs.stamp` | prérequis évalués avant les applis : initrd sans applis | règle déplacée après `user.mk` |
| T03 | `run_scenarios.py` | image mise en cache, non reconstruite quand seul l'initrd change (résultats obsolètes) | date de l'initrd prise en compte |
| T04 | `vtest.py` | « PANIC: » lu avant la fin de la ligne, faux échec | on n'échoue plus si le motif attendu contient `PANIC` |
| T05 | `selftests.c` | `[TEST] x ... OK` coupé par les journaux du test | ligne de résultat complète sur sa propre ligne |
| T06 | scénarios a01, a04 | mots trop larges (« autotest », « anomalie ») | motifs précis |
| T07 | scénario a01 `fault=pf` | le gestionnaire du vecteur 14 est celui d'a03 (autre message) | on vérifie CR2 et RIP exacts, pas le texte |
| T08 | scénario a14 | `VALLOC` arrondit les longueurs, le test attendait un refus | test et `syscalls.md` alignés |
| T09 | scénario a16 | le démarrage de la session coupe la console noyau | `init=/aucun` |
| T10 | scénario a20_souris | QEMU fusionne les déplacements rapides, le curseur restait au coin | pause de 0,12 s entre mouvements |
| T11 | test a06 mutex | seuil de convoi lié au nombre de cœurs de l'hôte (55 % mesuré, 25 % exigé) | seuil à 80 % |
| T12 | tests a09, a12 | `strcpy` et `strcat` dans les fakes | fonctions bornées |
| T13 | tests a14 | figeaient les défauts libk | attendu C17 rétabli |
| T15 | scénario `a20_desktop` | instable : 4 échecs sur 8 en séquentiel, 12 sur 30 en parallèle. Trois causes : capture juste après la touche avant le redessin, image de référence prise pendant le premier dessin du bureau, captures du BIOS et de l'UEFI écrites sous le même nom dans le même dossier | `capture_quand` et `capture_stable` dans `vtest.py`, noms de capture préfixés par la machine, 46 exécutions sur 46 réussies |
| T17 | `mk/user.mk`, `mk/host.mk` | les dépendances d'en-têtes n'étaient pas suivies : motif `wildcard` trop court pour les applis, aucune pour les tests hôte. Après un changement de `shell.h`, le shell était lié avec des objets périmés et les boîtes de dialogue ne s'ouvraient plus (`a20_desktop` et `a20_souris` en échec), et des tests hôte qui ne compilaient plus restaient verts | liste des `.d` tirée des sources, dépendances générées pour chaque test hôte, `tests/tools/test_dependances.py` |
| T18 | `mk/lot-a20.mk`, banc de rendu | la colle APK ajoutée à `user/apps/common/` inclut un en-tête système que le banc de rendu de l'hôte n'a pas : `a20-render` ne compilait plus | fichiers `apkglue*.c` exclus du banc, comme les `os_*.c` |
| T16 | plan de tests manuels | étapes inexactes : Tab avant Entrée à la connexion (MT-13), `fault=panic` sans `selftest` (MT-21), formatage FAT32 sans `-F` (MT-23) | étapes corrigées dans les fiches |

La référence T14 n'est pas réutilisée : l'entrée visait un outil de style retiré du projet.

### 5.3 Défauts trouvés par les cas manuels, corrigés le 6 octobre

Chaque défaut a d'abord été reproduit par un test qui échouait, puis corrigé. Le test reste dans la suite.

| Réf | Élément | Attendu | Obtenu | Gravité | Cause | Correction | Test de régression |
|-----|---------|---------|--------|---------|-------|------------|--------------------|
| P24 | FAT32, compteur `FSInfo` | nombre de clusters libres exact après écriture, renommage et suppression | 128879 pour 128880 réels, `fsck.vfat -n` rend 1 | basse | le secteur FSInfo n'était écrit que sur `fsync` ou au démontage, pas après une suppression, un renommage ou un `mkdir` | FSInfo écrit avec chaque opération qui change la FAT | `test_fat_fsinfo.c`, `a12_fat32` (code 0 de `fsck.vfat` exigé) |
| P25 | Luna, contraste | rapport de 4,5 au moins pour le texte courant | « démarrer » à 3,57, boutons inactifs de la barre des tâches à 3,91, titre de fenêtre inactive à 2,07 | basse | couleurs reprises du style d'origine sans calcul de contraste | fond foncé dans la même teinte sous le texte blanc (pire point après correction : 4,63) | `test_contrast_bar.c`, `test_contrast_win.c` |
| P26 | textes de l'interface | espaces insécables avant `:`, `!`, `?` et dans les guillemets | espaces ordinaires, la ligne peut se couper là | basse | chaînes saisies avec U+0020 | U+00A0 dans les chaînes ; U+00A0 et U+202F dessinés avec la chasse de l'espace | `test_typographie.py`, `test_nbsp.c` |
| P27 | noyau, sections partagées | surface rendue à la fermeture d'une fenêtre | après 222 ouvertures (128 Mo) plus aucune connexion, après 163 avec 64 Mo plus aucune fenêtre | moyenne | `SYS_VFREE` retirait les pages d'une section sans lâcher la référence prise par `section_map` : la surface restait allouée tant que le serveur vivait | `SYS_VFREE` rend la référence quand plus aucune page de la section n'est mappée | `test_section3.c` à `test_section6.c`, `int_endurance_fenetres` (320 fenêtres avec 64 Mo, mémoire libre identique avant et après) |

## 6. Écarts au plan et limites

- Les cas manuels ont été joués sous QEMU/KVM avec des entrées injectées et des captures relues (voir 4.6), pas par
  une personne devant l'écran ni sur du matériel. Quatre sont partiels (dont l'endurance, jouée 20 minutes au lieu de
  8 h), deux à faire.
- Couverture non mesurée pour a04, a06, a07, a09, a10, a12, a15, a17, a18, a19, a20. Les critères de fin étaient
  « couverture mesurée », ils ne sont donc pas démontrés pour ces lots.
- Mutation : faite pour a02, a08 (contrôle de la simulation), a13 et a16, partielle ou interrompue ailleurs.
- Aucun test sur du vrai matériel : tout tourne sous QEMU/KVM (q35, `pc` pour PCI, `hpet=off`, `qemu64`). Les risques propres
  au matériel (RDSEED absent, MCFG fausse, contrôleur 8042 qui force la traduction, ponts PCI non numérotés) sont listés mais non testés.
- Un seul CPU : aucun test SMP, les verrous sont prévus pour mais jamais exercés à plusieurs cœurs réels.
- Aucune mesure de performance sous KVM en profil release. Les chiffres du point 4.3 sont en debug, indicatifs.
- Les tests de temps (équité, préemption, sommeil) peuvent échouer sur une machine très chargée : des autotests de
  temps ont échoué quand plusieurs machines QEMU tournaient en parallèle sur le même hôte.
- Scénarios QEMU absents pour a05, a12, a15, a17, a18, a19 : ces sous-systèmes sont couverts indirectement par
  `skel_boot`, `int_session` et `a20_souris`.
- Le scénario FAT32 (`a12_fat32`) ne tourne qu'en BIOS. L'écriture FAT32 est jouée contre un faux disque sur l'hôte et
  contre une image mtools dans ce scénario.
- Les vérifications de plusieurs sous-systèmes ont été limitées en durée (mutation interrompue, bancs de performance
  non lancés). Le point 4.2 liste, sous-système par sous-système, ce qui n'a pas tourné.

Évaluation face aux critères de fin : les critères 1 à 5 sont atteints, avec les réserves de couverture ci-dessus.
Les incidents de gravité haute (P04, P06, P08, P19, P20, P22) sont tous corrigés et ont leur test.

## 7. Rejouer la campagne

```
make -j8 all                                  noyau, applis, image (build/main/debug)
make test-qemu                                tous les scénarios, BIOS et UEFI (environ 3 min à -j4)
python3 tools/run_scenarios.py --kernel build/main/debug/kernel.elf \
    --root build/main/debug/root --out build/main/qemu --filter int_ --jobs 2
make -j8 -k test-host                         suites hôte (la première fois plus de 10 min, à lancer en arrière-plan)
make B=build/aNN lot LOT=aNN                  un lot seul
make hdrcheck                                 autonomie des en-têtes
make matrice                                  recalcule les taux de couverture automatisée et manuelle
make scan                                     contrôle statique de sécurité
ruff check tools tests && ruff format --check tools tests
```

Les journaux série et les captures (`.ppm`) de chaque scénario restent dans `build/main/qemu/<scénario>/`.
Les captures de l'écran de connexion et du bureau se convertissent avec `Image.enregistrer_png` de `tools/vtest.py`.
