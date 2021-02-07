#!/usr/bin/env bash
# Test that the direction for decorated lines follows the direction
# and not orientation of a line.  See https://github.com/GenericMappingTools/gmt/issues/4763
# for background.  This test is based on the one submitted by Kristof Koch
cat > dir.def << END
-0.5   0.25 M
 0.5   0    D
-0.5  -0.25 D
-0.5   0.25  0.2 c -W- -Gred
-0.5  -0.25  0.2 c -W- -Ggreen
END

cat > pathWE+ap.txt << END
>
-0.5   0.4
 0.5   0.6
>
-0.5   0
 0.5   0
>
-0.5  -0.4
 0.5  -0.6
END

cat > pathEW+ap.txt << END
>
 0.5   0.6
-0.5   0.4
>
 0.5   0
-0.5   0
>
 0.5  -0.6
-0.5  -0.4
END

cat > pathWE_v+ap.txt << END
-0.5  0.5
 0    0.5
 0   -0.5
 0.5 -0.5
END

cat > pathEW_v+ap.txt << END
 0.5 -0.5
 0   -0.5
 0    0.5
-0.5  0.5
END

cat > pathWE_n+ap.txt << END
-0.5  0.5
-0.5 -0.5
 0.5  0.5
 0.5 -0.5
END

cat > pathEW_n+ap.txt << END
 0.5 -0.5
 0.5  0.5
-0.5 -0.5
-0.5  0.5
END

cat > pathWE_z+ap.txt << END
-0.5  0.5
 0.5  0.5
-0.5 -0.5
 0.5 -0.5
END

cat > pathEW_z+ap.txt << END
 0.5 -0.5
-0.5 -0.5
 0.5  0.5
-0.5  0.5
END

gmt begin symbol_orientation_cart ps
  gmt subplot begin 4x2 -Fs5.5c -M5p -A -Bwsentr -Bag1d -R-1/1/-1/1 -JX5.5c -X5c -Y1c -T"Symbol Orientation @%10%-S~:+sk+ap@%%"
    gmt subplot set
      gmt basemap
      gmt plot pathWE+ap.txt -W2p,darkgrey,8_4 -S~n1:+skdir/1c+p1p,black+gwhite+ap
      gmt plot pathWE+ap.txt -SqN-1:+i+l" START "
      gmt plot pathWE+ap.txt -SqN+1:+i+l" END "
    gmt subplot set
      gmt basemap
      gmt plot pathEW+ap.txt -W2p,darkgrey,8_4 -S~n1:+skdir/1c+p1p,black+gwhite+ap
      gmt plot pathEW+ap.txt -SqN-1:+i+l" START "
      gmt plot pathEW+ap.txt -SqN+1:+i+l" END "
    gmt subplot set
      gmt basemap
      gmt plot pathWE_v+ap.txt -W2p,darkgrey,8_4 -S~n1:+skdir/1c+p1p,black+gwhite+ap
      gmt plot pathWE_v+ap.txt -SqN-1:+i+l" START "
      gmt plot pathWE_v+ap.txt -SqN+1:+i+l" END "
    gmt subplot set
      gmt basemap
      gmt plot pathEW_v+ap.txt -W2p,darkgrey,8_4 -S~n1:+skdir/1c+p1p,black+gwhite+ap
      gmt plot pathEW_v+ap.txt -SqN-1:+i+l" START "
      gmt plot pathEW_v+ap.txt -SqN+1:+i+l" END "
    gmt subplot set
      gmt basemap
      gmt plot pathWE_n+ap.txt -W2p,darkgrey,8_4 -S~n3:+skdir/1c+p1p,black+gwhite+ap
      gmt plot pathWE_n+ap.txt -SqN-1:+i+l" START "
      gmt plot pathWE_n+ap.txt -SqN+1:+i+l" END "
    gmt subplot set
      gmt basemap
      gmt plot pathEW_n+ap.txt -W2p,darkgrey,8_4 -S~n3:+skdir/1c+p1p,black+gwhite+ap
      gmt plot pathEW_n+ap.txt -SqN-1:+i+l" START "
      gmt plot pathEW_n+ap.txt -SqN+1:+i+l" END "
    gmt subplot set
      gmt basemap
      gmt plot pathWE_z+ap.txt -W2p,darkgrey,8_4 -S~n3:+skdir/1c+p1p,black+gwhite+ap
      gmt plot pathWE_z+ap.txt -SqN-1:+i+l" START "
      gmt plot pathWE_z+ap.txt -SqN+1:+i+l" END "
    gmt subplot set
      gmt basemap
      gmt plot pathEW_z+ap.txt -W2p,darkgrey,8_4 -S~n3:+skdir/1c+p1p,black+gwhite+ap
      gmt plot pathEW_z+ap.txt -SqN-1:+i+l" START "
      gmt plot pathEW_z+ap.txt -SqN+1:+i+l" END "
  gmt subplot end
gmt end show
