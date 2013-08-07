#!/bin/bash
# $Id$
#
# Make an illustration of grid chunking
#

PS=GMT_chunking.ps
n=1 # cell number

for ((x=0;x<12;++x)); do
  # x: number of chunk
  for ((j=2;j>=0;--j)); do
    # j: y-coordinate in each chunk
    for ((i=0;i<3;++i)); do
      # i: x-coordinate in each chunk
      echo "$i $j $((n++))"
    done
  done > chunk.tmp

  # plot chunks
  if [ $x -eq 0 ]; then
    # first chunk
    pstext chunk.tmp -R-0.5/2.5/-0.5/2.5 -Bg1+0.5 -JX3c/0 -Y10c -K > $PS
  elif [ $x -eq 4 -o $x -eq 8 ]; then
    # new chunk row
    pstext chunk.tmp -R -J -B+glightblue -Bg1+0.5 -X-9.3c -Y-3.1c -O -K >> $PS
  elif [ $x -eq 5 -o $x -eq 9 -o $x -eq 10 ]; then
    # colored chunks
    pstext chunk.tmp -R -J -B+glightblue -Bg1+0.5 -X3.1c -O -K >> $PS
  else
    pstext chunk.tmp -R -J -Bg1+0.5 -X3.1c -O -K >> $PS
  fi
done

# finalize PS
psxy -R -J -O <<< "-99 -99" >> $PS

rm -f chunk.tmp
