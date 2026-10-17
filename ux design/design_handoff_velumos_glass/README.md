# Passation : VelumOS v2, thème « verre liquide »

## Vue d'ensemble
Refonte visuelle de la session graphique de VelumOS : connexion, bureau et barre des tâches, lanceur, explorateur de fichiers, réglages, centre de notifications et console, en thème clair et sombre. L'identité est propre à VelumOS. Seules les icônes de `lib/luna/icon_*.c` sont conservées.

## À propos des fichiers
Les fichiers `.dc.html` sont des **références de conception en HTML**, pas du code à reprendre. Il s'agit de reproduire ces écrans dans le moteur de rendu existant (`lib/luna`, `gfx.h`, serveur de fenêtres `winsrv`). Pour les ouvrir : `VelumOS UX v2.dc.html` dans un navigateur (garder `support.js`, `Dock.dc.html`, `Wallpaper.dc.html` et `icons/` à côté).

`velum_theme.h` reprend toutes les valeurs de ce document au format `t_color` (0xAARRGGBB). Il est prêt à être inclus.

## Fidélité
**Haute fidélité.** Les couleurs, rayons, dimensions et typographies sont définitifs. Écran de référence : 1280 × 800.

---

## 1. Le matériau verre (cœur du thème)

Chaque surface en verre se compose de 7 couches, dessinées dans cet ordre. Tout se fait en logiciel, sans GPU.

| # | Couche | Clair | Sombre |
|---|---|---|---|
| 1 | Ombre portée (texture 9 tranches précalculée par rayon) | rgb(40,40,90), 28 %, y+30, flou 80 (fenêtres) · 22 %, y+16, flou 40 (barre) | noir 50 % / 45 % |
| 2 | Capture de l'arrière-plan sous le rectangle | — | — |
| 3 | Flou de l'arrière-plan | rayon 24 à 34 selon la surface (voir tableau 1.1) | idem |
| 4 | Saturation : `c = lum + (c − lum) × s` | s = 1,8 (1,9 panneaux) | s = 1,6 (1,7 panneaux) |
| 5 | Teinte : `lp_mix` de la couleur de remplissage | blanc 30 à 72 % | encre 42 à 78 % |
| 6 | Reflet : dégradé vertical, de α début à 0 à 32 % de la hauteur | blanc 45 % (`0x73ffffff`) | blanc 12 % (`0x1fffffff`) |
| 7 | Liseré intérieur, 1 px : haut / tour / bas | `0xf2ffffff` / `0x73ffffff` / `0x4dffffff` | `0x38ffffff` / `0x14ffffff` / — |

**Flou conseillé (CPU) :** réduire la zone d'un facteur 4 (moyenne de boîte), puis 3 passes de flou de boîte horizontal + vertical de rayon `r/4`, puis agrandir en bilinéaire (`gfx_blit_smooth`). Mettre en cache le résultat par fenêtre et ne le recalculer que si ce qui est dessous change (déplacement d'une fenêtre inférieure). Le fond d'écran étant statique, le cache est presque toujours valable.

**Repli « Réduire la transparence »** (réglage dans Apparence, conseillé sur machine modeste) : sauter les couches 2 à 4 et remplir avec une couleur opaque. Clair : `#EEF0F5` pour les panneaux, `#FBFBFC` pour le contenu. Sombre : `#1B2030` et `#141824`. Les couches 6 et 7 restent.

### 1.1 Valeurs par surface

| Surface | Remplissage clair | Remplissage sombre | Flou | Rayon |
|---|---|---|---|---|
| Barre des tâches | blanc 32 % | rgba(20,24,36,.42) | 28 | 22 |
| Lanceur | blanc 38 % | rgba(22,26,38,.50) | 34 | 30 |
| Panneaux notifications | blanc 38 % | rgba(22,26,38,.50) | 34 | 26 |
| Carte de connexion | blanc 34 % | rgba(22,26,38,.45) | 30 | 30 |
| Puces bas d'écran (connexion) | blanc 34 % | rgba(22,26,38,.45) | 24 | pilule |
| Cadre Fichiers / Réglages (barre latérale) | blanc 30 % | rgba(22,26,38,.42) | 34 | 26 |
| Zone de contenu Fichiers / Réglages | blanc 62 % (sans flou propre) | rgba(16,19,28,.60) | — | — |
| Petite fenêtre (Bonjour) | blanc 56 % | rgba(26,30,44,.62) | 30 | 24 |
| Console | rgba(14,17,26,.78) dans les deux thèmes | idem | 30, sat. 1,6 | 24 |
| Barre d'actions flottante (Fichiers) | blanc 50 % | rgba(40,46,62,.60) | 24 | 22 |

