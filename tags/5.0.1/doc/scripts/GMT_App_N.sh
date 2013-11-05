#!/bin/bash
#	$Id$
#
#	Makes the insert for Appendix N(custom symbols)
#	Note that this script also assembles App N tex
#	file since the number of figures must be calculated.
#
. ./functions.sh

grep -v '^#' "${GMT_SHAREDIR}"/share/conf/gmt_custom_symbols.conf | awk '{print $1}' > $$.lis
n=`cat $$.lis | wc -l`

# Because of text, the first page figure will contain less symbol rows than
# subsequent pages.

width=0.85
n_cols=6
n_rows=6
n_rows_p1=6
fs=9
dy=0.15

n_pages=`gmtmath -Q $n $n_cols DIV CEIL $n_rows_p1 SUB 0 MAX $n_rows DIV CEIL 1 ADD =`
touch GMT_Appendix_N_inc.tex

p=0
s=0
while [ $p -lt $n_pages ]; do
	p=`expr $p + 1`
	if [ $p -eq 1 ]; then
		max_rows=$n_rows_p1
	else
		max_rows=$n_rows
		echo "\GMTfig[h]{GMT_App_N_$p}{Additional custom plot symbols}" >> GMT_Appendix_N_inc.tex
	fi
	
	n_rows_to_go=`gmtmath -Q $n $s SUB $n_cols DIV CEIL $max_rows MIN =`
	H=`gmtmath -Q $n_rows_to_go 1 $dy ADD MUL =`
	touch $$.lines $$.symbols $$.text $$.bars
	c=0
	while [ $c -lt $n_cols ]; do
		c=`expr $c + 1`
		cat << EOF >> $$.lines
> vertical line
$c	0
$c	$H
EOF
	done
	r=0
	while [ $r -lt $n_rows_to_go ]; do			# Loop over the rows that will fit this page
		r=`expr $r + 1`
		yt=`gmtmath -Q $n_rows_to_go $r SUB 1.0 $dy ADD MUL 0.5 $dy MUL ADD =`
		ys=`gmtmath -Q $yt 1.0 $dy ADD 0.5 MUL ADD =`
		ysb=`gmtmath -Q $ys 0.5 SUB =`
		ytb=`gmtmath -Q $yt 0.5 $dy MUL SUB =`
		cat << EOF >> $$.lines
> base of symbol line
0	$ysb
$n_cols	$ysb
> base of text line
0	$ytb
$n_cols	$ytb
EOF
		c=0
		while [ $c -lt $n_cols ] && [ $s -lt $n ]; do	# Loop over this row, but watch for end of symbols
			c=`expr $c + 1`
			s=`expr $s + 1`
			x=`gmtmath -Q $c 1 SUB 0.5 ADD =`
			symbol=`sed -n ${s}p $$.lis`
			echo "$x $ys k${symbol}" >> $$.symbols
			name=`echo $symbol | tr 'a-z' 'A-Z'`
			echo "$x $yt $name" >> $$.text
			echo "$x $yt $width $dy" >> $$.bars
		done
	done
	psxy -R0/$n_cols/0/$H -Jx${width}i -P -K $$.lines -Wthick -B0 > GMT_App_N_$p.ps
	psxy -R -J -O -K -S${width}i -Wthinnest $$.symbols >> GMT_App_N_$p.ps
	psxy -R -J -O -K -Sr -Gblack $$.bars >> GMT_App_N_$p.ps
	pstext -R -J -O $$.text -F+f${fs}p,white >> GMT_App_N_$p.ps
	rm -f $$.lines $$.symbols $$.text $$.bars
done

if ! test -s GMT_Appendix_N_inc.tex ; then
	rm -f GMT_Appendix_N_inc.tex
fi
