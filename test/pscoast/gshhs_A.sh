#!/usr/bin/env bash
#
# Test a that -A with Antarctica works again
# https://github.com/GenericMappingTools/gmt/issues/1295

gmt begin gshhs_A ps
  gmt subplot begin 2x1 -Fs12c -M10p -R-42/-39/-82:10/-77:45 -Ba1fg1 -BWsNe -Y1c
  gmt coast -Df  -A+ai -Gazure3 -Wfaint -c
  gmt coast -Df -A+ag -Glightpink -Wfaint
  gmt coast -Df -A7+ai -Gazure3 -Wfaint -c
  gmt coast -Df -A7+ag -Glightpink -Wfaint
  gmt subplot end
gmt end show
