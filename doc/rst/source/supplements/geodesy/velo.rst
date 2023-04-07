.. index:: ! velo
.. include:: ../module_supplements_purpose.rst_

******
velo
******

|velo_purpose|

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt velo**
[ *table* ]
|-J|\ *parameters*
|SYN_OPT-R|
|-S|\ *format*\ [*scale*][/*args*][**+f**\ *font*]
[ |-A|\ *parameters* ]
[ |SYN_OPT-B| ]
[ |-C|\ *cpt*]
[ |-D|\ *scale* ]
[ |-E|\ *fill* ]
[ |-G|\ *fill* ]
[ |-H|\ [*scale*] ]
[ |-I|\ [*intens*] ]
[ |-L|\ [*pen*\ [**+c**\ [**f**\|\ **l**]]] ]
[ |-N| ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ [*pen*][**+c**\ [**f**\|\ **l**]] ]
[ |-Z|\ [**m**\|\ **e**\|\ **n**\|\ **u**\ ][**+e**] ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-p| ]
[ |SYN_OPT-qi| ]
[ |SYN_OPT-tv| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

.. module_common_begins

Description
-----------

Reads data values from *files* [or standard input] and
will plot the selected geodesy symbol on a map.
You may choose from velocity vectors and their uncertainties,
rotational wedges and their uncertainties, anisotropy bars,
or strain crosses.  Symbol fills or their outlines may be colored
based on constant parameters or via color lookup tables.

.. figure:: /_images/GMT_velo.*
   :width: 600 px
   :align: center

   You can make four different geodetic symbols: Rotational wedges,
   velocity error ellipses with vector, anisotropy bars, and strain crosses.
   The first two symbols allow you to select separate colors and pens for the main
   symbol and its error component.

Required Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_intables.rst_

.. |Add_-J| replace:: |Add_-J_links|
.. include:: /explain_-J.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-R:

.. |Add_-Rgeo| replace:: |Add_-R_auto_table|
.. include:: ../../explain_-Rgeo.rst_

.. _-S:

**-S**
    Selects the meaning of the columns in the data file and the figure to be plotted.  In all
    cases, the scales are in data units per length unit and sizes are in length units (default length
    unit is controlled by :term:`PROJ_LENGTH_UNIT` unless **c**, **i**, or **p** is appended).

    **-Se**\ [*velscale*/]\ *confidence*\ [**+f**\ *font*]

        Velocity ellipses in (N,E) convention. The *velscale* sets the scaling of the velocity arrows.
        If *velscale* is not given the we read it from the data file as an extra column.
        The *confidence* sets the 2-dimensional confidence limit for the ellipse, e.g., 0.95 for 95% confidence ellipse.
        Use **+f** to set the font and size of the text [9p,Helvetica,black]; give **+f**\ 0 to deactivate labeling.
        The arrow will be drawn with the pen attributes specified by the |-W| option and arrow-head can be colored via |-G|.
        The ellipse will be filled with the color or shade specified by the |-E| option [default is transparent],
        and its outline will be drawn if |-L| is selected using the pen selected (by |-W| if not given by |-L|).
        **Note**: If *confidence* is nonzero and neither |-L| nor |-E| are set then we use |-W| (or default) to draw ellipse outlines.
        Parameters are expected to be in the following columns:

            **1**,\ **2**:
            longitude, latitude of station (**-:** option interchanges order)

            **3**,\ **4**:
            eastward, northward velocity (**-:** option interchanges order)

            **5**,\ **6**:
            uncertainty of eastward, northward velocities (1-sigma) (**-:** option interchanges order)

            **7**:
            correlation between eastward and northward components

            **Trailing text**:
            name of station (optional).

    **-Sn**\ [*barscale*]

        Anisotropy bars. The *barscale* sets the scaling of the bars.
        If *barscale* is not given the we read it from the data file as an extra column.
        Unless |-A| is used to change it via modifier **+h**\ *shape*, the vector shape defaults to 0.1.
        Parameters are expected to be in the following columns:

            **1**,\ **2**:
            longitude, latitude of station (**-:** option interchanges order)

            **3**,\ **4**:
            eastward, northward components of anisotropy vector (**-:** option interchanges order)

    **-Sr**\ [*velscale*/]\ *confidence*\ [**+f**\ *font*]

        Velocity ellipses in rotated convention. The *velscale* sets the scaling of the velocity arrows.
        If *velscale* is not given the we read it from the data file as an extra column.
        The *confidence* sets the 2-dimensional confidence limit for the ellipse, e.g., 0.95 for 95% confidence ellipse.
        Use **+f** to set the font and size of the text [9p,Helvetica,black]; give **+f**\ 0 to deactivate labeling.
        The arrow will be drawn with the pen attributes specified by the |-W| option and arrow-head can be colored via |-G|.
        The ellipse will be filled with the color or shade specified by the |-E| option [default is transparent],
        and its outline will be drawn if |-L| is selected using the pen selected (by |-W| if not given by |-L|).
        **Note**: If *confidence* is nonzero and neither |-L| nor |-E| are set then we use |-W| (or default) to draw ellipse outlines.
        Parameters are expected to be in the following columns:

            **1**,\ **2**:
            longitude, latitude, of station (**-:** option interchanges order)

            **3**,\ **4**:
            eastward, northward velocity (**-:** option interchanges order)

            **5**,\ **6**:
            semi-major, semi-minor axes

            **7**:
            counter-clockwise angle, in degrees, from horizontal axis to major axis of ellipse.

            **Trailing text**:
            name of station (optional)

    **-Sw**\ [*wedgescale*/]\ *wedgemag*

        Rotational wedges. The *wedgescale* sets the size of the wedges.
        If *wedgescale* is not given the we read it from the data file as an extra column.
        Rotation values are multiplied by *wedgemag* before plotting. For example, setting
        *wedgemag* to 1.e7 works well for rotations of the order of 100
        nanoradians/yr. Use |-G| to set the fill color or shade for the wedge,
        and |-E| to set the color or shade for the uncertainty. Parameters are
        expected to be in the following columns:

            **1**,\ **2**:
            longitude, latitude, of station (**-:** option interchanges order)

            **3**:
            rotation in radians

            **4**:
            rotation uncertainty in radians

    **-Sx**\ [*cross_scale*]

        gives Strain crosses. The *cross_scale* sets the size of the cross.
        If *cross_scale* is not given the we read it from the data file as an extra column.
        Parameters are expected to be in the following columns:

            **1**,\ **2**:
            longitude, latitude, of station (**-:** option interchanges order)

            **3**:
            eps1, the most extensional eigenvalue of strain tensor, with
            extension taken positive.

            **4**:
            eps2, the most compressional eigenvalue of strain tensor, with extension taken positive.

            **5**:
            azimuth of eps2 in degrees CW from North.

