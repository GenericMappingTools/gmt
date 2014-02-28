#!/bin/bash
# Test gmt psxy plotting with gaps

ps=gaps_time.ps

gmt gmtset IO_NAN_RECORDS pass TIME_UNIT y TIME_EPOCH 2000-01-01 FORMAT_DATE_MAP mm
gmt gmtmath "gtec_tx_daily.nc?time/gtec" -C0 1 MOD = | \
	gmt psxy -R2000-01-01T/2001-01-01T/5/65 -gx0.5 -JX6it/3i -W0.5p,blue -P -Bx1O+lMonth -By20f5+lGTEC > $ps

