#!/usr/bin/env bash
gmt begin GMT_color_interpolate
	gmt basemap -Jx1i -R0/6.8/0/2.0 -B0

# Plot polar color map in the left; right (top) and wrong (bottom)
	gmt makecpt -Cpolar -T-1/1
	gmt colorbar -D1.7/1.6+w3i/0.3i+h+jTC -C -B0.5f0.1
	gmt colorbar -D1.7/0.7+w3i/0.3i+h+jTC -C -B0.5f0.1 --COLOR_MODEL=hsv

# Plot rainbow color map in the left; right (top) and wrong (bottom)
	gmt makecpt -Crainbow -T-1/1
	gmt colorbar -D5.1/1.6+w3i/0.3i+h+jTC -C -B0.5f0.1
	gmt colorbar -D5.1/0.7+w3i/0.3i+h+jTC -C -B0.5f0.1 --COLOR_MODEL=rgb

	gmt plot -Sd0.1i -Wblack -Gwhite << END
0.2 1.6
1.7 1.6
3.2 1.6
3.6 1.6
6.6 1.6
0.2 0.7
1.7 0.7
3.2 0.7
3.6 0.7
6.6 0.7
END

	gmt text -F+f14p,Helvetica-Bold+jBC << END
1.7 1.7 polar (RGB)
1.7 0.8 polar (HSV)
5.1 1.7 rainbow (HSV)
5.1 0.8 rainbow (RGB)
END
gmt end show
