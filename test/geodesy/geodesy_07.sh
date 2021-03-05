#!/usr/bin/env bash
# Test various combinations of pen/fill/cpt color in velo, using a small dataset
# provided by Katherine Guns
cat << EOF > GPS_test.txt
235.6036	40.4414	-0.598	3.184	0.026	0.012	1e-05	CME6
235.6036	40.4417	-0.537	3.142	0.024	0.013	1e-05	CME5
235.6036	40.4417	-0.798	3.109	0.026	0.007	1e-05	CME1
235.6919	40.2475	-1.89	3.287	0.014	0.005	1e-05	P157
235.7172	40.5047	-0.231	2.678	0.023	0.008	1e-05	P159
235.7629	40.691	0.267	2.143	0.026	0.008	1e-05	P162
235.7869	40.6373	0.105	2.147	0.02	0.018	1e-05	P161
235.8667	40.5512	-0.229	2.137	0.025	0.006	1e-05	P160
235.8927	40.4224	-0.527	2.307	0.013	0.034	1e-05	P158
235.9246	40.8763	0.18	1.724	0.017	0.008	1e-05	P058
235.9427	40.2195	-1.437	2.486	0.01	0.008	1e-05	P163
236.0323	40.7911	0.071	1.605	0.018	0.005	1e-05	P169
236.0938	40.0244	-1.974	2.438	0.009	0.006	1e-05	P156
236.1185	40.6686	-0.168	1.493	0.01	0.018	1e-05	P168
236.1198	40.5437	-0.362	1.616	0.011	0.006	1e-05	P167
236.1367	40.8802	-0.051	1.184	0.052	0.045	1e-05	P170
236.1371	40.4351	-0.565	1.685	0.011	0.006	1e-05	P166
236.1467	40.2455	-1.147	1.827	0.02	0.008	1e-05	P165
236.3010	40.5753	-0.592	0.994	0.02	0.005	1e-05	P326
236.3066	40.1192	-1.466	1.589	0.009	0.004	1e-05	P164
236.3442	40.2568	-1.093	1.454	0.021	0.007	1e-05	P324
236.4268	40.4787	-0.801	0.974	0.018	0.011	1e-05	P793
236.4269	40.4788	-0.769	0.95	0.031	0.015	1e-05	P327
236.5479	40.0822	-1.425	1.283	0.013	0.01	1e-05	P329
EOF
gmt begin -C geodesy_07 ps
	gmt makecpt -Cturbo -T0/5
	gmt set MAP_FRAME_TYPE plain
	gmt subplot begin 4x2 -Fs5c -JM5c -R-124.6/-123.3/40/41 -A+jTR+gwhite -SCb -SRl+p -BWSrt -M6p -Ba1
		gmt velo GPS_test.txt -A10p+e+a40+n6k+h0.5   -Se0.50/0/0 -Gorange -W1.5p -c
		gmt velo GPS_test.txt -A10p+e+a40+n6k+h0.5   -Se0.50/0/0 -Gorange -W1.5p,orange -c
		gmt velo GPS_test.txt -A10p+e+a40+n6k+h0.5   -Se0.50/0/0 -C -W1.5p -c
		gmt velo GPS_test.txt -A10p+e+a40+n6k+h0.5   -Se0.50/0/0 -C -W1.5p+c -c
		gmt velo GPS_test.txt -A10p+e+a40+n6k+h0.5+p -Se0.50/0/0 -C -W1.5p+c -c
		gmt velo GPS_test.txt -A10p+e+a40+n6k+h0.5+p -Se0.50/0.95/0 -D10 -C -W1.5p+c -c
		gmt velo GPS_test.txt -A10p+e+a40+n6k+h0.5+p -Se0.50/0.95/0 -D10 -C -W1.5p+c -L0.25p -Elightgray -c
		gmt velo GPS_test.txt -A10p+e+a40+n6k+h0.5+p -Se0.50/0.0/0 -D10 -C -W1.5p+c
		gmt velo GPS_test.txt -A10p+e+a40+n6k+h0.5+p -Se0.50/0.95/0 -D10 -C -W1.5p+c -L0.25p+c -c -Zn
	gmt subplot end
gmt end show
