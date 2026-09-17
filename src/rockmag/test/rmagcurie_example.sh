#!/usr/bin/env bash
#
#       Example / sanity check for rmagcurie
#
# Two synthetic, noise-free thermomagnetic runs, each built so its Curie
# temperature is known exactly (Tc = 500), to check that each method recovers
# it independently:
#
#   Specimen-A: column 2 follows the exact Curie-Weiss law chi = 1000/(T-500)
#               above Tc (paramagnetic tail only, all 8 points chosen so
#               1/chi is an exact terminating decimal) -> -C should recover
#               Tc = 500 with R^2 = 1 (to floating-point precision).
#   Specimen-B: column 2 is two exact straight lines that cross at (T=500,
#               M=0) - a steep "flank" below Tc and a shallow "baseline"
#               above it -> -B/-S should recover Tc = 500 with R^2 = 1 for
#               both fits. (The shape above Tc is not meant to be physically
#               realistic - a real susceptibility baseline is not a straight
#               line, see rmagcurie.c and Petrovsky & Kapicka, 2006 - this
#               is only to exercise the two-tangent intersection formula
#               against an exact, independently-checkable answer.)

cat > rmagcurie_cw_data.txt << EOF
> Specimen-A -L"exact Curie-Weiss tail, Tc=500"
520 50
525 40
540 25
550 20
600 10
625 8
700 5
750 4
EOF

echo "--- Curie-Weiss method: should read Tc = 500, R^2 = 1, fitted over all 8 points ---"
gmt rmagcurie rmagcurie_cw_data.txt -C520/750

cat > rmagcurie_tangent_data.txt << EOF
> Specimen-B -L"two exact lines crossing at T=500, M=0"
350 300
400 200
450 100
480 40
495 10
510 1
550 5
600 10
650 15
700 20
EOF

echo "--- Two-tangent method: should read Tc = 500, R^2 = 1 for both fits (5 points each) ---"
gmt rmagcurie rmagcurie_tangent_data.txt -B510/700 -S350/495
