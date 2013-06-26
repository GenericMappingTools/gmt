@echo off

REM	$Id$
REM
REM 1. Convert to PNG or PDF the PS files used in GMT_docs.
REM
REM 2. Create RST include files for those PNG files in source/fig_includes and source/gallery
REM    Those include files will carry the location of the PNG's as a relative path.
REM
REM 3. A small number of images come from the SVN source. Create RST includes form them too
REM
REM 4. To create PDF versions used by the latexpdf version do: 
REM
REM       do_fig_uncludes.sh pdf
REM
REM    or just run it witout any arg to create the PNG versions used in creating the HTML doc version
REM
REM This script creates a sub-directory 'docfigs' under 'build'. If there is no 'build'
REM dir, it will be created as well.

set DPI=150
set frmt=g
set ext=png

IF "%1"=="pdf" (
	set frmt=f
	set ext=pdf
) ELSE (
IF "%1"=="jpg" (
	set frmt=j
	set ext=jpg
) )

REM Path to where ps2raster dumps the converted PNGs
set path_build=%~dp0..\..\build\docfigs
set com="-A -E%DPI% -P -T%frmt% -D%path_build% -Qt4"
REM Shrink example figures by 40%
set comPDF="-A+S0.6 -P -T%frmt% -D%path_build%"
REM Path to where the to-be-included files will be created
set pato=%~dp0source\fig_includes\
REM Path to where the to-be-included files will be created (For GALLERY)
set pathGallery=%~dp0source\gallery\


IF %ext%=="pdf" (
echo MERDA PDF
	REM For LATEX/PDF we want to include the examples chapter
	echo .. include: ../examples_chapter_opt.rst_ > %pathGallery%/examples_chapter_opt.rst_
) ELSE (
	REM For HTML the examples chapter is NOT included
	echo.   > %pathGallery%/examples_chapter_opt.rst_
)


IF NOT EXIST %path_build% (
	md %path_build%
)

set name=anim_01
call %~dp0fromAnimations 01 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Animation of a simple sine function. >> %pathGallery%/fig_%name%.rst_
set name=anim_02
call %~dp0fromAnimations 02 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Animation of a DEM using variable illumination. >> %pathGallery%/fig_%name%.rst_
set name=anim_03
call %~dp0fromAnimations 03 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Orbiting a static map. >> %pathGallery%/fig_%name%.rst_
set name=anim_04
call %~dp0fromAnimations 04 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Flying over topography. >> %pathGallery%/fig_%name%.rst_


set name=example_01
call %~dp0fromExamples 01 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Contour maps of gridded data. >> %pathGallery%/fig_%name%.rst_
set name=example_02
call %~dp0fromExamples 02 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Color images from gridded data. >> %pathGallery%/fig_%name%.rst_
set name=example_03
call %~dp0fromExamples 03 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Spectral estimation and x=y-plots. >> %pathGallery%/fig_%name%.rst_
set name=example_04
call %~dp0fromExamples 04 %name% %ext% %frmt% %path_build% %pathGallery%
echo    3-D perspective mesh plot (left) and colored version (right). >> %pathGallery%/fig_%name%.rst_
set name=example_05
call %~dp0fromExamples 05 %name% %ext% %frmt% %path_build% %pathGallery%
echo    3-D illuminated surface. >> %pathGallery%/fig_%name%.rst_
set name=example_06
call %~dp0fromExamples 06 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Two kinds of histograms. >> %pathGallery%/fig_%name%.rst_
set name=example_07
call %~dp0fromExamples 07 %name% %ext% %frmt% %path_build% %pathGallery%
echo    A typical location map. >> %pathGallery%/fig_%name%.rst_
set name=example_08
call %~dp0fromExamples 08 %name% %ext% %frmt% %path_build% %pathGallery%
echo    A 3-D histogram. >> %pathGallery%/fig_%name%.rst_
set name=example_09
call %~dp0fromExamples 09 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Time-series as "wiggles" along a track. >> %pathGallery%/fig_%name%.rst_
set name=example_10
call %~dp0fromExamples 10 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Geographical bar graph. >> %pathGallery%/fig_%name%.rst_

