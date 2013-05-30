#! /bin/bash
#	$Id$
#
# 1. Convert to PNG the PS files used in GMT_docs.
#
# 2. Create RST include files for those PNG files in source/fig_includes
#    Those include files will carry the location of the PNG's as a relative path.
#
# 3. A small number of images come from the SVN source. Create RST includes form them too
#
# This script creates a sub-directory 'docfigs' under 'build'. If there is no 'build'
# dir, it will be created as well.

DPI=200
path_build=../../build/docfigs		# Path to where ps2raster dumps the converted PNGs
com="-A -E${DPI} -P -Tg -D${path_build}"
pato=source/fig_includes/		# Path to where the to-be-included files will be created

if [ ! -d "${path_build}" ]; then
	mkdir -p ${path_build}
fi

function from_scripts {
ps2raster ../scripts/${name}.ps $com
W=`grdinfo -C ${path_build}/${name}.png | awk '{print $3}'`
H=`grdinfo -C ${path_build}/${name}.png | awk '{print $5}'`
echo ".. figure:: ../${path_build}/${name}.png" > ${pato}/fig_${name}.rst_
echo "    :height: $H px" >> ${pato}/fig_${name}.rst_
echo "    :width: $W px" >> ${pato}/fig_${name}.rst_
echo "    :align: center" >> ${pato}/fig_${name}.rst_
echo "    :scale: 50 %" >> ${pato}/fig_${name}.rst_
echo "" >> ${pato}/fig_${name}.rst_
}

function from_examples {
ps2raster ../examples/ex$1/example_$1.ps $com
W=`grdinfo -C ${path_build}/${name}.png | awk '{print $3}'`
H=`grdinfo -C ${path_build}/${name}.png | awk '{print $5}'`
echo ".. figure:: ../${path_build}/${name}.png" > ${pato}/fig_${name}.rst_
echo "   :height: $H px" >> ${pato}/fig_${name}.rst_
echo "   :width: $W px" >> ${pato}/fig_${name}.rst_
echo "   :align: center" >> ${pato}/fig_${name}.rst_
# Compute scale such image ~= 15 cm Width on a 96 DPI screen
scale=`echo | awk -v D=$DPI -v W=$W '{printf "%d",  15 / (W / D * 2.54) * 96 / D * 100}'`
echo "   :scale: $scale %" >> ${pato}/fig_${name}.rst_
echo "" >> ${pato}/fig_${name}.rst_
}

function from_fig {
W=`grdinfo -C ../fig/${name} | awk '{print $3}'`
H=`grdinfo -C ../fig/${name} | awk '{print $5}'`
echo ".. figure:: ../../fig/${name}" > ${pato}/fig_${name}.rst_
echo "    :height: $H px" >> ${pato}/fig_${name}.rst_
echo "    :width: $W px" >> ${pato}/fig_${name}.rst_
echo "    :align: center" >> ${pato}/fig_${name}.rst_
echo "    :scale: 50 %" >> ${pato}/fig_${name}.rst_
echo "" >> ${pato}/fig_${name}.rst_
}

