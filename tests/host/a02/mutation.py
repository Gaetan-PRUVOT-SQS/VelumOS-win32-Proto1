#!/usr/bin/env python3
import concurrent.futures
import re
import subprocess
import sys
import time
from pathlib import Path

RACINE = Path(__file__).resolve().parents[3]
SORTIE = RACINE / "build" / "a02" / "mutation"
DELAI_TEST = 90

MUTANTS = [
    ("M01", "pmm_alloc.c", "g_pmm.hint = c + n;", "g_pmm.hint = c;", "indice next-fit non avancé"),
    (
        "M02",
        "pmm_alloc.c",
        "claim(c, n, o, max == 0);",
        "claim(c, n, o, 1);",
        "indice avancé aussi avec max",
    ),
    (
        "M03",
        "pmm_alloc.c",
        "g_pmm.stats.free_pages < n",
        "g_pmm.stats.free_pages <= n",
        "préfiltre trop strict",
    ),
    ("M04", "pmm_bits.c", "if (to - from >= 64)", "if (to - from > 64)", "masque de mot plein"),
    (
        "M05",
        "pmm_bits.c",
        "word &= ~0ull << (from % 64);",
        "word &= ~0ull;",
        "départ à l'intérieur d'un mot ignoré",
    ),
    (
        "M06",
        "pmm_bits.c",
        "n += 64 - popcount(g_pmm.bits[w]);",
        "n += popcount(g_pmm.bits[w]);",
        "comptage des libres inversé",
    ),
    ("M07", "pmm_free.c", "n > g_pmm.span - f", "n > g_pmm.span", "débordement de plage non vu"),
    (
        "M08",
        "pmm_free.c",
        "if (g_pmm.owner[f + i] == PMM_FREE)",
        "if (0)",
        "double libération non vue",
    ),
    (
        "M09",
        "pmm_free.c",
        "g_pmm.stats.owned[o] -= n;",
        "g_pmm.stats.owned[o] -= 1;",
        "compteur propriétaire faux",
    ),
    (
        "M10",
        "pmm_free.c",
        "if (!is_aligned(phys, PAGE_SIZE))",
        "if (0)",
        "adresse non alignée acceptée",
    ),
    (
        "M11",
        "pmm_free.c",
        "memset(pmm_virt(f << PAGE_SHIFT), PMM_POISON, n << PAGE_SHIFT);",
        "memset(pmm_virt(f << PAGE_SHIFT), 0, n << PAGE_SHIFT);",
        "poison absent",
    ),
    ("M12", "pmm_free.c", "g_pmm.stats.free_calls++;", "", "free_calls non compté"),
    ("M13", "pmm_req.c", "q->lo = 1;", "q->lo = 0;", "frame 0 donnée"),
    ("M14", "pmm_req.c", "max <= PMM_LOW_END", "max < PMM_LOW_END", "limite 1 Mio"),
    (
        "M15",
        "pmm_req.c",
        "max >> PAGE_SHIFT",
        "(max + PAGE_SIZE - 1) >> PAGE_SHIFT",
        "max arrondi vers le haut",
    ),
    ("M16", "pmm_req.c", "(al & (al - 1))", "0", "alignement non puissance de 2 accepté"),
    ("M17", "pmm_req.c", "if (!al)\n\t\tal = 1;", "", "alignement 0 refusé ou indéfini"),
    ("M18", "pmm_req.c", "g_pmm.fail_left--;", "", "budget d'injection jamais consommé"),
    ("M19", "pmm_scan.c", "c = align_up(f, q->align);", "c = f;", "alignement ignoré"),
    ("M20", "pmm_scan.c", "min_u64(q->start, limit)", "q->start", "seconde passe sans borne"),
    (
        "M21",
        "pmm_scan.c",
        "if (u == c + q->count)",
        "if (u >= c + q->count - 1)",
        "course trop courte acceptée",
    ),
    (
        "M22",
        "pmm_plan.c",
        "s.hi - s.lo >= best_len",
        "s.hi - s.lo > best_len",
        "égalité des plus grandes plages",
    ),
    (
        "M23",
        "pmm_plan.c",
        "best_hi - (pages << PAGE_SHIFT)",
        "best_hi - (pages << PAGE_SHIFT) - PAGE_SIZE",
        "métadonnées décalées",
    ),
    (
        "M24",
        "pmm_plan.c",
        "if (plan->span > PMM_MAX_FRAMES)",
        "if (0)",
        "largeur physique non bornée",
    ),
    (
        "M25",
        "pmm_range.c",
        "out->hi = align_down(pmm_range_end(r), PAGE_SIZE);",
        "out->hi = align_up(pmm_range_end(r), PAGE_SIZE);",
        "fin arrondie vers le haut",
    ),
    (
        "M26",
        "pmm_range.c",
        "a->base < pmm_range_end(b) &&",
        "a->base <= pmm_range_end(b) &&",
        "plages jointives vues comme recouvrantes",
    ),
    (
        "M27",
        "pmm_region.c",
        "if (g_pmm.owner[i] != PMM_MARK_RESERVED)",
        "if (0)",
        "reprise de frames déjà libres",
    ),
    (
        "M28",
        "pmm_region.c",
        "&& r.lo <= s->lo && s->hi <= r.hi)",
        "&& r.lo <= s->lo)",
        "reprise hors de la plage du chargeur",
    ),
    ("M29", "pmm_init.c", "if (g_pmm.bits)\n\t\treturn (E_BUSY);", "", "double init acceptée"),
    ("M30", "pmm_init.c", "if (!g_pmm.stats.free_pages)", "if (0)", "pool vide accepté"),
    (
        "M31",
        "pmm_build.c",
        "PMM_MARK_META, g_pmm.meta_pages",
        "PMM_MARK_RESERVED, g_pmm.meta_pages",
        "métadonnées marquées reprenables",
    ),
    (
        "M32",
        "pmm_check.c",
        "bad += padding_bad();",
        "(void)padding_bad();",
        "bits de bourrage non contrôlés",
    ),
    (
        "M33",
        "pmm_misc.c",
        "memset(pmm_virt(phys), 0, PAGE_SIZE);",
        "memset(pmm_virt(phys), 0, PAGE_SIZE - 1);",
        "dernier octet non remis à zéro",
    ),
    (
        "M34",
        "pmm_lock.c",
        "g_pmm.serving, __ATOMIC_RELAXED) + 1;",
        "g_pmm.serving, __ATOMIC_RELAXED) + 2;",
        "ticket rendu de travers",
    ),
    (
        "M35",
        "pmm_misc.c",
        "*out = g_pmm.stats;\n\tpmm_unlock(flags);\n\tout->owned[PMM_FREE] = out->free_pages;",
        "*out = g_pmm.stats;\n\tpmm_unlock(flags);",
        "owned[PMM_FREE] absent",
    ),
    ("M36", "pmm_scan.c", "known = c + 1;", "known = c + 2;", "frame suivante non vérifiée"),
    (
        "M37",
        "pmm_alloc.c",
        "pmm_inject_fail() || g_pmm.stats.free_pages < n",
        "pmm_inject_fail() && g_pmm.stats.free_pages < n",
        "injection seulement si pool plein",
    ),
    (
        "M38",
        "pmm_misc.c",
        "g_pmm.fail_left = (uint64_t)n;",
        "g_pmm.fail_left = (uint64_t)n + 1;",
        "budget d'injection décalé",
    ),
    (
        "M39",
        "pmm_misc.c",
        "g_pmm.fail_armed = n >= 0;",
        "g_pmm.fail_armed = n > 0;",
        "injection n = 0 désarmée",
    ),
    (
        "M40",
        "pmm_lock.c",
        "__atomic_fetch_add(&g_pmm.ticket, 1,",
        "__atomic_fetch_add(&g_pmm.ticket, 2,",
        "tickets sautés",
    ),
    (
        "M41",
        "pmm_region.c",
        "while (i < bi->nranges && i < BOOT_MAX_RANGES)",
        "while (i < bi->nranges)",
        "lecture des plages non bornée",
    ),
]

