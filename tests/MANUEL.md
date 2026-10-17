# Plan de tests manuels

Fichier généré par `tools/matrice_couverture.py` à partir de [`matrice.toml`](matrice.toml). Il couvre ce que
l'automatisation ne vérifie pas : jugement visuel, ergonomie, clavier et matériel réels, endurance, revue
linguistique, reproductibilité sur machine propre. Les liens avec les fonctionnalités sont dans
[`MATRICE.md`](MATRICE.md).

Exécution : 19 cas sur 22 ont été joués les 5 et 6 octobre 2026 sous QEMU/KVM, avec le clavier et la souris
injectés par le moniteur QEMU (`tools/manuel.py`) et des captures relues une par une. Aucun n'a été joué sur du matériel
réel ni par une personne devant l'écran. Restent à faire : la fluidité perçue (MT-19), le matériel réel (MT-18), l'endurance
de 8 h (MT-17 n'a duré que 10 minutes), la machine vierge (MT-24) et la comparaison avec de vraies captures d'époque
(MT-04). MT-14, MT-17, MT-22 et MT-23, en échec le 5 octobre, ont été rejoués le 6 octobre après correction (voir leur
fiche). Ces passages ont eu lieu sur l'ancien thème Luna : MT-25 (fidélité à la maquette verre, thèmes clair et sombre)
n'a pas encore été joué, et les fiches de rendu antérieures (MT-01, MT-04, MT-05, MT-14, MT-22) sont à rejouer sur le
thème verre. Un statut « Échec » renvoie à un incident
du registre de [`../TESTS.md`](../TESTS.md). À chaque nouvelle exécution, noter la date, le résultat et les anomalies
dans la fiche.

22 cas, durée estimée 15 h 15 min. Statuts : 15 réussi, 4 partiel, 3 a exécuter.

| ID | Cas | Durée (min) | Statut |
|----|-----|-------------|--------|
| MT-01 | Écran de démarrage | 5 | Réussi |
| MT-02 | Démarrage en UEFI | 10 | Réussi |
| MT-04 | Fidélité visuelle au style Luna | 45 | Partiel |
| MT-05 | Lisibilité des polices | 10 | Partiel |
| MT-06 | Manipulation des fenêtres à la souris | 15 | Réussi |
| MT-07 | Menu Démarrer au clavier | 10 | Réussi |
| MT-08 | Boîtes Exécuter et Éteindre | 10 | Réussi |
| MT-09 | Redémarrage ACPI | 5 | Réussi |
| MT-10 | Horloge de la barre des tâches | 5 | Réussi |
| MT-11 | Saisie au clavier AZERTY | 15 | Réussi |
| MT-13 | Session entière au clavier seul | 15 | Réussi |
| MT-14 | Contraste des textes | 10 | Réussi |
| MT-16 | Plusieurs fenêtres dans la barre des tâches | 10 | Réussi |
| MT-17 | Endurance | 480 | Partiel |
| MT-18 | Matériel réel | 60 | A exécuter |
| MT-19 | Fluidité perçue | 10 | A exécuter |
| MT-20 | Mort du serveur de fenêtres | 15 | Réussi |
| MT-21 | Lisibilité de l'écran d'arrêt | 5 | Réussi |
| MT-22 | Revue linguistique de l'interface | 20 | Réussi |
| MT-23 | FAT32 sur une image réelle | 30 | Réussi |
| MT-24 | Construction sur une machine propre | 90 | Partiel |
| MT-25 | Fidélité à la maquette verre, thèmes clair et sombre | 40 | A exécuter |

## Fiches

### MT-01 : Écran de démarrage

- Fonctionnalités : F47
- Objectif : Vérifier l'aspect de l'écran de démarrage et la progression de la barre.
- Préconditions : make all, image construite.
- Durée estimée : 5 min
- Statut : Réussi

1. Lancer make run.
2. Observer l'écran dès l'ouverture de la fenêtre QEMU.
3. Noter l'emblème, le nom, la barre à trois blocs et le texte.

Résultat attendu : Fond noir, emblème centré, nom VelumOS, barre de progression qui avance, aucun artefact.

Exécution du 2026-10-05 : QEMU sans KVM (émulation pure), 70 captures successives pendant le démarrage.

Constat : Fond noir, emblème centré, nom VelumOS, barre à trois blocs et texte « Démarrage en cours ». Les blocs avancent d'une capture à l'autre (positions relevées : 382, 408 puis 547 px) et le filet sous la barre se remplit. Sous KVM l'écran ne reste affiché qu'environ 0,2 s, la progression n'est visible qu'en émulation lente.

Observations :

- Avant l'écran de démarrage, la console texte du BIOS reste visible environ une seconde.

### MT-02 : Démarrage en UEFI

- Fonctionnalités : F79
- Objectif : Vérifier que le système démarre avec OVMF et affiche la même session qu'en BIOS.
- Préconditions : OVMF installé, image construite.
- Durée estimée : 10 min
- Statut : Réussi

1. Lancer make run-uefi.
2. Ouvrir la session.
3. Comparer avec le démarrage BIOS.

Résultat attendu : Même écran de démarrage, même bureau, mêmes entrées clavier et souris.

Exécution du 2026-10-05 : QEMU/KVM avec OVMF, clavier et souris injectés par le moniteur, captures comparées au BIOS.

Constat : L'écran de connexion est identique pixel pour pixel à celui du BIOS (0 pixel différent). Le bureau et le menu ne diffèrent que par l'heure (28 pixels). Clavier (Entrée, touche Windows, Échap) et souris (clic sur Démarrer) fonctionnent. Le journal série indique « boot: UEFI ».

### MT-04 : Fidélité visuelle au style Luna

- Fonctionnalités : F61
- Objectif : Comparer les dimensions et les couleurs avec des captures de référence documentées.
- Préconditions : Captures de référence de l'interface d'époque (documentation publique).
- Durée estimée : 45 min
- Statut : Partiel

1. Capturer le bureau, une fenêtre et le menu Démarrer à 1024x768.
2. Mesurer barre des tâches, barre de titre, boutons de légende, bouton Démarrer.
3. Noter les écarts en pixels.

Résultat attendu : Écarts de un pixel au plus sur les dimensions, couleurs proches de la palette Luna.

Note : Joué le 5 octobre sur l'ancien thème Luna. Ne vaut plus pour le thème verre : voir MT-25.

Exécution du 2026-10-05 : QEMU/KVM, mesures en pixels sur captures 1024x768.

Constat : Barre des tâches 30 px, barre de titre 30 px, cadre 4 px, boutons de légende 21x21, bouton Démarrer 100 px et zone de notification 80 px : conformes aux valeurs documentées dans la justification a17. L'entrée du menu Démarrer mesure 30 px alors que 22 est documenté. Aucune capture de référence d'époque n'est disponible hors ligne, la comparaison à de vraies captures reste à faire.

Observations :

- Entrée de menu à 30 px contre 22 px documentés (valeur marquée incertaine dans la justification).

### MT-05 : Lisibilité des polices

- Fonctionnalités : F58
- Objectif : Vérifier la netteté des textes et le rendu des accents français.
- Préconditions : Session ouverte.
- Durée estimée : 10 min
- Statut : Partiel

1. Ouvrir la fenêtre Bonjour.
2. Lire les libellés des icônes, le titre, le menu Démarrer.
3. Vérifier é, è, ê, à, ç, œ, «, ».

Résultat attendu : Textes lisibles à 100 %, accents complets, graisses régulières.

Exécution du 2026-10-05 : QEMU/KVM, captures agrandies.

Constat : Textes lisibles, é è ê à ç ù « » affichés correctement dans les boîtes, le menu et un champ de saisie. Le caractère œ n'a pas pu être vérifié à l'écran : aucune chaîne de l'interface ne le contient et la disposition clavier ne le produit pas. Sa présence dans la police est couverte par test_font_cover.c.

Observations :

- Le gras du texte de la fenêtre Bonjour est plus grossier que le texte normal (pixels visibles), celui de l'écran de connexion est lissé.

### MT-06 : Manipulation des fenêtres à la souris

- Fonctionnalités : F65
- Objectif : Déplacer, redimensionner, réduire, agrandir, restaurer et fermer une fenêtre.
- Préconditions : Session ouverte, fenêtre Bonjour ouverte.
- Durée estimée : 15 min
- Statut : Réussi

1. Glisser la barre de titre.
2. Redimensionner par les bords et les coins.
3. Utiliser les trois boutons de légende.
4. Cliquer une autre fenêtre et vérifier l'ordre.

Résultat attendu : Mouvements fluides, tailles minimales respectées, ordre et focus cohérents.

Exécution du 2026-10-05 : QEMU/KVM, souris injectée par le moniteur (déplacements relatifs), captures relues.

Constat : Glisser de la barre de titre (déplacement exact de 400 x 200 px), redimensionnement par le coin inférieur droit (agrandi de 62 x 82 px, puis réduit jusqu'à une taille minimale d'environ 110 x 130 px), réduire, agrandir (la zone de travail est remplie, le bouton devient « restaurer »), restaurer (ancienne taille et position), fermer par la croix. Quand on réduit ou ferme la fenêtre active, le focus passe à la fenêtre suivante.

Observations :

- Les nouvelles fenêtres s'ouvrent toutes au même endroit, sans décalage en cascade.
- À la taille minimale, le titre est tronqué sous les boutons de légende et le bouton Fermer est coupé.
- Le contenu ne se remet pas en page quand la fenêtre est agrandie : le bouton Fermer reste à sa place.

### MT-07 : Menu Démarrer au clavier

- Fonctionnalités : F74
- Objectif : Parcourir le menu avec les flèches, Entrée, Échap et la touche Windows.
- Préconditions : Session ouverte.
- Durée estimée : 10 min
- Statut : Réussi

1. Appuyer sur la touche Windows.
2. Naviguer avec les flèches, ouvrir Tous les programmes.
3. Fermer avec Échap.

Résultat attendu : Sélection visible, sous-menu ouvert et fermé correctement, focus rendu à la fenêtre précédente.

Exécution du 2026-10-05 : QEMU/KVM, clavier injecté par le moniteur, captures relues.

Constat : La touche Windows ouvre le menu, les flèches haut et bas déplacent la sélection (visible en bleu), la flèche droite ou Entrée sur « Tous les programmes » ouvre la liste (Bonjour, Retour), la flèche gauche ou Échap revient au menu principal, un second Échap ferme le menu et le bureau redevient identique. Avec une fenêtre ouverte, son titre reste actif pendant et après l'ouverture du menu (0 pixel différent).

Observations :

- La flèche droite sur « Fermer la session » ne passe pas à « Éteindre » : il faut la flèche bas.

### MT-08 : Boîtes Exécuter et Éteindre

- Fonctionnalités : F74
- Objectif : Vérifier les boutons Arrêter, Redémarrer et Annuler.
- Préconditions : Session ouverte.
- Durée estimée : 10 min
- Statut : Réussi

1. Menu Démarrer puis Éteindre.
2. Essayer Annuler, puis Redémarrer, puis Arrêter.
3. Ouvrir Exécuter et saisir une commande inconnue.

Résultat attendu : Annuler ferme la boîte, Redémarrer relance le système, Arrêter coupe la machine, message d'erreur clair pour la commande.

Exécution du 2026-10-05 : QEMU/KVM (BIOS pour Annuler et Arrêter, UEFI pour Redémarrer), clavier injecté, captures relues.

Constat : Boîte « Éteindre l'ordinateur » : Annuler referme sans effet (bureau identique), Redémarrer relance la machine (second démarrage dans le journal), Arrêter éteint la machine (QEMU quitte, journal « power: extinction »). Exécuter avec « inconnu » affiche « Impossible de trouver « inconnu ». Vérifiez le nom, puis réessayez. ».

### MT-09 : Redémarrage ACPI

- Fonctionnalités : F20
- Objectif : Vérifier que Redémarrer relance bien la machine.
- Préconditions : Image construite.
- Durée estimée : 5 min
- Statut : Réussi

1. Choisir Redémarrer dans la boîte d'extinction.
2. Observer le redémarrage sous QEMU sans l'option -no-reboot.

Résultat attendu : La machine redémarre et affiche de nouveau l'écran de démarrage.

Exécution du 2026-10-05 : QEMU/KVM en UEFI, sans l'option -no-reboot.

Constat : Redémarrer depuis le bureau : le journal affiche « power: redémarrage » puis « RESET_REG port 0xcf9 valeur 0xf », la machine redémarre et atteint l'écran de connexion, identique à celui du premier démarrage (0 pixel différent).

### MT-10 : Horloge de la barre des tâches

- Fonctionnalités : F75
- Objectif : Vérifier l'heure affichée et sa mise à jour.
- Préconditions : Session ouverte.
- Durée estimée : 5 min
- Statut : Réussi

1. Comparer l'heure affichée avec l'heure de l'hôte.
2. Attendre un changement de minute.

Résultat attendu : Heure cohérente (UTC pour l'instant), mise à jour à la minute.

Exécution du 2026-10-05 : QEMU/KVM, capture de la zone d'horloge comparée à date -u de l'hôte.

Constat : Horloge à 16:52 pour une heure UTC de l'hôte de 16:52:54, puis 16:53 à 16:53:02 : mise à jour à la minute. L'heure affichée est l'UTC, soit deux heures de moins que l'heure locale de l'hôte (CEST).

Observations :

- Limite connue : pas de fuseau horaire.

### MT-11 : Saisie au clavier AZERTY

- Fonctionnalités : F38
- Objectif : Vérifier les caractères spéciaux, AltGr et touches mortes.
- Préconditions : Session ouverte, un champ de saisie (boîte Exécuter).
- Durée estimée : 15 min
- Statut : Réussi

1. Taper les chiffres avec Maj.
2. Taper @ # { [ | \ ^ ] } avec AltGr.
3. Taper ^e, ¨u, ~n et `a avec les touches mortes.

Résultat attendu : Caractères conformes à la disposition française ; les touches mortes ~ et ` sont à confirmer.

Exécution du 2026-10-05 : QEMU/KVM, touches envoyées par le moniteur, champ de la boîte Exécuter agrandi.

Constat : Chiffres avec Maj : 1234567890. AltGr : @ # { [ | \ ^ ] }. Touches mortes : ^ puis e donne ê, ¨ puis u donne ü, ~ puis n donne ñ, ` puis a donne à ; les quatre accents isolés s'obtiennent avec la touche morte puis espace. Touches directes é è ç à ù, AltGr+E donne €, et $ £ µ ² sont corrects. Les touches mortes ~ et ` sont confirmées.

Observations :

- Une touche morte appuyée deux fois donne un seul accent (^ puis ^ donne ^).

### MT-13 : Session entière au clavier seul

- Fonctionnalités : F70
- Objectif : Parcourir connexion, bureau et menu sans souris.
- Préconditions : Image construite.
- Durée estimée : 15 min
- Statut : Réussi

1. Valider la connexion avec Entrée (Tab déplace le focus sur « Éteindre l'ordinateur », Échap ferme la confirmation).
2. Ouvrir le menu Démarrer avec la touche Windows, lancer Bonjour.
3. Fermer la fenêtre et éteindre.

Résultat attendu : Tout est atteignable au clavier, le focus est toujours visible.

Exécution du 2026-10-05 : QEMU/KVM, clavier seul.

Constat : Connexion par Entrée (le compte a déjà le focus), menu par la touche Windows, lancement de Bonjour, fermeture par Entrée sur le bouton Fermer, extinction par le menu : QEMU quitte. Le focus est visible à chaque étape (liseré pointillé, sélection bleue).

Observations :

- Tab déplace le focus sur « Éteindre l'ordinateur », Entrée y ouvre la confirmation, Échap la ferme. L'étape 1 du cas a été corrigée en conséquence.

### MT-14 : Contraste des textes

- Fonctionnalités : F58
- Objectif : Vérifier la lisibilité des textes sur leurs arrière-plans.
- Préconditions : Session ouverte.
- Durée estimée : 10 min
- Statut : Réussi

1. Observer libellés d'icônes, horloge, invite de connexion, menu Démarrer.

Résultat attendu : Aucun texte difficile à lire (rapport de contraste d'au moins 4,5 pour le texte courant).

Exécution du 2026-10-06 : QEMU/KVM, 27 zones de texte mesurées avec tools/contraste.py sur 7 captures (connexion, bureau, menu Démarrer, sous-menu, boîtes Exécuter et Éteindre, fenêtres active et inactive).

Constat : Les 27 textes respectent 4,5 (de 4,97 à 21,00). Les trois textes en échec le 5 octobre passent le seuil : « démarrer » à 5,36 (3,57 avant), libellé d'un bouton inactif de la barre des tâches à 5,34 (3,91 avant), titre d'une fenêtre inactive à 4,97 (2,07 avant).

Observations :

- Échec le 5 octobre (P25), corrigé le 6 octobre en fonçant le fond sous le texte, puis rejoué.
- Les états survolés ne sont pas mesurés sur capture (l'outil de pilotage ne place pas le curseur sans cliquer) : ils sont couverts par tests/host/a17/test_contrast_bar.c.

Incidents : P25 (voir `../TESTS.md`).

### MT-16 : Plusieurs fenêtres dans la barre des tâches

- Fonctionnalités : F73
- Objectif : Vérifier le rétrécissement des boutons, l'activation et la réduction.
- Préconditions : Session ouverte.
- Durée estimée : 10 min
- Statut : Réussi

1. Ouvrir plusieurs fois Bonjour.
2. Cliquer les boutons de la barre des tâches.
3. Réduire et restaurer.

Résultat attendu : Boutons de largeur régulière, clic = activer ou réduire, ordre cohérent.

Exécution du 2026-10-05 : QEMU/KVM, clavier et souris injectés, captures relues.

Constat : Trois puis treize fenêtres Bonjour : un bouton par fenêtre, de largeur régulière, dans l'ordre de création. Un clic sur un bouton inactif active et restaure la fenêtre, un clic sur le bouton actif la réduit. À treize fenêtres les boutons rétrécissent à largeur égale (environ 62 px) puis s'élargissent à la fermeture.

Observations :

- Le libellé est tronqué sans points de suspension (« Bonjo »).
- Un espace vide d'environ 24 px précède le libellé.

### MT-17 : Endurance

- Fonctionnalités : F80
- Objectif : Détecter fuites de mémoire et gels sur une longue durée.
- Préconditions : Session ouverte, journal série ouvert.
- Durée estimée : 480 min
- Statut : Partiel

1. Laisser le système tourner 8 heures avec des actions répétées (ouverture et fermeture de fenêtres).
2. Relever mémoire libre et tas toutes les 30 minutes.

Résultat attendu : Mémoire libre stable, aucun gel, aucune panique.

Exécution du 2026-10-06 : QEMU/KVM, boucle de 300 ouvertures (double-clic sur l'icône Bonjour) et fermetures (Alt+F4) en 20 minutes, sur deux machines : 128 Mo et 64 Mo de RAM.

Constat : Les 300 fenêtres s'ouvrent et se ferment sur les deux machines (300 lignes « hello: fenêtre prête » chacune), aucune panique, aucun gel, aucune ligne d'erreur ni d'avertissement dans le journal. Les captures des cycles 50, 150, 250 et de la fin sont identiques hors horloge. Le 5 octobre, plus rien ne s'ouvrait après 222 fenêtres (128 Mo) ou 163 (64 Mo). L'endurance de 8 h n'a pas été jouée, d'où le statut partiel.

Observations :

- Échec le 5 octobre (P27), corrigé le 6 octobre, puis rejoué.
- La mémoire libre n'est toujours pas visible à l'écran : sa stabilité est mesurée par tests/qemu/int_endurance_fenetres.py (320 fenêtres avec 64 Mo, même valeur avant et après), pas par ce cas.
- Quand un lancement échoue, le shell n'affiche aucun message à l'utilisateur (non revérifié, aucun échec cette fois).

Incidents : P27 (voir `../TESTS.md`).

### MT-18 : Matériel réel

- Fonctionnalités : F79
- Objectif : Démarrer sur une machine physique.
- Préconditions : Clé USB écrite avec l'image, machine UEFI avec clavier PS/2 ou émulation.
- Durée estimée : 60 min
- Statut : A exécuter

1. Écrire l'image sur la clé.
2. Démarrer la machine.
3. Ouvrir la session, utiliser clavier et souris.

Résultat attendu : Démarrage complet ; sinon consigner le point d'arrêt à partir du journal série.

### MT-19 : Fluidité perçue

- Fonctionnalités : F55
- Objectif : Évaluer la latence de la souris et le déplacement d'une fenêtre.
- Préconditions : Session ouverte, KVM actif.
- Durée estimée : 10 min
- Statut : A exécuter

1. Déplacer rapidement le curseur.
2. Glisser une grande fenêtre.

Résultat attendu : Pas de saccade visible ni de retard notable.

### MT-20 : Mort du serveur de fenêtres

- Fonctionnalités : F29
- Objectif : Vérifier que init relance le serveur et la session.
- Préconditions : Image construite, option de ligne de commande init.mort-winsrv.
- Durée estimée : 15 min
- Statut : Réussi

1. Arrêter le processus winsrv.
2. Observer l'écran et le journal.

Résultat attendu : L'écran de connexion revient, le noyau ne panique pas, le journal montre la relance.

Exécution du 2026-10-06 : QEMU/KVM, BIOS et UEFI, scénario tests/qemu/int_winsrv_mort.py (session ouverte au clavier, captures comparées).

Constat : Avec init.mort-winsrv, init arrête winsrv une fois la session ouverte (« winsrv terminé (code 99) »), le relance aussitôt, logon ferme la session et revient à l'écran de connexion sur le nouveau serveur. Une seconde session s'ouvre et le bureau s'affiche avec sa barre des tâches. Aucune panique, aucun service abandonné.

Observations :

- Le cas était bloqué le 5 octobre faute de moyen d'arrêter winsrv : l'option de test a été ajoutée à init le 6 octobre.

### MT-21 : Lisibilité de l'écran d'arrêt

- Fonctionnalités : F45
- Objectif : Relire le texte de l'écran d'arrêt.
- Préconditions : Image construite.
- Durée estimée : 5 min
- Statut : Réussi

1. Démarrer avec les options selftest et fault=panic.
2. Lire l'écran.

Résultat attendu : Fond bleu, texte en français lisible, code d'arrêt et identifiant de build présents.

Exécution du 2026-10-05 : QEMU/KVM, capture relue.

Constat : Avec « selftest fault=panic » : fond bleu (0, 0, 170), texte blanc en français, code d'arrêt 0x750F6822, message « a13: panne volontaire (fault=panic) », identifiant de build de 40 caractères et consigne finale. La machine reste figée sur l'écran.

Observations :

- Avec fault=panic sans selftest, rien ne se passe : l'étape 1 du cas a été corrigée.

### MT-22 : Revue linguistique de l'interface

- Fonctionnalités : F81
- Objectif : Relire tous les textes de l'interface.
- Préconditions : Session ouverte.
- Durée estimée : 20 min
- Statut : Réussi

1. Parcourir connexion, bureau, menu, boîtes de dialogue, fenêtre Bonjour.
2. Noter fautes, espaces insécables, majuscules.

Résultat attendu : Orthographe et typographie françaises correctes.

Exécution du 2026-10-06 : relecture des textes dans les sources (une soixantaine de chaînes : shell, connexion, Bonjour, écran d'arrêt) et sur 8 captures.

Constat : Orthographe : aucune faute relevée. Typographie : espace insécable devant « : », « ! » et « ? », à l'intérieur des guillemets et devant les unités (« 1 min 59 s »), dans les sources comme à l'écran, où elle s'affiche comme une espace (aucune boîte de remplacement).

Observations :

- Échec le 5 octobre (P26), corrigé le 6 octobre, puis rejoué.
- « Exécuter... » garde trois points au lieu du caractère « … », comme l'interface d'origine.
- Les apostrophes sont droites (U+0027), pas typographiques.
- Les messages d'échec de connexion et l'écran d'arrêt n'ont été relus que dans les sources.

Incidents : P26 (voir `../TESTS.md`).

### MT-23 : FAT32 sur une image réelle

- Fonctionnalités : F43
- Objectif : Vérifier lecture, écriture et cohérence d'un volume FAT32.
- Préconditions : mtools et dosfstools installés.
- Durée estimée : 30 min
- Statut : Réussi

1. Créer une image FAT32 avec mformat -F et y copier des fichiers (mcopy, mmd).
2. La brancher en virtio-blk et démarrer avec selftest.
3. Lire la ligne d'autotest du VFS.
4. Contrôler l'image avec fsck.vfat -n.

Résultat attendu : Autotest du VFS réussi, aucune erreur de fsck.vfat.

Exécution du 2026-10-06 : QEMU/KVM, scénario tests/qemu/a12_fat32.py : image FAT32 de 64 Mo faite avec mformat, disque virtio-blk, contrôle avec fsck.vfat et mtools.

Constat : Le noyau monte l'image, l'autotest du VFS passe (lecture 8.3 et nom long, mkdir, écriture, relecture, renommage, suppression). Après l'arrêt, mtools relit le fichier écrit et fsck.vfat -n rend le code 0 : le compteur de clusters libres de FSInfo est exact.

Observations :

- Les dates écrites par le noyau sont en UTC (16:58) alors que mtools écrit l'heure locale (18:58) : FAT ne stocke pas de fuseau.
- Échec le 5 octobre (P24, compteur FSInfo décalé de 1), corrigé le 6 octobre : le scénario n'accepte plus aucune anomalie de fsck.vfat.

Incidents : P24 (voir `../TESTS.md`).

### MT-24 : Construction sur une machine propre

- Fonctionnalités : F82
- Objectif : Rejouer la procédure du README sur une machine vierge.
- Préconditions : Une distribution Linux récente.
- Durée estimée : 90 min
- Statut : Partiel

1. Installer les prérequis.
2. Lancer tools/build-toolchain.sh.
3. make limine, make all, make test-qemu.

Résultat attendu : Chaque commande réussit sans intervention.

Exécution du 2026-10-05 : clone du dépôt public GitHub dans build/manuel/clone, machine de développement.

Constat : Dépôt public cloné dans un dossier vide, puis make limine (téléchargement vérifié par empreinte, 0,9 s), make -j8 all (noyau, applis et image, 15 s) et make -j6 test-qemu : 82 scénarios sur 83 réussis. Le scénario a20_desktop a échoué : il était instable (corrigé depuis, voir T15). Le compilateur croisé déjà installé sur la machine a servi : tools/build-toolchain.sh n'a pas été rejoué ici (il l'avait été le même jour, avec succès).

Observations :

- Ce n'est pas une machine vierge : les paquets du système et le compilateur croisé étaient déjà présents.
- Le clone a été fait avant le correctif de a20_desktop, il faudra refaire le passage sur le dépôt corrigé.

Incidents : T15 (voir `../TESTS.md`).

### MT-25 : Fidélité à la maquette verre, thèmes clair et sombre

- Fonctionnalités : F61
- Objectif : Comparer à l'œil la connexion, le bureau, le lanceur, une fenêtre et le dock avec les captures de la maquette.
- Préconditions : Image construite avec les polices Instrument Sans et JetBrains Mono. Pour le thème sombre, un binaire d'essai qui appelle luna_theme_set, puisqu'aucune appli ne le fait.
- Durée estimée : 40 min
- Statut : A exécuter

1. Démarrer sous QEMU, capturer la connexion, le bureau et le lanceur à 1280x800.
2. Ouvrir la fenêtre Bonjour et glisser une fenêtre sous le dock.
3. Comparer chaque capture à celle de la maquette : verre, liseré, rayons, polices, accent.
4. Relever séparément les écarts attendus (ombres de fenêtres, flou des fenêtres sous le dock, client opaque) et les écarts inattendus.

Résultat attendu : Aucun écart inattendu. Les écarts attendus sont ceux de la section Limites connues du README.