name=example_01;	from_examples 01
echo "   Figure ex_01: Contour maps of gridded data." >> ${pato}/fig_${name}.rst_
name=example_02;	from_examples 02
echo "   Figure ex_02: Color images from gridded data." >> ${pato}/fig_${name}.rst_
name=example_03;	from_examples 03
echo "   Figure ex_03: Spectral estimation and x=y-plots." >> ${pato}/fig_${name}.rst_
name=example_04;	from_examples 04
echo "   Figure ex_04: 3-D perspective mesh plot (left) and colored version (right)." >> ${pato}/fig_${name}.rst_
name=example_05;	from_examples 05
echo "   Figure ex_05: 3-D illuminated surface." >> ${pato}/fig_${name}.rst_
name=example_06;	from_examples 06
echo "   Figure ex_06: Two kinds of histograms." >> ${pato}/fig_${name}.rst_
name=example_07;	from_examples 07
echo "   Figure ex_07: A typical location map." >> ${pato}/fig_${name}.rst_
name=example_08;	from_examples 08
echo "   Figure ex_08: A 3-D histogram." >> ${pato}/fig_${name}.rst_
name=example_09;	from_examples 09
echo "   Figure ex_09: Time-series as "wiggles" along a track." >> ${pato}/fig_${name}.rst_
name=example_10;	from_examples 10
echo "   Figure ex_10: Geographical bar graph." >> ${pato}/fig_${name}.rst_
name=example_11;	from_examples 11
echo "   Figure ex_11: The RGB color cube." >> ${pato}/fig_${name}.rst_
name=example_12;	from_examples 12
echo "   Figure ex_12: Optimal triangulation of data." >> ${pato}/fig_${name}.rst_
name=example_13;	from_examples 13
echo "   Figure ex_13: Display of vector fields in GMT." >> ${pato}/fig_${name}.rst_
name=example_14;	from_examples 14
echo "   Figure ex_14: Gridding of data and trend surfaces." >> ${pato}/fig_${name}.rst_
name=example_15;	from_examples 15
echo "   Figure ex_15: Gridding, contouring, and masking of data." >> ${pato}/fig_${name}.rst_
name=example_16;	from_examples 16
echo "   Figure ex_16: More ways to grid data." >> ${pato}/fig_${name}.rst_
name=example_17;	from_examples 17
echo "   Figure ex_17: Clipping of images using coastlines." >> ${pato}/fig_${name}.rst_
name=example_18;	from_examples 18
echo "   Figure ex_18: Volumes and geo-spatial selections." >> ${pato}/fig_${name}.rst_
name=example_19;	from_examples 19
echo "   Figure ex_19: Using color patterns and additional PostScript material in illustrations." >> ${pato}/fig_${name}.rst_
name=example_20;	from_examples 20
echo "   Figure ex_20: Using custom symbols in GMT." >> ${pato}/fig_${name}.rst_
name=example_21;	from_examples 21
echo "   Figure ex_21: Time-series of RedHat stock price since IPO." >> ${pato}/fig_${name}.rst_
name=example_22;	from_examples 22
echo "   Figure ex_22: World-wide seismicity the last 7 days." >> ${pato}/fig_${name}.rst_
name=example_23;	from_examples 23
echo "   Figure ex_23: All great-circle paths lead to Rome." >> ${pato}/fig_${name}.rst_
name=example_24;	from_examples 24
echo "   Figure ex_24: Data selection based on geospatial criteria." >> ${pato}/fig_${name}.rst_
name=example_25;	from_examples 25
echo "   Figure ex_25: Global distribution of antipodes." >> ${pato}/fig_${name}.rst_
name=example_26;	from_examples 26
echo "   Figure ex_26: General vertical perspective projection." >> ${pato}/fig_${name}.rst_
name=example_27;	from_examples 27
echo "   Figure ex_27: Plotting Sandwell/Smith Mercator img grids." >> ${pato}/fig_${name}.rst_
name=example_28;	from_examples 28
echo "   Figure ex_28: Mixing UTM and geographic data sets requires knowledge of the map region domain in" >> ${pato}/fig_${name}.rst_
echo "   both UTM and lon/lat coordinates and consistent use of the same map scale." >> ${pato}/fig_${name}.rst_
name=example_29;	from_examples 29
echo "   Figure ex_29: Gridding of spherical surface data using Green's function splines." >> ${pato}/fig_${name}.rst_
name=example_30;	from_examples 30
echo "   Figure ex_30: Trigonometric functions plotted in graph mode." >> ${pato}/fig_${name}.rst_
name=example_31;	from_examples 31
echo "   Figure ex_31: Using non-default fonts in *PostScript*." >> ${pato}/fig_${name}.rst_
name=example_32;	from_examples 32
echo "   Figure ex_32: Draping an image over topography." >> ${pato}/fig_${name}.rst_
name=example_33;	from_examples 33
echo "   Figure ex_33: Stacking automatically generated cross-profiles." >> ${pato}/fig_${name}.rst_
name=example_34;	from_examples 34
echo "   Figure ex_34: Using country polygons for plotting and shading." >> ${pato}/fig_${name}.rst_
name=example_35;	from_examples 35
echo "   Figure ex_35: Spherical triangulation and distance calculations" >> ${pato}/fig_${name}.rst_
name=example_36;	from_examples 36
echo "   Figure ex_36: Spherical gridding using Renka\'s algorithms." >> ${pato}/fig_${name}.rst_
name=example_37;	from_examples 37
echo "   Figure ex_37: Spectral coherence between gravity and bathymetry grids." >> ${pato}/fig_${name}.rst_
name=example_38;	from_examples 38
echo "   Figure ex_38: Histogram equalization of bathymetry grids." >> ${pato}/fig_${name}.rst_
name=example_39;	from_examples 39
echo "   Figure ex_39: Evaluation of spherical harmonics coefficients." >> ${pato}/fig_${name}.rst_

