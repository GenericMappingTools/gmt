#!/usr/bin/env bash
#
# Convert grids between netcdf and several of the other "native" formats

log=reformat_bs.log

gmt grdmath -R-10/10/-10/10 -I1 X = lixo.nc

# First convert to int
gmt grdconvert lixo.nc lixo.bs=bs
gmt grdmath lixo.nc lixo.bs=bs SUB = lixo_dif.nc
gmt grd2xyz lixo_dif.nc -ZTLa > $log

# Now convert back to .nc
gmt grdconvert lixo.bs=bs lixo.nc
gmt grdmath lixo.nc lixo.bs=bs SUB = lixo_dif.nc
gmt grd2xyz lixo_dif.nc -ZTLa >> $log

res=$(gmt info -C $log)
echo ${res[0]} ${res[1]} | $AWK '{if($1 != 0 || $2 != 0) print 1}' > fail
