# v2 port of Mirone's testa_barnabeu.m — same benchmark as examples/testa_barnabeu.jl, rewritten to
# keep everything as GMTgrid end-to-end instead of separate (Matrix,head) pairs, and to use GMT's
# own `sample1d` (its `interp1`) instead of a hand-rolled interpolator.
# Run: julia --project=. examples/testa_barnabeu2.jl
#
# Bathymetry/source/nested grids are passed to `nswing` as in-memory GMTgrid objects via GMT.jl's
# generic virtual-file mechanism: `gmt(cmd, G1, G2, ...)` with NO filename text at all in `cmd` for
# them (neither positional bat/src nor the flagged -1/-2/... nest levels) — GMT's option-encoding
# fills each unattached grid slot from the trailing args, in order. CONFIRMED empirically (2026-07-07)
# for both the two required positional grids and bare `-1`/`-2` nest flags — real files are never
# written for any of these. `-T<mareg>` (maregraph position) is DIFFERENT: nswing parses that flag's
# value itself (sscanf/fopen, not GMT_Read_Data), so it does NOT support a Matrix/GMTdataset given
# as a trailing arg — tested, both a bare `-T` and a Matrix arg fail (nswing mis-parses the next
# token, e.g. `+o...`, as if it were `-T`'s own filename). `-T` must stay a literal `x/y` string (or
# point at a real positions file on disk) — that part alone still isn't "pure GMTgrid/GMTdataset".
# `-G` output also stays on disk (nswing writes timestepped .grd files by path, no single in-memory
# return value fits that many-file output). Everything else (grid construction, read-back of
# results) stays GMTgrid the whole way, no manual Matrix/head juggling anywhere.

using GMT