name=GMT_App_E;			from_scripts
name=GMT_App_F_dingbats;	from_scripts
name=GMT_App_F_iso+;		from_scripts
name=GMT_App_F_stand+;		from_scripts
name=GMT_App_F_symbol;		from_scripts
echo "    Figure F.1: Octal codes and corresponding symbols for StandardEncoding (left) and ISOLatin1Encoding (right) fonts." >> ${pato}/fig_${name}.rst_

name=GMT_App_G;			from_scripts
echo "    The standard 35 **PS** fonts recognized by *GMT*." >> ${pato}/fig_${name}.rst_

name=GMT_App_M_1;		from_scripts
echo "    Figure M.1: The standard 22 CPT files supported by GMT" >> ${pato}/fig_${name}.rst_

name=GMT_App_M_2;		from_scripts
name=GMT_App_P_2;		from_scripts
echo "    Figure P.1: Example created in isolation mode" >> ${pato}/fig_${name}.rst_

name=GMT_color_interpolate;	from_scripts
echo "    Figure I.4: When interpolating colors, the color system matters. The polar palette on the left needs to" >> ${pato}/fig_${name}.rst_
echo "    be interpolated in RGB, otherwise hue will change between blue (240) and white (0). The rainbow" >> ${pato}/fig_${name}.rst_
echo "    palette should be interpolated in HSV, since only hue should change between magenta (300) and red (0)." >> ${pato}/fig_${name}.rst_
echo "    Diamonds indicate which colors are defined in the palettes; they are fixed, the rest is interpolated." >> ${pato}/fig_${name}.rst_

name=GMT_coverlogo;		from_scripts

name=GMT_RGBchart_a4;		from_scripts
echo "    Figure I.2: The 555 unique color names that can be used in GMT. Lower, upper, or mixed cases, as well as" >> ${pato}/fig_${name}.rst_
echo "    the british spelling of "grey" are allowed. A4, Letter, and Tabloid sized versions of this RGB chart can be" >> ${pato}/fig_${name}.rst_
echo "    found in the GMT documentation directory." >> ${pato}/fig_${name}.rst_

name=GMT_volcano;		from_scripts

name=GMT_Defaults_1a;		from_scripts
echo "    Some *GMT* parameters that affect plot appearance." >> ${pato}/fig_${name}.rst_

name=GMT_Defaults_1b;		from_scripts
echo "    Figure: More *GMT* parameters that affect plot appearance." >> ${pato}/fig_${name}.rst_

name=GMT_Defaults_1c;		from_scripts
echo "    Even more *GMT* parameters that affect plot appearance." >> ${pato}/fig_${name}.rst_

name=GMT_-R;			from_scripts
echo "    The plot region can be specified in two different ways. (a) Extreme values" >> ${pato}/fig_${name}.rst_
echo "    for each dimension, or (b) coordinates of lower left and upper right corners." >> ${pato}/fig_${name}.rst_

name=GMT_-J;			from_scripts
echo "    The 30+ map projections and coordinate transformations available in *GMT*." >> ${pato}/fig_${name}.rst_

name=GMT_-B_geo_1;		from_scripts
echo "    Geographic map border using separate selections for annotation," >> ${pato}/fig_${name}.rst_
echo "    frame, and grid intervals.  Formatting of the annotation is controlled by" >> ${pato}/fig_${name}.rst_
echo "    the parameter **FORMAT_GEO_MAP** in your ``gmt.conf``." >> ${pato}/fig_${name}.rst_

