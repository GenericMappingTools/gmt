******
psvelo
******

psvelo - Plot velocity vectors, crosses, and wedges on maps

`Synopsis <#toc1>`_
-------------------

**psvelo** [ *table* ] **-J**\ *parameters*
**-R**\ *west*/*east*/*south*/*north*\ [**r**\ ] [ **-A**\ *parameters*
] [ **-B**\ [**p**\ \|\ **s**]\ *parameters* ] [ **-D**\ *sigma\_scale*
] [ **-F**\ *color* ] [ **-E**\ *color* ] [ **-G**\ *fill* ] [ **-K** ]
[ **-L** ] [ **-N** ] [ **-O** ] [ **-P** ] [
**-S**\ *symbol*/*scale*/*conf*/*font\_size* ] [
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] ] [ **-V**\ [*level*\ ]
] [ **-W**\ *pen* ] [
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
] [
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
] [ **-c**\ *copies* ] [ **-h**\ [**i**\ \|\ **o**][*n*\ ] ] [
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*]
] [ **-:**\ [**i**\ \|\ **o**] ]

`Description <#toc2>`_
----------------------

**psvelo** reads data values from *files* [or standard input] and
generates *PostScript* code that will plot velocity arrows on a map.
Most options are the same as for **psxy**, except **-S**. The
*PostScript* code is written to standard output. The previous version
(**psvelomeca**) is now obsolete. It has been replaced by **psvelo** and
**psmeca**.

`Common Arguments And Specifications <#toc3>`_
----------------------------------------------

All options marked with an asterisk (\*) are common GMT command-line
options. Their full syntax as well as how to specify pens, pattern
fills, colors, and fonts can be found in the **gmt** man page. Note: No
space is allowed between the option flag and the associated arguments.

`Required Arguments <#toc4>`_
-----------------------------

*table*

One or more ASCII (or binary, see **-bi**\ [*ncols*\ ][*type*\ ]) data
table file(s) holding a number of data columns. If no tables are given
then we read from standard input.

**-J**\ *parameters* (\*)

Select map projection.

**-R**\ *west*/*east*/*south*/*north*\ [/*zmin*/*zmax*][**r**\ ]

*west*, *east*, *south*, and *north* specify the region of interest, and
you may specify them in decimal degrees or in
[+-]dd:mm[:ss.xxx][W\|E\|S\|N] format. Append **r** if lower left and
upper right map coordinates are given instead of w/e/s/n. The two
shorthands **-Rg** and **-Rd** stand for global domain (0/360 and
-180/+180 in longitude respectively, with -90/+90 in latitude).
Alternatively, specify the name of an existing grid file and the **-R**
settings (and grid spacing, if applicable) are copied from the grid.
Using **-R**\ *unit* expects projected (Cartesian) coordinates
compatible with chosen **-J** and we inversely project to determine
actual rectangular geographic region.

**-S**

Selects the meaning of the columns in the data file and the figure to be
plotted.

**-Se**\ *velscale/confidence/fontsize*.

Velocity ellipses in (N,E) convention. *Vscale* sets the scaling of the
velocity arrows. This scaling gives inches (unless **c**, **i**, **m**,
or **p** is appended). *Confidence* sets the 2-dimensional confidence
limit for the ellipse, e.g., 0.95 for 95% confidence ellipse. *Fontsize*
sets the size of the text in points. The ellipse will be filled with the
color or shade specified by the **-G** option [default transparent]. The
arrow and the circumference of the ellipse will be drawn with the pen
attributes specified by the **-W** option. Parameters are expected to be
in the following columns:

    **1**,\ **2**:
    longitude, latitude of station (**-:** option interchanges order)
    **3**,\ **4**:
    eastward, northward velocity (**-:** option interchanges order)
    **5**,\ **6**:
    uncertainty of eastward, northward velocities (1-sigma) (**-:**
    option interchanges order)
    **7**:
    correlation between eastward and northward components
    **8**:
    name of station (optional).

**-Sn**\ *barscale.*

Anisotropy bars. *Barscale* sets the scaling of the bars This scaling
gives inches (unless **c**, **i**, **m**, or **p** is appended).
Parameters are expected to be in the following columns:

    **1**,\ **2**:
    longitude, latitude of station (**-:** option interchanges order)
    **3**,\ **4**:
    eastward, northward components of anisotropy vector (**-:** option
    interchanges order)

**-Sr**\ *velscale/confidence/fontsize*

