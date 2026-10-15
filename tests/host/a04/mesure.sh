#!/bin/sh
set -eu
b=${1:?dossier de build}
dest="$b/bench"
mkdir -p -- "$dest"
renames="-Dstrlen=vk_strlen -Dstrnlen=vk_strnlen -Dstrcmp=vk_strcmp \
-Dstrncmp=vk_strncmp -Dstrchr=vk_strchr -Dstrrchr=vk_strrchr \
-Dstrstr=vk_strstr -Dstrlcpy=vk_strlcpy -Dstrlcat=vk_strlcat \
-Dmemcmp=vk_memcmp -Dmemchr=vk_memchr"
sources=""
for f in kernel/mm/heap*.c kernel/mm/slab*.c; do
	[ "$f" = kernel/mm/heap_pages.c ] && continue
	sources="$sources $f"
done
for profil in debug release; do
	if [ "$profil" = debug ]; then
		defs="-DVELUM_DEBUG"
	else
		defs="-DNDEBUG"
	fi
	# shellcheck disable=SC2086
	gcc -std=gnu11 -O2 -g -Wall -Wextra -Werror -Iinclude -Ilib/libk \
		-Itests/host -DVELUM_HOST $renames -Ikernel/mm $defs \
		tests/host/a04/bench_heap.c tests/host/harness.c \
		tests/host/harness_eq.c $sources lib/libk/str.c lib/libk/str2.c \
		lib/libk/memcmp.c tests/host/a04/fake_*.c -o "$dest/bench_$profil"
	printf 'profil %s\n' "$profil"
	"$dest/bench_$profil" | tee "$dest/bench_$profil.txt"
done