---

## 2. Couleurs

| Rôle | Clair | Sombre |
|---|---|---|
| Accent (barres de progression, radio, curseurs) | `#0F7D83` | `#2BB3B1` |
| Texte accent (liens, « Tout effacer ») | `#0F7D83` | `#4FD1CC` |
| Bouton principal | dégradé `#2BB3B1` → `#0F7D83` (haut → bas), texte blanc, liseré haut blanc 45 % | identique |
| Sélection (fichier) | fond accent 12 %, contour 1,5 px accent 45 % | `#4FD1CC` 12 % / 50 % |
| Texte principal | `#141824` | `#F1F3F8` |
| Texte secondaire | `#2C3242` | `#C9D0DC` |
| Légende / glyphes de commande | `#4A5163` | `#AAB2C3` |
| Désactivé | `#AAB2C3` | `#5D667A` |
| Danger (Supprimer) | `#B3361F` | `#FF9A7A` |
| Interrupteur éteint | encre 14 % | blanc 16 % |
| Piste de curseur | encre 10 à 12 % | blanc 10 % |

Contrastes : le texte principal reste au-dessus de 4,5:1 sur toutes les teintes de verre, à condition d'appliquer la teinte minimale indiquée. Ne pas descendre en dessous de 30 % de blanc en clair, ni de 42 % d'encre en sombre. `tools/contraste.py` peut servir à vérifier.

---

## 3. Typographie

- **Interface :** Instrument Sans (licence OFL, compatible avec `LICENSES/OFL-1.1.txt`). Rastériser avec `tools/genfont.py` aux tailles 11, 12, 13, 14, 15, 22, 24, 26, 28 et 120 px, en graisses 400, 600 et 700. La maquette utilise 12,5 et 13,5 px : arrondir à 13 et 14.
- **Console :** JetBrains Mono (OFL), 14 px, interligne 22 px (1,6), graisse 400. Prompt en 600.

| Usage | Taille / graisse | Interlettrage |
|---|---|---|
| Horloge de connexion | 120 / 600 | −0,045 em |
| Titre de page (Réglages) | 28 / 700 | −0,01 em |
| Titre de dossier (Fichiers) | 26 / 700 | −0,01 em |
| Titre de fenêtre Bonjour | 24 / 700 | −0,01 em |
| Nom d'utilisateur (connexion), date | 22 / 600 et 22 / 400 | 0 |
| Champ de recherche du lanceur | 15 / 400 | 0 |
| Titre de fenêtre (barre), éléments de liste | 14 / 600 et 14 / 400 | 0 |
| Libellés de tuiles, boutons, barre des tâches | 13 / 600 | 0 |
| Légendes, horodatages | 12 / 400 | 0 |
| Date de la zone de notification | 11 / 400 | 0 |

---

## 4. Écrans

Coordonnées en px sur un écran de 1280 × 800, origine en haut à gauche.

### 4.1 Barre des tâches (tous les écrans de session)
- Rectangle : x = 12, largeur = écran − 24, bas à 12 px du bord, hauteur 56, rayon 22. Remplissage horizontal : marge intérieure 8, espacement 8.
- **Orbe du lanceur :** disque de 42 (verre blanc 55 %, liseré haut blanc), logo `ICON_LOGO` de 32 centré. Lanceur ouvert : anneau de 2 px accent à 3 px de l'orbe.
- **Boutons de tâche :** hauteur 40, rayon 14, marges 9 à gauche et 14 à droite, icône 24, espacement 9, texte 13 / 600. Inactif : blanc 22 % (sombre : blanc 5 %). Actif : blanc 70 % avec ombre y+2 flou 8 (sombre : blanc 16 %), et indicateur de 14 × 3 px en accent, centré, à 3 px du bas.
- **Zone de notification :** hauteur 42, rayon 16, blanc 35 % (sombre : blanc 7 %), marges 14 à gauche et 6 à droite, espacement 14. Elle contient : « FR » 12 / 600 ; trois témoins d'état (deux disques de 7, une batterie de 14 × 8 au trait de 1,5) ; l'heure 13 / 700 sur la date 11 / 400, alignées à droite ; une pastille de 30 × 30, rayon 11, avec le nombre de notifications.