name=GMT_-B_geo_2;		from_scripts
echo "    Geographic map border with both primary (P) and secondary (S) components." >> ${pato}/fig_${name}.rst_

name=GMT_-B_linear;		from_scripts
echo "    Linear Cartesian projection axis.  Long tickmarks accompany" >> ${pato}/fig_${name}.rst_
echo "    annotations, shorter ticks indicate frame interval. The axis label is" >> ${pato}/fig_${name}.rst_
echo "    optional. For this example we used -R0/12/0/1 -JX3i/0.4i -Ba4f2g1:Frequency:" >> ${pato}/fig_${name}.rst_

name=GMT_-B_log;		from_scripts
echo "    Logarithmic projection axis using separate values for annotation," >> ${pato}/fig_${name}.rst_
echo "    frame, and grid intervals.  (top) Here, we have chosen to annotate the actual" >> ${pato}/fig_${name}.rst_
echo "    values.  Interval = 1 means every whole power of 10, 2 means 1, 2, 5 times" >> ${pato}/fig_${name}.rst_
echo "    powers of 10, and 3 means every 0.1 times powers of 10.  We used" >> ${pato}/fig_${name}.rst_
echo "    -R1/1000/0/1 -JX3il/0.4i -Ba1f2g3.  (middle) Here, we have chosen to" >> ${pato}/fig_${name}.rst_
echo "    annotate log :math:\`_10\` of the actual values, with -Ba1f2g3l." >> ${pato}/fig_${name}.rst_
echo "    (bottom) We annotate every power of 10 using log :math:\`_10\` of the actual values" >> ${pato}/fig_${name}.rst_
echo "    as exponents, with -Ba1f2g3p." >> ${pato}/fig_${name}.rst_

name=GMT_-B_pow;		from_scripts
echo "    Exponential or power projection axis. (top) Using an exponent of 0.5" >> ${pato}/fig_${name}.rst_
echo "    yields a :math:\`sqrt(x)\` axis.  Here, intervals refer to actual data values, in" >> ${pato}/fig_${name}.rst_
echo "    -R0/100/0/1 -JX3ip0.5/0.4i -Ba20f10g5." >> ${pato}/fig_${name}.rst_
echo "    (bottom) Here, intervals refer to projected values, although the annotation" >> ${pato}/fig_${name}.rst_
echo "    uses the corresponding unprojected values, as in -Ba3f2g1p." >> ${pato}/fig_${name}.rst_

name=GMT_-B_time1;		from_scripts
echo "    Cartesian time axis, example 1." >> ${pato}/fig_${name}.rst_

name=GMT_-B_time2;		from_scripts
echo "    Cartesian time axis, example 2." >> ${pato}/fig_${name}.rst_

name=GMT_-B_time3;		from_scripts
echo "    Cartesian time axis, example 3." >> ${pato}/fig_${name}.rst_

name=GMT_-B_time4;		from_scripts
echo "    Cartesian time axis, example 4." >> ${pato}/fig_${name}.rst_

name=GMT_-B_time5;		from_scripts
echo "    Cartesian time axis, example 5." >> ${pato}/fig_${name}.rst_

name=GMT_-B_time6;		from_scripts
echo "    Cartesian time axis, example 6." >> ${pato}/fig_${name}.rst_

name=GMT_-B_time7;		from_scripts
echo "    Cartesian time axis, example 7." >> ${pato}/fig_${name}.rst_

name=GMT_-B_custom;		from_scripts
echo "    Custom and irregular annotations, tick-marks, and gridlines." >> ${pato}/fig_${name}.rst_

name=GMT_-P;			from_scripts
echo "    Users can specify Landscape [Default] or Portrait -P) orientation." >> ${pato}/fig_${name}.rst_

name=GMT_-OK;			from_scripts
echo "    A final **PS** file consists of any number of individual pieces." >> ${pato}/fig_${name}.rst_

