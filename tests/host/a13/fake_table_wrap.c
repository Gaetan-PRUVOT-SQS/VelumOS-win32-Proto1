#include "cases.h"

const t_wrapcase	g_wrap_cases[] = {
{"vide", "", 10, 3, 0, false, {NULL}},
{"texte nul", NULL, 10, 3, 0, false, {NULL}},
{"court", "abc", 10, 3, 1, false, {"abc"}},
{"exactement cols", "0123456789", 10, 3, 1, false, {"0123456789"}},
{"cols+1 avec espace", "aaaa bbbbb", 9, 3, 2, false, {"aaaa", "bbbbb"}},
{"espace a la limite", "aaaaa bbbbb", 5, 3, 2, false, {"aaaaa", "bbbbb"}},
{"mot long", "abcdefghijkl", 5, 5, 3, false, {"abcde", "fghij", "kl"}},
{"retour a la ligne", "ab\ncd", 10, 3, 2, false, {"ab", "cd"}},
{"ligne vide", "ab\n\ncd", 10, 4, 3, false, {"ab", "", "cd"}},
{"retour final", "ab\n", 10, 3, 1, false, {"ab"}},
{"espace initial", " ab", 10, 3, 1, false, {" ab"}},
{"coupe a l'espace", "aaa bbb ccc", 7, 2, 2, false, {"aaa bbb", "ccc"}},
{"troncature", "aaa bbb ccc ddd eee", 7, 2, 2, true, {"aaa bbb", "ccc "}},
{"troncature apres retour", "abc\ndef", 10, 1, 1, true, {"abc"}},
{"utf8 coupe", "éééééé", 4, 3, 2, false, {"éééé", "éé"}},
{"utf8 troncature", "ééééé", 4, 1, 1, true, {"é"}},
{"colonne 1", "abc", 1, 5, 3, false, {"a", "b", "c"}},
{"colonne 1 tronquee", "abc", 1, 2, 2, true, {"a", ""}},
{"cols 0", "abc", 0, 5, 0, false, {NULL}},
{"max 0", "abc", 10, 0, 0, false, {NULL}},
{"octet invalide", "a\xff" "b", 10, 3, 1, false, {"a\xff" "b"}},
{"espaces seuls", "     ", 3, 5, 2, false, {"   ", " "}},
{NULL, NULL, 0, 0, 0, false, {NULL}}
};
