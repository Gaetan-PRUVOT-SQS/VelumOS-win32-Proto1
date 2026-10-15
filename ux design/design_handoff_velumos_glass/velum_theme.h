/* VelumOS — thème « verre liquide » v2
 * Valeurs extraites des maquettes HTML (VelumOS UX v2.dc.html).
 * Couleurs au format t_color de gfx.h : 0xAARRGGBB.
 * Référence d'écran : 1280 x 800. Toutes les dimensions sont en pixels.
 */
#ifndef VELUM_THEME_H
# define VELUM_THEME_H

/* ---------- Accent ---------- */
# define VT_ACCENT              0xff0f7d83  /* lagune : barres, sélection, radio (clair) */
# define VT_ACCENT_HI           0xff2bb3b1  /* haut du dégradé bouton ; accent barres (sombre) */
# define VT_ACCENT_TEXT_DARK    0xff4fd1cc  /* texte/liens accent en thème sombre */
# define VT_ACCENT_SEL_BG       0x1f0f7d83  /* fond élément sélectionné (12 %) */
# define VT_ACCENT_SEL_RING     0x730f7d83  /* contour élément sélectionné (45 %), 1.5 px */
# define VT_ACCENT_SEL_BG_D     0x1f4fd1cc
# define VT_ACCENT_SEL_RING_D   0x804fd1cc

/* ---------- Texte ---------- */
# define VT_INK                 0xff141824  /* clair : texte principal */
# define VT_INK_2               0xff2c3242  /* clair : texte secondaire */
# define VT_MUTED               0xff4a5163  /* clair : légendes, icônes de commande */
# define VT_DIM                 0xffaab2c3  /* clair : désactivé */
# define VT_INK_D               0xfff1f3f8  /* sombre : texte principal */
# define VT_INK_2_D             0xffc9d0dc
# define VT_MUTED_D             0xffaab2c3
# define VT_DIM_D               0xff5d667a
# define VT_DANGER              0xffb3361f
# define VT_DANGER_D            0xffff9a7a

/* ---------- Verre : remplissage (teinte posée sur l'arrière-plan flouté) ---------- */
/* clair */
# define VT_GLASS_FRAME         0x4dffffff  /* 30 % : cadre Fichiers / Réglages, barre latérale */
# define VT_GLASS_DOCK          0x52ffffff  /* 32 % : barre des tâches */
# define VT_GLASS_CHIP          0x57ffffff  /* 34 % : carte connexion, puces bas d'écran */
# define VT_GLASS_PANEL         0x61ffffff  /* 38 % : lanceur, notifications */
# define VT_GLASS_WINDOW        0x8fffffff  /* 56 % : petite fenêtre (Bonjour) */
# define VT_GLASS_CONTENT       0x9effffff  /* 62 % : zone de contenu des fenêtres */
# define VT_GLASS_CONTROL       0x8cffffff  /* 55 % : capsule de commandes, boutons secondaires */
# define VT_GLASS_ITEM_SEL      0xb3ffffff  /* 70 % : élément de liste actif, cartes */
# define VT_GLASS_INPUT         0xb8ffffff  /* 72 % : champs de saisie */
/* sombre */
# define VT_GLASS_FRAME_D       0x6b161a26  /* 42 % */
# define VT_GLASS_DOCK_D        0x6b141824  /* 42 % */
# define VT_GLASS_CHIP_D        0x73161a26  /* 45 % */
# define VT_GLASS_PANEL_D       0x80161a26  /* 50 % */
# define VT_GLASS_WINDOW_D      0x9e1a1e2c  /* 62 % */
# define VT_GLASS_CONTENT_D     0x9910131c  /* 60 % */
# define VT_GLASS_CONSOLE       0xc70e111a  /* 78 % : console, clair comme sombre */
# define VT_GLASS_CONTROL_D     0x14ffffff  /* 8 % */
# define VT_GLASS_ITEM_SEL_D    0x1fffffff  /* 12 % */
# define VT_GLASS_INPUT_D       0x14ffffff  /* 8 % */

