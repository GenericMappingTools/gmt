#!/usr/bin/env bash
#
# Script that can be run after full testing to examining the RMS
# failures.  We assume this script is run from the top GMT directory
# and that there is a build or rbuild directory there.
BLDDIR=rbuild
if [ ! -d admin ]; then
	echo "examine-failure-log.sh: Must be run from top-level gmt directory" >&2
	exit 1
fi
if [ ! -d ${BLDDIR} ]; then
	BLDDIR=build
	if [ ! -d ${BLDDIR} ]; then
		echo "examine-failure-log.sh: Neither build nor rbuild directory found" >&2
	exit 1
	fi
fi

# 0. Change into build directory

cd ${BLDDIR}

# 1. Get one-record-per test results

grep "RMS Error" Testing/Temporary/LastTest.log | egrep -v "Thread" | sed -e 'sBN/AB-1Bgp '> /tmp/raw.log

# 2. Extract info from raw log

awk '{print $5, $1 }' /tmp/raw.log | sort -k 1 -n | awk '{printf "%d\t%s\t%s\n", NR, $1, $2}' > /tmp/sorted.log

# 3. Plot the rms errors sorted

xmin=0
xmax=$(gmt info -C -I10 /tmp/sorted.log -o1)
xc=$(gmt math -Q ${xmin} ${xmax} ADD 2 DIV =)
ymax=$(gmt info -C -I10/0.1 /tmp/sorted.log -o3)
yNaN=$(gmt info -C -I10/0.05 /tmp/sorted.log -o3)
xNaN=$(awk '{if ($2 < 0) print $1}' /tmp/sorted.log | sort -r -n | gmt info -C -o1)
xzero=$(awk '{if ($2 > 0) print $1}' /tmp/sorted.log | gmt info -C -o0)
Np=$(awk '{if ($2 == 0.0) print $0}' /tmp/sorted.log | gmt info -Fi -o2)
chip=$(uname -m)
N=$(gmt info /tmp/sorted.log -C -o1)
rms_limit=$(grep GRAPHICSMAGICK_RMS ../cmake/*.cmake | grep set | awk '{print $4}' | tr '")' '  ' | awk '{print $1}')
Nbelow=$(awk '{if ($2 > '"${rms_limit}"') print $0}' /tmp/sorted.log | gmt info -C -o0)
FAIL=$(gmt math -Q ${N} ${Nbelow} SUB =)
xp=$(gmt math -Q ${xmin} ${xzero} ADD 2 DIV =)
xf=$(gmt math -Q ${xzero} ${xmax} ADD 2 DIV =)
gmt begin rms-errors png
ymin=0.0001
	gmt basemap -R${xmin}/${xmax}/${ymin}/${ymax} -JX16c/21cl -Bxaf+l"Test number (after sorting)" \
		-Bya1f3+l"RMS error" -B+t"${FAIL} of ${N} scripts fail on ${chip}" -U${USER}
	cat <<- EOF | gmt plot -Glightgray
	${xmin}	${rms_limit}
	${xmax}	${rms_limit}
	${xmax}	0.0655
	${xmin}	0.0655
	EOF
	echo ${xp} 0.0655 "Range of adjusted RMS threshold" | gmt text -F+f12p+jTC -Dj0/6p
	cat <<- EOF | gmt plot -W0.25p,darkgreen
	> RMS threshold
	0	${rms_limit}
	${xmax}	${rms_limit}
	> last known failure
	${xNaN}	${ymin}
	${xNaN}	${ymax}
	> Start of nonzero RMS
	${xzero}	${ymin}
	${xzero}	${ymax}
	EOF
	echo ${xp} 0.001 "${Np} scripts has zero r.m.s." | gmt text -F+f12p+jTC -Dj0/6p -N
	echo ${xp} ${ymax} "PERFECT" | gmt text -F+f12p+jTC -Dj0/6p -N
	echo ${xf} ${ymax} "FAILURES" | gmt text -F+f12p+jTC -Dj0/6p -N
	echo ${xc} ${rms_limit} "DEFAULT RMS THRESHOLD = ${rms_limit}" | gmt text -F+f12p+jTC -Dj0/6p -N -Gwhite
	echo 30 ${ymax} "KNOWN FAILURES" | gmt text -F+f12p+jTR+a90 -Dj2p/2p -N
	grep -v NaN /tmp/sorted.log | gmt plot -W0.25p
	grep -v NaN /tmp/sorted.log | gmt plot -Sc2p -Gorange -l"Test r.m.s. below RMS limit"+jBL
	grep -v NaN /tmp/sorted.log | awk '{if ($2 > '"${rms_limit}"') print $0}' | gmt plot -Sc2p -Gred -l"Test r.m.s. above RMS limit"
	awk '{if ($2 < 0) print $1, '"${yNaN}"'}' /tmp/sorted.log | gmt plot -Ss2p -Gblue -l"Known failure"
gmt end show
