#!/usr/bin/env bash
# Test the long-format version of -R -J -B and -U
gmt begin longbasemap ps
  gmt subplot begin 2x1 -F6i/8.5i -M5p -A1+c+o0.2i
  gmt basemap --region=0/40/0/50 --projection=X? --frame=WSne+fill=lightblue --axis=x:af+unit=k+angle=45 --axis=y:afg+prefix="$" -c1
  gmt basemap --region=0/70/-20/25 --projection=M? --frame=WSne+fill=lightgreen+oblique-pole=60/20 --axis=x:afg --axis=y:afg -c0
  gmt subplot end
gmt end show
