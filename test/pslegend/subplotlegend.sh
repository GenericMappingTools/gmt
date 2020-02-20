#!/usr/bin/env bash
# Test that legends in subplots work with -C and that
# origins are reset for other overlays.
# Test added due to https://github.com/GenericMappingTools/gmt/issues/1590

gmt begin subplotlegend ps
    gmt subplot begin 2x2 -Fs7c/5c -A

    gmt subplot set 0
    gmt basemap -R0/10/0/10
    gmt legend -DjRT+w1.2c -F+p0.5p << EOF
S 0.1c c 0.1c red - 0.3c a
S 0.1c s 0.1c blue - 0.3c b
EOF
    echo 5 5 | gmt plot -Sc0.5c -Gred

    gmt subplot set 1 -Cw1c -Cs1c
    #gmt subplot set 1
    gmt basemap -R0/10/0/10
    gmt legend -DjRT+w1.2c -F+p0.5p << EOF
S 0.1c c 0.1c red - 0.3c a
S 0.1c s 0.1c blue - 0.3c b
EOF
    echo 5 5 | gmt plot -Sc0.5c -Gred

    gmt subplot set 2 -Ce1c
    #gmt subplot set 1
    gmt basemap -R0/10/0/10
    gmt legend -DjRT+w1.2c -F+p0.5p << EOF
S 0.1c c 0.1c red - 0.3c a
S 0.1c s 0.1c blue - 0.3c b
EOF
    echo 5 5 | gmt plot -Sc0.5c -Gred

    gmt subplot set 3 -Ce1c -Cn2c
    #gmt subplot set 1
    gmt basemap -R0/10/0/10
    gmt legend -DjRT+w1.2c -F+p0.5p << EOF
S 0.1c c 0.1c red - 0.3c a
S 0.1c s 0.1c blue - 0.3c b
EOF
    echo 5 5 | gmt plot -Sc0.5c -Gred
    gmt subplot end
gmt end show
