# Phase 0 — which GMT code actually reads the grid halo

This is the experiment the issue asks for:

> [forcing *GMT_Create_Session* to set pad to 0, recompile, and run tests might
> tell us more!]

Done a bit more precisely than that. Rather than removing the pad and watching
for breakage, the pad is **poisoned**: `gmt_grd_BC_set` computes the boundary
conditions as usual and then overwrites every pad node with `1e20`
(`gmt_support.c`, guarded by `GMT_POISON_PAD=1`, so one build serves both
runs). Every grid-using test in `test/` that does not need remote data was then
run twice from the same binary, with and without poisoning, and all produced
files compared (PostScript compared ignoring `%` comment lines).

A test whose output moves is a test whose result depends on halo values. A test
whose output does not move did not read the halo on that code path.

Every difference below survived a determinism control: each script was also run
twice *without* poisoning, and only the ones that were stable across that pair
are listed. One script, `test/grdtrack/crosstrack_geo.sh`, is not
reproducible run-to-run and has been excluded rather than counted.

## Result: 34 of 257 scripts changed, 33 of them reproducibly

```
grdimage           7 /  19
grdgradient        6 /   7
grdmask            3 /   7
grdinterpolate     3 /   7
surface            2 /   3
grdtrack           2 /   4
grdpaste           2 /   7
x2sys              1 /   5
sph                1 /   3
psxyz              1 /   1
greenspline        1 /   7
grdvolume          1 /   3
grdsample          1 /   4
grdproject         1 /   2
grdmath            1 /  13
grdcut             1 /   6
xyz2grd            0 /   1
windbarbs          0 /   2
triangulate        0 /   1
time               0 /   1
spotter            0 /   4
sph2grd            0 /   2
segy               0 /   2
psxy               0 /   2
pstext             0 /   1
psscale            0 /   1
psmask             0 /   1
pscontour          0 /   1
potential          0 /  21
nearneighbor       0 /   2
img                0 /   2
grdview            0 /   7
grdvector          0 /  15
grdtrend           0 /   4
grdselect          0 /   4
grdmix             0 /   2
grdlandmask        0 /   6
grdinfo            0 /   2
grdhisteq          0 /   1
grdgdal            0 /   1
grdfilter          0 /   4
grdfill            0 /   1
grdfft             0 /   4
grdedit            0 /   4
grdconvert         0 /  11
grdcontour         0 /  12
grdclip            0 /   2
grdblend           0 /   8
grd2xyz            0 /   3
grd2kml            0 /   1
grd2cpt            0 /   2
gmtselect          0 /   1
gmtmex             0 /   1
gmtinfo            0 /   1
gmt2kml            0 /   1
geodesy            0 /   2
genper             0 /   2
gdal               0 /   2
api                0 /  11
```

## What this confirms, and what it corrects

**grdcontour does not need a pad.** The issue lists it as an open question
("I am not sure if **grdcontour** may also be in this category"). All ten
`test/grdcontour` scripts are unaffected by poisoning. The contour tracer walks
cell corners strictly inside the grid.

**grdgradient does need one, and it is not on the issue's list.** Six of seven
`test/grdgradient` scripts change. That is the strongest single signal in the
sweep — unsurprising in hindsight, since a gradient at an edge node needs the
node beyond the edge, but it is exactly the kind of module the "recompile with
pad 0" experiment would have missed if its tests happened to still produce a
plausible-looking picture.

**Several modules on the issue's list are confirmed:** grdimage (7/19),
grdsample, grdproject, grdtrack. These all reach the halo through
`gmt_bcr.c`, which is why Phase 3 converts that one file first.

**grdview shows no differences here (0/7).** That is not a claim that grdview
is halo-free; it is a claim that the seven scripts in `test/grdview` do not
exercise a halo-reading path. Treat it as untested, not as clean.

## Two important limits of this measurement

