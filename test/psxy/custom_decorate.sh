#!/usr/bin/env bash
#
# Decorate lines with custom symbols

cat > path.txt << END
-1.5 -1
-0.5  0
 1.5  1
END
gmt begin custom_decorate ps
  gmt plot path.txt -S~d2c:+sklcrescent/60p+gred+pthin -W0.25p -R-2/2/-1/1.2 -JM15c
  gmt plot path.txt -S~d2c:+skdeltoid/50p+pthin+gblue -W0,white -Y5c
  gmt plot path.txt -S~d2c:+skstarp/40p+pthick+ggreen+a0 -Wthinnest,red -Y5c
  gmt plot path.txt -S~d1c:+skcrosshair/25p+pthin -W2p -Y5c
gmt end show