PRIORITE = [
    "unit_bits",
    "unit_scan",
    "unit_find",
    "unit_range",
    "alloc_one",
    "alloc_pages",
    "alloc_zone",
    "alloc_args",
    "free",
    "free_faults",
    "free_faults2",
    "states",
    "fail_after",
    "region",
    "hint",
    "nextfit",
    "init_ranges",
    "init_reclaim",
    "init_errors",
    "init_limits",
    "init_layout",
    "init_misc",
    "check",
    "lock",
    "lock_spin",
    "selftest",
    "selftest_fault",
    "metamorphic",
    "budget",
    "complexity",
    "model",
    "selftest_hooks",
]


def lancer(commande, delai=None):
    return subprocess.run(
        commande, cwd=RACINE, capture_output=True, text=True, timeout=delai, check=False
    )


def variable_make(nom):
    sortie = lancer(["make", "-pn", "-q", "B=build/a02", "LOT=a02"]).stdout
    trouve = re.search(rf"^{nom} :?= (.*)$", sortie, re.MULTILINE)
    return trouve.group(1).split()


def preparer():
    SORTIE.mkdir(parents=True, exist_ok=True)
    drapeaux = variable_make("HOSTFLAGS") + variable_make("HINC_a02")
    sources_pmm = [s for s in variable_make("HSRC_a02")]
    fakes = sorted(str(p.relative_to(RACINE)) for p in (RACINE / "tests/host/a02").glob("fake_*.c"))
    tests = sorted(str(p.relative_to(RACINE)) for p in (RACINE / "tests/host/a02").glob("test_*.c"))
    communs = ["tests/host/harness.c", "tests/host/harness_eq.c"]
    return drapeaux, sources_pmm, fakes + communs, tests