1. **It is a lower bound.** The poison is applied when boundary conditions are
   set. A module that *fills the pad itself afterwards* — grdfft and gravfft
   taper into it, surface uses it as working space — overwrites the poison and
   therefore shows as unchanged. All 21 `test/potential` scripts come out
   SAME for this reason, and that is a false negative.

   This turns out to be a useful distinction rather than a nuisance. Those
   modules do not want *boundary conditions*; they want a **larger array** to
   work in. Ghost cells do not serve them and should not try to. They need a
   working buffer, which is a separate and much easier change: allocate the
   bigger array explicitly instead of borrowing the grid's pad.

2. **Remote-data tests were excluded** (57 scripts referencing `@earth_relief`
   and friends), as were tests of non-grid modules. Extending the sweep to the
   full suite is cheap and is the natural next refinement.

## What happened to these once ghost cells existed

The point of the inventory is to be worked through. Running the same 257
scripts with the halo moved out of the data matrix (`GMT_GHOST_CELLS=1`) now
leaves **all 257 byte-identical**. Every script in this list produces the same
answer with no pad in the grid at all.

Worth noting which way the two lists differ. Poisoning finds code that *reads*
the halo; the ghost sweep finds code that *depends on the layout*. They overlap
but are not the same set — grdmath, for instance, reads no halo yet broke on the
layout, because it indexes every stack item with node numbers computed from a
template grid that had kept its pad.

## Reproducing it

```
GMT_POISON_PAD=1 gmt grdtrack points.txt -Ggrid.nc     # halo-dependent values blow up
GMT_POISON_PAD=nan gmt ...                             # or poison with NaN instead
```

## The scripts, and the files that moved

```
test/grdcut/vert_cube_cuts.sh fake_xyz_cube.nc
test/grdgradient/aspect.sh aspect.nc aspect.ps
test/grdgradient/illum_classic.sh illum_classic.ps
test/grdgradient/illum_lambert.sh illum_lambert.ps
test/grdgradient/illum_manip.sh illum_manip.ps
test/grdgradient/illum_norm_control.sh illum_classic.ps
test/grdgradient/illum_var.sh illum_var.ps
test/grdimage/force_global.sh force_global.ps
test/grdimage/grdimage.sh grdimage.ps
test/grdimage/grdimage_img_tif.sh grdimage_img_tif.ps grdimg.tiff lixo_utm.grd lixo_utm.tiff
test/grdimage/hovmuller.sh hovmuller.ps
test/grdimage/oneincshift.sh oneincshift.ps
test/grdimage/rendering.sh rendering.ps
test/grdimage/subset.sh subset.ps
test/grdinterpolate/fake_geot_cube.sh fake_geot_cube.nc fake_geot_cube.ps
test/grdinterpolate/fake_geoz_cube.sh fake_geoz_cube.nc fake_geoz_cube.ps
test/grdinterpolate/fake_xyz_cube.sh fake_xyz_cube.nc
test/grdmask/geoholes.sh geoholes.ps
test/grdmask/polarhole.sh polarhole.ps
test/grdmask/twoblobs.sh twoblobs.ps
test/grdmath/ops.sh B.grd
test/grdpaste/paste_esri.sh lixo_y.nc tmp.nc
test/grdpaste/paste_gridreg_underlap_bf.sh lixo_x.grd lixo_xy.grd lixo_y.grd paste_gridreg_underlap_bf.ps
test/grdproject/units.sh c.nc i.nc
test/grdsample/sample.sh out.nc sample.ps
test/grdtrack/profiles.sh a.txt b.txt
test/grdvolume/sphere_volume.sh int.grd sphere_volume.ps
test/greenspline/gspline_8.sh gspline_8.ps
test/psxyz/3dbars.sh 3dbars.ps
test/sph/sph_2.sh sph_2.ps
test/surface/periodic.sh data.txt datad.nc datag.nc periodic.ps
test/surface/periodic_pix.sh data.txt datad.nc datag.nc periodic_pix.ps
test/x2sys/x2sys_01.sh ss_faa_int.nc x2sys_01.ps
```
