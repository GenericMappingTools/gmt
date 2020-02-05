#!/usr/bin/env bash
#
# Example command to reformat the DST indices downloaded from
# http://wdc.kugi.kyoto-u.ac.jp/dstae/index.html
#
# Thanks to Michael Chandler



awk '{printf "%.10s      %+04d%+04d%+04d%+04d%+04d%+04d%+04d%+04d%+04d%+04d%+04d%+04d%+04d%+04d%+04d%+04d%+04d%+04d%+04d%+04d%+04d%+04d%+04d%+04d%+04d%+04d\n",$1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13,$14,$15,$16,$17,$18,$19,$20,$21,$22,$23,$24,$25,$26,$27}' WWW_dstae00022758.dat | sed -e 's/+/ /g' -e 's/*/Q/'