def objet_de(source):
    return SORTIE / "obj" / (source.replace("/", "_") + ".o")


def compiler(drapeaux, source, sortie):
    sortie.parent.mkdir(parents=True, exist_ok=True)
    res = lancer(["gcc", *drapeaux, "-c", source, "-o", str(sortie)])
    if res.returncode:
        raise SystemExit(f"compilation de {source} impossible\n{res.stderr}")


def compiler_base(drapeaux, sources):
    with concurrent.futures.ThreadPoolExecutor(max_workers=8) as pool:
        travaux = [pool.submit(compiler, drapeaux, s, objet_de(s)) for s in sources]
        for travail in travaux:
            travail.result()


def ordre_des_tests(tests):
    def rang(chemin):
        nom = Path(chemin).stem.removeprefix("test_")
        return PRIORITE.index(nom) if nom in PRIORITE else len(PRIORITE)

    return sorted(tests, key=rang)


def executer(test, objets, drapeaux):
    binaire = SORTIE / "bin" / Path(test).stem
    binaire.parent.mkdir(parents=True, exist_ok=True)
    liaison = lancer(["gcc", *drapeaux, str(objet_de(test)), *map(str, objets), "-o", str(binaire)])
    if liaison.returncode:
        return f"édition de liens : {liaison.stderr[-200:]}"
    try:
        res = lancer([str(binaire)], DELAI_TEST)
    except subprocess.TimeoutExpired:
        return "délai dépassé"
    if res.returncode:
        return f"code {res.returncode}"
    return None


def jouer_mutant(mutant, drapeaux, sources_pmm, communs, tests):
    identifiant, fichier, avant, apres, _ = mutant
    chemin = RACINE / "kernel" / "mm" / fichier
    texte = chemin.read_text(encoding="utf-8")
    if texte.count(avant) != 1:
        raise SystemExit(f"{identifiant} : motif absent ou ambigu dans {fichier}")
    source_mutee = SORTIE / "src" / fichier
    source_mutee.parent.mkdir(parents=True, exist_ok=True)
    source_mutee.write_text(texte.replace(avant, apres), encoding="utf-8")
    objet_mute = SORTIE / "obj" / f"mutant_{identifiant}.o"
    compiler(drapeaux, str(source_mutee), objet_mute)
    base = [s for s in sources_pmm if not s.endswith("/" + fichier)]
    objets = [objet_de(s) for s in base + communs] + [objet_mute]
    for test in tests:
        verdict = executer(test, objets, drapeaux)
        if verdict:
            return Path(test).stem, verdict
    return None, None


def main():
    drapeaux, sources_pmm, communs, tests = preparer()
    compiler_base(drapeaux, sources_pmm + communs + tests)
    tests = ordre_des_tests(tests)
    survivants = 0
    debut = time.monotonic()
    for mutant in MUTANTS:
        test, verdict = jouer_mutant(mutant, drapeaux, sources_pmm, communs, tests)
        etat = "tué par " + test + " (" + verdict + ")" if test else "SURVIVANT"
        survivants += test is None
        print(f"{mutant[0]} {mutant[1]:14} {mutant[4]:50} {etat}", flush=True)
    total = len(MUTANTS)
    print(f"mutants : {total - survivants}/{total} tués en {time.monotonic() - debut:.0f} s")
    return 1 if survivants else 0


if __name__ == "__main__":
    sys.exit(main())