# --------------------------------------------------------------------------------
# Benchmark table: x(m), eta(m), u(m/s) triplets at t=0s, 160s, 175s, 220s (12 cols). Identical to
# examples/testa_barnabeu.jl's ANALYTIC — byte-for-byte copy of the .m `analytic()` literal.
const ANALYTIC = Float64[
50328.0	0.00000	0.00000	50316.9	1.10722	0.04919	50317.9	1.00656	0.04919	50324.5	0.35230	0.02108
49326.5	0.00000	0.00000	49315.4	1.10722	0.04919	49316.9	0.95623	0.04919	49324.0	0.25164	0.01405
48335.0	0.00000	0.00000	48324.4	1.05689	0.04919	48326.0	0.90590	0.04216	48334.0	0.10066	0.00703
47353.6	0.00000	0.00000	47343.0	1.05689	0.04919	47345.1	0.85558	0.04216	47355.1	-0.15098	0.00000
46382.3	0.00000	0.00000	46372.2	1.00656	0.04919	46374.2	0.80525	0.04216	46386.3	-0.40262	-0.01405
45421.0	0.00000	0.00000	45411.5	0.95623	0.04919	45414.0	0.70459	0.03513	45429.1	-0.80525	-0.03513
44469.8	0.00000	0.00000	44460.8	0.90590	0.04216	44463.3	0.65426	0.03513	44481.9	-1.20787	-0.04919
43528.7	0.00000	0.00000	43520.6	0.80525	0.04216	43522.6	0.60394	0.03513	43545.3	-1.66082	-0.07729
42597.6	0.00000	0.00000	42590.1	0.75492	0.04216	42592.6	0.50328	0.02811	42618.3	-2.06345	-0.09837
41676.6	0.00000	0.00000	41669.6	0.70459	0.03513	41672.1	0.45295	0.02811	41700.8	-2.41574	-0.11242
40765.7	0.00000	0.00000	40759.6	0.60394	0.03513	40762.2	0.35230	0.02108	40791.9	-2.61706	-0.12648
39864.8	0.00000	0.00000	39859.3	0.55361	0.03513	39862.3	0.25164	0.02108	39891.5	-2.66738	-0.13350
38974.0	0.00000	0.00000	38969.5	0.45295	0.02811	38973.5	0.05033	0.00703	39000.2	-2.61706	-0.12648
38093.3	0.00000	0.00000	38089.2	0.40262	0.02811	38094.8	-0.15098	0.00000	38117.4	-2.41574	-0.11945
37222.6	0.00000	0.00000	37219.6	0.30197	0.02108	37227.1	-0.45295	-0.01405	37243.7	-2.11378	-0.10540
36361.5	0.05033	0.00000	36360.5	0.15098	0.01405	36370.5	-0.85558	-0.04216	36380.1	-1.81181	-0.09134
35510.9	0.05033	0.00000	35512.4	-0.10066	0.00000	35524.5	-1.30853	-0.06324	35526.0	-1.45951	-0.07729
34670.0	0.10066	0.00000	34674.5	-0.35230	-0.01405	34689.1	-1.81181	-0.09134	34682.5	-1.15754	-0.06324
33839.0	0.15098	0.00000	33848.1	-0.75492	-0.03513	33862.7	-2.21443	-0.11242	33849.6	-0.90590	-0.04919
33018.2	0.20131	0.00000	33032.3	-1.20787	-0.05621	33045.9	-2.56673	-0.13350	33026.7	-0.65426	-0.03513
32207.4	0.25164	0.00000	32226.5	-1.66082	-0.08432	32238.1	-2.81837	-0.14756	32215.0	-0.50328	-0.02811
31406.2	0.35230	0.00000	31431.3	-2.16410	-0.11242	31438.4	-2.86870	-0.15458	31413.2	-0.35230	-0.02108
30614.5	0.50328	0.00000	30644.7	-2.51640	-0.14053	30647.2	-2.76804	-0.15458	30622.1	-0.25164	-0.01405
29832.9	0.65426	0.00000	29867.7	-2.81837	-0.15458	29864.6	-2.51640	-0.14756	29841.5	-0.20131	-0.01405
29061.4	0.80525	0.00000	29098.6	-2.91902	-0.16864	29091.6	-2.21443	-0.12648	29071.0	-0.15098	-0.01405
28299.4	1.00656	0.00000	28338.2	-2.86870	-0.16864	28328.1	-1.86214	-0.11242	28310.5	-0.10066	-0.00703
27547.5	1.20787	0.00000	27586.3	-2.66738	-0.15458	27574.7	-1.50984	-0.09134	27560.6	-0.10066	-0.00703
26805.7	1.40918	0.00000	26843.4	-2.36542	-0.14053	26831.9	-1.20787	-0.07729	26820.3	-0.05033	-0.00703
26073.4	1.66082	0.00000	26110.2	-2.01312	-0.12648	26099.1	-0.90590	-0.05621	26090.5	-0.05033	-0.00703
25351.7	1.86214	0.00000	25387.0	-1.66082	-0.10540	25377.4	-0.70459	-0.04919	25370.8	-0.05033	-0.00703
24640.1	2.06345	0.00000	24673.8	-1.30853	-0.08432	24665.8	-0.50328	-0.03513	24661.2	-0.05033	-0.00703
23938.5	2.26476	0.00000	23971.2	-1.00656	-0.07027	23964.7	-0.35230	-0.02811	23961.7	-0.05033	-0.00703
23247.5	2.41574	0.00000	23279.2	-0.75492	-0.05621	23274.2	-0.25164	-0.02108	23272.2	-0.05033	-0.00703
22566.6	2.56673	0.00000	22597.8	-0.55361	-0.04216	22594.3	-0.20131	-0.01405	22592.7	-0.05033	-0.00703
21896.2	2.66738	0.00000	21926.9	-0.40262	-0.03513	21924.4	-0.15098	-0.01405	21923.4	-0.05033	-0.00703
21236.4	2.71771	0.00000	21266.6	-0.30197	-0.02811	21264.6	-0.10066	-0.01405	21264.1	-0.05033	-0.00703
20587.2	2.71771	0.00000	20616.4	-0.20131	-0.02108	20615.4	-0.10066	-0.00703	20614.9	-0.05033	-0.00703
19948.5	2.66738	0.00000	19976.7	-0.15098	-0.01405	19975.7	-0.05033	-0.00703	19975.7	-0.05033	-0.00703
19319.9	2.61706	0.00000	19347.1	-0.10066	-0.01405	19346.6	-0.05033	-0.00703	19346.6	-0.05033	-0.00703
18701.9	2.51640	0.00000	18728.1	-0.10066	-0.01405	18727.6	-0.05033	-0.00703	18727.6	-0.05033	-0.00703
18093.9	2.41574	0.00000	18118.6	-0.05033	-0.01405	18118.6	-0.05033	-0.00703	18118.6	-0.05033	-0.00703
17496.5	2.26476	0.00000	17519.7	-0.05033	-0.00703	17519.7	-0.05033	-0.00703	17519.7	-0.05033	-0.01405
16909.2	2.11378	0.00000	16930.8	-0.05033	-0.01405	16930.8	-0.05033	-0.00703	16930.8	-0.05033	-0.01405
16331.9	1.96279	0.00000	16352.1	-0.05033	-0.00703	16352.1	-0.05033	-0.00703	16352.1	-0.05033	-0.01405
15765.2	1.76148	0.00000	15783.4	-0.05033	-0.01405	15783.4	-0.05033	-0.01405	15783.9	-0.10066	-0.01405
15208.1	1.61050	0.00000	15224.7	-0.05033	-0.01405	15224.7	-0.05033	-0.01405	15225.2	-0.10066	-0.01405
14661.6	1.40918	0.00000	14676.1	-0.05033	-0.01405	14676.1	-0.05033	-0.01405	14676.7	-0.10066	-0.01405
14125.1	1.20787	0.00000	14137.6	-0.05033	-0.01405	14137.6	-0.05033	-0.01405	14138.1	-0.10066	-0.01405
13598.6	1.00656	0.00000	13609.2	-0.05033	-0.01405	13609.2	-0.05033	-0.01405	13609.7	-0.10066	-0.01405
13082.8	0.75492	0.00000	13090.8	-0.05033	-0.01405	13090.8	-0.05033	-0.01405	13091.3	-0.10066	-0.01405
12578.0	0.40262	0.00000	12582.5	-0.05033	-0.01405	12582.5	-0.05033	-0.01405	12583.0	-0.10066	-0.02108
12084.3	-0.05033	0.00000	12083.8	0.00000	-0.01405	12084.3	-0.05033	-0.01405	12084.8	-0.10066	-0.02108
11603.1	-0.75492	0.00000	11595.6	0.00000	-0.02108	11596.1	-0.05033	-0.02108	11596.6	-0.10066	-0.02108
11134.1	-1.66082	0.00000	11117.5	0.00000	-0.02108	11118.0	-0.05033	-0.02108	11118.5	-0.10066	-0.02108
10677.6	-2.81837	0.00000	10648.9	0.05033	-0.02811	10649.9	-0.05033	-0.02108	10650.9	-0.15098	-0.02108
10232.2	-4.07657	0.00000	10190.4	0.10066	-0.03513	10191.9	-0.05033	-0.02811	10192.9	-0.15098	-0.02811
9797.4	-5.38510	0.00000	9742.0	0.15098	-0.04216	9743.5	0.00000	-0.02811	9745.0	-0.15098	-0.02811
9371.1	-6.54264	0.00000	9303.6	0.20131	-0.05621	9305.6	0.00000	-0.03513	9307.2	-0.15098	-0.02811
8952.3	-7.44854	0.00000	8874.8	0.30197	-0.06324	8877.4	0.05033	-0.04216	8879.4	-0.15098	-0.03513
8539.2	-7.90150	0.00000	8456.1	0.40262	-0.08432	8459.1	0.10066	-0.04919	8461.6	-0.15098	-0.03513
8132.0	-7.95182	0.00000	8046.9	0.55361	-0.09837	8050.5	0.20131	-0.06324	8054.5	-0.20131	-0.03513
7730.9	-7.59953	0.00000	7648.3	0.65426	-0.11945	7652.4	0.25164	-0.07729	7656.9	-0.20131	-0.04216
7336.3	-6.89494	0.00000	7259.3	0.80525	-0.14053	7263.8	0.35230	-0.09134	7269.4	-0.20131	-0.04216
6949.8	-5.98903	0.00000	6880.3	0.95623	-0.16864	6884.9	0.50328	-0.11242	6891.9	-0.20131	-0.04919
6572.8	-5.03280	0.00000	6511.4	1.10722	-0.19674	6516.5	0.60394	-0.13350	6524.5	-0.20131	-0.05621
6205.4	-4.02624	0.00000	6152.6	1.25820	-0.23187	6157.6	0.75492	-0.16161	6167.2	-0.20131	-0.06324
5849.6	-3.17066	0.00000	5803.8	1.40918	-0.25998	5808.9	0.90590	-0.19674	5819.9	-0.20131	-0.07729
5504.9	-2.41574	0.00000	5465.1	1.56017	-0.28809	5469.6	1.10722	-0.22485	5482.7	-0.20131	-0.08432
5171.2	-1.76148	0.00000	5137.0	1.66082	-0.32322	5141.0	1.25820	-0.26701	5155.1	-0.15098	-0.10540
4849.1	-1.25820	0.00000	4819.4	1.71115	-0.35133	4822.4	1.40918	-0.30214	4837.5	-0.10066	-0.12648
4538.6	-0.90590	0.00000	4511.9	1.76148	-0.38646	4514.4	1.50984	-0.34430	4530.0	-0.05033	-0.14756
4238.6	-0.60394	0.00000	4214.5	1.81181	-0.41456	4216.5	1.61050	-0.38646	4232.6	0.00000	-0.18269
3949.7	-0.40262	0.00000	3927.6	1.81181	-0.44267	3928.6	1.71115	-0.42862	3945.2	0.05033	-0.21782
3671.9	-0.30197	0.00000	3651.3	1.76148	-0.46375	3651.3	1.76148	-0.47078	3667.4	0.15098	-0.27403
3404.2	-0.20131	0.00000	3385.6	1.66082	-0.49186	3384.1	1.81181	-0.51293	3399.7	0.25164	-0.33025
3146.5	-0.10066	0.00000	3129.4	1.61050	-0.51293	3127.4	1.81181	-0.55509	3142.0	0.35230	-0.40051
2899.9	-0.10066	0.00000	2884.3	1.45951	-0.54104	2881.3	1.76148	-0.59725	2894.4	0.45295	-0.48483
2662.9	-0.05033	0.00000	2648.8	1.35886	-0.56212	2645.7	1.66082	-0.63941	2656.8	0.55361	-0.58320
2436.4	-0.05033	0.00000	2423.8	1.20787	-0.59023	2420.3	1.56017	-0.68157	2429.8	0.60394	-0.69562
2219.5	0.00000	0.00000	2208.9	1.05689	-0.61833	2204.9	1.45951	-0.73076	2213.4	0.60394	-0.82913
2013.1	0.00000	0.00000	2004.1	0.90590	-0.65346	2000.0	1.30853	-0.77994	2007.1	0.60394	-0.99074
1816.8	0.00000	0.00000	1809.8	0.70459	-0.69562	1805.8	1.10722	-0.83615	1811.8	0.50328	-1.16640
1630.6	0.00000	0.00000	1625.6	0.50328	-0.74481	1621.6	0.90590	-0.90642	1627.1	0.35230	-1.37719
1454.5	0.00000	0.00000	1451.5	0.30197	-0.79400	1447.4	0.70459	-0.99074	1453.0	0.15098	-1.61610
1288.4	0.00000	0.00000	1287.9	0.05033	-0.85723	1283.9	0.45295	-1.08911	1288.4	0.00000	-1.88310
1132.4	0.00000	0.00000	1135.4	-0.30197	-0.91345	1130.9	0.15098	-1.21559	1134.4	-0.20131	-2.16416
986.4	0.00000	0.00000	994.0	-0.75492	-0.94858	988.4	-0.20131	-1.37719	988.9	-0.25164	-2.45225
850.5	0.00000	0.00000	864.6	-1.40918	-0.93453	856.1	-0.55361	-1.58096	852.1	-0.15098	-2.71223
724.7	0.00000	0.00000	748.4	-2.36542	-0.83615	735.3	-1.05689	-1.82689	722.2	0.25164	-2.91600
609.0	0.00000	0.00000	645.7	-3.67394	-0.59023	626.6	-1.76148	-2.12903	598.9	1.00656	-3.02842
503.3	0.00000	0.00000	556.6	-5.33477	-0.14053	530.0	-2.66738	-2.48738	482.6	2.06345	-3.02140
407.7	0.00000	0.00000	481.1	-7.34789	0.56212	446.4	-3.87526	-2.90195	372.9	3.47263	-2.88087
322.1	0.00000	0.00000	417.2	-9.51199	1.51070	376.5	-5.43542	-3.37975	271.3	5.08313	-2.59981
246.6	0.00000	0.00000	361.9	-11.52511	2.60683	320.6	-7.39822	-3.90674	178.7	6.79428	-2.19227
181.2	0.00000	0.00000	311.5	-13.03495	3.67486	278.3	-9.71330	-4.51804	95.6	8.55576	-1.68636
125.8	0.00000	0.00000	265.7	-13.99118	4.58831	248.6	-12.28003	-5.24177	24.2	10.16626	-1.11721
80.5	0.00000	0.00000	225.0	-14.44414	5.29096	229.5	-14.89709	-6.18332	-36.2	11.67610	-0.53401
45.3	0.00000	0.00000	190.2	-14.49446	5.78984	218.4	-17.31283	-7.50431	-84.0	12.93430	0.01405
20.1	0.00000	0.00000	164.1	-14.39381	6.11306	212.9	-19.27562	-9.23985	-118.8	13.89053	0.46375
5.0	0.00000	0.00000	147.5	-14.24282	6.29575	209.4	-20.43317	-10.75758	-140.4	14.54479	0.75886
0.0	0.00000	0.00000	141.9	-14.19250	6.35196	207.9	-20.78546	-11.31970	-147.5	14.74610	0.85723
]