/* ---------- Verre : flou et saturation ---------- */
# define VT_BLUR_CHIP           24  /* rayon gaussien équivalent */
# define VT_BLUR_DOCK           28
# define VT_BLUR_WINDOW         30
# define VT_BLUR_PANEL          34
# define VT_SAT_LIGHT_X10       18  /* saturation x1.8 (x1.9 pour panneaux) */
# define VT_SAT_DARK_X10        16  /* saturation x1.6 (x1.7 pour panneaux) */

/* ---------- Verre : liseré et reflet ---------- */
# define VT_RIM_TOP             0xf2ffffff  /* 1 px intérieur, bord haut (clair) */
# define VT_RIM_IN              0x73ffffff  /* 1 px intérieur, tout le tour (clair) */
# define VT_RIM_BOTTOM          0x4dffffff  /* 1 px intérieur, bord bas (clair) */
# define VT_RIM_TOP_D           0x38ffffff
# define VT_RIM_IN_D            0x14ffffff
# define VT_SPEC_TOP            0x73ffffff  /* reflet : début du dégradé (clair) */
# define VT_SPEC_TOP_D          0x1fffffff  /* reflet : début du dégradé (sombre) */
# define VT_SPEC_END_PCT        32          /* reflet : transparent à 32 % de la hauteur */

/* ---------- Ombres portées ---------- */
# define VT_SHADOW_WIN          0x47282850  /* 28 % rgb(40,40,90), y+30, flou 80 */
# define VT_SHADOW_DOCK         0x38282850  /* 22 %, y+16, flou 40 */
# define VT_SHADOW_WIN_D        0x80000000  /* 50 %, y+30, flou 80 */
# define VT_SHADOW_DOCK_D       0x73000000  /* 45 %, y+16, flou 40 */

/* ---------- Repli « Réduire la transparence » (opaque) ---------- */
# define VT_OPAQUE_PANEL        0xffeef0f5
# define VT_OPAQUE_CONTENT      0xfffbfbfc
# define VT_OPAQUE_PANEL_D      0xff1b2030
# define VT_OPAQUE_CONTENT_D    0xff141824

/* ---------- Rayons ---------- */
# define VT_R_WINDOW            24  /* Bonjour, Console */
# define VT_R_WINDOW_LG         26  /* Fichiers, Réglages, panneaux notifications */
# define VT_R_DOCK              22
# define VT_R_PANEL             30  /* lanceur, carte connexion */
# define VT_R_TILE              20  /* tuiles du lanceur */
# define VT_R_CARD              18  /* fichiers, cartes notif, groupes réglages, tuiles rapides */
# define VT_R_TRAY              16
# define VT_R_TASK              14
# define VT_R_ITEM              12  /* lignes barre latérale, capsule de commandes */
# define VT_R_CAPBTN            9
# define VT_R_PILL(h)           ((h) / 2)  /* champs, boutons, puces : toujours pilule */

/* ---------- Barre des tâches ---------- */
# define VT_DOCK_MARGIN         12
# define VT_DOCK_H              56
# define VT_DOCK_PAD_X          8
# define VT_DOCK_GAP            8
# define VT_LAUNCHER_D          42  /* orbe ; logo 32 */
# define VT_TASK_H              40
# define VT_TASK_ICON           24
# define VT_TASK_PAD_L          9
# define VT_TASK_PAD_R          14
# define VT_TASK_IND_W          14  /* indicateur actif 14 x 3, à 3 px du bas */
# define VT_TRAY_H              42
# define VT_BADGE               30

/* ---------- Fenêtres ---------- */
# define VT_CAPTION_H           50
# define VT_CAPBTN_W            28
# define VT_CAPBTN_H            24
# define VT_CAPGROUP_PAD        3
# define VT_CAPGROUP_GAP        2
# define VT_CAPTION_ICON        22
# define VT_RESIZE_ZONE         6   /* zone de redimensionnement invisible (pas de bordure) */
# define VT_SIDEBAR_W           240 /* Fichiers ; 230 pour Réglages */

#endif
