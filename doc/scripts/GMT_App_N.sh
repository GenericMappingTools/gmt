#!/bin/sh
#	$Id: GMT_App_N.sh,v 1.5 2004-08-17 02:30:32 pwessel Exp $
#
#	Makes the insert for Appendix N(custom symbols)
#	Note that this script also assembles App N tex
#	file since the number of figures must be calculated.
#

grep -v '^#' ../../share/GMT_CustomSymbols.lis | awk '{print $1}' > $$.lis
n=`cat $$.lis | wc -l`

# Because of text, the first page figure will contain less symbol rows than
# subsequent pages.

n_cols=5
n_rows=7
n_rows_p1=5
fs=9
dy=0.15

cp -f ../GMT_Appendix_N_main.tex ../GMT_Appendix_N.tex

n_pages=`gmtmath -Q $n $n_cols DIV CEIL $n_rows_p1 SUB 0 MAX $n_rows DIV CEIL 1 ADD =`

p=0
s=0
while [ $p -lt $n_pages ]; do
	p=`expr $p + 1`
	if [ $p -eq 1 ]; then
		max_rows=$n_rows_p1
	else
		max_rows=$n_rows
		echo "\GMTfig[h]{GMT_App_N_$p}{Additional custom plot symbols}" >>  ../GMT_Appendix_N.tex
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
			echo "$x $yt $fs 0 0 CM $name" >> $$.text
			echo "$x $yt 1 $dy" >> $$.bars
		done
	done
	psxy -R0/$n_cols/0/$H -Jx1i -P -K -M $$.lines -W1p -B0 > GMT_App_N_$p.ps
	psxy -R -Jx -O -K -S1i -W0.25p $$.symbols >> GMT_App_N_$p.ps
	psxy -R -Jx -O -K -Sr -Gblack $$.bars >> GMT_App_N_$p.ps
	pstext -R -Jx -O $$.text -Gwhite >> GMT_App_N_$p.ps
	rm -f $$.lines $$.symbols $$.text $$.bars
done
rm -f $$.lis

	
