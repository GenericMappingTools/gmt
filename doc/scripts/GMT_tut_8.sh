#!/usr/bin/env bash
gmt begin GMT_tut_8 ps
gmt plot @tut_data.txt -R0/6/0/6 -Jx1i -Baf -Wthinner 
gmt plot tut_data.txt -W -Si0.2i 
gmt end
