# Guide de lecture en cinq minutes

Ce dépôt est un système d'exploitation en C (x86-64) et la campagne de tests qui va avec. Si vous avez peu de temps,
ouvrez ces trois fichiers, dans cet ordre.

## 1. Un test (2 minutes) : `tests/host/a14/test_sys_spawnv.c`

109 lignes pour une seule fonction, `v_spawnv` (lancer un processus avec des arguments). Quatre cas, et le nom de chacun
dit la technique de test utilisée : exigence, supposition d'erreur, injection de panne, flot de données.

À regarder :
- les noms de cas, lignes 101 à 107 ;
- `spawnv_nomem` : l'allocation de pages est faussée pour refuser (`valloc_budget = 0`), le test attend `E_NOMEM`, une
  seule tentative, et aucun bloc vivant de plus ;
- `spawnv_leak` : une page allouée, une page rendue, aucun mappage restant. Le test compte les ressources, il ne se
  contente pas de regarder le code de retour.

## 2. La traçabilité (1 minute) : `tests/MATRICE.md`

121 fonctionnalités, chacune avec son mode de vérification (A automatisé, AM automatisé avec complément manuel, M manuel),
les tests qui la couvrent et les cas manuels associés. C'est de là que sort le taux de couverture fonctionnelle du README.
Le fichier est généré par `tools/matrice_couverture.py` depuis `tests/matrice.toml`, qui refuse un test cité qui
n'existe pas.

À regarder : les lignes en M et en AM, c'est-à-dire ce qui n'est pas entièrement automatisé, et pourquoi.

## 3. Un défaut documenté (2 minutes) : P27 dans `TESTS.md` (section 5.3), puis la fiche MT-17 de `tests/MANUEL.md`

Après plusieurs centaines d'ouvertures et de fermetures de fenêtres, le système cessait d'en ouvrir : pas de panique, pas
de gel, mais plus aucune fenêtre. Le cas a été trouvé par un test d'endurance manuel, et mesuré avec deux tailles de mémoire
(222 ouvertures avec 128 Mo, 163 avec 64 Mo). La cause était dans le noyau : démapper une surface partagée ne rendait
pas la référence sur sa mémoire.

À regarder : le chemin du constat au test de régression (`tests/qemu/int_endurance_fenetres.py` et l'appli
`user/apps/wstress`), écrit avant la correction.

## Ce que le dépôt ne prouve pas

- Aucun test sur du matériel réel : tout tourne sous QEMU. Trois cas manuels ne sont pas joués (matériel, fluidité perçue,
  arrêt du serveur de fenêtres).
- Le thème verre (clair et sombre) est vérifié sur des valeurs calculées depuis la maquette et sur des galeries relues,
  pas par un jeu de captures QEMU : le cas manuel MT-25 est à exécuter. Ses limites sont dans le README, section Limites
  connues.
- Le classement des fonctionnalités en A, AM ou M est un jugement, pas une mesure de couverture de code.
- L'intégration continue (`tools/ci.sh`) ne rejoue qu'un sous-ensemble : sept lots de tests hôte et cinq scénarios QEMU
  sans KVM. La suite complète se lance en local avec `make test`.