# --------------------------------------------------------------------------------
# `interp` is :linear (matches interp1 'linear') or :cubic (natural cubic
# spline — sample1d's "c", matches interp1 'spline' closer than any hand-rolled version would).
# sample1d returns NaN for query points outside the data's own x-range (with a warning); `fillval`
# replaces those NaNs afterwards — this reproduces `interp1(...,'linear'|'spline',fillval)` exactly,
# not sample1d's own (untested-here) extrapolation behaviour.
function gmt_interp1(xp::AbstractVector, yp::AbstractVector, xq::AbstractVector;
                      interp::Symbol=:linear, fillval::Float64=NaN)
	idx = sortperm(xp)
	ds = hcat(Float64.(xp)[idx], Float64.(yp)[idx])
	r = sample1d(ds; T=collect(Float64, xq), F=interp, V=:q)
	out = r.data[:, 2]
	isnan(fillval) || (out[isnan.(out)] .= fillval)
	return out
end

# --------------------------------------------------------------------------------
# cmp_horizont_coords: same two live dispatches as v1 (the .m source's 3rd nargin<2 branch
# references undefined vars even in MATLAB — dead code, not ported; see testa_barnabeu.jl).

# .m 2-arg form: cmp_horizont_coords([], dxOuter) — no grid involved at all.
function cmp_horizont_coords(dxOuter::Float64)
	x = collect(-200.0:dxOuter:50000.0)
	cEnd = round(Int, (800.0 + 200.0) / dxOuter) + 1
	xToPlot = collect(range(-200.0, 800.0, length=cEnd))
	return x, xToPlot, cEnd