Velocity ellipses in rotated convention. *Vscale* sets the scaling of
the velocity arrows. This scaling gives inches (unless **c**, **i**,
**m**, or **p** is appended). *Confidence* sets the 2-dimensional
confidence limit for the ellipse, e.g., 0.95 for 95% confidence ellipse.
*Fontsize* sets the size of the text in points. The ellipse will be
filled with the color or shade specified by the **-G** option [default
transparent]. The arrow and the circumference of the ellipse will be
drawn with the pen attributes specified by the **-W** option. Parameters
are expected to be in the following columns:

    **1**,\ **2**:
    longitude, latitude, of station (**-:** option interchanges order)
    **3**,\ **4**:
    eastward, northward velocity (**-:** option interchanges order)
    **5**,\ **6**:
    semi-major, semi-minor axes
    **7**:
    counter-clockwise angle, in degrees, from horizontal axis to major
    axis of ellipse.
    **8**:
    name of station (optional)

**-Sw**\ *wedge\_scale/wedge\_mag*.

Rotational wedges. *Wedge\_scale* sets the size of the wedges in inches
(unless **c**, **i**, **m**, or **p** is appended). Values are
multiplied by *Wedge\_mag* before plotting. For example, setting
*Wedge\_mag* to 1.e7 works well for rotations of the order of 100
nanoradians/yr. Use **-G** to set the fill color or shade for the wedge,
and **-E** to set the color or shade for the uncertainty. Parameters are
expected to be in the following columns:

    **1**,\ **2**:
    longitude, latitude, of station (**-:** option interchanges order)
    **3**:
    rotation in radians
    **4**:
    rotation uncertainty in radians

**-Sx**\ *cross\_scale*

gives Strain crosses. *Cross\_scale* sets the size of the cross in
inches (unless **c**, **i**, **m**, or **p** is appended). Parameters
are expected to be in the following columns:

    **1**,\ **2**:
    longitude, latitude, of station (**-:** option interchanges order)
    **3**:
    eps1, the most extensional eigenvalue of strain tensor, with
    extension taken positive.
    **4**:
    eps2, the most compressional eigenvalue of strain tensor, with
    extension taken positive.
    **5**:
    azimuth of eps2 in degrees CW from North.

`Optional Arguments <#toc5>`_
-----------------------------

**-A**\ *parameters*
    Modify vector parameters. For vector heads, append vector head
    *size* [Default is 9p]. See VECTOR ATTRIBUTES for specifying
    additional attributes.
**-B**\ [**p**\ \|\ **s**]\ *parameters* (\*)
    Set map boundary intervals.
**-D**\ *Sigma\_scale*
    can be used to rescale the uncertainties of velocities (**-Se** and
    **-Sr**) and rotations (**-Sw**). Can be combined with the
    *confidence* variable.
**-F**\ *fill*
    Sets the color or shade used for frame and annotation. [Default is
    black]
**-E**\ *fill*
    Sets the color or shade used for filling uncertainty wedges
    (**-Sw**) or velocity error ellipses (**-Se** or **-Sr**). [If
    **-E** is not specified, the uncertainty regions will be
    transparent.]
**-G**\ *fill*
    Specify color (for symbols/polygons) or pattern (for polygons)
    [Default is black]. Optionally, specify
    **-Gp**\ *icon\_size/pattern*, where *pattern* gives the number of
    the image pattern (1-90) OR the name of a icon-format file.
    *icon\_size* sets the unit size in inches. To invert black and white
    pixels, use **-GP** instead of **-Gp**. See **pspatterns** for
    information on individual patterns.
**-K** (\*)
    Do not finalize the *PostScript* plot.
**-L**
    Draw lines. Ellipses and fault planes will have their outlines drawn
    using current pen (see **-W**).
**-N**
    Do **NOT** skip symbols that fall outside the frame boundary
    specified by **-R**. [Default plots symbols inside frame only].
**-O** (\*)
    Append to existing *PostScript* plot.
**-P** (\*)
    Select "Portrait" plot orientation.
**-U**\ [*just*/*dx*/*dy*/][**c**\ \|\ *label*] (\*)
    Draw GMT time stamp logo on plot.
**-V**\ [*level*\ ] (\*)
    Select verbosity level [c].
**-W**
    Set pen attributes for velocity arrows, ellipse circumference and
    fault plane edges. [Defaults: width = default, color = black, style
    = solid].
**-X**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *x-shift*\ [**u**\ ]]
**-Y**\ [**a**\ \|\ **c**\ \|\ **f**\ \|\ **r**][\ *y-shift*\ [**u**\ ]]
(\*)
    Shift plot origin.
**-c**\ *copies* (\*)
    Specify number of plot copies [Default is 1].
