#!/usr/bin/env bash
#
#	Makes the octal code charts in Appendix F
#
# Use the row, col values to generate the octal code needed and
# plot it with gmt pstext, including the header row and left column

bash "${src:-.}"/func_F_iso+.sh > GMT_App_F_iso+.ps
