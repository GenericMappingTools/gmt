#!/usr/bin/env bash
#
# Make an illustration of grid chunking
#

gmt begin GMT_chunking
gmt set MAP_FRAME_PEN thick FONT_ANNOT_PRIMARY 9p

n=1 # current cell number

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
    gmt text chunk.tmp -R-0.5/2.5/-0.5/2.5 -Bg1+0.5 -JX2c/0 -Y10c
  elif [ $x -eq 4 -o $x -eq 8 ]; then
    # new chunk row
    gmt text chunk.tmp -B+glightblue -Bg1+0.5 -X-6.3c -Y-2.1c
  elif [ $x -eq 5 -o $x -eq 9 -o $x -eq 10 ]; then
    # colored chunks
    gmt text chunk.tmp -B+glightblue -Bg1+0.5 -X2.1c
  else
    gmt text chunk.tmp -Bg1+0.5 -X2.1c
  fi
done

rm -f chunk.tmp
gmt end show