**-h**\ [**i**\ \|\ **o**][*n*\ ] (\*)
    Skip or produce header record(s).
**-i**\ *cols*\ [**l**\ ][\ **s**\ *scale*][\ **o**\ *offset*][,\ *...*](\*)
    Select input columns.
**-:**\ [**i**\ \|\ **o**] (\*)
    Swap 1st and 2nd column on input and/or output.
**-^** (\*)
    Print a short message about the syntax of the command, then exits.
**-?** (\*)
    Print a full usage (help) message, including the explanation of
    options, then exits.
**--version** (\*)
    Print GMT version and exit.
**--show-sharedir** (\*)
    Print full path to GMT share directory and exit.

`Vector Attributes <#toc6>`_
----------------------------

Several modifiers may be appended to the vector-producing options to
specify the placement of vector heads, their shapes, and the
justification of the vector:

**+a**\ *angle* sets the angle of the vector head apex [30].

**+b** places a vector head at the beginning of the vector path [none].

**+e** places a vector head at the end of the vector path [none].

**+g**-\|\ *fill* turns off vector head fill (if -) or sets the vector
head fill [Default fill is used, which may be no fill].

**+l** draws half-arrows, using only the left side [both].

**+n**\ *norm* scales down vector attributes (pen thickness, head size)
with decreasing length, where vectors shorter than *norm* will have
their attributes scaled by length/\ *norm* [arrow attributes remains
invariant to length].

**+p**\ [-][*pen*\ ] sets the vector pen attributes. If *pen* has a
leading - then the head outline is not drawn. [Default pen is used, and
head outline is drawn]

**+r** draws half-arrows, using only the right side [both].

In addition, all but circular vectors may take these modifiers:

**+j**\ *just* determines how the input *x*,\ *y* point relates to the
vector. Choose from **b**\ eginning [default], **e**\ nd, or
**c**\ enter.

**+s** means the input *angle*, *length* is instead the *x*, *y*
coordinates of the vector end point.

`Examples <#toc7>`_
-------------------

The following should make big red arrows with green ellipses, outlined
in red. Note that the 39% confidence scaling will give an ellipse which
fits inside a rectangle of dimension Esig by Nsig.

psvelo << END -h2 -R-10/10/-10/10 -W0.25p,red -Ggreen -L -Se0.2/0.39/18
-B1g1 -Jx0.4/0.4 -A0.3p -P -V > test.ps

Long. Lat. Evel Nvel Esig Nsig CorEN SITE

(deg) (deg) (mm/yr) (mm/yr)

0. -8. 0.0 0.0 4.0 6.0 0.500 4x6

-8. 5. 3.0 3.0 0.0 0.0 0.500 3x3

0. 0. 4.0 6.0 4.0 6.0 0.500

-5. -5. 6.0 4.0 6.0 4.0 0.500 6x4

5. 0. -6.0 4.0 6.0 4.0 -0.500 -6x4

0. -5. 6.0 -4.0 6.0 4.0 -0.500 6x-4

END

This example should plot some residual rates of rotation in the Western
Transverse Ranges, California. The wedges will be dark gray, with light
gray wedges to represent the 2-sigma uncertainties.

psvelo << END -Sw0.4/1.e7 -W0.75p -Gdarkgray -Elightgray -h1 -D2 -Jm2.2
-R240./243./32.5/34.75 -Bf10ma60m/WeSn -P > test.ps

lon lat spin(rad/yr) spin\_sigma (rad/yr)

241.4806 34.2073 5.65E-08 1.17E-08

241.6024 34.4468 -4.85E-08 1.85E-08

241.0952 34.4079 4.46E-09 3.07E-08

241.2542 34.2581 1.28E-07 1.59E-08

242.0593 34.0773 -6.62E-08 1.74E-08

241.0553 34.5369 -2.38E-07 4.27E-08

241.1993 33.1894 -2.99E-10 7.64E-09

241.1084 34.2565 2.17E-08 3.53E-08

END

`See Also <#toc8>`_
-------------------

`*GMT*\ (1) <GMT.html>`_ , `*psbasemap*\ (1) <psbasemap.html>`_ ,
`*psxy*\ (1) <psxy.html>`_

`References <#toc9>`_
---------------------

Bomford, G., Geodesy, 4th ed., Oxford University Press, 1980.

`Authors <#toc10>`_
-------------------

Kurt Feigl CNRS UMR 5562 Toulouse, France (Kurt.Feigl@.cnes.fr)

Genevieve Patau CNRS UMR 7580 Seismology Dept. Institut de Physique du
Globe de Paris (patau@ipgp.jussieu.fr)