name=GMT_-U;			from_scripts
echo "    The -U option makes it easy to ``date`` a plot." >> ${pato}/fig_${name}.rst_

name=GMT_-XY;			from_scripts
echo "    Plot origin can be translated freely with -X -Y." >> ${pato}/fig_${name}.rst_

name=GMT_registration;		from_scripts
echo "    Gridline- and pixel-registration of data nodes." >> ${pato}/fig_${name}.rst_

name=GMT_linecap;		from_scripts
echo "    Line appearance can be varied by using **PS_LINE_CAP**" >> ${pato}/fig_${name}.rst_

name=GMT_arrows;		from_scripts
echo "    Examples of Cartesian (left), circular (middle), and geo-vectors (right) for different" >> ${pato}/fig_${name}.rst_
echo "    attribute specifications. Note that both full and half arrow-heads can be specified, as well as no head at all." >> ${pato}/fig_${name}.rst_

name=GMT_linear;		from_scripts
echo "    Linear transformation of Cartesian coordinates." >> ${pato}/fig_${name}.rst_

name=GMT_linear_d;		from_scripts
echo "    Linear transformation of map coordinates." >> ${pato}/fig_${name}.rst_

name=GMT_linear_cal;		from_scripts
echo "    Linear transformation of calendar coordinates." >> ${pato}/fig_${name}.rst_

name=GMT_log;			from_scripts
echo "    Logarithmic transformation of $x$-coordinates." >> ${pato}/fig_${name}.rst_

name=GMT_pow;			from_scripts
echo "    Exponential or power transformation of $x$-coordinates." >> ${pato}/fig_${name}.rst_

name=GMT_polar;			from_scripts
echo "    Polar (Cylindrical) transformation of (:math:\`\theta, r\`) coordinates." >> ${pato}/fig_${name}.rst_

name=GMT_albers;		from_scripts
echo "    Albers equal-area conic map projection." >> ${pato}/fig_${name}.rst_

name=GMT_equidistant_conic;	from_scripts
echo "    Equidistant conic map projection." >> ${pato}/fig_${name}.rst_

name=GMT_lambert_conic;		from_scripts
echo "    Lambert conformal conic map projection." >> ${pato}/fig_${name}.rst_

name=GMT_polyconic;		from_scripts
echo "    (American) polyconic projection." >> ${pato}/fig_${name}.rst_

name=GMT_lambert_az_rect;	from_scripts
echo "    Rectangular map using the Lambert azimuthal equal-area projection." >> ${pato}/fig_${name}.rst_

name=GMT_lambert_az_hemi;	from_scripts
echo "    Hemisphere map using the Lambert azimuthal equal-area projection." >> ${pato}/fig_${name}.rst_

name=GMT_stereonets;		from_scripts
echo "    Equal-Area (Schmidt) and Equal-Angle (Wulff) stereo nets." >> ${pato}/fig_${name}.rst_

name=GMT_stereographic_polar;	from_scripts
echo "    Polar stereographic conformal projection." >> ${pato}/fig_${name}.rst_

name=GMT_stereographic_rect;	from_scripts
echo "    Polar stereographic conformal projection with rectangular borders." >> ${pato}/fig_${name}.rst_

name=GMT_stereographic_general;	from_scripts
echo "    General stereographic conformal projection with rectangular borders." >> ${pato}/fig_${name}.rst_

name=GMT_perspective;		from_scripts
echo "    View from the Space Shuttle in Perspective projection." >> ${pato}/fig_${name}.rst_

name=GMT_orthographic;		from_scripts
echo "    Hemisphere map using the Orthographic projection." >> ${pato}/fig_${name}.rst_

name=GMT_az_equidistant;	from_scripts
echo "    World map using the equidistant azimuthal projection." >> ${pato}/fig_${name}.rst_

name=GMT_gnomonic;		from_scripts
echo "    Gnomonic azimuthal projection." >> ${pato}/fig_${name}.rst_

name=GMT_mercator;		from_scripts
echo "    Simple Mercator map." >> ${pato}/fig_${name}.rst_

name=GMT_transverse_merc;	from_scripts
echo "    Rectangular Transverse Mercator map." >> ${pato}/fig_${name}.rst_

