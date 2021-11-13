#!/usr/bin/env bash
# Ensure that -Dj works for pstext -M
# See https://github.com/GenericMappingTools/gmt/issues/5981
gmt begin para_offset
	# First one in the center
    gmt text  -R-8/8/-8/8 -Jx1c -Bafg3 -M -F+f+a+j -C0 -Glightpink <<- EOF
	> 0 0 12p,Times-Bold,black 0 CM 0.32c 3c c
	Original  X
	Multiline.
	EOF
	# Then place then at the other 8 justification points
    gmt text -M -F+f+a+j -Dj3c -C0 -Glightgray <<- EOF
	> 0 0 12p,Times-Bold,black 0 TR 0.32c 3c c
	TR Offset   X
	Multiline.
	> 0 0 12p,Times-Bold,black 0 TL 0.32c 3c c
	TL Offset   X
	Multiline.
	> 0 0 12p,Times-Bold,black 0 BR 0.32c 3c c
	BR Offset   X
	Multiline.
	> 0 0 12p,Times-Bold,black 0 BL 0.32c 3c c
	BL Offset   X
	Multiline.
	> 0 0 12p,Times-Bold,black 0 TC 0.32c 3c c
	TC Offset   X
	Multiline.
	> 0 0 12p,Times-Bold,black 0 BC 0.32c 3c c
	BC Offset   X
	Multiline.
	> 0 0 12p,Times-Bold,black 0 ML 0.32c 3c c
	ML Offset   X
	Multiline.
	> 0 0 12p,Times-Bold,black 0 MR 0.32c 3c c
	MR Offset   X
	Multiline.
	EOF
gmt end show
