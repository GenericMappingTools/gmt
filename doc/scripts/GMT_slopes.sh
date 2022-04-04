#!/usr/bin/env bash
# Show gmtregress -A option in effect
gmt begin GMT_slopes
    gmt set GMT_THEME cookbook  
cat << EOF > data
5.2957	-19.5631
7.087	-36.0337
4.7318	-22.9612
10.7389	-43.3934
13.2405	-12.6665
10.8035	-28.1462
17.9197	-13.7448
5.0468	-30.0907
6.6409	-20.6262
2.0459	-38.0396
0.0963	-45.2655
15.1622	-25.3824
14.108	-30.4538
15.1111	-32.6112
5.07	-26.6111
8.81	-25.579
5.89	-31.2512
11.28	-37.4959
EOF

    gmt regress data -A-90/90/0.1 -Eo -Nr > tmp.txt
    gmt plot tmp.txt -R-90/90/0/35 -JX15c/5c -Bxa30+u@.+l"Regression line angle, @~a@~" -Byaf+l"Misfit, E(@~a@~)" -BWSrt -W0.75p
    echo -90 5.29462 90 5.29462 | gmt plot -Sv12p+s -W0.25p,dashed
    gmt plot -Sv12p+b -Gblack -W0.5p --MAP_VECTOR_SHAPE=0.5 << EOF
78.6 6.32 90 2.6c
-78.3 5.3 90 2.7c
EOF
    gmt text -F+f12p+jCB -Dj0/0.1c << EOF
78.6 25 78.6@.
-78.3 25 -78.3@.
EOF
gmt end show
rm -f data tmp.txt
