#!/usr/bin/env bash
#
ps=seis_10.ps

# Added in response to issue #894.  Kurt Feigl wrote back:
#I checked Aki and Richards, p. 106:
#
#Rake \lambda is the angel between strike direction and slip [such that] -\pi .lt. \lambda .le. \pi.
#If dip \delta is neither 0 nor \pi/2, and lambda is within the range (0, \pi), the fault is termed a reverse fault or a thrust fault.
#However, if \lambda is within the range (-\pi, 0), the fault is termed a normal fault.
#Since both ranges for \lambda include 0, the situation seems ambiguous.
#In this case, we have dip = 90 for the second plane, so the first part of the conjunctive condition in the "if" statement is not satisfied.
#The Centroid Moment Tensor (CMT) shown in red is a normal faulting mechanism, with white (dilatational arrivals) in the center, arguing for a negative value of lambda.
#Since zero is neither positive nor negative, we can only appeal to consistency. Using the script below, I find the attached figure. I agree, that Case B is plotting incorrectly. It should look like Case D. If you have the time to hunt for the bug, that would be great.

# Case B should look like Case D
gmt psmeca -JM6i -R116/127/23/26 -Baf -Sa2c -Fa0.2c/cc -P -K << EOF > $ps
118.68 24.39 15.0 351 81 -0.001  5.4 0 0 Arake=-0.001
120.68 24.39 15.0 351 81      0  5.4 0 0 Brake=0
122.68 24.39 15.0 351 81  0.001  5.4 0 0 Crake=0.001
124.68 24.39 15.0  81 90 -171    5.4 0 0 Drake=-171
EOF

# Paul then added this case with rakes straddling 180 just to make sure nothing fishy happening there.
gmt psmeca -J -R -Baf -Sa2c -Fa0.2c/cc -O -Y3i << EOF >> $ps
118.68 24.39 15.0 351 81 179.999 5.4 0 0 Arake=179.999
120.68 24.39 15.0 351 81 180     5.4 0 0 Brake=180
122.68 24.39 15.0 351 81 180.001 5.4 0 0 Crake=180.001
124.68 24.39 15.0  81 90 -171    5.4 0 0 Drake=-171
EOF
