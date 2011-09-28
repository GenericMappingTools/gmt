#!/bin/bash

GMT_SOURCE_DIR="$1/src"
AWK=$(which gawk || which awk)

# no awk needed, these are all regex only:
grep -v '^#' ${GMT_SOURCE_DIR}/Colors.txt | LANG=C ${AWK} '{printf "\"%s\",\n", $4}' | sed '$s/,$//' > gmt_colornames.h

grep -v '^#' ${GMT_SOURCE_DIR}/Datums.txt | ${AWK} -F'	' '{printf "\t\t{ \"%s\", \"%s\", \"%s\", {%s, %s, %s}},\n", $1, $2, $6, $3, $4, $5}' | sed '$s/},/}/' > gmt_datums.h

egrep -v '#' ${GMT_SOURCE_DIR}/GMTprogs.txt | ${AWK} '{printf "{\"%s\",\t%s},\n", $1, $2}' > gmt_prognames.h

grep -v '^#' ${GMT_SOURCE_DIR}/Colors.txt | LANG=C ${AWK} '{printf "{ %3i, %3i, %3i },\n", $1, $2, $3}' | sed '$s/},/}/' > gmt_color_rgb.h


# these involve awk calculations
grep -v '^#' ${GMT_SOURCE_DIR}/Ellipsoids.txt | LANG=C ${AWK} '{{printf "\t\t{ \"%s\", %d, %s, ", $1, $2, $3}; \
	if ($4 == 0) {printf "0.0},\n"} else if ($4 < 0.5*$3) {printf "1.0/%s},\n", $4} else \
	{printf "1.0-%s/%s},\n", $4, $3}} END{printf "\t\t{ \"Custom\", 0, 6371008.7714, 0.0}\n"}' > gmt_ellipsoids.h

egrep 'session.grdformat.* = ' ${GMT_SOURCE_DIR}/gmt_customio.c | egrep -v 'not supported' | cut -d\" -f2 | \
	tr '[a-z]' '[A-Z]' | ${AWK} '{printf "#define GMT_GRD_IS_%s\t%d\n", $1, NR-1}' > gmt_grdkeys.h

# can be replaced with:
#  file(READ filename variable)
#  split into list
#  list operations
cat ${GMT_SOURCE_DIR}/gmt_keywords.txt ${GMT_SOURCE_DIR}/gmt_keywords.d | grep -v '^#' | ${AWK} '{print $1}' | sort -u | ${AWK} '{printf "\"%s\",\n", $1}' | sed '$s/,$//' > gmt_keywords.h

# these can all be exchanged by macro:
#  file(READ filename variable)
#  split into list
#  foreach + counter
grep -v '^#' ${GMT_SOURCE_DIR}/gmtapi_errors.d | awk '{printf "#define %s\t%d\n", $1, 1-NR}' > gmtapi_errno.h

cat ${GMT_SOURCE_DIR}/gmt_keywords.txt ${GMT_SOURCE_DIR}/gmt_keywords.d | grep -v '^#' | ${AWK} '{print $1}' | sort -u | ${AWK} '{printf "#define GMTCASE_%s\t%d\n", $1, NR-1}' > gmt_keycases.h

grep -v '^#' ${GMT_SOURCE_DIR}/gmtapi_errors.d | awk '{printf "\"%s\",\n", $1}' > gmtapi_errstr.h

egrep -v '#' ${GMT_SOURCE_DIR}/GMTprogs.txt | ${AWK} '{printf "\t\tcase %d:\n\t\t\tfunc = (PFL)GMT_%s;\n\t\t\t*mode = %s;\n\t\t\tbreak;\n", NR-1, $1, $2}' > gmt_progcases.h


# these can all be exchanged by macro:
#  file(READ filename variable)
#  split into list
#  list(LENGTH <list> GMT_N_...)
#  put GMT_N_ in config.h.cmake
printf "#define GMT_N_COLOR_NAMES\t%d\t/* Lines in %s */\n" `wc -l gmt_colornames.h` > gmt_dimensions.h
printf "#define GMT_N_DATUMS\t\t%d\t/* Lines in %s */\n" `wc -l gmt_datums.h` >> gmt_dimensions.h
printf "#define GMT_N_ELLIPSOIDS\t%d\t/* Lines in %s */\n" `wc -l gmt_ellipsoids.h` >> gmt_dimensions.h
printf "#define GMT_N_GRD_FORMATS\t%d\t/* Lines in %s */\n" `wc -l gmt_grdkeys.h` >> gmt_dimensions.h
printf "#define GMT_N_KEYS\t\t%d\t/* Lines in %s */\n" `wc -l gmt_keywords.h` >> gmt_dimensions.h
printf "#define GMT_N_MEDIA\t\t%d\t/* Lines in %s */\n" `wc -l ${GMT_SOURCE_DIR}/gmt_media_name.h` >> gmt_dimensions.h
printf "#define GMT_N_PEN_NAMES\t\t%d\t/* Lines in %s */\n" `wc -l ${GMT_SOURCE_DIR}/gmt_pennames.h` >> gmt_dimensions.h
printf "#define GMT_N_UNIQUE\t\t%d\t/* Lines in %s */\n" `wc -l ${GMT_SOURCE_DIR}/gmt_unique.h` >> gmt_dimensions.h
printf "#define GMT_N_PROGRAMS\t\t%d\t/* Lines in %s */\n" `wc -l gmt_prognames.h` >> gmt_dimensions.h


# manpages

# can all be replaced with regex:
egrep 'session.grdformat.* = ' ${GMT_SOURCE_DIR}/gmt_customio.c | egrep -v 'not supported' | cut -d\" -f2 | ${AWK} '{printf "BD(%s)\t%s\n.br\n", $1, substr($0,6,length($0)-5)}'  > grdreformat_man.i

grep -v '^#' ${GMT_SOURCE_DIR}/Colors.txt | LANG=C ${AWK} '{printf ".br\n%3i\t%3i\t%3i\t%s\n", $1, $2, $3, $4}' > Colors.i

grep -v '^#' ${GMT_SOURCE_DIR}/Ellipsoids.txt > tmp0.txt
${AWK} '{printf "%s :\n", $1, $2}' tmp0.txt > tmp1.txt
${AWK} -F: '{print $2}' tmp0.txt | sed -e 's/^ //g' > tmp2.txt
${AWK} '{printf "(%d)\n", $2}' tmp0.txt > tmp3.txt

paste -d' ' tmp[123].txt | ${AWK} '{printf ".br\n%s\n", $0}' > Ellipsoids.i

rm -f tmp[0123].txt

# replace with file read + list operations + counter
grep -v '^#' ${GMT_SOURCE_DIR}/../share/pslib/PS_font_info.d | ${AWK} '{printf ".br\n%i\t%s\n", NR - 1, $1}' > Fonts.i

exit 0
