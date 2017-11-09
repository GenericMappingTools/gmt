.. index:: ! grd2kml

********
grd2kml
********

.. only:: not man

    grd2kml - Create KML image quadtree from single grid

Synopsis
--------

.. include:: common_SYN_OPTs.rst_

**grd2kml** *grid* 
[ |-C|\ *cpt* ]
[ |-D| ]
[ |-E|\ *URL* ]
[ |-F|\ *prefix* ]
[ |-I|\ *intensfile* ]
[ |-L|\ *tilesize* ]
[ |SYN_OPT-V| ]

|No-spaces|

Description
-----------

**grd2kml** reads a large 2-D grid file and makes a quadtree set of
PNG images and KML wrappers for Google Earth.
Optionally, illumination may be added by providing a grid file with
intensities in the (-1,+1) range. Values outside this range will be
clipped. Such intensity files can be created from the grid using
:doc:`grdgradient` and, optionally, modified by :doc:`grdmath` or
:doc:`grdhisteq`.  Colors are specified via a color palette lookup table.


Required Arguments
------------------

*grid*
    A 2-D gridded data set (See GRID FILE FORMATS below.)

Optional Arguments
------------------

.. _-C:

**-C**\ *cpt*
    Name of the CPT. Alternatively,
    supply the name of a GMT color master dynamic CPT [rainbow] to
    automatically determine a continuous CPT from
    the grid's z-range.  If the dynamic CPT has a default range then
    that range will be imposed instead.
    Yet another option is to specify **-C**\ *color1*\ ,\ *color2*\ [,\ *color3*\ ,...]
    to build a linear continuous CPT from those colors automatically.
    In this case *color1* etc can be a r/g/b triplet, a color name,
    or an HTML hexadecimal color (e.g. #aabbcc ).

.. _-D:

**-D**
    Write a listing of the quadtree dependencies to stdout [Default is
    no listing.]

.. _-E:

**-E**\ *URL*
    Instead of hosting the files locally, prepend a URL to all KML and PNG files except
    for the top-level named KML file [local setup only].

.. _-F:

**-F**\ *prefix*
    Sets a unique prefixed used for the top-level KML filename and the
    directory where all products will be written [GMT_Quadtree].

.. _-I:

**-I**\ *intensfile*
    Gives the name of a grid file with intensities in the (-1,+1) range.
    [Default is no illumination].

.. _-L:

**-L**\ *tilesize*
    Sets the size of the image building blocks.  Must be an integer that
    is radix 2.  Typical values are 256 or 512 [256].

.. _-V:

.. |Add_-V| unicode:: 0x20 .. just an invisible code
.. include:: explain_-V.rst_


Examples
--------

To make a quadtree image representation of the large topography grid file ellice_basin.nc, using
the default tile size, and supply shading based on the topography, try

   ::

    gmt grdgradient ellice_basin.nc -A-10,80 -Nt1 -Gellice_basin_int.nc
    gmt grd2kml ellice_basin.nc -Iellice_basin_int.nc -Fellice

See Also
--------

:doc:`gmt`,
:doc:`gmt.conf`,
:doc:`gmt2kml`,
:doc:`grdgradient`,
:doc:`grdhisteq`,
:doc:`grdmath`,
:doc:`kml2gmt`,
:doc:`psconvert`