### 4.2 Connexion
- Fond : fond d'écran « voile » (§5), sans barre des tâches.
- Horloge centrée, haut du texte à y = 100. Date juste dessous (22 px, `#2C3242`).
- Carte : largeur 380, centrée, 52 px sous la date. Marges intérieures 28 (haut) et 24 (côtés, bas), rayon 30, espacement vertical 16. Elle contient :
  - l'avatar : carré de 88, rayon 26, verre blanc 60 %, avec `ICON_USER` de 68 ;
  - le nom (22 / 600) ;
  - le champ mot de passe : hauteur 48, pilule, blanc 72 %, marge gauche 18, points 20 px espacés de 5, et un bouton rond de 38 avec dégradé accent et « → » blanc ;
  - le lien « Mot de passe oublié ? » (13 px).
- Bas d'écran, à 28 px des bords : à gauche, les puces « Changer d'utilisateur » (`ICON_USER` 32) et « FR » ; à droite, la puce « Éteindre » (`ICON_POWER` 32). Puces de hauteur 44, pilule, 14 / 600.

### 4.3 Bureau
- Icônes du bureau : colonne à (24, 24), espacement vertical 18, cellule de 90 de large. Icône de 56 avec ombre y+4 flou 8 (rgba(40,40,90,.25)). Libellé 12,5 / 600 dans une étiquette blanc 50 % (sombre : encre 50 %), marges 2 × 8, rayon 8.
- Ordre des icônes : Ordinateur (`ICON_COMPUTER`), Documents (`ICON_FOLDER`), Corbeille (`ICON_RECYCLE`), Bonjour (`ICON_PROGRAM`).
- Fenêtre Bonjour : 480 de large, à (330, 160), verre « petite fenêtre ».

### 4.4 Barre de titre (toutes les fenêtres)
- Hauteur 50. Marge gauche 14 à 16, marge droite 10. Icône de l'application 22, espacement 10, titre 14 / 600.
- **Capsule de commandes** à droite : marge intérieure 3, espacement 2, rayon 12, blanc 55 % avec liseré haut blanc (sombre : blanc 8 %). Elle contient trois boutons de 28 × 24, rayon 9 :
  - réduire : barre de 10 × 2 ;
  - agrandir : carré de 9 × 9 au trait de 1,5, rayon 3 ;
  - fermer : « × » de 17 px.
  
  Glyphes en `#4A5163` (sombre : `#AAB2C3`).
- Pas de bordure visible. La zone de redimensionnement est invisible, 6 px autour de la fenêtre. Le test de zone (`luna_hit_test`) reste inchangé.

### 4.5 Lanceur
- Panneau de 540 de large, x = 12, bas à 80 px du bord (soit 12 px au-dessus de la barre). Marges intérieures 18, espacement 18, rayon 30.
- Recherche : hauteur 48, pilule, blanc 72 %, `ICON_SEARCH` de 26, texte « Rechercher une app, un fichier, un réglage » en 15 px `#4A5163`.
- « Épinglées » (13 / 700), avec à droite « Toutes les apps › » (13 / 600, accent). Grille de 4 colonnes, espacement 6. Tuile : marges 14 × 4, rayon 20, icône 44, libellé 13 / 600. Tuile survolée ou focalisée : blanc 62 %, liseré haut blanc, ombre y+4 flou 12 à 8 %.
- Apps : Fichiers (dossier), Console (`ICON_RUN`), Réglages, Bonjour (`ICON_PROGRAM`), Ordinateur, Disques (`ICON_DRIVE`), Corbeille, Aide.
- « Récents » : lignes avec marges 8 × 10, rayon 14, `ICON_DOCUMENT` de 28, nom en 14 px, date à droite en 12,5 px.
- Pied, séparé par 1 px blanc 60 % : `ICON_USER` de 34, nom 14 / 600, bouton « Fermer la session » (hauteur 36, pilule), bouton rond de 36 avec `ICON_POWER` de 26.