set name=example_11
call %~dp0fromExamples 11 %name% %ext% %frmt% %path_build% %pathGallery%
echo    The RGB color cube. >> %pathGallery%/fig_%name%.rst_
set name=example_12
call %~dp0fromExamples 12 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Optimal triangulation of data. >> %pathGallery%/fig_%name%.rst_
set name=example_13
call %~dp0fromExamples 13 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Display of vector fields in GMT. >> %pathGallery%/fig_%name%.rst_
set name=example_14
call %~dp0fromExamples 14 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Gridding of data and trend surfaces. >> %pathGallery%/fig_%name%.rst_
set name=example_15
call %~dp0fromExamples 15 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Gridding, contouring, and masking of data. >> %pathGallery%/fig_%name%.rst_
set name=example_16
call %~dp0fromExamples 16 %name% %ext% %frmt% %path_build% %pathGallery%
echo    More ways to grid data. >> %pathGallery%/fig_%name%.rst_
set name=example_17
call %~dp0fromExamples 17 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Clipping of images using coastlines. >> %pathGallery%/fig_%name%.rst_
set name=example_18
call %~dp0fromExamples 18 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Volumes and geo-spatial selections. >> %pathGallery%/fig_%name%.rst_
set name=example_19
call %~dp0fromExamples 19 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Using color patterns and additional PostScript material in illustrations. >> %pathGallery%/fig_%name%.rst_
set name=example_20
call %~dp0fromExamples 20 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Using custom symbols in GMT. >> %pathGallery%/fig_%name%.rst_

set name=example_21
call %~dp0fromExamples 21 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Time-series of RedHat stock price since IPO. >> %pathGallery%/fig_%name%.rst_
set name=example_22
call %~dp0fromExamples 22 %name% %ext% %frmt% %path_build% %pathGallery%
echo    World-wide seismicity the last 7 days. >> %pathGallery%/fig_%name%.rst_
set name=example_23
call %~dp0fromExamples 23 %name% %ext% %frmt% %path_build% %pathGallery%
echo    All great-circle paths lead to Rome. >> %pathGallery%/fig_%name%.rst_
set name=example_24
call %~dp0fromExamples 24 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Data selection based on geospatial criteria. >> %pathGallery%/fig_%name%.rst_
set name=example_25
call %~dp0fromExamples 25 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Global distribution of antipodes. >> %pathGallery%/fig_%name%.rst_
set name=example_26
call %~dp0fromExamples 26 %name% %ext% %frmt% %path_build% %pathGallery%
echo    General vertical perspective projection. >> %pathGallery%/fig_%name%.rst_
set name=example_27
call %~dp0fromExamples 27 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Plotting Sandwell/Smith Mercator img grids. >> %pathGallery%/fig_%name%.rst_
set name=example_28
call %~dp0fromExamples 28 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Mixing UTM and geographic data sets requires knowledge of the map region domain in >> %pathGallery%/fig_%name%.rst_
echo    both UTM and lon/lat coordinates and consistent use of the same map scale. >> %pathGallery%/fig_%name%.rst_
set name=example_29
call %~dp0fromExamples 29 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Gridding of spherical surface data using Green's function splines. >> %pathGallery%/fig_%name%.rst_
set name=example_30
call %~dp0fromExamples 30 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Trigonometric functions plotted in graph mode. >> %pathGallery%/fig_%name%.rst_

set name=example_31
call %~dp0fromExamples 31 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Using non-default fonts in *PostScript*. >> %pathGallery%/fig_%name%.rst_
set name=example_32
call %~dp0fromExamples 32 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Draping an image over topography. >> %pathGallery%/fig_%name%.rst_
set name=example_33
call %~dp0fromExamples 33 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Stacking automatically generated cross-profiles. >> %pathGallery%/fig_%name%.rst_ %path_build% %pathGallery%
set name=example_34
call %~dp0fromExamples 34 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Using country polygons for plotting and shading. >> %pathGallery%/fig_%name%.rst_
set name=example_35
call %~dp0fromExamples 35 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Spherical triangulation and distance calculations >> %pathGallery%/fig_%name%.rst_
set name=example_36
call %~dp0fromExamples 36 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Spherical gridding using Renka's algorithms. >> %pathGallery%/fig_%name%.rst_
set name=example_37
call %~dp0fromExamples 37 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Spectral coherence between gravity and bathymetry grids. >> %pathGallery%/fig_%name%.rst_
set name=example_38
call %~dp0fromExamples 38 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Histogram equalization of bathymetry grids. >> %pathGallery%/fig_%name%.rst_
set name=example_39
call %~dp0fromExamples 39 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Evaluation of spherical harmonics coefficients. >> %pathGallery%/fig_%name%.rst_
set name=example_40
call %~dp0fromExamples 40 %name% %ext% %frmt% %path_build% %pathGallery%
echo    Illustrate line simplification and area calculations. >> %pathGallery%/fig_%name%.rst_


