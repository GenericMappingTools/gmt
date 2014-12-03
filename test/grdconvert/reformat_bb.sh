#!/bin/bash
#	$Id$
#
# Convert grids between netcdf and several of the other "native" formats

log=reformat_bb.log

gmt grdmath -R-10/10/-10/10 -I1 X = lixo.nc

# First conver to int
gmt grdconvert lixo.nc lixo.bb=bb
gmt grdmath lixo.nc lixo.bb=bb SUB = lixo_dif.nc
gmt grd2xyz lixo_dif.nc -ZTLa > $log

# Now convert back to .nc
gmt grdconvert lixo.bb=bb lixo.nc
gmt grdmath lixo.nc lixo.bb=bb SUB = lixo_dif.nc
gmt grd2xyz lixo_dif.nc -ZTLa >> $log

res=`gmt info -C $log`
echo ${res[0]} ${res[1]} | $AWK '{if($1 != 0 || $2 != 0) print 1}' > fail