### 4.6 Fichiers
- Fenêtre de 1100 × 650 à (90, 36), rayon 26. Barre latérale de 240 (verre du cadre) ; contenu en blanc 62 % séparé par 1 px blanc 70 % sur sa gauche.
- **Barre latérale :** marges 16 × 12, espacement 18. Titre de l'app (icône 22, 14 / 700). Section « Favoris » (12 / 600 `#4A5163`) avec des lignes de marges 8 × 10, rayon 12, icône 20, texte 14 px ; la ligne active est en blanc 70 %, liseré blanc, ombre y+2 flou 8, texte 600. Section « Disques » : `ICON_DRIVE` de 26, nom et système de fichiers en 13 px, jauge de 6 px (rayon 3, piste encre 10 %, remplissage accent), légende 11,5 px.
- **Barre d'outils** (marges 10, 10, 10, 16 ; espacement 10) :
  - capsule précédent / suivant : boutons de 32 × 30, rayon 11, dans un conteneur encre 5 % de rayon 14 ; bouton actif blanc avec ombre ;
  - fil d'Ariane : hauteur 36, pilule, encre 5 %, séparateur « / » en `#AAB2C3`, dernier élément en 600 ;
  - recherche : 200 × 36 ;
  - capsule de commandes.
- Titre « Documents » (26 / 700) avec le nombre d'éléments à droite. Grille de 6 colonnes, rangées de 124, espacement 8, marges 12 × 20. Élément : rayon 18, icône 60, nom 13 px.
- **Barre d'actions flottante** (visible si au moins 1 élément est sélectionné) : centrée, à 18 px du bas du contenu, pilule, marge intérieure 5. Elle contient « 1 sélectionné » en accent, puis les boutons Ouvrir, Renommer, Copier vers… et Supprimer (hauteur 32, marges 0 × 14). « Supprimer » est en couleur danger, sur blanc 70 %.

