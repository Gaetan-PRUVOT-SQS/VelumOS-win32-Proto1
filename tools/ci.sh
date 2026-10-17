#!/bin/sh
set -eu

racine=$(CDPATH='' cd -- "$(dirname -- "$0")/.." && pwd -P)
cd -- "$racine"

sortie="${B:-build/main}"
taches=$(nproc 2>/dev/null || echo 2)
lots_hote="a02 a03 a04 a08 a09 a11 a13 d00 d07 d11"
scenarios="a01_boot skel_boot int_session a20_desktop a12_fat32 int_apk_installation"
scenarios_release="a01_boot skel_boot"

etape_nom=''
etape_debut=0
debut_total=$(date +%s)

usage()
{
	printf 'usage : %s [statique|construction]\n' "$0" >&2
	printf '  statique      contrôles statiques, sans rien compiler\n' >&2
	printf '  construction  tests des outils, compilation, tests hôte, QEMU\n' >&2
	printf '  (sans argument : les deux, dans cet ordre)\n' >&2
}

clore_etape()
{
	[ -n "$etape_nom" ] || return 0
	printf '   durée : %ss\n' "$(($(date +%s) - etape_debut))" >&2
	etape_nom=''
}

etape()
{
	clore_etape
	printf '\n== %s\n' "$1" >&2
	etape_nom=$1
	etape_debut=$(date +%s)
}

bilan()
{
	statut=$?
	trap - EXIT
	if [ "$statut" -ne 0 ]; then
		printf 'ci.sh : échec à l'\''étape « %s » (code %s) après %ss\n' \
			"${etape_nom:-initialisation}" "$statut" \
			"$(($(date +%s) - debut_total))" >&2
	fi
	exit "$statut"
}

mode="${1:-tout}"
case "$mode" in
tout | statique | construction) ;;
*)
	usage
	exit 2
	;;
esac

trap bilan EXIT
trap 'exit 1' HUP INT TERM

exiger_outil()
{
	if command -v "$1" >/dev/null 2>&1; then
		return 0
	fi
	if [ "${CI:-}" = "true" ]; then
		printf 'ci.sh : outil requis absent : %s\n' "$1" >&2
		exit 1
	fi
	printf 'ci.sh : %s absent, contrôle ignoré hors intégration continue\n' "$1" >&2
	return 1
}

sur_scripts_shell()
{
	find . \( -path ./build -o -path ./.git \) -prune -o \
		-type f -name '*.sh' -exec "$@" {} +
}

depot_git()
{
	[ "$(git rev-parse --show-toplevel 2>/dev/null || true)" = "$racine" ]
}

controle_ruff()
{
	exiger_outil ruff || return 0
	ruff check .
}

controle_shellcheck()
{
	exiger_outil shellcheck || return 0
	sur_scripts_shell shellcheck
}

controle_shfmt()
{
	exiger_outil shfmt || return 0
	sur_scripts_shell shfmt -fn -d
}

controle_reuse()
{
	exiger_outil reuse || return 0
	if depot_git; then
		reuse lint -l
		return 0
	fi
	find . \( -path ./build -o -path ./.git -o -path ./.ruff_cache \
		-o -path ./.pytest_cache -o -name __pycache__ \) -prune -o \
		-type f ! -name '*.pyc' ! -name '*.log' -exec reuse lint-file {} +
}

versions_genfont_attendues()
{
	grep -E '^(python|pillow|freetype) ' lib/font/gen/MANIFEST
}

versions_genfont_courantes()
{
	python3 -c 'import sys
sys.path.insert(0, "tools")
import genfont
for nom, version in genfont.tool_versions().items():
    print(nom, version)' 2>/dev/null
}

controle_genfont()
{
	if ! make -n B="$sortie" genfont-check >/dev/null 2>&1; then
		printf 'ci.sh : cible genfont-check absente, contrôle ignoré\n' >&2
		return 0
	fi
	if [ "$(versions_genfont_attendues)" != "$(versions_genfont_courantes)" ]; then
		printf 'ci.sh : genfont-check ignoré, chaîne de génération différente du MANIFEST\n' >&2
		return 0
	fi
	make CROSS= B="$sortie" genfont-check
}

lancer_scenario()
{
	profil="${2:-debug}"
	python3 tools/run_scenarios.py --kernel "$sortie/$profil/kernel.elf" \
		--root "$sortie/$profil/root" --out "$sortie/qemu" --filter "$1" --jobs 2
}

executer_statique()
{
	etape "Python : ruff check"
	controle_ruff
	etape "Shell : shellcheck"
	controle_shellcheck
	etape "Shell : shfmt -fn -d"
	controle_shfmt
	etape "Licences : reuse lint"
	controle_reuse
	etape "En-têtes autonomes : make hdrcheck"
	make CROSS= B="$sortie" hdrcheck
	etape "Matrice de couverture à jour"
	python3 tools/matrice_couverture.py --check
	etape "Contrôle statique de sécurité"
	python3 tools/scan_securite.py
	etape "Polices générées : make genfont-check"
	controle_genfont
}

executer_construction()
{
	etape "Tests des outils Python"
	python3 -m pytest -q tests/tools

	etape "Limine"
	make limine

	etape "Compilation avec le gcc du système"
	make CROSS= B="$sortie" -j"$taches" all

	etape "Tests unitaires hôte"
	set --
	for lot in $lots_hote; do
		set -- "$@" "host-$lot"
	done
	make CROSS= B="$sortie" -j"$taches" "$@"

	etape "Scénarios QEMU de contrôle, sans KVM"
	VTEST_SANS_KVM=1
	export VTEST_SANS_KVM
	for scenario in $scenarios; do
		lancer_scenario "$scenario"
	done

	etape "Profil release : compilation et démarrage QEMU"
	make CROSS= B="$sortie" PROFIL=release -j"$taches" all
	for scenario in $scenarios_release; do
		lancer_scenario "$scenario" release
	done
}

case "$mode" in
tout)
	executer_statique
	executer_construction
	;;
statique)
	executer_statique
	;;
construction)
	executer_construction
	;;
esac

clore_etape
printf '\n== Terminé en %ss\n' "$(($(date +%s) - debut_total))" >&2
