#!/bin/sh
set -eu

racine=$(cd "$(dirname "$0")/../../.." && pwd)
cd "$racine"
base=${1:-build/a16/cov}
flags=$(make -s a16-hostflags)
rm -rf -- "$base"
mkdir -p -- "$base/obj" "$base/bin"

couvrir="--coverage -fcondition-coverage -fprofile-abs-path"
for source in lib/font/*.c; do
	# shellcheck disable=SC2086
	gcc $flags $couvrir -Ilib/font -c "$source" -o "$base/obj/$(basename -- "$source" .c).o"
done
for source in lib/font/gen/*.c lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c \
	tests/host/harness.c tests/host/harness_eq.c tests/host/a16/fake_*.c; do
	# shellcheck disable=SC2086
	gcc $flags -Ilib/font -Itests/host/a16 -c "$source" -o "$base/obj/$(basename -- "$source" .c).o"
done

mkdir -p -- "$base/tobj"
for test in tests/host/a16/test_*.c; do
	nom=$(basename -- "$test" .c)
	(
		# shellcheck disable=SC2086
		gcc $flags -Ilib/font -c "$test" -o "$base/tobj/$nom.o"
		# shellcheck disable=SC2086
		gcc $flags --coverage "$base/tobj/$nom.o" "$base"/obj/*.o -o "$base/bin/$nom"
	) &
done
wait

for exe in "$base"/bin/test_*; do
	A16_OUT="$base" "$exe" >/dev/null
done

cd "$base"
for source in "$racine"/lib/font/*.c; do
	LC_ALL=C gcov -b -c -g -o obj "$source" | grep -A5 "^File .*/lib/font/font_[a-z0-9_]*[.]c"
done