Optional Arguments
------------------

.. _-A:

**-A**\ *parameters*
    Modify vector parameters. For vector heads, append vector head *size* [Default is 9p].
    See `Vector Attributes`_ for specifying additional attributes.

.. |Add_-B| replace:: |Add_-B_links|
.. include:: ../../explain_-B.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-C:

**-C**\ *cpt*
    Give a CPT and let symbol color normally set by |-G| be
    determined from the magnitude.  See |-Z| for other selections.

.. _-D:

**-D**\ *scale*
    Can be used to rescale the uncertainties of velocities (**-Se** and
    **-Sr**) and rotations (**-Sw**) [1].

.. _-E:

**-E**\ *fill* :ref:`(more ...) <-Gfill_attrib>`
    Sets the color or shade used for filling uncertainty wedges
    (**-Sw**) or velocity error ellipses (**-Se** or **-Sr**). If
    |-E| is not specified, the uncertainty regions will be transparent.
    **Note**: Using |-C| and **-Z+e** will update the uncertainty fill
    color based on the selected measure in |-Z| [magnitude error].

.. _-G:

**-G**\ *fill* :ref:`(more ...) <-Gfill_attrib>`
    Select color or pattern for filling of symbols [Default is no fill].
    **Note**: Using |-C| (and optionally |-Z|) will update the symbol fill
    color based on the selected measure in |-Z| [magnitude].

.. _-H:

**-H**\ [*scale*]
    Scale symbol sizes and pen widths on a per-record basis using the *scale* read from the
    data set, given as the first column after the (optional) *z* and *size* columns [no scaling].
    The symbol size is either provided by |-S| or via the input *size* column.  Alternatively,
    append a constant *scale* that should be used instead of reading a scale column.

.. _-I:

**-I**\ *intens*
    Use the supplied *intens* value (nominally in the -1 to +1 range) to
    modulate the symbol fill color by simulating illumination [none].
    If no intensity is provided we will instead read *intens* from an extra
    data column after the required input columns determined by |-S|.

.. _-L:

**-L**\ [*pen*\ [**+c**\ [**f**\|\ **l**]]]
    Draw lines. Ellipses and rotational wedges will have their outlines drawn
    using current pen (see |-W|).  Alternatively, append a separate pen to
    use for the error outlines.
    If the modifier **+cl** is appended then the color of the pen are updated from the CPT (see
    |-C|). If instead modifier **+cf** is appended then the color from the cpt
    file is applied to error fill only [Default].  Use just **+c** to set both
    pen and fill color.

.. _-N:

**-N**
    Do **not** skip symbols that fall outside the frame boundary
    specified by |-R|. [Default plots symbols inside frame only].

