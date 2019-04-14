#!/bin/bash
# This script demonstrates the problem with the basic line rendering
# algorithm in ghostscript for fat lines.
# https://github.com/GenericMappingTools/gmt/issues/431
cat > gc.d << END
-85.5 85
-4.5  85
END
gmt begin GMT_fatline ps
	gmt psxy -R-85/82/-5/87+r -JM-45/84.5/2.5i -W30p gc.d
	gmt psxy -W1p,red gc.d
	gmt psxy -X3.25i -W30p gc.d --PS_LINE_CAP=round --PS_LINE_JOIN=round
	gmt psxy -W1p,red gc.d
gmt end
