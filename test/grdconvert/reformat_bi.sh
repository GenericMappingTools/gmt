#!/bin/bash
#	$Id$
#
# Convert grids between netcdf and several of the other "native" formats

log=reformat_bi.log

gmt grdmath -R-10/10/-10/10 -I1 X = lixo.nc

# First conver to int
gmt grdconvert lixo.nc lixo.bi=bi
gmt grdmath lixo.nc lixo.bi=bi SUB = lixo_dif.nc
gmt grd2xyz lixo_dif.nc -ZTLa > $log

# Now convert back to .nc
gmt grdconvert lixo.bi=bi lixo.nc
gmt grdmath lixo.nc lixo.bi=bi SUB = lixo_dif.nc
gmt grd2xyz lixo_dif.nc -ZTLa >> $log

res=`gmt info -C $log`
echo ${res[0]} ${res[1]} | $AWK '{if($1 != 0 || $2 != 0) print 1}' > fail