.. |Add_-U| replace:: |Add_-U_links|
.. include:: ../../explain_-U.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. |Add_-V| replace:: |Add_-V_links|
.. include:: /explain_-V.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-W:

**-W**\ [*pen*][**+c**\ [**f**\|\ **l**]]
    Set pen attributes for velocity arrows, ellipse circumference and
    fault plane edges. [Defaults: width = 0.25p, color = black, style = solid].
    If the modifier **+cl** is appended then the color of the pen are updated from the CPT (see
    |-C|). If instead modifier **+cf** is appended then the color from the cpt
    file is applied to symbol fill only [Default].  Use just **+c** to set both
    pen and fill color.

.. |Add_-XY| replace:: |Add_-XY_links|
.. include:: ../../explain_-XY.rst_
    :start-after: **Syntax**
    :end-before: **Description**

.. _-Z:

**-Z**\ [**m**\|\ **e**\|\ **n**\|\ **u**\ ][**+e**]
    Select the quantity that will be used with the CPT given via |-C| to
    set the fill color.  Choose from **m**\ agnitude (vector magnitude or
    rotation magnitude), **e**\ ast-west velocity, **n**\ orth-south velocity,
    or **u**\ ser-supplied data column (supplied after the required columns).
    To instead use the corresponding error estimates (i.e., vector or rotation
    uncertainty) to lookup the color and paint the error ellipse or wedge
    instead, append **+e**.

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-e.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-h.rst_

.. include:: ../../explain_-icols.rst_

.. |Add_perspective| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_perspective.rst_

.. include:: ../../explain_-qi.rst_
.. include:: ../../explain_-tv_full.rst_
.. include:: ../../explain_colon.rst_
.. include:: ../../explain_help.rst_

.. include:: ../../explain_vectors.rst_

Data Column Order
-----------------

The |-S| option determines how many data columns are required to generate
the selected symbol.  In addition, your use of options |-H|,  |-I| and **-t** will
require extra columns, as will a |-S| option without the *size* or a user-column
selected via **-Zu** for color lookup purposes.  The order of the data record
is fixed regardless of option order, even if not all items may be activated.
We expect data columns to come in the following order::

    lon lat symbol-columns [usercol] [size] [scale] [intens] [transp [transp2]] [trailing-text]

where *symbol-columns* represent the normally required data columns, and items
given in brackets are optional and under the control of the stated options
(the trailing text is always optional).  **Note**:  You can use **-i** to
rearrange your data record to match the expected format.

.. module_common_ends

Examples
--------

.. include:: ../../oneliner_info.rst_

The following should make big red arrows with green ellipses, outlined
in red. Note that the 39% confidence scaling will give an ellipse which
fits inside a rectangle of dimension Esig by Nsig::

  gmt velo << END -R-10/10/-10/10 -W0.6p,red -Egreen -L -Se0.2/0.39/18 -B1g1 -Jx0.4/0.4 -A0.3c+p1p+e+gred -png test
  #Long. Lat. Evel Nvel Esig Nsig CorEN SITE
  #(deg) (deg) (mm/yr) (mm/yr)
  0. -8. 0.0 0.0 4.0 6.0 0.500 4x6
  -8. 5. 3.0 3.0 0.0 0.0 0.500 3x3
  0. 0. 4.0 6.0 4.0 6.0 0.500
  -5. -5. 6.0 4.0 6.0 4.0 0.500 6x4
  5. 0. -6.0 4.0 6.0 4.0 -0.500 -6x4
  0. -5. 6.0 -4.0 6.0 4.0 -0.500 6x-4
  END


This example should plot some residual rates of rotation in the Western
Transverse Ranges, California. The wedges will be dark gray, with light
gray wedges to represent the 2-sigma uncertainties::

    gmt velo << END -Sw0.4i/1.e7 -W0.75p -Gdarkgray -Elightgray -D2 -Jm2.2i -R240./243./32.5/34.75 -Baf -BWeSn -pdf test
    #lon lat spin(rad/yr) spin_sigma (rad/yr)
    241.4806 34.2073 5.65E-08 1.17E-08
    241.6024 34.4468 -4.85E-08 1.85E-08
    241.0952 34.4079 4.46E-09 3.07E-08
    241.2542 34.2581 1.28E-07 1.59E-08
    242.0593 34.0773 -6.62E-08 1.74E-08
    241.0553 34.5369 -2.38E-07 4.27E-08
    241.1993 33.1894 -2.99E-10 7.64E-09
    241.1084 34.2565 2.17E-08 3.53E-08
    END

`Kurt L. Feigl <https://geoscience.wisc.edu/people/feigl-kurt-l/>`_, Department of Geology and
Geophysics at University of Wisconsin-Madison, Madison, Wisconsin, USA

See Also
--------

:doc:`gmt </gmt>`, :doc:`basemap </basemap>`, :doc:`plot </plot>`