end

# .m 3-arg form: cmp_horizont_coords(hdr_P, dxOuter, dxInner) — dxOuter unused (kept for parity).
# `Gp` replaces the raw 9-el `hdr_P` vector: x0/x1/dx come straight off the GMTgrid's own range/inc.
function cmp_horizont_coords(Gp::GMTgrid, dxOuter::Float64, dxInner::Float64)
	x0, x1 = Gp.range[1], Gp.range[2]
	x = collect(x0:Gp.inc[1]:x1)
	cEnd = round(Int, (x1 - x0) / Gp.inc[1]) + 1
	xToPlot = collect(range(x0, x1, length=cEnd))
	return x, xToPlot, cEnd
end

# --------------------------------------------------------------------------------
# faz_bat: builds a bathymetry grid. One function name, 3 methods (multiple dispatch — same pattern
# as cmp_horizont_coords above) cover the .m source's faz_bat/faz_batL2/faz_batL3/faz_batL4:
#   faz_bat(dx)                                     level-0 (outer) flume w/ banks
#   faz_bat(a,b,y0,dxInner,dxOuter)                 level-2 nest, window given as raw a/b/y0 bounds
#   faz_bat(Gp,dxInner,dxOuter,xSpan,rowA,rowB)     level-3/4 nest, window offset off parent `Gp`
# The 3 nested-level forms all reduce to the same y-invariant x-slope, so they share one core,
# `_bat_slope_grid` — the slope/header math exists exactly once instead of 3x.
function faz_bat(dx::Float64=25.0)                              # level-0 (outer) flume
	x = collect(-200.0:dx:50000.0)
	nx = length(x)
	z = collect(range(20.0, -5000.0, length=nx))
	bat = vcat(fill(10.0, 10, nx), repeat(reshape(z, 1, nx), 31, 1), fill(10.0, 10, nx))
	head = [-200.0, 50000.0, 0.0, 50*dx*4, -5000.0, 20.0, 0.0, dx, 4*dx]
	return mat2grid(bat; hdr=head)
