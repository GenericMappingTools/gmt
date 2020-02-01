#!/bin/bash
# This script demonstrates the problem with the basic line rendering
# algorithm in ghostscript for fat lines.
# https://github.com/GenericMappingTools/gmt/issues/431
cat > gc.d << END
-82 85
-8  85
END
gmt begin GMT_fatline
	gmt plot -R-90/82/0/87+r -JM-45/84.5/2.5i -W30p gc.d
	gmt plot -W1p,red gc.d
	gmt plot -X3.25i -W30p gc.d --PS_LINE_CAP=round --PS_LINE_JOIN=round
	gmt plot -W1p,red gc.d
gmt end show
