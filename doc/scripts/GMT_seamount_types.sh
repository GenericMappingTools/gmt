#!/usr/bin/env bash
# Plot five different seamount types selectable in grdseamount
# organized by smallest to largest volume for same r,h
gmt begin GMT_seamount_types
    gmt set GMT_THEME cookbook
    gmt set MAP_VECTOR_SHAPE 0.5
    # 1. Gaussian seamount
    echo "0	1" > tmp
    echo "1	1" >> tmp
    gmt math -T1/4/0.1 T 4 DIV 2 POW 4.5 MUL NEG EXP 0.25 2 POW 4.5 MUL EXP MUL = >> tmp
    gmt math -T-4/-1/0.1 T 4 DIV 2 POW 4.5 MUL NEG EXP 0.25 2 POW 4.5 MUL EXP MUL = body
    gmt math -T1/4/0.1 T 4 DIV 2 POW 4.5 MUL NEG EXP 0.25 2 POW 4.5 MUL EXP MUL = >> body
    gmt math -T-4/4/0.1 T 4 DIV 2 POW 4.5 MUL NEG EXP 0.25 2 POW 4.5 MUL EXP MUL = line
    gmt plot -R-5/5/-0.05/1.35 -JX6.5i/1.25i -Glightgray body -Y8i
    gmt plot -W2p tmp
    gmt plot -W0.5p,- line
    gmt plot -Sv0.1i+e+s -Gblack -W0.5p -N << EOF
-5	0	5	0
0	0	0	1.6
EOF
    gmt plot -W0.25p,- << EOF
>
-0.5	0.2
4.3	0.2
>
1	0
1	1
>
2.59	0
2.59	0.3
EOF
    gmt text -F+f16p,Times-Italic+j -N << EOF
1	-0.05	TC	r@-t@- = fr@-0@-
4	-0.05	TC	r@-0@-
4.3	0.2	LM	h@-n@-
2.59	-0.05	TC	r@-c@-
-0.2	1	RM	h@-0@-
EOF
    echo "@%1%g@%% (Gaussian)" | gmt text -F+f18p+cTL -Dj0.1i
    # 2. Polynomial seamount
    echo "0	1" > tmp
    echo "1	1" >> tmp
h=$(gmt math -Q 0.25 STO@U 1 ADD 3 POW 1 @U SUB 3 POW MUL @U 3 POW 1 ADD DIV INV =)
    gmt math -T1/4/0.1 T 4 DIV STO@U 1 ADD 3 POW 1 @U SUB 3 POW MUL @U 3 POW 1 ADD DIV $h MUL = >> tmp
    gmt math -T-4/-1/0.1 T 4 DIV STO@U 1 ADD 3 POW 1 @U SUB 3 POW MUL @U 3 POW 1 ADD DIV $h MUL = | sed -e 's/NaN/0/g' > body
    gmt math -T1/4/0.1 T 4 DIV STO@U 1 ADD 3 POW 1 @U SUB 3 POW MUL @U 3 POW 1 ADD DIV $h MUL = >> body
    gmt math -T-4/4/0.1 T 4 DIV STO@U 1 ADD 3 POW 1 @U SUB 3 POW MUL @U 3 POW 1 ADD DIV $h MUL = | sed -e 's/NaN/0/g' > line
    gmt plot -R-5/5/-0.05/1.35 -Glightgray body -Y-1.6i
    gmt plot -W2p tmp
    gmt plot -W0.5p,- line
    gmt plot -Sv0.1i+e+s -Gblack -W0.5p -N << EOF
-5	0	5	0
0	0	0	1.6
EOF
    gmt plot -W0.25p,- << EOF
>
-0.5	0.2
4.3	0.2
>
1	0
1	1
>
2.59	0
2.59	0.3
EOF
    gmt text -F+f16p,Times-Italic+j -N << EOF