set name=GMT_App_E
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%

set name=GMT_App_F_stand+_iso+
call %~dp0fromScripts  %name% %ext% %frmt% %path_build% %pato%
echo    Octal codes and corresponding symbols for StandardEncoding (left) and ISOLatin1Encoding (right) fonts. >> %pato%/fig_%name%.rst_

set name=GMT_App_F_symbol_dingbats
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Octal codes and corresponding symbols for Symbol (left) and ZapfDingbats (right) fonts. >> %pato%/fig_%name%.rst_

set name=GMT_App_G
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    The standard 35 **PS** fonts recognized by *GMT*. >> %pato%/fig_%name%.rst_

set name=GMT_App_M_1
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    The standard 22 CPT files supported by GMT >> %pato%/fig_%name%.rst_

set name=GMT_App_M_2
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
set name=GMT_App_P_2
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Example created in isolation mode >> %pato%/fig_%name%.rst_

set name=GMT_color_interpolate
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    When interpolating colors, the color system matters. The polar palette on the left needs to >> %pato%/fig_%name%.rst_
echo    be interpolated in RGB, otherwise hue will change between blue (240) and white (0). The rainbow >> %pato%/fig_%name%.rst_
echo    palette should be interpolated in HSV, since only hue should change between magenta (300) and red (0). >> %pato%/fig_%name%.rst_
echo    Diamonds indicate which colors are defined in the palettes; they are fixed, the rest is interpolated. >> %pato%/fig_%name%.rst_

set name=GMT_coverlogo
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%

set name=GMT_RGBchart_a4
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    The 555 unique color names that can be used in GMT. Lower, upper, or mixed cases, as well as >> %pato%/fig_%name%.rst_
echo    the british spelling of "grey" are allowed. A4, Letter, and Tabloid sized versions of this RGB chart can be >> %pato%/fig_%name%.rst_
echo    found in the GMT documentation directory. >> %pato%/fig_%name%.rst_

set name=GMT_volcano
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
REM For this one we also need a copy in the gallery
echo .. figure:: %path_build%/%name%.%ext% > %pathGallery%/fig_%name%.rst_
echo    :width: 500 px >> %pathGallery%/fig_%name%.rst_
echo    :align: center >> %pathGallery%/fig_%name%.rst_
echo.   >> %pathGallery%/fig_%name%.rst_

set name=GMT_Defaults_1a
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Some *GMT* parameters that affect plot appearance. >> %pato%/fig_%name%.rst_

set name=GMT_Defaults_1b
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    More *GMT* parameters that affect plot appearance. >> %pato%/fig_%name%.rst_

set name=GMT_Defaults_1c
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Even more *GMT* parameters that affect plot appearance. >> %pato%/fig_%name%.rst_

set name=GMT_-R
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    The plot region can be specified in two different ways. (a) Extreme values >> %pato%/fig_%name%.rst_
echo    for each dimension, or (b) coordinates of lower left and upper right corners. >> %pato%/fig_%name%.rst_

set name=GMT_-J
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    The 30+ map projections and coordinate transformations available in *GMT*. >> %pato%/fig_%name%.rst_

set name=GMT_-B_geo_1
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Geographic map border using separate selections for annotation, >> %pato%/fig_%name%.rst_
echo    frame, and grid intervals.  Formatting of the annotation is controlled by >> %pato%/fig_%name%.rst_
echo    the parameter **FORMAT_GEO_MAP** in your ``gmt.conf``. >> %pato%/fig_%name%.rst_

set name=GMT_-B_geo_2
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Geographic map border with both primary (P) and secondary (S) components. >> %pato%/fig_%name%.rst_

set name=GMT_-B_linear
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Linear Cartesian projection axis.  Long tickmarks accompany >> %pato%/fig_%name%.rst_
echo    annotations, shorter ticks indicate frame interval. The axis label is >> %pato%/fig_%name%.rst_
echo    optional. For this example we used -R0/12/0/1 -JX3i/0.4i -Ba4f2g1:Frequency: >> %pato%/fig_%name%.rst_

