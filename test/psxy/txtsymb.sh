#!/bin/bash
#	$Id$
# Test custom symbols with different fonts and both fixed and variable sizes.
ps=txtsymb.ps

# coords where to place the symbol
cat > label_places1.txt << END
120 32
130 32
END
cat > label_places2.txt << END
120 24
130 24
END

# custom symbol definition
cat > lat_label.def << END
0a R
-0.5 -0.5 M
0.5 -0.5 D
0.5 0.5 D
-0.5 0.5 D
-0.5 -0.5 D
-0.45 0.45 18p LEFT l+jTL -W- -Gblack
0.4 0.4 0.2 c -Gyellow -Wfaint
0.45 -0.45 16p RIGHT l+jRB+f12p,Times-Roman -Gred -W-
0.0 0.0 0.4 ABC l+jCM+fHelvetica-Bold -Ggreen -Wfaint,black
END

# generate simple location map
pscoast -R115/20/135/36r -JB130/30/25/45/6i -Bag -Dl -Ggrey -Wthinnest -A500 -P -K -Xc > $ps

# insert custom symbol at 5 cm size
psxy -R -J label_places1.txt -Sklat_label/5c -O -K -W1p >> $ps
# insert custom symbol at 3 cm size
psxy -R -J label_places2.txt -Sklat_label/3c -O -W1p,red -Glightblue >> $ps
