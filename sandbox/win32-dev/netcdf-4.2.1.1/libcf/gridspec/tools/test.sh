#!/bin/sh -f

set -e
echo ""
echo "*** Testing gridspec grid commands."

echo "*** make_vgrid will make a grid file with 30 grid cells..."
../tools/make_vgrid/make_vgrid --nbnds 3 --bnds 10,200,1000 --nz 10,20

echo "*** make a torus mosaic..."
#../tools/make_hgrid/make_hgrid --grid_type regular_lonlat_grid --nxbnd 2 --nybnd 2 --xbnd 0,360 --ybnd -90,90 --nlon 160 --nlat 180 --grid_name torus_grid

#../tools/make_mosaic/make_mosaic --num_tiles 1 --tile_file torus_grid.nc --periodx 360 --periody 180 --mosaic_name torus_mosaic

echo "*** make a simple mosaic with four tiles..."

#../tools/make_hgrid/make_hgrid --grid_type regular_lonlat_grid --nxbnd 2 --nybnd 2 --xbnd 0,360 --ybnd -90,90 --nlon 160 --nlat 180 --grid_name four_tile_grid --ndivx 2 --ndivy 2
#../tools/make_mosaic/make_mosaic --num_tiles 4 --tile_file four_tile_grid --periodx 360 --mosaic_name four_tile_mosaic

#echo "*** make a coupler mosaic with C48 atmosphere grid, N45 land grid and "
#echo "    2 degree tripolar grid for ice and ocean model..."

echo "*** make_hgrid and make_solo_mosaic C48 atmos grid..."
../tools/make_hgrid/make_hgrid --grid_type gnomonic_ed --nlon 96 --grid_name C48_grid
#../tools/make_mosaic/make_mosaic --num_tiles 6 --tile_file C48_grid --mosaic_name C48_mosaic 

#echo "*** make_hgrid and make_solo_mosaic N45 land grid..."
#../tools/make_hgrid/make_hgrid --grid_type regular_lonlat_grid --nxbnd 2 --nybnd 2 --xbnd 0,360 --ybnd -90,90 --nlon 288 --nlat 180 --grid_name N45_grid
#../tools/make_mosaic/make_mosaic --num_tiles 1 --mosaic_name N45_mosaic --tile_file N45_grid.nc --periodx 360

#echo "*** make_hgrid and make_solo_mosaic 2-degree tripolar.."
#../tools/make_hgrid/make_hgrid --grid_type tripolar_grid --nxbnd 2 --nybnd 2 --xbnd -280,80 --ybnd -90,90 --nlon 360 --nlat 180 --grid_name tripolar_grid 
#../tools/make_mosaic/make_mosaic --num_tiles 1 --mosaic_name tripolar_mosaic --tile_file tripolar_grid --periodx 360

#echo "*** make_topog of tripolar grid..."
#../tools/make_topog/make_topog --mosaic  tripolar_mosaic.nc --topog_file $indir/OCCAM_p5degree.nc --topog_field TOPO --scale_factor -1 --topog_mosaic tripolar_topog_mosaic

#echo "*** make_coupler_mosaic..."
#../tools/make_coupler_mosaic/make_coupler_mosaic --atmos_mosaic  $outdir/C48_mosaic.nc --land_mosaic  $outdir/N45_mosaic.nc --ice_mosaic  $outdir/tripolar_mosaic.nc --ice_topog_mosaic  $outdir/tripolar_topog_mosaic.nc 

#echo "*** use fregrid to remap data from C48 onto N45 using first-order conservative interpolation..."
#../tools/fregrid/fregrid --input_mosaic C48_mosaic.nc --input_dir $srcdir/../input --input_file 19800101.atmos_daily --scalar_field zsurf,temp,t_surf --output_mosaic N45_mosaic.nc --interp_method conserve_order2 --output_file 19800101.atmos_daily.N45.order1 --remap_file C48_to_N45_remap.order1.nc

echo "*** All gridspec tests passed!"
exit 0