name=GMT_TM;			from_scripts
echo "    A global transverse Mercator map." >> ${pato}/fig_${name}.rst_

name=GMT_utm_zones;		from_scripts
echo "    Figure : Universal Transverse Mercator zone layout." >> ${pato}/fig_${name}.rst_

name=GMT_obl_merc;		from_scripts
echo "    Oblique Mercator map using **-Joc**. We make it clear which direction is North by" >> ${pato}/fig_${name}.rst_
echo "    adding a star rose with the **-T** option.}" >> ${pato}/fig_${name}.rst_

name=GMT_cassini;		from_scripts
echo "    Cassini map over Sardinia." >> ${pato}/fig_${name}.rst_

name=GMT_equi_cyl;		from_scripts
echo "    World map using the Plate Carr\'{e}e projection." >> ${pato}/fig_${name}.rst_

name=GMT_general_cyl;		from_scripts
echo "    World map using the Behrman cylindrical equal-area projection." >> ${pato}/fig_${name}.rst_

name=GMT_miller;		from_scripts
echo "    World map using the Miller cylindrical projection." >> ${pato}/fig_${name}.rst_

name=GMT_gall_stereo;		from_scripts
echo "    World map using Gall's stereographic projection." >> ${pato}/fig_${name}.rst_

name=GMT_hammer;		from_scripts
echo "    World map using the Hammer projection." >> ${pato}/fig_${name}.rst_

name=GMT_mollweide;		from_scripts
echo "    World map using the Mollweide projection." >> ${pato}/fig_${name}.rst_

name=GMT_winkel;		from_scripts
echo "    World map using the Winkel Tripel projection." >> ${pato}/fig_${name}.rst_

name=GMT_robinson;		from_scripts
echo "    World map using the Robinson projection." >> ${pato}/fig_${name}.rst_

name=GMT_eckert4;		from_scripts
echo "    World map using the Eckert IV projection." >> ${pato}/fig_${name}.rst_

name=GMT_eckert6;		from_scripts
echo "    World map using the Eckert VI projection." >> ${pato}/fig_${name}.rst_

name=GMT_sinusoidal;		from_scripts
echo "    World map using the Sinusoidal projection." >> ${pato}/fig_${name}.rst_

name=GMT_sinus_int;		from_scripts
echo "    World map using the Interrupted Sinusoidal projection." >> ${pato}/fig_${name}.rst_

name=GMT_grinten;		from_scripts
echo "    World map using the Van der Grinten projection." >> ${pato}/fig_${name}.rst_

name=GMT_App_J_1;		from_scripts
echo "    Impulse responses for *GMT* filters." >> ${pato}/fig_${name}.rst_

name=GMT_App_J_2;		from_scripts
echo "    Transfer functions for 1-D *GMT* filters." >> ${pato}/fig_${name}.rst_

name=GMT_App_J_3;		from_scripts
echo "    Transfer functions for 2-D (radial) *GMT* filters." >> ${pato}/fig_${name}.rst_

name=GMT_App_K_1;		from_scripts
echo "    Map using the crude resolution coastline data." >> ${pato}/fig_${name}.rst_

name=GMT_App_K_2;		from_scripts
echo "    Map using the low resolution coastline data." >> ${pato}/fig_${name}.rst_

name=GMT_App_K_3;		from_scripts
echo "    Map using the intermediate resolution coastline data. We have added a compass" >> ${pato}/fig_${name}.rst_
echo "    rose just because we have the power to do so." >> ${pato}/fig_${name}.rst_

name=GMT_App_K_4;		from_scripts
echo "    Map using the high resolution coastline data." >> ${pato}/fig_${name}.rst_

name=GMT_App_K_5;		from_scripts
echo "    Map using the full resolution coastline data." >> ${pato}/fig_${name}.rst_

name=GMT_App_N_1;		from_scripts
echo "    Custom plot symbols supported by *GMT*. Note that we only show" >> ${pato}/fig_${name}.rst_
echo "    the symbol outline and not any fill. These are all single-parameter symbols." >> ${pato}/fig_${name}.rst_
echo "    Be aware that some symbols may have a hardwired fill or no-fill component," >> ${pato}/fig_${name}.rst_
echo "    while others duplicate what is already available as standard built-in symbols." >> ${pato}/fig_${name}.rst_