set name=GMT_-B_log
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Logarithmic projection axis using separate values for annotation, >> %pato%/fig_%name%.rst_
echo    frame, and grid intervals.  (top) Here, we have chosen to annotate the actual >> %pato%/fig_%name%.rst_
echo    values.  Interval = 1 means every whole power of 10, 2 means 1, 2, 5 times >> %pato%/fig_%name%.rst_
echo    powers of 10, and 3 means every 0.1 times powers of 10.  We used >> %pato%/fig_%name%.rst_
echo    -R1/1000/0/1 -JX3il/0.4i -Ba1f2g3.  (middle) Here, we have chosen to >> %pato%/fig_%name%.rst_
echo    annotate log :math:\`_10\` of the actual values, with -Ba1f2g3l. >> %pato%/fig_%name%.rst_
echo    (bottom) We annotate every power of 10 using log :math:\`_10\` of the actual values >> %pato%/fig_%name%.rst_
echo    as exponents, with -Ba1f2g3p. >> %pato%/fig_%name%.rst_

set name=GMT_-B_pow
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Exponential or power projection axis. (top) Using an exponent of 0.5 >> %pato%/fig_%name%.rst_
echo    yields a :math:\`sqrt(x)\` axis.  Here, intervals refer to actual data values, in >> %pato%/fig_%name%.rst_
echo    -R0/100/0/1 -JX3ip0.5/0.4i -Ba20f10g5. >> %pato%/fig_%name%.rst_
echo    (bottom) Here, intervals refer to projected values, although the annotation >> %pato%/fig_%name%.rst_
echo    uses the corresponding unprojected values, as in -Ba3f2g1p. >> %pato%/fig_%name%.rst_

set name=GMT_-B_time1
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Cartesian time axis, example 1. >> %pato%/fig_%name%.rst_

set name=GMT_-B_time2
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Cartesian time axis, example 2. >> %pato%/fig_%name%.rst_

set name=GMT_-B_time3
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Cartesian time axis, example 3. >> %pato%/fig_%name%.rst_

set name=GMT_-B_time4
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Cartesian time axis, example 4. >> %pato%/fig_%name%.rst_

set name=GMT_-B_time5
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Cartesian time axis, example 5. >> %pato%/fig_%name%.rst_

set name=GMT_-B_time6
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Cartesian time axis, example 6. >> %pato%/fig_%name%.rst_

set name=GMT_-B_time7
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Cartesian time axis, example 7. >> %pato%/fig_%name%.rst_

set name=GMT_-B_custom
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Custom and irregular annotations, tick-marks, and gridlines. >> %pato%/fig_%name%.rst_

set name=GMT_-P
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Users can specify Landscape [Default] or Portrait -P) orientation. >> %pato%/fig_%name%.rst_

set name=GMT_-OK
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    A final **PS** file consists of any number of individual pieces. >> %pato%/fig_%name%.rst_

set name=GMT_-U
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    The -U option makes it easy to ``date`` a plot. >> %pato%/fig_%name%.rst_

set name=GMT_-XY
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Plot origin can be translated freely with -X -Y. >> %pato%/fig_%name%.rst_

set name=GMT_registration
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Gridline- and pixel-registration of data nodes. >> %pato%/fig_%name%.rst_

set name=GMT_linecap
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Line appearance can be varied by using **PS_LINE_CAP** >> %pato%/fig_%name%.rst_

set name=GMT_arrows
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Examples of Cartesian (left), circular (middle), and geo-vectors (right) for different >> %pato%/fig_%name%.rst_
echo    attribute specifications. Note that both full and half arrow-heads can be specified, as well as no head at all. >> %pato%/fig_%name%.rst_

set name=GMT_linear
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Linear transformation of Cartesian coordinates. >> %pato%/fig_%name%.rst_

set name=GMT_linear_d
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Linear transformation of map coordinates. >> %pato%/fig_%name%.rst_

set name=GMT_linear_cal
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Linear transformation of calendar coordinates. >> %pato%/fig_%name%.rst_

set name=GMT_log
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Logarithmic transformation of x-coordinates. >> %pato%/fig_%name%.rst_

set name=GMT_pow
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Exponential or power transformation of x-coordinates. >> %pato%/fig_%name%.rst_

set name=GMT_polar
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Polar (Cylindrical) transformation of (:math:\`\theta, r\`) coordinates. >> %pato%/fig_%name%.rst_

set name=GMT_albers
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Albers equal-area conic map projection. >> %pato%/fig_%name%.rst_

set name=GMT_equidistant_conic
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Equidistant conic map projection. >> %pato%/fig_%name%.rst_

set name=GMT_lambert_conic
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Lambert conformal conic map projection. >> %pato%/fig_%name%.rst_

set name=GMT_polyconic
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    (American) polyconic projection. >> %pato%/fig_%name%.rst_

set name=GMT_lambert_az_rect
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Rectangular map using the Lambert azimuthal equal-area projection. >> %pato%/fig_%name%.rst_

set name=GMT_lambert_az_hemi
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Hemisphere map using the Lambert azimuthal equal-area projection. >> %pato%/fig_%name%.rst_

set name=GMT_stereonets
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Equal-Area (Schmidt) and Equal-Angle (Wulff) stereo nets. >> %pato%/fig_%name%.rst_

set name=GMT_stereographic_polar
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Polar stereographic conformal projection. >> %pato%/fig_%name%.rst_

set name=GMT_stereographic_rect
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Polar stereographic conformal projection with rectangular borders. >> %pato%/fig_%name%.rst_

set name=GMT_stereographic_general
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    General stereographic conformal projection with rectangular borders. >> %pato%/fig_%name%.rst_

set name=GMT_perspective
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    View from the Space Shuttle in Perspective projection. >> %pato%/fig_%name%.rst_

set name=GMT_orthographic
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Hemisphere map using the Orthographic projection. >> %pato%/fig_%name%.rst_

set name=GMT_az_equidistant
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    World map using the equidistant azimuthal projection. >> %pato%/fig_%name%.rst_

set name=GMT_gnomonic
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Gnomonic azimuthal projection. >> %pato%/fig_%name%.rst_

set name=GMT_mercator
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Simple Mercator map. >> %pato%/fig_%name%.rst_

set name=GMT_transverse_merc
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Rectangular Transverse Mercator map. >> %pato%/fig_%name%.rst_

set name=GMT_TM
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    A global transverse Mercator map. >> %pato%/fig_%name%.rst_

set name=GMT_utm_zones
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Figure : Universal Transverse Mercator zone layout. >> %pato%/fig_%name%.rst_

set name=GMT_obl_merc
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Oblique Mercator map using **-Joc**. We make it clear which direction is North by >> %pato%/fig_%name%.rst_
echo    adding a star rose with the **-T** option. >> %pato%/fig_%name%.rst_

set name=GMT_cassini
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Cassini map over Sardinia. >> %pato%/fig_%name%.rst_

set name=GMT_equi_cyl
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    World map using the Plate Carr\'%e%e projection. >> %pato%/fig_%name%.rst_

set name=GMT_general_cyl
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    World map using the Behrman cylindrical equal-area projection. >> %pato%/fig_%name%.rst_

set name=GMT_miller
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    World map using the Miller cylindrical projection. >> %pato%/fig_%name%.rst_

set name=GMT_gall_stereo
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    World map using Gall's stereographic projection. >> %pato%/fig_%name%.rst_

set name=GMT_hammer
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    World map using the Hammer projection. >> %pato%/fig_%name%.rst_

set name=GMT_mollweide
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    World map using the Mollweide projection. >> %pato%/fig_%name%.rst_

set name=GMT_winkel
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    World map using the Winkel Tripel projection. >> %pato%/fig_%name%.rst_

set name=GMT_robinson
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    World map using the Robinson projection. >> %pato%/fig_%name%.rst_

set name=GMT_eckert4
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    World map using the Eckert IV projection. >> %pato%/fig_%name%.rst_

set name=GMT_eckert6
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    World map using the Eckert VI projection. >> %pato%/fig_%name%.rst_

set name=GMT_sinusoidal
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    World map using the Sinusoidal projection. >> %pato%/fig_%name%.rst_

set name=GMT_sinus_int
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    World map using the Interrupted Sinusoidal projection. >> %pato%/fig_%name%.rst_

set name=GMT_grinten
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    World map using the Van der Grinten projection. >> %pato%/fig_%name%.rst_

set name=GMT_App_J_1
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Impulse responses for *GMT* filters. >> %pato%/fig_%name%.rst_

set name=GMT_App_J_2
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Transfer functions for 1-D *GMT* filters. >> %pato%/fig_%name%.rst_

set name=GMT_App_J_3
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Transfer functions for 2-D (radial) *GMT* filters. >> %pato%/fig_%name%.rst_

set name=GMT_App_K_1
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Map using the crude resolution coastline data. >> %pato%/fig_%name%.rst_

set name=GMT_App_K_2
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Map using the low resolution coastline data. >> %pato%/fig_%name%.rst_

set name=GMT_App_K_3
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Map using the intermediate resolution coastline data. We have added a compass >> %pato%/fig_%name%.rst_
echo    rose just because we have the power to do so. >> %pato%/fig_%name%.rst_

set name=GMT_App_K_4
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Map using the high resolution coastline data. >> %pato%/fig_%name%.rst_

set name=GMT_App_K_5
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Map using the full resolution coastline data. >> %pato%/fig_%name%.rst_

set name=GMT_App_N_1
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Custom plot symbols supported by *GMT*. Note that we only show >> %pato%/fig_%name%.rst_
echo    the symbol outline and not any fill. These are all single-parameter symbols. >> %pato%/fig_%name%.rst_
echo    Be aware that some symbols may have a hardwired fill or no-fill component, >> %pato%/fig_%name%.rst_
echo    while others duplicate what is already available as standard built-in symbols. >> %pato%/fig_%name%.rst_

set name=GMT_App_O_1
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Equidistant contour label placement with **-Gd**, the only algorithm >> %pato%/fig_%name%.rst_
echo    available in previous *GMT* versions. >> %pato%/fig_%name%.rst_

set name=GMT_App_O_2
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Placing one label per contour that exceed 1 inch in length, centered on the segment with **-Gn**. >> %pato%/fig_%name%.rst_

set name=GMT_App_O_3
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Four labels are positioned on the points along the contours that are >> %pato%/fig_%name%.rst_
echo    closest to the locations given in the file ``fix.txt`` in the **-Gf** option. >> %pato%/fig_%name%.rst_

set name=GMT_App_O_4
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Labels are placed at the intersections between contours and the great circle specified in the **-GL** option. >> %pato%/fig_%name%.rst_

set name=GMT_App_O_5
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Labels are placed at the intersections between contours and the >> %pato%/fig_%name%.rst_
echo    multi-segment lines specified in the **-GX** option. >> %pato%/fig_%name%.rst_

set name=GMT_App_O_6
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Labels attributes are controlled with the arguments to the **-Sq** option. >> %pato%/fig_%name%.rst_

set name=GMT_App_O_7
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Another label attribute example. >> %pato%/fig_%name%.rst_

set name=GMT_App_O_8
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Labels based on another data set (here bathymetry) while the placement is based on distances. >> %pato%/fig_%name%.rst_

set name=GMT_App_O_9
call %~dp0fromScripts %name% %ext% %frmt% %path_build% %pato%
echo    Tsunami travel times from the Canary Islands to places in the Atlantic, >> %pato%/fig_%name%.rst_
echo    in particular New York. Should a catastrophic landslide occur it is possible >> %pato%/fig_%name%.rst_
echo    that New York will experience a large tsunami about 8 hours after the event. >> %pato%/fig_%name%.rst_


set name=GMT5_Summit_2011.jpg
call %~dp0fromFig %name% %pato%
echo    The four horsemen of the GMT apocalypse: Remko Scharroo, Paul Wessel, Walter H.F. Smith, >> %pato%/fig_%name%.rst_
echo    and Joaquim Luis at the GMT Developer Summit in Honolulu, Hawaii during February 14-18, 2011. >> %pato%/fig_%name%.rst_

set name=hsv-cone.png
call %~dp0fromFig %name% %pato%
echo    The HSV color space >> %pato%/fig_%name%.rst_ 

set name=rendering.png
call %~dp0fromFig %name% %pato%
echo    Examples of rendered images in a PowerPoint presentation >> %pato%/fig_%name%.rst_ 

set name=formatpicture.png
call %~dp0fromFig %name% %pato%
echo    **PowerPoint**'s "Format Picture" dialogue to set scale and rotation. >> %pato%/fig_%name%.rst_

set name=GMT4_mode.png
call %~dp0fromFig %name% %pato%
echo    GMT 4 programs contain all the high-level functionality. >> %pato%/fig_%name%.rst_ 

set name=GMT5_mode.png
call %~dp0fromFig %name% %pato%
echo    GMT 5 programs contain all the high-level functionality. >> %pato%/fig_%name%.rst_ 

set name=gimp-sliders.png
call %~dp0fromFig %name% %pato%

set name=gimp-panel.png
call %~dp0fromFig %name% %pato%
echo    Chartreuse in *GIMP*. *(a)* Sliders indicate how the color is altered >> %pato%/fig_%name%.rst_ 
echo    when changing the H, S, V, R, G, or B levels. *(b)* For a constant hue (here 90) >> %pato%/fig_%name%.rst_
echo    value increases to the right and saturation increases up, so the "pure" >> %pato%/fig_%name%.rst_ 
echo    color is on the top right. >> %pato%/fig_%name%.rst_

:fim

