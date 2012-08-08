#!/bin/sh 
set -e

#river_regrid
# cd $river_regrid_outdir
# $tooldir/river_regrid --mosaic $indir/M45/mosaic.nc --river_src $indir/z1l_river_output_M45_tripolar_aug24.nc --output river_data_M45

# $tooldir/river_regrid --mosaic $indir/C48/mosaic.nc --river_src $indir/z1l_river_output_M45_tripolar_aug24.nc --output river_data_C48

# #transfer_to_mosaic_grid
# cd $transfer_outdir
# $tooldir/transfer_to_mosaic_grid --input_file $indir/M45.tripolar.grid_spec.nc

# cd $outdir

# #make_vgrid will make a grid file with 60 supergrid cells.

# $tooldir/make_vgrid --nbnds 3 --bnds 10,200,1000 --nz 10,20

# #make_hgrid and make_solo_mosaic C48
# $tooldir/make_hgrid --grid_type gnomonic_ed --nlon 96

# $tooldir/make_solo_mosaic --num_tiles 6 --dir $PWD --mosaic C48_mosaic

# #make_hgrid and make_solo_mosaic N45
# $tooldir/make_hgrid --grid_type regular_lonlat_grid --nxbnd 2 --nybnd 2 --xbnd 0,360 --ybnd -90,90 --nlon 288 --nlat 180 --grid_name N45_grid
# $tooldir/make_solo_mosaic --num_tiles 1 --dir $PWD --mosaic N45_mosaic --tile_file N45_grid.nc --periodx 360

# #make_hgrid and make_solo_mosaic 2-degree tripolar
# $tooldir/make_hgrid --grid_type tripolar_grid --nxbnd 2 --nybnd 2 --xbnd -280,80 --ybnd -90,90 --nlon 360 --nlat 180 --grid_name tripolar_grid 

# $tooldir/make_solo_mosaic --num_tiles 1 --dir $PWD --mosaic tripolar_mosaic --tile_file tripolar_grid --periodx 360

# #make_topog of tripolar grid
# $tooldir/make_topog --mosaic  $outdir/tripolar_mosaic.nc --topog_file $indir/OCCAM_p5degree.nc --topog_field TOPO --scale_factor -1 --output tripolar_topog.nc

# #make_coupler_mosaic
# $tooldir/make_coupler_mosaic --atmos_mosaic  $outdir/C48_mosaic.nc --land_mosaic  $outdir/N45_mosaic.nc --ocean_mosaic  $outdir/tripolar_mosaic.nc --ocean_topog  $outdir/tripolar_topog.nc 

# mpirun -np 10 $tooldir/make_topog_parallel --mosaic $outdir/tripolar_mosaic.nc --topog_file $indir/OCCAM_p5degree.nc --topog_field TOPO --scale_factor -1 --output tripolar_topog.nc

# mpirun -np 10 $tooldir/make_coupler_mosaic_parallel --atmos_mosaic $outdir/C48_mosaic.nc --land_mosaic $outdir/N45_mosaic.nc --ocean_mosaic $outdir/tripolar_mosaic.nc --ocean_topog $outdir/tripolar_topog.nc 

# #compare data
# foreach ncfile (`ls *.nc`)
#   nccmp -md $ncfile ../$ncfile
# end
