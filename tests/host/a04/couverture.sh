#!/bin/sh
set -eu
b=${1:?dossier de build}
cov="$b/cov"
rm -rf -- "$cov"
mkdir -p -- "$cov/obj"
renames="-Dstrlen=vk_strlen -Dstrnlen=vk_strnlen -Dstrcmp=vk_strcmp \
-Dstrncmp=vk_strncmp -Dstrchr=vk_strchr -Dstrrchr=vk_strrchr \
-Dstrstr=vk_strstr -Dstrlcpy=vk_strlcpy -Dstrlcat=vk_strlcat \
-Dmemcmp=vk_memcmp -Dmemchr=vk_memchr"
flags="-std=gnu11 -O0 -g --coverage -fcondition-coverage \
-fsanitize=address,undefined -fno-sanitize-recover=undefined \
-Wall -Wextra -Werror -Iinclude -Ilib/libk -Itests/host -DVELUM_HOST \
$renames -Ikernel/mm -DVELUM_DEBUG"
heap_src=""
for f in kernel/mm/heap*.c kernel/mm/slab*.c; do
	[ "$f" = kernel/mm/heap_pages.c ] && continue
	heap_src="$heap_src $f"
done
support="lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c \
tests/host/harness.c tests/host/harness_eq.c"
objs=""
for f in $heap_src $support tests/host/a04/fake_*.c; do
	o="$cov/obj/$(printf '%s' "$f" | tr / _).o"
	# shellcheck disable=SC2086
	gcc $flags -c "$f" -o "$o"
	objs="$objs $o"
done
for t in tests/host/a04/test_*.c; do
	n=$(basename "$t" .c)
	# shellcheck disable=SC2086
	gcc $flags "$t" $objs -o "$cov/$n"
	"$cov/$n" >"$cov/$n.txt" 2>&1 || {
		printf 'echec de %s\n' "$n" >&2
		exit 1
	}
done
: >"$cov/resume.txt"
for f in $heap_src; do
	o="$cov/obj/$(printf '%s' "$f" | tr / _).o"
	gcov -b -c -g -o "$cov/obj" "$o" 2>/dev/null |
		grep -A5 "^File '$f'" >>"$cov/resume.txt" || true
done
rm -f -- ./*.gcov
cat "$cov/resume.txt"
