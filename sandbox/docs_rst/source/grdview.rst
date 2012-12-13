*******
grdview
*******

grdview - Create 3-D perspective image or surface mesh from a grid

`Synopsis <#toc1>`_
-------------------

**grdview** *relief\_file* **-J**\ *parameters* [
**-B**\ [**p**\ \|\ **s**]\ *parameters* ] [ **-C**\ *cptfile*] [
**-G**\ *drapefile* \| **-G**\ *grd\_r*,\ *grd\_g*,\ *grd\_b* ] [
**-I**\ *intensfile* ] [ **-Jz**\ \|\ **Z**\ *parameters* ] [ **-K** ] [
**-N**\ *level*\ [/*color*] ] [ **-O** ] [ **-P** ] [
**-Q**\ *type*\ [**g**\ ] ] [
**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ] ] [
**-S**\ *smooth* ] [ **-T**\ [**s**\ ][\ **o**\ [*pen*\ ]] ] [
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [ **-V**\ [*level*\ ]
] [ **-W**\ **type**\ *pen* ] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
] [ **-c**\ *copies* ] [
**-n**\ [**b**\ \|\ **c**\ \|\ **l**\ \|\ **n**][**+a**\ ][\ **+b**\ *BC*][\ **+t**\ *threshold*]
] [
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
] [ **-t**\ [*transp*\ ] ]

`Description <#toc2>`_
----------------------

**grdview** reads a 2-D grid file and produces a 3-D perspective plot by
drawing a mesh, painting a colored/grayshaded surface made up of
polygons, or by scanline conversion of these polygons to a raster image.
Options include draping a data set on top of a surface, plotting of
contours on top of the surface, and apply artificial illumination based
on intensities provided in a separate grid file.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*relief\_file*
    2-D gridded data set to be imaged (the relief of the surface). (See
    GRID FILE FORMAT below.)
**-J**\ *parameters* (\*)
    Select map projection.
**-Jz**\ \|\ **Z**\ *parameters* (\*)
    Set z-axis scaling; same syntax as **-Jx**.

`Optional Arguments <#toc5>`_
-----------------------------

**-B**\ [**p**\ \|\ **s**]\ *parameters* (\*)
    Set map boundary intervals.
**-C**\ *cptfile*
    name of the color palette file. Must be present if you `want
    (1) <want.html>`_ mesh plot with contours (**-Qm**), `or
    (2) <or.2.html>`_ shaded/colored perspective image (**-Qs** or
    **-Qi**). For **-Qs**: You can specify that you want to skip a
    z-slice by setting red = -; to use a pattern give red =
    **P\|p**\ *dpi/pattern*\ [:**F**\ *color*\ [**B**\ *color*]].
**-G**\ *drapefile* \| **-G**\ *grd\_r*,\ *grd\_g*,\ *grd\_b*
    Drape the image in *drapefile* on top of the relief provided by
    *relief\_file*. [Default is *relief\_file*]. Note that **-Jz** and
    **-N** always refers to the *relief\_file*. The *drapefile* only
    provides the information pertaining to colors, which is looked-up
    via the cpt file (see **-C**). Alternatively, give three grid files
    separated by commas. These files must contain the red, green, and
    blue colors directly (in 0-255 range) and no cpt file is needed. The
    *drapefile* may be of higher resolution than the *relief\_file*.
**-I**\ *intensfile*
    Gives the name of a grid file with intensities in the (-1,+1) range.
    [Default is no illumination].
**-K** (\*)
    Do not finalize the *PostScript* plot.
**-N**\ *level*\ [/*color*]
    Draws a plane at this z-level. If the optional *color* is provided,
    the frontal facade between the plane and the data perimeter is
    colored. See **-Wf** for setting the pen used for the outline.
**-O** (\*)
    Append to existing *PostScript* plot.
**-P** (\*)
    Select "Portrait" plot orientation.
**-Q**\ *type*\ [**g**\ ]
    Select one of four settings: 1. Specify **m** for mesh plot
    [Default], and optionally append /*color* for a different mesh paint
    [white]. 2. Specify **s** for surface plot, and optionally append
    **m** to have mesh lines drawn on top of surface. 3. Specify **i**
    for image plot, and optionally append the effective dpi resolution
    for the rasterization [100]. 4. Specify **c**. Same as **-Qi** but
    will make nodes with z = NaN transparent, using the colormasking
    feature in *PostScript* Level 3 (the PS device must support PS Level
    3). For any of these choices, you may force a monochrome image by
    appending **g**. Colors are then converted to shades of gray using
    the (television) YIQ transformation.
**-R**\ [*unit*\ ]\ *xmin*/*xmax*/*ymin*/*ymax*\ [**r**\ ] (\*)
    Specify the region of interest.
    For perspective view (**-p**), optionally append /*zmin*/*zmax*.
    This option may be used to indicate the range used for the 3-D axes
    [Default is region given by the *relief\_file*]. You may ask for a
    larger *w/e/s/n* region to have more room between the image and the
    axes. A smaller region than specified in the *relief\_file* will
    result in a subset of the grid.
**-S**\ *smooth*
    Smooth the contours before plotting (see **grdcontour**) [Default is
    no smoothing].
**-T**\ [**s**\ ][\ **o**\ [*pen*\ ]]
    Plot image without any interpolation. This involves converting each
    node-centered bin into a polygon which is then painted separately.
    Append **s** to skip nodes with z = NaN. This option is useful for
    categorical data where interpolating between values is meaningless.
    Optionally, append **o** to draw the tile outlines, and specify a
    custom pen if the default pen is not to your liking. As this option
    produces a flat surface it cannot be combined with **-JZ** or
    **-Jz**.
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] (\*)
    Draw GMT time stamp logo on plot.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-W**\ **type**\ *pen*
**-Wc**
    Draw contour lines on top of surface or mesh (not image). Append pen
    attributes used for the contours. [Default: width = 0.75p, color =
    black, style = solid].
**-Wm**
    Sets the pen attributes used for the mesh. [Default: width = 0.25p,
    color = black, style = solid]. You must also select **-Qm** or
    **-Qsm** for meshlines to be drawn.
**-Wf**
    Sets the pen attributes used for the facade. [Default: width =
    0.25p, color = black, style = solid]. You must also select **-N**
    for the facade outline to be drawn.
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
(\*)
    Shift plot origin.
**-c**\ *copies* (\*)
    Specify number of plot copies [Default is 1].
**-n**\ [**b**\ \|\ **c**\ \|\ **l**\ \|\ **n**][**+a**\ ][\ **+b**\ *BC*][\ **+t**\ *threshold*]
(\*)
    Select interpolation mode for grids.
**-p**\ *azim*/*elev*\ [/*zlevel*][\ **+w**\ *lon0*/*lat0*\ [/*z0*]][\ **+v**\ *x0*/*y0*]
(\*)
    Select perspective view.
**-t**\ [*transp*\ ] (\*)
    Set PDF transparency level.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

`Grid File Formats <#toc6>`_
----------------------------

**GMT** is able to recognize many of the commonly used grid file
formats, as well as the precision, scale and offset of the values
contained in the grid file. When **GMT** needs a little help with that,
you can add the suffix
**=**\ *id*\ [**/**\ *scale*\ **/**\ *offset*\ [**/**\ *nan*]], where
*id* is a two-letter identifier of the grid type and precision, and
*scale* and *offset* are optional scale factor and offset to be applied
to all grid values, and *nan* is the value used to indicate missing
data. See `**grdreformat**\ (1) <grdreformat.html>`_ and Section 4.17 of
the GMT Technical Reference and Cookbook for more information.

When reading a netCDF file that contains multiple grids, **GMT** will
read, by default, the first 2-dimensional grid that can find in that
file. To coax **GMT** into reading another multi-dimensional variable in
the grid file, append **?**\ *varname* to the file name, where *varname*
is the name of the variable. Note that you may need to escape the
special meaning of **?** in your shell program by putting a backslash in
front of it, or by placing the filename and suffix between quotes or
double quotes. See `**grdreformat**\ (1) <grdreformat.html>`_ and
Section 4.18 of the GMT Technical Reference and Cookbook for more
information, particularly on how to read splices of 3-, 4-, or
5-dimensional grids.

`Examples <#toc7>`_
-------------------

To make a mesh plot from the file hawaii\_grav.nc and drawing the
contours given in the color palette file hawaii.cpt on a Lambert map at
1.5 cm/degree along the standard parallels 18 and 24, with vertical
scale 20 mgal/cm, and looking at the surface from SW at 30 degree
elevation, run

grdview hawaii\_grav.nc **-Jl**\ 18/24/1.5\ **c** -Chawaii.cpt
**-Jz**\ 0.05\ **c** -Qm -N-100 -p225/30 -Wc > hawaii\_grav\_image.ps

To create a illuminated color perspective plot of the gridded data set
image.nc, using the color palette file color.rgb, with linear scaling at
10 cm/x-unit and tickmarks every 5 units, with intensities provided by
the file intens.nc, and looking from the SE, use

grdview image.nc **-Jx**\ 10.0\ **c** -Ccolor.rgb -Qs -p135/30
-Iintens.nc > image3D.ps

To make the same plot using the rastering option with dpi = 50, use

grdview image.nc **-Jx**\ 10.0\ **c** -Ccolor.rgb -Qi50 -p135/30
-Iintens.nc > image3D.ps

To create a color *PostScript* perspective plot of the gridded data set
magnetics.nc, using the color palette file mag\_intens.cpt, draped over
the relief given by the file topography.nc, with Mercator map width of 6
inch and tickmarks every 1 degree, with intensities provided by the file
topo\_intens.nc, and looking from the SE, run

grdview topography.nc **-JM**\ 6\ **i** -Gmagnetics.nc -Cmag\_intens.cpt
-Qs -p140/30 -Itopo\_intens.nc > draped3D.ps

Given topo.nc and the Landsat image veggies.ras, first run **grd2rgb**
to get the red, green, and blue grids, and then drape this image over
the topography and shade the result for good measure. The commands are

grd2rgb veggies.ras -Glayer\_%c.nc

grdview topo.nc **-JM**\ 6\ **i** -Qi -p140/30 -Itopo\_intens.nc
-Glayer\_r.nc,layer\_g.nc,layer\_b.nc > image.ps

`Remarks <#toc8>`_
------------------

For the **-Qs** option: *PostScript* provides no way of smoothly varying
colors within a polygon, so colors can only vary from polygon to
polygon. To obtain smooth images this way you may resample the grid
file(s) using **grdsample** or use a finer grid size when running
gridding programs like **surface** or **nearneighbor**. Unfortunately,
this produces huge *PostScript* files. The alternative is to use the
**-Qi** option, which computes bilinear or bicubic continuous color
variations within polygons by using scanline conversion to image the
polygons.

`See Also <#toc9>`_
-------------------

`*gmt*\ (1) <gmt.html>`_ , `*grd2rgb*\ (1) <grd2rgb.html>`_ ,
`*gmtcolors*\ (5) <gmtcolors.html>`_ ,
`*grdcontour*\ (1) <grdcontour.html>`_ ,
`*grdimage*\ (1) <grdimage.html>`_ ,
`*nearneighbor*\ (1) <nearneighbor.html>`_ ,
`*psbasemap*\ (1) <psbasemap.html>`_ ,
`*pscontour*\ (1) <pscontour.html>`_ , `*pstext*\ (1) <pstext.html>`_ ,
`*surface*\ (1) <surface.html>`_