end

function _bat_slope_grid(x0::Float64, x1::Float64, y0::Float64, y1::Float64, dxInner::Float64)
	x = collect(x0:dxInner:x1)
	nx = length(x)
	z = collect(range(abs(x0)/10, -x1/10, length=nx))
	n = round(Int, (y1 - y0) / (4*dxInner) + 1)
	bat = repeat(reshape(z, 1, nx), n, 1)
	head = [x0, x1, y0, y1, z[end], z[1], 0.0, dxInner, 4*dxInner]
	return mat2grid(bat; hdr=head)
end

# level-2: window given as raw a/b/y0 bounds — no GMTgrid parent exists yet at this nesting level.
function faz_bat(a::Float64, b::Float64, y0::Float64, dxInner::Float64, dxOuter::Float64)
	x0 = a + dxOuter/2 + dxInner/2
	x1 = b - dxOuter/2 - dxInner/2
	y0b = y0 + dxOuter*4/2 + dxInner*4/2
	return _bat_slope_grid(x0, x1, y0b, y0b + 29*dxInner*4, dxInner)
end

# level-3/4: window offset off the parent grid `Gp`, .m-exact per level. `xSpan` is the raw x1
# offset from Gp.range[1] — 160*Gp.inc[1] (cells) for L3, a literal 700.0 (m) for L4 in the .m
# source, genuinely not the same formula, so it's passed in already-computed rather than forced
# into one shared expression. `rowA:rowB` is the y-window in parent rows (16:25 for L3, 10:30 L4).
function faz_bat(Gp::GMTgrid, dxInner::Float64, dxOuter::Float64, xSpan::Float64, rowA::Int, rowB::Int)
	x0 = Gp.range[1] + dxOuter/2 + dxInner/2
	x1 = Gp.range[1] + xSpan - dxOuter/2 - dxInner/2
	y0 = Gp.range[3] + rowA*Gp.inc[2] + dxOuter*4/2 + dxInner*4/2
	y1 = Gp.range[3] + rowB*Gp.inc[2] - dxOuter*4/2 - dxInner*4/2
	return _bat_slope_grid(x0, x1, y0, y1, dxInner)