### 4.7 Réglages (Apparence)
- Fenêtre de 980 × 650 à (150, 36). Barre latérale de 230. Contenu : marges 18 × 32, espacement 22.
- Barre latérale : recherche (hauteur 36, pilule), puis les catégories Système, Apparence (active), Stockage, Utilisateurs et, en bas, « À propos de VelumOS ».
- Titre 28 / 700 et sous-titre 14 px.
- **Thème :** 3 cartes (Clair, Sombre, Selon l'heure), espacement 14. Carte : marge 8, rayon 18, contour 1 px encre 12 %, ou 2 px accent si sélectionnée. Vignette de 96 de haut, rayon 12. Radio de 14 : contour 4 px accent si coché, 1,5 px `#8A93A8` sinon.
- **Groupe de réglages :** rayon 18, blanc 70 %, lignes avec marges 14 × 18 séparées par 1 px encre 6 % (retrait de 18 px). Lignes :
  - Intensité du verre : curseur de 220 px (piste 6, bouton de 22) ;
  - Teinte du verre : sélecteur Neutre / Teintée / Givrée (boutons de hauteur 30, rayon 11, dans un conteneur de rayon 14) ;
  - Couleur d'accent : 5 pastilles de 26, sélection par anneau 2 px de fond + 2 px de couleur. Palette : `#0F7D83` `#3D6FD9` `#7A5AD6` `#C4643A` `#4A5163` ;
  - Réduire la transparence : interrupteur de 46 × 28, bouton de 22.

### 4.8 Centre de notifications
- Colonne de 380 de large, à 12 px du haut et de la droite, bas à 80 px du bord. Deux panneaux espacés de 12, marges 14, rayon 26.
- **Panneau 1 :** grille 2 × 2 de tuiles rapides (marges 12 × 14, rayon 18, titre 13,5 / 600, état 12 px). Tuile active en dégradé accent, texte blanc ; inactive en blanc 60 %. Dessous, curseurs Volume et Luminosité (libellé de 72 px, piste 6, bouton de 20).
- **Panneau 2 :** en-tête « Notifications » (15 / 700) avec « Tout effacer » en accent. Cartes : marge 12, rayon 18, blanc 70 %, icône 34, titre 13,5 / 600, horodatage 12 px à droite, texte 13 px avec interligne 1,4. Boutons d'action : hauteur 30, pilule.

### 4.9 Console
- Fenêtre de 880 × 560 à (200, 70), verre console. Les onglets remplacent le titre :
  - onglet actif : hauteur 32, pilule, blanc 12 %, `ICON_RUN` de 20 ;
  - onglets inactifs en `#AAB2C3` ;
  - bouton « + » de 32.
- Corps : marges 14 × 24. Couleurs : texte `#D6DCE7` ; utilisateur `#4FD1CC` ; « @velum » `#AAB2C3` ; chemin `#A99CF0` ; dossiers `#F3CD68` ; exécutables `#7DE673` ; curseur bloc de 9 × 17 en `#4FD1CC`.
- Barre d'état de 32 de haut, 12 px `#8A93A8`, filet haut 1 px blanc 6 %.

---

## 5. Fond d'écran « voile » (remplace `luna_wallpaper`)

Une couleur de base, quatre taches radiales à décroissance douce (lissage de α de 1 au centre à 0 à 62 % du rayon), puis deux ellipses blanches floutées. Le rendu est peu coûteux et se calcule une fois au démarrage.

| Couche | Centre (fraction de l'écran) | Rayon (fraction L × H) | Clair | Sombre |
|---|---|---|---|---|
| Base | — | — | `#E3E6F0` | `#0D1120` |
| Tache 1 | (0,12 ; 0,18) | 0,55 × 0,75 | `#8FD6D2` | `#1F6F72` |
| Tache 2 | (0,88 ; 0,12) | 0,50 × 0,70 | `#BCAAF2` | `#4B3C8F` |
| Tache 3 | (0,72 ; 0,95) | 0,65 × 0,70 | `#F6C890` | `#8A5A2A` |
| Tache 4 | (0,18 ; 1,00) | 0,55 × 0,60 | `#EEA3A6` (s'efface à 60 %) | `#7A3640` |
| Voile A | ellipse de 0,8 L × 0,38 H, à x = −0,1 L et y = 0,30 H, tournée de −12°, flou 40 | blanc 75 % → 0 (de haut en bas) | blanc 14 % → 0 |
| Voile B | ellipse de 0,7 L × 0,30 H, à droite −0,15 L et y = 0,08 H, tournée de +18°, flou 50 | 0 → blanc 55 % | 0 → blanc 10 % |

---

## 6. Correspondance avec `t_lunametrics` / `t_lunadetail`

| Champ | Actuel | Proposé |
|---|---|---|
| caption_h | 30 | 50 |
| frame_w / frame_bottom | 4 / 4 | 0 / 0 (zone de redimensionnement invisible de 6) |
| btn_w / btn_h | 21 / 21 | 28 / 24 (dans une capsule avec marge 3) |
| taskbar_h | 30 | 80 réservés (56 de barre + 2 × 12 de marge) |
| start_w | 100 | 42 (orbe) |
| menu_item_h | 22 | 36 (lignes de liste), 44 (récents) |
| icon_small / icon_large | 16 / 32 | 20 à 24 / 56 |
| title_radius (detail) | 8 | 24 (26 pour les grandes fenêtres) |
| avatar (detail) | 48 | 88 à la connexion, 34 dans le lanceur |

Les écrans ont été conçus pour 1280 × 800. En 1024 × 768 (QEMU par défaut), garder les mêmes métriques ; seules les fenêtres sont positionnées autrement.

---

## 7. Interactions et états
- **Survol** d'un bouton ou d'un élément de liste : fond blanc 40 % en clair, blanc 8 % en sombre. **Appui :** blanc 70 % en clair, 14 % en sombre.
- **Focus clavier :** anneau de 2 px accent, décalé de 2 px.
- **Fermer :** au survol, le « × » passe en couleur danger. Pas de fond rouge.
- **Fenêtre inactive :** même verre, reflet réduit de moitié, texte du titre en `#4A5163`.
- **Ouverture du lanceur et des notifications :** fondu + translation de 8 px en 160 ms, avec ralenti en fin de mouvement. Sur machine modeste, apparition sans animation.
- **Barre d'actions de Fichiers :** apparaît quand au moins 1 élément est sélectionné et disparaît à la désélection.

## 8. Ressources
- `icons/*.svg` : conversion exacte des tracés de `lib/luna/icon_a.c`, `icon_b.c` et `icon_c.c` (grille 64 × 64, dégradés verticaux entre c0 et c1). Le moteur dessine déjà ces icônes avec `luna_icon` : aucun changement côté C, les SVG servent uniquement de référence.
- Polices : Instrument Sans et JetBrains Mono (Google Fonts, licence OFL).

## 9. Fichiers
- `VelumOS UX v2.dc.html` : les 14 écrans (2a à 2i, 2j à 2n pour les versions sombres).
- `Dock.dc.html` : barre des tâches (clair et sombre).
- `Wallpaper.dc.html` : fond d'écran « voile ».
- `support.js` : moteur d'affichage des maquettes.
- `velum_theme.h` : jetons au format `t_color`.
- `captures/` : 14 PNG en 1280 × 800 (01 à 07 en clair, 08 à 14 en sombre). Le flou d'arrière-plan n'apparaît pas toujours dans ces captures : se fier au §1 pour le rendu du verre.
