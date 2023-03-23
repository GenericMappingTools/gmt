#!/usr/bin/env bash
# Purpose:  Check D2DXDY in grdmath
# Prevent a repeat of https://forum.generic-mapping-tools.org/t/grdmath-d2dxy-operator-usage/3773

gmt begin d2dxy

	# generate grid representation of a Gaussian function
	gmt grdmath -R-6.0/6.0/-6.0/6.0 -I0.05 2.0 PI MUL SQRT INV STO@nc POP \
		X Y HYPOT DUP MUL 2.0 DIV NEG EXP @nc MUL CLR@nc = gaussfunc.nc
	
	# generate meshes for the x and y coordinates
	gmt grdmath -R-6.0/6.0/-6.0/6.0 -I0.05 X = mesh_x.nc
	gmt grdmath -R-6.0/6.0/-6.0/6.0 -I0.05 Y = mesh_y.nc

	# exact solution for the derivative
	gmt grdmath gaussfunc.nc mesh_x.nc MUL mesh_y.nc MUL = d2gaussdxdy_ex.nc
	# compute derivative using the D2DXY operator
	gmt grdmath gaussfunc.nc D2DXY = gauss_d2dxy.nc
	# compute derivative using DDY and then DDX
	gmt grdmath gaussfunc.nc DDY DDX = gauss_ddx_ddy.nc
	# Save CPT for original function
	gmt makecpt -Cbatlow -T0.0/0.5/0.01 -Z -H > z.cpt

	# plot
	gmt subplot begin 2x2 -Fs6c -C0.25c -M0.5/2c
		gmt makecpt -Cvik -T-0.25/0.25/0.01 -Z	# Set default subplot CPT
		gmt grdimage gaussfunc.nc -Ba2f1g1 -BWeSn+t"Original Function" -Bx+l"x" -By+l"y" -Cz.cpt -c
		gmt colorbar -DJBC+o0c/1.25c+h -Ba0.1+l"@[ f(x,y) @[ " -Cz.cpt

		gmt grdimage d2gaussdxdy_ex.nc -Ba2f1g1 -BWeSn+t"Exact Formula" -Bx+l"x" -By+l"y" -c
		gmt colorbar -DJBC+o0c/1.25c+h -Ba0.1+l"@[ \frac{\partial^2 f}{\partial x \partial y} @[ "

		gmt grdimage gauss_d2dxy.nc -Ba2f1g1 -BWeSn+t"D2DXY" -Bx+l"x" -By+l"y" -c
		gmt colorbar -DJBC+o0c/1.25c+h -Ba0.1+l"@[ \frac{\partial^2 f}{\partial x \partial y} @[ "

		gmt grdimage gauss_ddx_ddy.nc -Ba2f1g1 -BWeSn+t"DDY then DDX" -Bx+l"x" -By+l"y" -c
		gmt colorbar -DJBC+o0c/1.25c+h -Ba0.1+l"@[ \frac{\partial }{\partial x } \left( \frac{\partial f}{\partial y} \right) @[ "
	gmt subplot end
gmt end show
