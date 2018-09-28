#!/bin/bash
gmt coast -R7:30/38:30/10:30/41:30r -JC8.75/40/2.5i -Bafg -LjBR+c40+w100+f+o0.15i/0.2i -Dh -Gspringgreen \
	-Sazure -Wthinnest -Ia/thinner --FONT_LABEL=10p -ps GMT_cassini
