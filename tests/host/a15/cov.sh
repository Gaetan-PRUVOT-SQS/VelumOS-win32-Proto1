#!/bin/sh
set -eu

racine=$(cd "$(dirname "$0")/../../.." && pwd)
cd "$racine"
sortie=${1:-build/a15/cov}

cc_plain()
{
	gcc -std=gnu11 -O0 -g -Wall -Wextra -Werror -Iinclude -Ilib/libk \
		-Ilib/gfx -Itests/host -Itests/host/a15 -DVELUM_HOST \
		-Dstrlen=vk_strlen -Dstrnlen=vk_strnlen -Dstrcmp=vk_strcmp \
		-Dstrncmp=vk_strncmp -Dstrchr=vk_strchr -Dstrrchr=vk_strrchr \
		-Dstrstr=vk_strstr -Dstrlcpy=vk_strlcpy -Dstrlcat=vk_strlcat \
		-Dmemcmp=vk_memcmp -Dmemchr=vk_memchr "$@"
}

nom_objet()
{
	basename "${1%.c}"
}

rm -rf -- "$sortie"
mkdir -p -- "$sortie/cov" "$sortie/sup" "$sortie/tst" "$sortie/bin"

for src in lib/gfx/*.c; do
	cc_plain --coverage -fcondition-coverage -c "$src" \
		-o "$sortie/cov/$(nom_objet "$src").o"
done
for src in tests/host/a15/fake_*.c tests/host/a15/ppm.c \
	tests/host/harness.c tests/host/harness_eq.c lib/libk/str.c \
	lib/libk/str2.c lib/libk/memcmp.c; do
	cc_plain -c "$src" -o "$sortie/sup/$(nom_objet "$src").o"
done
for src in tests/host/a15/test_*.c; do
	nom=$(nom_objet "$src")
	cc_plain -c "$src" -o "$sortie/tst/$nom.o"
	gcc --coverage "$sortie/tst/$nom.o" "$sortie"/sup/*.o "$sortie"/cov/*.o \
		-o "$sortie/bin/$nom"
	printf 'cov %s\n' "$nom"
	"$sortie/bin/$nom" || exit 1
done

: >"$sortie/gcov.txt"
for src in lib/gfx/*.c; do
	LC_ALL=C gcov -b -c -g -o "$sortie/cov" "$src" >>"$sortie/gcov.txt"
done
mv -- ./*.gcov "$sortie/" 2>/dev/null || true
printf 'couverture : %s/gcov.txt\n' "$sortie"
