#!/bin/sh
set -eu

racine=$(cd "$(dirname "$0")/../../.." && pwd)
cd "$racine"
base=${1:-build/a16/mutants}
[ "$#" -eq 0 ] || shift
filtre=" $* "
tuees=0
total=0
vivantes=""

lancer()
{
	nom=$1
	fichier=$2
	motif=$3
	if [ "$filtre" != "  " ]; then
		case "$filtre" in
		*" $nom "*) ;;
		*) return 0 ;;
		esac
	fi
	dest="$base/$nom"
	mkdir -p -- "$dest/src"
	sed -e "$motif" -- "$fichier" >"$dest/src/$(basename -- "$fichier")"
	if cmp -s -- "$fichier" "$dest/src/$(basename -- "$fichier")"; then
		printf 'mutant %s : motif introuvable\n' "$nom" >&2
		exit 2
	fi
	liste=$(
		{
			find lib/font -name '*.c' | sort
			printf '%s\n' lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c
		} | sed -e "s#^$fichier\$#$dest/src/$(basename -- "$fichier")#" | tr '\n' ' '
	)
	total=$((total + 1))
	if timeout 600 make -j8 B="$dest/build" HSRC_a16="$liste" host-a16 >"$dest/log" 2>&1; then
		vivantes="$vivantes $nom"
		printf 'VIVANT  %s\n' "$nom"
	elif grep -Eq ':[0-9]+:[0-9]+: error:' "$dest/log"; then
		printf 'INVALIDE %s (ne compile pas)\n' "$nom"
		total=$((total - 1))
	else
		tuees=$((tuees + 1))
		printf 'TUE     %s\n' "$nom"
	fi
}

lancer utf8-tete-c0 lib/font/font_utf8.c 's/lead >= 0xC2/lead >= 0xC0/'
lancer utf8-e0-sur-longueur lib/font/font_utf8.c 's/low = 0xA0/low = 0x80/'
lancer utf8-substituts lib/font/font_utf8.c 's/high = 0x9F/high = 0xBF/'
lancer utf8-hors-plage lib/font/font_utf8.c 's/high = 0x8F/high = 0xBF/'
lancer utf8-f0-sur-longueur lib/font/font_utf8.c 's/low = 0x90/low = 0x80/'
lancer utf8-tete-f5 lib/font/font_utf8.c 's/lead <= 0xF4/lead <= 0xF7/'
lancer utf8-lecture-apres-fin lib/font/font_utf8.c 's/n + 1 < avail/n + 1 <= avail/'
lancer utf8-sous-partie lib/font/font_utf8.c 's/\*s += 1 + got;/*s += 1 + need;/'
lancer utf8-bits-de-tete lib/font/font_utf8.c 's/0x3F >> need/0xFF >> need/'
lancer glyphe-indice-direct lib/font/font_glyph.c 's/slot >= (uint32_t)f->nglyphs/slot > (uint32_t)f->nglyphs/'
lancer glyphe-sans-remplacement lib/font/font_glyph.c 's/found = glyph_search(f, FONT_REPLACEMENT);/found = NULL;/'
lancer glyphe-recherche-haut lib/font/font_glyph.c 's/high = mid - 1;/high = mid - 2;/'
lancer largeur-sans-saturation lib/font/font_text.c 's/sum > INT32_MAX/sum > INT64_MAX/'
lancer coupe-trop-serree lib/font/font_text.c 's/g->advance > left/g->advance >= left/'
lancer coupe-trop-large lib/font/font_text.c 's/g->advance > left/g->advance > left + 1/'
lancer clip-largeur-ignoree lib/font/font_pen.c 's/(int64_t)clip->x + clip->w/(int64_t)clip->x + dst->w/'
lancer clip-haut-ignore lib/font/font_pen.c 's/clamp_to(clip->y, 0, dst->h)/0/'
lancer dessin-colonne-en-trop lib/font/font_draw.c 's/col < cut->c1/col <= cut->c1/'
lancer dessin-ordre-des-bits lib/font/font_draw.c 's/7 - (col \& 7)/(col \& 7)/'
lancer dessin-chasse lib/font/font_draw.c 's/pen.x += g->advance;/pen.x += g->advance + 1;/'
lancer dessin-sortie-anticipee lib/font/font_draw.c 's/pen.x - FONT_XOFF_MIN < pen.box.right/pen.x < pen.box.right/'
lancer police-inconnue lib/font/font_get.c 's/return (NULL);/return (\&g_font_ui);/'

printf 'mutants : %s tues sur %s (vivants :%s)\n' "$tuees" "$total" "$vivantes"
[ "$tuees" -eq "$total" ]