end

# faz_fonte: the initial free-surface deformation (source), cubic-spline-sampled off the ANALYTIC
# t=0/160/175/220 columns (ind_z per the .m `switch`) via `gmt_interp1`, repeated across all 51 rows.
function faz_fonte(dx::Float64=25.0, t::Float64=0.0)
	ind_z = t == 0 ? 2 : t == 160 ? 5 : t == 175 ? 8 : t == 220 ? 11 : error("Asneira no t")
	x = collect(-200.0:dx:50000.0)
	z = gmt_interp1(ANALYTIC[:, ind_z-1], ANALYTIC[:, ind_z], x; interp=:cubic, fillval=0.0)
	fonte = repeat(reshape(z, 1, length(x)), 51, 1)
	head = [-200.0, 50000.0, 0.0, 50*dx*4, minimum(z), maximum(z), 0.0, dx, 4*dx]
	return mat2grid(fonte; hdr=head)
end

# --------------------------------------------------------------------------------
# Build the nswing CLI command and run it BLOCKING. bat/src are positional inputs — GMT.jl's
# `gmt()` virtual-file mechanism matches them by trailing-arg order with no text in `cmd` at all
# (same convention as e.g. `gmt("grdtrack -G -n+a -o2", D, G)`), so they need no entry in `args`.
function run_nswing(bat::GMTgrid, src::GMTgrid, nests::Vector{<:GMTgrid}, pato::String;
                    ncycles::Int=4400, interval::Int=100, dt::Float64=0.05, velocity::Bool=true,
                    maregs_pos::Tuple{<:Float64,<:Float64}=(300.0, 2370.0), mareg_interval::Int=20)
	outfile = joinpath(pato, "maregs.dat")

	args = String[]
	for i in eachindex(nests)
		push!(args, "-$(i)")                               # -1<lev1> -2<lev2> ...
	end
	push!(args, "-N$(ncycles)")
	push!(args, "-G$(joinpath(pato,"tsu_time_"))+m,$(interval)")
	velocity && push!(args, "-S")
	push!(args, "-t$(dt)")
	#push!(args, "-T$(maregs_pos[1])/$(maregs_pos[2])+o$(outfile)+t$(mareg_interval)")

	cmd = "nswing -V " * join(args, " ")
	println(cmd)
	gmt(cmd, bat, src, nests...)
	return nothing