name=GMT_App_O_1;		from_scripts
echo "    Equidistant contour label placement with **-Gd**, the only algorithm" >> ${pato}/fig_${name}.rst_
echo "    available in previous *GMT* versions." >> ${pato}/fig_${name}.rst_

name=GMT_App_O_2;		from_scripts
echo "    Placing one label per contour that exceed 1 inch in length, centered on the segment with **-Gn**." >> ${pato}/fig_${name}.rst_

name=GMT_App_O_3;		from_scripts
echo "    Four labels are positioned on the points along the contours that are" >> ${pato}/fig_${name}.rst_
echo "    closest to the locations given in the file ``fix.txt`` in the **-Gf** option." >> ${pato}/fig_${name}.rst_

name=GMT_App_O_4;		from_scripts
echo "    Labels are placed at the intersections between contours and the great circle specified in the **-GL** option." >> ${pato}/fig_${name}.rst_

name=GMT_App_O_5;		from_scripts
echo "    Labels are placed at the intersections between contours and the" >> ${pato}/fig_${name}.rst_
echo "    multi-segment lines specified in the **-GX** option." >> ${pato}/fig_${name}.rst_

name=GMT_App_O_6;		from_scripts
echo "    Labels attributes are controlled with the arguments to the **-Sq** option." >> ${pato}/fig_${name}.rst_

name=GMT_App_O_7;		from_scripts
echo "    Another label attribute example." >> ${pato}/fig_${name}.rst_

name=GMT_App_O_8;		from_scripts
echo "    Labels based on another data set (here bathymetry) while the placement is based on distances." >> ${pato}/fig_${name}.rst_

name=GMT_App_O_9;	from_scripts
echo "    Tsunami travel times from the Canary Islands to places in the Atlantic," >> ${pato}/fig_${name}.rst_
echo "    in particular New York. Should a catastrophic landslide occur it is possible" >> ${pato}/fig_${name}.rst_
echo "    that New York will experience a large tsunami about 8 hours after the event." >> ${pato}/fig_${name}.rst_

name=GMT5_Summit_2011.jpg;	from_fig
echo "    Figure 1: The four horsemen of the GMT apocalypse: Remko Scharroo, Paul Wessel, Walter H.F. Smith," >> ${pato}/fig_${name}.rst_
echo "    and Joaquim Luis at the GMT Developer Summit in Honolulu, Hawaii during February 14-18, 2011." >> ${pato}/fig_${name}.rst_

name=hsv-cone.png;	from_fig
echo "    Figure I.1: The HSV color space" >> ${pato}/fig_${name}.rst_ 

name=rendering.png;		from_fig
echo "    Figure C.1: Examples of rendered images in a PowerPoint presentation" >> ${pato}/fig_${name}.rst_ 

name=formatpicture.png;	from_fig
echo "    Figure C.2: **PowerPoint**'s "Format Picture" dialogue to set scale and rotation." >> ${pato}/fig_${name}.rst_

name=GMT4_mode.png;	from_fig
echo "    Figure 1: GMT 4 programs contain all the high-level functionality." >> ${pato}/fig_${name}.rst_ 

name=GMT5_mode.png;	from_fig
echo "    Figure 2: GMT 5 programs contain all the high-level functionality." >> ${pato}/fig_${name}.rst_ 

name=gimp-sliders.png;	from_fig

name=gimp-panel.png;	from_fig
echo "    Chartreuse in *GIMP*. *(a)* Sliders indicate how the color is altered" >> ${pato}/fig_${name}.rst_ 
echo "    when changing the H, S, V, R, G, or B levels. *(b)* For a constant hue (here 90)" >> ${pato}/fig_${name}.rst_
echo "    value increases to the right and saturation increases up, so the "pure"" >> ${pato}/fig_${name}.rst_ 
echo "    color is on the top right." >> ${pato}/fig_${name}.rst_
