#!/usr/bin/env bash
set -euo pipefail

BINUTILS=binutils-2.47
GCC=gcc-15.3.0
BINUTILS_SHA256=154ab23b60070e8f27013c22977f1129425d67d1e8acd6e13010e617811e4cff
GCC_SHA256=fa59c1beef8995f27c4d71c1df227587189315d3e6faff1bb4306e61b0c530eb
TARGET=x86_64-elf
PREFIX="${PREFIX:-$HOME/.local/opt/velum-cross}"
SRC="${SRC:-$PWD/build/toolchain}"

telecharger()
{
	local nom="$1" url="$2" somme="$3"
	[ -f "$SRC/$nom.tar.xz" ] || curl -fL -o "$SRC/$nom.tar.xz" "$url"
	printf '%s  %s\n' "$somme" "$SRC/$nom.tar.xz" | sha256sum -c -
	[ -d "$SRC/$nom" ] || tar -xf "$SRC/$nom.tar.xz" -C "$SRC"
}

multilib_sans_zone_rouge()
{
	printf 'MULTILIB_OPTIONS += mno-red-zone\nMULTILIB_DIRNAMES += no-red-zone\n' \
		>"$SRC/$GCC/gcc/config/i386/t-x86_64-elf"
	local cfg="$SRC/$GCC/gcc/config.gcc"
	if ! grep -A1 '^x86_64-\*-elf\*)' "$cfg" | grep -q 't-x86_64-elf'; then
		sed -i '/^x86_64-\*-elf\*)/r /dev/stdin' "$cfg" <<'EOT'
	tmake_file="${tmake_file} i386/t-x86_64-elf"
EOT
	fi
}

mkdir -p "$SRC"
export PATH="$PREFIX/bin:$PATH"
telecharger "$BINUTILS" "https://ftp.gnu.org/gnu/binutils/$BINUTILS.tar.xz" "$BINUTILS_SHA256"
telecharger "$GCC" "https://ftp.gnu.org/gnu/gcc/$GCC/$GCC.tar.xz" "$GCC_SHA256"
(cd "$SRC/$GCC" && ./contrib/download_prerequisites)
multilib_sans_zone_rouge

rm -rf "$SRC/build-binutils" "$SRC/build-gcc"
mkdir "$SRC/build-binutils" "$SRC/build-gcc"
(
	cd "$SRC/build-binutils"
	"../$BINUTILS/configure" --target="$TARGET" --prefix="$PREFIX" \
		--with-sysroot --disable-nls --disable-werror
	make -j"$(nproc)" MAKEINFO=true
	make install MAKEINFO=true
)
(
	cd "$SRC/build-gcc"
	"../$GCC/configure" --target="$TARGET" --prefix="$PREFIX" \
		--disable-nls --enable-languages=c --without-headers
	make -j"$(nproc)" all-gcc all-target-libgcc MAKEINFO=true
	make install-gcc install-target-libgcc MAKEINFO=true
)
"$PREFIX/bin/$TARGET-gcc" --version | head -1
"$PREFIX/bin/$TARGET-gcc" -print-multi-lib | grep -q no-red-zone
printf 'Chaîne prête dans %s : ajouter %s/bin au PATH.\n' "$PREFIX" "$PREFIX"
