#!/bin/bash
#	$Id$
#
# Convert grids between netcdf and several of the other "native" formats

. functions.sh
header "Convert between netcdf and ESRI integer format"

log=reformat_ei.log

grdmath -R-10/10/-10/10 -I1 X = lixo.nc

# First conver to int
grdreformat lixo.nc lixo.ei=ei
grdmath lixo.nc lixo.ei=ei SUB = lixo_dif.nc
grd2xyz lixo_dif.nc -ZTLa > $log

# Now convert back to .nc
grdreformat lixo.ei=ei lixo.nc
grdmath lixo.nc lixo.ei=ei SUB = lixo_dif.nc
grd2xyz lixo_dif.nc -ZTLa >> $log

res=`minmax -C $log`
echo ${res[0]} ${res[1]} | awk '{if($1 != 0 || $2 != 0) print 1}' > fail

passfail reformat_ei
