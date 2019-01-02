.. index:: ! velo

******
velo
******

.. only:: not man

    Plot velocity vectors, crosses, and wedges on maps

Synopsis
--------

.. include:: ../../common_SYN_OPTs.rst_

**gmt velo** [ *table* ] |-J|\ *parameters*
|SYN_OPT-R|
[ |-A|\ *parameters* ]
[ |SYN_OPT-B| ]
[ |-E|\ *color* ]
[ |-F|\ *color* ]
[ |-G|\ *color* ]
[ |-L| ]
[ |-N| ]
[ |-S|\ *symbol*/*scale*\ [/*args* ] ]
[ |SYN_OPT-U| ]
[ |SYN_OPT-V| ]
[ |-W|\ *pen* ]
[ |SYN_OPT-X| ]
[ |SYN_OPT-Y| ]
[ |SYN_OPT-di| ]
[ |SYN_OPT-e| ]
[ |SYN_OPT-h| ]
[ |SYN_OPT-i| ]
[ |SYN_OPT-t| ]
[ |SYN_OPT-:| ]
[ |SYN_OPT--| ]

|No-spaces|

Description
-----------

**velo** reads data values from *files* [or standard input] and
will plot velocity arrows on a map.
Most options are the same as for :doc:`plot </plot>`, except **-S**. The previous version
(**velomeca**) is now obsolete. It has been replaced by **velo** and
:doc:`meca`.

Required Arguments
------------------

.. |Add_intables| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_intables.rst_

.. _-J:

.. |Add_-J| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-J.rst_

.. _-R:

.. |Add_-Rgeo| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-Rgeo.rst_

.. _-S:

**-S**

Selects the meaning of the columns in the data file and the figure to be plotted.

    **-Se**\ *velscale/confidence/fontsize*.

        Velocity ellipses in (N,E) convention. *velscale* sets the scaling of the
        velocity arrows. This scaling gives inches (unless **c**, **i**,
        or **p** is appended). *confidence* sets the 2-dimensional confidence
        limit for the ellipse, e.g., 0.95 for 95% confidence ellipse. *fontsize*
        sets the size of the text in points. The ellipse will be filled with the
        color or shade specified by the |-G| option [default transparent]. The
        arrow and the circumference of the ellipse will be drawn with the pen
        attributes specified by the |-W| option. Parameters are expected to be
        in the following columns:

            **1**,\ **2**:
            longitude, latitude of station (**-:** option interchanges order)
            **3**,\ **4**:
            eastward, northward velocity (**-:** option interchanges order)
            **5**,\ **6**:
            uncertainty of eastward, northward velocities (1-sigma) (**-:** option interchanges order)
            **7**:
            correlation between eastward and northward components
            **8**:
            name of station (optional).

    **-Sn**\ *barscale.*

        Anisotropy bars. *barscale* sets the scaling of the bars. This scaling
        gives inches (unless **c**, **i**, or **p** is appended).
        Parameters are expected to be in the following columns:

            **1**,\ **2**:
            longitude, latitude of station (**-:** option interchanges order)
            **3**,\ **4**:
            eastward, northward components of anisotropy vector (**-:** option interchanges order)

    **-Sr**\ *velscale/confidence/fontsize*

        Velocity ellipses in rotated convention. *velscale* sets the scaling of
        the velocity arrows. This scaling gives inches (unless **c**, **i**,
        or **p** is appended). *confidence* sets the 2-dimensional
        confidence limit for the ellipse, e.g., 0.95 for 95% confidence ellipse.
        *fontsize* sets the size of the text in points. The ellipse will be
        filled with the color or shade specified by the |-G| option [default
        transparent]. The arrow and the circumference of the ellipse will be
        drawn with the pen attributes specified by the |-W| option. Parameters
        are expected to be in the following columns:

            **1**,\ **2**:
            longitude, latitude, of station (**-:** option interchanges order)
            **3**,\ **4**:
            eastward, northward velocity (**-:** option interchanges order)
            **5**,\ **6**:
            semi-major, semi-minor axes
            **7**:
            counter-clockwise angle, in degrees, from horizontal axis to major axis of ellipse.
            **8**:
            name of station (optional)

    **-Sw**\ *wedgescale/wedgemag*.

        Rotational wedges. *wedgescale* sets the size of the wedges in inches
        (unless **c**, **i**, or **p** is appended). Values are
        multiplied by *wedgemag* before plotting. For example, setting
        *wedgemag* to 1.e7 works well for rotations of the order of 100
        nanoradians/yr. Use **-G** to set the fill color or shade for the wedge,
        and **-E** to set the color or shade for the uncertainty. Parameters are
        expected to be in the following columns:

            **1**,\ **2**:
            longitude, latitude, of station (**-:** option interchanges order)
            **3**:
            rotation in radians
            **4**:
            rotation uncertainty in radians

    **-Sx**\ *cross_scale*

        gives Strain crosses. *cross_scale* sets the size of the cross in
        inches (unless **c**, **i**, or **p** is appended). Parameters
        are expected to be in the following columns:

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

.. _-B:

.. include:: ../../explain_-B.rst_

.. _-D:

**-D**\ *Sigma\_scale*
    can be used to rescale the uncertainties of velocities (**-Se** and
    **-Sr**) and rotations (**-Sw**). Can be combined with the *confidence* variable.

.. _-E:

**-E**\ *fill*
    Sets the color or shade used for filling uncertainty wedges
    (**-Sw**) or velocity error ellipses (**-Se** or **-Sr**). [If
    **-E** is not specified, the uncertainty regions will be transparent.]

.. _-F:

**-F**\ *fill*
    Sets the color or shade used for frame and annotation. [Default is black]

.. _-G:

**-G**\ *fill*
    Specify color (for symbols/polygons) or pattern (for polygons)
    [Default is black]. Optionally, specify
    **-Gp**\ *icon\_size/pattern*, where *pattern* gives the number of
    the image pattern (1-90) OR the name of a icon-format file.
    *icon\_size* sets the unit size in inches. To invert black and white
    pixels, use **-GP** instead of **-Gp**. See the CookBook for
    information on individual patterns.

.. _-L:

**-L**
    Draw lines. Ellipses and fault planes will have their outlines drawn
    using current pen (see **-W**).

.. _-N:

**-N**
    Do **NOT** skip symbols that fall outside the frame boundary
    specified by **-R**. [Default plots symbols inside frame only].

.. _-U:

.. include:: ../../explain_-U.rst_

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-V.rst_

.. _-W:

**-W**
    Set pen attributes for velocity arrows, ellipse circumference and
    fault plane edges. [Defaults: width = default, color = black, style = solid].

.. _-X:

.. include:: ../../explain_-XY.rst_

.. |Add_-di| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-di.rst_

.. |Add_-e| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-e.rst_

.. |Add_-h| unicode:: 0x20 .. just an invisible code
.. include:: ../../explain_-h.rst_

.. include:: ../../explain_-icols.rst_
.. include:: ../../explain_-t.rst_
.. include:: ../../explain_colon.rst_
.. include:: ../../explain_help.rst_

.. include:: ../../explain_vectors.rst_

Examples
--------

The following should make big red arrows with green ellipses, outlined
in red. Note that the 39% confidence scaling will give an ellipse which
fits inside a rectangle of dimension Esig by Nsig.

   ::

    gmt velo << END -h2 -R-10/10/-10/10 -W0.25p,red -Ggreen -L -Se0.2/0.39/18
        -B1g1 -Jx0.4/0.4 -A1c+p3p+e -V -pdf test
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
gray wedges to represent the 2-sigma uncertainties.

   ::

    gmt velo << END -Sw0.4/1.e7 -W0.75p -Gdarkgray -Elightgray -h1 -D2 -Jm2.2i
        -R240./243./32.5/34.75 -Baf -BWeSn -pdf test.ps
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

See Also
--------

:doc:`meca`,
:doc:`polar`,
:doc:`coupe`,
:doc:`gmt </gmt>`, :doc:`basemap </basemap>`, :doc:`plot </plot>`

References
----------

Bomford, G., Geodesy, 4th ed., Oxford University Press, 1980.

Authors
-------

`Kurt L. Feigl <http://www.geology.wisc.edu/~feigl/>`_, Department of Geology and
Geophysics at University of Wisconsin-Madison, Madison, Wisconsin, USA

Genevieve Patau, `Laboratory of Seismogenesis <http://www.ipgp.fr/rech/sismogenese/>`_,
Institut de Physique du Globe de Paris, Departement de Sismologie, Paris, France
