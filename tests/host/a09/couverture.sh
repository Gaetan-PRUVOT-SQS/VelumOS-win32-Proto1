#!/bin/sh
set -eu

racine=$(cd "$(dirname "$0")/../../.." && pwd)
cd "$racine"
sortie=build/a09/cov
rm -rf "$sortie"
mkdir -p "$sortie/obj" "$sortie/bin"

renommages="-Dstrlen=vk_strlen -Dstrnlen=vk_strnlen -Dstrcmp=vk_strcmp
	-Dstrncmp=vk_strncmp -Dstrchr=vk_strchr -Dstrrchr=vk_strrchr
	-Dstrstr=vk_strstr -Dstrlcpy=vk_strlcpy -Dstrlcat=vk_strlcat
	-Dmemcmp=vk_memcmp -Dmemchr=vk_memchr"
base="-std=gnu11 -O0 -g -fsanitize=address,undefined -fno-sanitize-recover=undefined
	-Wall -Wextra -Werror -Iinclude -Ilib/libk -Itests/host -DVELUM_HOST
	-Itests/host/a09 -Ilib/crypto -Ikernel/random -Idrivers/pci $renommages"
couverture="--coverage -fprofile-abs-path -fcondition-coverage"

produit="lib/crypto/*.c kernel/random/*.c drivers/pci/*.c kernel/bootinfo.c"
autres="lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c tests/host/harness.c
	tests/host/harness_eq.c tests/host/a09/fake_*.c"

objets=""
for src in $produit; do
	case "$src" in
	kernel/random/rng_plat.c | drivers/pci/pci_legacy.c) continue ;;
	esac
	obj="$sortie/obj/$(printf '%s' "$src" | tr '/' '_').o"
	# shellcheck disable=SC2086
	gcc $base $couverture -c "$src" -o "$obj"
	objets="$objets $obj"
done
for src in $autres; do
	obj="$sortie/obj/$(printf '%s' "$src" | tr '/' '_').o"
	# shellcheck disable=SC2086
	gcc $base -c "$src" -o "$obj"
	objets="$objets $obj"
done

for test in tests/host/a09/test_*.c; do
	bin="$sortie/bin/$(basename "$test" .c)"
	# shellcheck disable=SC2086
	gcc $base --coverage "$test" $objets -o "$bin"
	"$bin" >/dev/null
done

liste=$(for f in $produit; do printf '%s\n' "$f"; done)
cd "$sortie/obj"
for src in $liste; do
	case "$src" in
	kernel/random/rng_plat.c | drivers/pci/pci_legacy.c) continue ;;
	esac
	obj="$(printf '%s' "$src" | tr '/' '_').o"
	LC_ALL=C gcov -b -g -o . "$obj" 2>/dev/null |
		awk -v f="$src" '
			/^File/ { cur = ($0 ~ f) }
			cur && /^Lines executed/ { l = $0 }
			cur && /^Branches executed/ { b = $0 }
			cur && /^Taken at least once/ { t = $0 }
			cur && /^Condition outcomes covered/ { c = $0 }
			END { printf "%s | %s | %s | %s | %s\n", f, l, b, t, c }'
done
