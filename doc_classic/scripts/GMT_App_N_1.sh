#!/usr/bin/env bash
#
#	Makes the inset for Appendix N(custom symbols)
#	Note that this script also assembles App N tex
#	file since the number of figures must be calculated.
#	Dimensions are in inches
#
grep -v '^#' "${GMT_SOURCE_DIR}"/share/custom/gmt_custom_symbols.conf | $AWK '{print $1}' > tt.lis
n=`cat tt.lis | wc -l`

# Because of text, the first page figure will contain less symbol rows than
# subsequent pages.

width=0.85
n_cols=6
n_rows=6
n_rows_p1=6
fs=9
dy=0.15

n_pages=`gmt math -Q $n $n_cols DIV CEIL $n_rows_p1 SUB 0 MAX $n_rows DIV CEIL 1 ADD =`
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
	
	n_rows_to_go=`gmt math -Q $n $s SUB $n_cols DIV CEIL $max_rows MIN =`
	H=`gmt math -Q $n_rows_to_go 1 $dy ADD MUL =`
	rm -f tt.lines tt.symbols tt.text tt.bars
	touch tt.lines tt.symbols tt.text tt.bars
	c=0
	while [ $c -lt $n_cols ]; do
		c=`expr $c + 1`
		cat << EOF >> tt.lines
> vertical line
$c	0
$c	$H
EOF
	done
	r=0
	while [ $r -lt $n_rows_to_go ]; do			# Loop over the rows that will fit this page
		r=`expr $r + 1`
		yt=`gmt math -Q $n_rows_to_go $r SUB 1.0 $dy ADD MUL 0.5 $dy MUL ADD =`
		ys=`gmt math -Q $yt 1.0 $dy ADD 0.5 MUL ADD =`
		ysb=`gmt math -Q $ys 0.5 SUB =`
		ytb=`gmt math -Q $yt 0.5 $dy MUL SUB =`
		cat << EOF >> tt.lines
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
			x=`gmt math -Q $c 1 SUB 0.5 ADD =`
			symbol=`sed -n ${s}p tt.lis`
			echo "$x $ys k${symbol}" >> tt.symbols
			name=`echo $symbol | tr 'a-z' 'A-Z'`
			echo "$x $yt $name" >> tt.text
			echo "$x $yt $width $dy" >> tt.bars
		done
	done
	gmt psxy -R0/$n_cols/0/$H -Jx${width}i -P -K tt.lines -Wthick -B0 > GMT_App_N_$p.ps
	gmt psxy -R -J -O -K -S${width}i -Wthinnest tt.symbols >> GMT_App_N_$p.ps
	gmt psxy -R -J -O -K -Sri -Gblack tt.bars >> GMT_App_N_$p.ps
	gmt pstext -R -J -O tt.text -F+f${fs}p,white >> GMT_App_N_$p.ps
done

if ! test -s GMT_Appendix_N_inc.tex ; then
	rm -f GMT_Appendix_N_inc.tex
fi
