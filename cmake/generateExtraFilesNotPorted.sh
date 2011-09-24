#!/bin/bash
#
GMT_SOURCE_DIR="$1/src"
AWK=$(which gawk || which awk)

grep -v '^#' ${GMT_SOURCE_DIR}/Colors.txt | LANG=C ${AWK} '{printf "\"%s\",\n", $4}' | sed '$s/,$//' > gmt_colornames.h
grep -v '^#' ${GMT_SOURCE_DIR}/Datums.txt | ${AWK} -F'	' '{printf "\t\t{ \"%s\", \"%s\", \"%s\", {%s, %s, %s}},\n", $1, $2, $6, $3, $4, $5}' | sed '$s/},/}/' > gmt_datums.h
grep -v '^#' ${GMT_SOURCE_DIR}/Ellipsoids.txt | LANG=C ${AWK} '{{printf "\t\t{ \"%s\", %d, %s, ", $1, $2, $3}; \
			if ($4 == 0) {printf "0.0},\n"} else if ($4 < 0.5*$3) {printf "1.0/%s},\n", $4} else \
			{printf "1.0-%s/%s},\n", $4, $3}} END{printf "\t\t{ \"Custom\", 0, 6371008.7714, 0.0}\n"}' > gmt_ellipsoids.h
egrep 'session.grdformat.* = ' ${GMT_SOURCE_DIR}/gmt_customio.c | egrep -v 'not supported' | cut -d\" -f2 | \
			tr '[a-z]' '[A-Z]' | ${AWK} '{printf "#define GMT_GRD_IS_%s\t%d\n", $1, NR-1}' > gmt_grdkeys.h
cat ${GMT_SOURCE_DIR}/gmt_keywords.txt ${GMT_SOURCE_DIR}/gmt_keywords.d | grep -v '^#' | ${AWK} '{print $1}' | sort -u | ${AWK} '{printf "\"%s\",\n", $1}' | sed '$s/,$//' > gmt_keywords.h
egrep -v '#' ${GMT_SOURCE_DIR}/GMTprogs.txt | ${AWK} '{printf "{\"%s\",\t%s},\n", $1, $2}' > gmt_prognames.h

printf "#define GMT_N_COLOR_NAMES\t%d\t/* Lines in %s */\n" `wc -l gmt_colornames.h` > gmt_dimensions.h
printf "#define GMT_N_DATUMS\t\t%d\t/* Lines in %s */\n" `wc -l gmt_datums.h` >> gmt_dimensions.h
printf "#define GMT_N_ELLIPSOIDS\t%d\t/* Lines in %s */\n" `wc -l gmt_ellipsoids.h` >> gmt_dimensions.h
printf "#define GMT_N_GRD_FORMATS\t%d\t/* Lines in %s */\n" `wc -l gmt_grdkeys.h` >> gmt_dimensions.h
printf "#define GMT_N_KEYS\t\t%d\t/* Lines in %s */\n" `wc -l gmt_keywords.h` >> gmt_dimensions.h
printf "#define GMT_N_MEDIA\t\t%d\t/* Lines in %s */\n" `wc -l ${GMT_SOURCE_DIR}/gmt_media_name.h` >> gmt_dimensions.h
printf "#define GMT_N_PEN_NAMES\t\t%d\t/* Lines in %s */\n" `wc -l ${GMT_SOURCE_DIR}/gmt_pennames.h` >> gmt_dimensions.h
printf "#define GMT_N_UNIQUE\t\t%d\t/* Lines in %s */\n" `wc -l ${GMT_SOURCE_DIR}/gmt_unique.h` >> gmt_dimensions.h
printf "#define GMT_N_PROGRAMS\t\t%d\t/* Lines in %s */\n" `wc -l gmt_prognames.h` >> gmt_dimensions.h
grep -v '^#' ${GMT_SOURCE_DIR}/gmtapi_errors.d | awk '{printf "#define %s\t%d\n", $1, 1-NR}' > gmtapi_errno.h

grep -v '^#' ${GMT_SOURCE_DIR}/Colors.txt | LANG=C ${AWK} '{printf "{ %3i, %3i, %3i },\n", $1, $2, $3}' | sed '$s/},/}/' > gmt_color_rgb.h
cat ${GMT_SOURCE_DIR}/gmt_keywords.txt ${GMT_SOURCE_DIR}/gmt_keywords.d | grep -v '^#' | ${AWK} '{print $1}' | sort -u | ${AWK} '{printf "#define GMTCASE_%s\t%d\n", $1, NR-1}' > gmt_keycases.h
grep -v '^#' ${GMT_SOURCE_DIR}/gmtapi_errors.d | awk '{printf "\"%s\",\n", $1}' > gmtapi_errstr.h
egrep -v '#' ${GMT_SOURCE_DIR}/GMTprogs.txt | ${AWK} '{printf "\t\tcase %d:\n\t\t\tfunc = (PFL)GMT_%s;\n\t\t\t*mode = %s;\n\t\t\tbreak;\n", NR-1, $1, $2}' > gmt_progcases.h

exit 0