1	-0.05	TC	r@-t@- = fr@-0@-
4	-0.05	TC	r@-0@-
4.3	0.2	LM	h@-n@-
2.59	-0.05	TC	r@-c@-
-0.2	1	RM	h@-0@-
EOF
    echo "@%1%o@%% (polynomial)" | gmt text -F+f18p+cTL -Dj0.1i
# 3. Conical seamount
cat << EOF > tmp
0	1
1	1
4	0
EOF
cat << EOF > body
-4	0
-1	1
0	1
1	1
4	0
EOF
    gmt plot -R-5/5/-0.05/1.5 -Glightgray body -Y-1.6i
    gmt plot -W2p tmp
    gmt plot -W0.5p,- << EOF
-4	0
0	1.33333
4	0
EOF
        gmt plot -Sv0.1i+e+s -Gblack -W0.5p -N << EOF
-5	0	5	0
0	0	0	1.7
EOF
gmt plot -W0.25p,- << EOF
>
-0.5	0.2
4.3	0.2
>
1	0
1	1
>
3.4	0
3.4	0.3
EOF
    gmt text -F+f16p,Times-Italic+j -N << EOF
1	-0.05	TC	r@-t@- = fr@-0@-
4	-0.05	TC	r@-0@-
3.4	-0.05	TC	r@-c@-
4.3	0.2	LM	h@-n@-
-0.2	1	RM	h@-0@-
EOF
    echo "@%1%c@%% (cone)" | gmt text -F+f18p+cTL -Dj0.1i
    # 4. Parabolic seamount
    echo "0	1" > tmp
    echo "1	1" >> tmp
    gmt math -T1/4/0.1 T 4 DIV 2 POW NEG 1 ADD 1 0.25 2 POW SUB DIV = >> tmp
    gmt math -T-4/-1/0.1 T 4 DIV 2 POW NEG 1 ADD 1 0.25 2 POW SUB DIV = body
    gmt math -T1/4/0.1 T 4 DIV 2 POW NEG 1 ADD 1 0.25 2 POW SUB DIV = >> body
    gmt math -T-4/4/0.1 T 4 DIV 2 POW NEG 1 ADD 1 0.25 2 POW SUB DIV = line
    gmt plot -Glightgray body -Y-1.4i
    gmt plot -W2p tmp
    gmt plot -W0.5p,- line
    gmt plot -Sv0.1i+e+s -Gblack -W0.5p -N << EOF
-5	0	5	0
0	0	0	1.5
EOF
    gmt plot -W0.25p,- << EOF
>
-0.5	0.2
4.3	0.2
>
1	0
1	1
>
3.60555	0
3.60555	0.3
EOF
    gmt text -F+f16p,Times-Italic+j -N << EOF
1	-0.05	TC	r@-t@- = fr@-0@-
4	-0.05	TC	r@-0@-
3.60555	-0.05	TC	r@-c@-
4.3	0.2	LM	h@-n@-
-0.2	1	RM	h@-0@-
EOF
    echo "@%1%p@%% (parabola)" | gmt text -F+f18p+cTL -Dj0.1i
    # 5. Disc
cat << EOF > tmp
0	1
4	1
4	0
EOF
cat << EOF > body
-4	0
-4	1
4	1
4	0
EOF
    gmt plot -Glightgray body -Y-1.45i
    gmt plot -W2p tmp
    gmt plot -Sv0.1i+e+s -Gblack -W0.5p -N << EOF
-5	0	5	0
0	0	0	1.4
EOF
    gmt plot -W0.25p,- << EOF
>
-0.5	0.2
4.3	0.2
EOF
    gmt text -F+f16p,Times-Italic+j -N << EOF
4	-0.05	TC	r@-0@- = r@-c@-
4.3	0.2	LM	h@-n@-
-0.2	1	RM	h@-0@-
EOF
    echo "@%1%d@%% (disc)" | gmt text -F+f18p+cTL -Dj0.1i
gmt end show
rm -f tmp body line
