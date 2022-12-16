#!/usr/bin/env bash
# Test the new +e modifier to -B
gmt begin skipannots ps
  gmt set MAP_FRAME_TYPE plain PS_MEDIA letter
  echo "-Bx -By" | gmt text -R0/80/0/30 -Jx0.1c -Bx -By -F+cCM+f14p -Y1.25c
  echo "-Bx+e -By" | gmt text -Bx+e -By -Y3.75c -F+cCM+f14p
  echo "-Bx+el -By" | gmt text -Bx+el -By -Y3.75c -F+cCM+f14p
  echo "-Bx+eu -By" | gmt text -Bx+eu -By -Y3.75c -F+cCM+f14p
  echo "-Bx -By+e" | gmt text -Bx -By+e -Y3.75c -F+cCM+f14p
  echo "-Bx -By+el" | gmt text -Bx -By+el -Y3.75c -F+cCM+f14p
  echo "-Bx -By+eu" | gmt text -Bx -By+eu -Y3.75c -F+cCM+f14p
  echo "-Bx -By" | gmt text -R0/60/0/30 -Jq0.1c -Bx -By -X10c -F+cCM+f14p
  echo "-Bx+e -By+el" | gmt text -Bx+e -By+el -Y-3.75c -F+cCM+f14p
  echo "-Bx+e -By+eu" | gmt text -Bx+e -By+eu -Y-3.75c -F+cCM+f14p
  echo "-Bx+e -By+e" | gmt text -Bx+e -By+e -Y-3.75c -F+cCM+f14p
  echo "-Bx+el -By+el" | gmt text -Bx+el -By+el -Y-3.75c -F+cCM+f14p
  echo "-Bx+el -By+eu" | gmt text -Bx+el -By+eu -Y-3.75c -F+cCM+f14p
  echo "-Bx+eu -By+eu" | gmt text -Bx+eu -By+eu -Y-3.75c -F+cCM+f14p
gmt end show
