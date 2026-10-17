#!/bin/sh
set -eu

if [ "$#" -lt 4 ]; then
	printf 'usage : %s CIBLE DOSSIER_BUILD "SOURCES" ENTREES...\n' "$0" >&2
	exit 2
fi
cible=$1
dossier=$2
sources=$3
shift 3
: "${CC:=gcc}"
: "${CFLAGS:=}"
: "${INCS:=}"
: "${LIBS:=}"

rm -rf -- "$dossier"
mkdir -p -- "$dossier/tmp"
TMPDIR="$(cd -- "$dossier/tmp" && pwd)"
export TMPDIR
objets=""
for src in tests/fuzz/fuzz_core.c tests/fuzz/replay_main.c \
	"tests/fuzz/fuzz_$cible.c" $sources; do
	obj="$dossier/$(basename -- "$src").o"
	# shellcheck disable=SC2086
	"$CC" $CFLAGS $INCS --coverage -fsanitize=address,undefined \
		-fno-sanitize-recover=undefined -c "$src" -o "$obj"
	objets="$objets $obj"
done
# shellcheck disable=SC2086
"$CC" --coverage -fsanitize=address,undefined $objets $LIBS -o "$dossier/replay"
"$dossier/replay" "$@" >/dev/null
printf 'couverture de lignes (gcov) pour la cible %s :\n' "$cible"
for src in $sources; do
	case "$src" in
	lib/libk/*) continue ;;
	esac
	obj="$dossier/$(basename -- "$src").o"
	gcov -n -b -o "$dossier" "$obj" 2>/dev/null | awk -v f="$src" '
		$1 == "File" { garde = ($2 == "\047" f "\047"); next }
		garde && /^Lines executed/ { l = $0; next }
		garde && /^Branches executed/ { b = $0; next }
		garde && /^Taken at least once/ { t = $0; printf "  %-40s %s ; %s\n", f, l, b; garde = 0 }
		'
done