end

# --------------------------------------------------------------------------------
"""
    testa_barnabeu(level=3; pato=mktempdir())

v2 port of testa_barnabeu.m. `level` mirrors the original `QUANTAS`: 1 = single (outer) grid only,
2/3 = 1 or 2 nested grids, >=3 also builds (but never simulates — same as the .m source) an extra
level-4 grid purely to source `cmp_horizont_coords`'s x-axis. Runs nswing, then plots a 3x2 panel
comparing the simulated eta/u profiles at t=160/175/220 s against the ANALYTIC solution. Every grid
here is a GMTgrid from the moment it's built to the moment it's read back (`gmtread` already
returns one) — only the nswing call itself touches disk (see run_nswing's banner comment).
"""
function testa_barnabeu(level::Int=3; pato::String=mktempdir())
	QUANTAS = level
	dxOuter = 25.0::Float64
	dxInner = 5.0::Float64
	benchT  = [160, 175, 220]
	benchI  = [4, 7, 10]

	G_bat   = faz_bat(dxOuter)
	G_src   = faz_fonte(dxOuter, 0.0)
	G_batL1 = faz_bat(-200.0, 1025.0, 1900.0, dxInner, dxOuter)
	G_batL2 = faz_bat(G_batL1, 1.0, dxInner, 160*G_batL1.inc[1], 16, 25)

	local x, xToPlot, cEnd
	if QUANTAS == 1
		x, xToPlot, cEnd = cmp_horizont_coords(dxOuter)
		run_nswing(G_bat, G_src, GMTgrid[], pato; interval=50)
	else
		if QUANTAS == 2
			x, xToPlot, cEnd = cmp_horizont_coords(G_batL1, dxOuter, dxInner)
			nests = [G_batL1]
		elseif QUANTAS == 3
			nests = [G_batL1, G_batL2]
			x, xToPlot, cEnd = cmp_horizont_coords(G_batL2, dxOuter, dxInner)
		else
			# QUANTAS >= 4: batL4 built for its coords only — NEVER added to nests, same quirk as
			# the .m source (its own nswing().
			G_batL3 = faz_bat(G_batL2, 0.2, 1.0, 700.0, 10, 30)
			nests = [G_batL1, G_batL2, G_batL3]
			x, xToPlot, cEnd = cmp_horizont_coords(G_batL3, 0.2, 1.0)
		end
		run_nswing(G_bat, G_src, nests, pato; interval=100)
	end

	# --- read the benchmark-time grids back (gmtread -> GMTgrid already), compare, plot 3x2 ---
	axes_ranges = [(0.0,600.0,-25.0,0.0)    (0.0,600.0,-1.0,8.0);
	               (0.0,600.0,-25.0,0.0)    (0.0,600.0,-12.0,0.0);
	               (-200.0,600.0,0.0,20.0)  (-200.0,600.0,-4.0,2.0)]

	# One combined legend box per panel: model (solid blue) vs analytic (dashed red). Built as an
	# explicit entries tuple for `legend()`, NOT via the `legend=` kwarg on `plot`/`plot!` — that
	# path was tested and rejected: GMT auto-flushes each command's own "-l" entry separately
	# (first entry alone at the default TR spot, only later ones landing where `legend()` says),
	# splitting the two series into two stray boxes per panel instead of one combined box.
	_bench_legend() = (symbol1=(marker="-", size=0.6, dx_left=0.0, pen=(1,:blue),      dx_right=0.3, text="Model"),
	                   symbol2=(marker="-", size=0.6, dx_left=0.0, pen=(1,:red,:dash), dx_right=0.3, text="Analytic"))

	# margins: bigger north (4th) value than w/e/s — leaves headroom for each panel's own
	# "t = ... s" title so it doesn't collide with the row above's x-axis label (checked visually).
	subplot(grid=(3,2), F=(width=20,height=28), margins="0.5c/0.5c/0.3c/3c")
	for k in 1:3
		stem = joinpath(pato, "tsu_time_00$(lpad(benchT[k],3,'0'))")
		G1 = gmtread(stem * ".grd")
		Gu = gmtread(stem * "_U.grd")
		row = size(G1.z, 1) ÷ 2                            # fix(size(Z,1)/2), 1-based both sides
		z1 = Float64.(G1.z[row, 1:cEnd])
		zu = Float64.(Gu.z[row, 1:cEnd])

		eta_a = gmt_interp1(ANALYTIC[:, benchI[k]], ANALYTIC[:, benchI[k]+1], xToPlot; interp=:linear)
		u_a   = gmt_interp1(ANALYTIC[:, benchI[k]], ANALYTIC[:, benchI[k]+2], xToPlot; interp=:linear)

		plot(xToPlot, z1; panel=(k,1), region=axes_ranges[k,1], lc=:blue,
		     frame=(axes=:WSen,), xlabel="x (m)", ylabel="eta (m)", title="eta,  t = $(benchT[k]) s")
		plot!(xToPlot, eta_a; panel=(k,1), lc=:red, ls=:dash,)
		legend(_bench_legend(); position=(inside=:TR, width=0.0))

		plot(xToPlot, zu; panel=(k,2), region=axes_ranges[k,2], lc=:blue,
		     frame=(axes=:WSen,), xlabel="x (m)", ylabel="u (m/s)", title="u,  t = $(benchT[k]) s")
		plot!(xToPlot, u_a;   panel=(k,2), lc=:red, ls=:dash)
		legend(_bench_legend(); position=(inside=:TR, width=0.0))
	end
	subplot(show=true)
	return nothing
end

testa_barnabeu()
