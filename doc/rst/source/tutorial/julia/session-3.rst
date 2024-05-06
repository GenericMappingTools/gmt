Session Three
=============

Contouring gridded data sets
----------------------------

GMT comes with several utilities that can create gridded data sets; we will discuss two such modules later
this session.  The data sets needed for this tutorial are obtained via the Internet as they are needed.
Here, we will use :doc:`/grdcut` to obtain and extract a GMT-ready grid that we will next use for contouring::

    G = grdcut("@earth_relief_05m", region=(-66,-60,30,35), verbose=true)

Here we use the file extension .nc instead of the generic .grd to indicate that this is a netCDF file.
It is good form, but not essential, to use .nc for netCDF grids. Using that extension will help other
programs installed on your system to recognize these files and might give it an identifiable icon in
your file browser.  Learn about other programs that read netCDF files at the
`netCDF website <https://www.unidata.ucar.edu/software/netcdf/>`_.
You can also obtain tut_bathy.nc from the GMT cache server as we are doing below. Feel free to open it
in any other program and compare results with GMT.

We first use the GMT module :doc:`/grdinfo` to see what's in this file::

    grdinfo(G)

The file contains bathymetry for the Bermuda region and has depth values from -5475 to -89 meters. We want to

As with previous plot commands we need to set up the map projection with **proj**. Here, however, we do not have
to specify the region since that is by default assumed to be the extent of the grid file. To generate any plot we
will in addition need to supply information about which contours to draw. Unfortunately,
`grdcontour <https://www.generic-mapping-tools.org/GMTjl_doc/documentation/modules/grdcontour>`_ is a complicated module with too
many options. We put a positive spin on this situation by touting its flexibility. Here are the most useful options:

  +-----------------------------------------------------------------+----------------------------------------------------------------------+
  | Option                                                          |  Purpose                                                             |
  +=================================================================+======================================================================+
  | **annot**\ =\ *annot\_int*                                      | Annotation interval and attributes                                   |
  +-----------------------------------------------------------------+----------------------------------------------------------------------+
  | **cont**\ =\ *cont\_int*                                        | Contour interval                                                     |
  +-----------------------------------------------------------------+----------------------------------------------------------------------+
  | **labels**\ *gap*                                               | Controls placement of contour annotations                            |
  +-----------------------------------------------------------------+----------------------------------------------------------------------+
  | **range**\ =\ *(low,high)*                                      | Only draw contours within the *low* to *high* range                  |
  +-----------------------------------------------------------------+----------------------------------------------------------------------+
  | **cut**\ =\ *cut*                                               | Do not draw contours with fewer than *cut* points                    |
  +-----------------------------------------------------------------+----------------------------------------------------------------------+
  | **smooth**\ =\ *factor*                                         | Resample contours *smooth* times per grid cell increment             |
  +-----------------------------------------------------------------+----------------------------------------------------------------------+
  | **ticks**\ =(local_high=true, local_low=true,                   | Draw tick-marks in downhill                                          |
  |  gap=\ *gap*, closed=true, labels=lab)                          |                                                                      |
  +-----------------------------------------------------------------+----------------------------------------------------------------------+
  |                                                                 | direction for innermost closed contours.  Add tick spacing           |
  +-----------------------------------------------------------------+----------------------------------------------------------------------+
  |                                                                 | and length, and characters to plot at the center of closed contours  |
  +-----------------------------------------------------------------+----------------------------------------------------------------------+
  | **pen**\ =(annot=true, contour=true, pen=\ *pen*,               | Set contour and annotation pens                                      |
  |  colored=true, cline=true, ctext=true)                          |                                                                      |
  +-----------------------------------------------------------------+----------------------------------------------------------------------+
  | **scale**\ =(factor=\ *factor*, shift=\ *shift*, periodic=true) | Subtract *offset* and multiply data by *factor* prior to processing  |
  +-----------------------------------------------------------------+----------------------------------------------------------------------+

We will first make a plain contour map using 1 km as annotation interval and 250 m as contour interval.
We choose a 15-cm-wide Mercator plot and annotate the borders every 2°:

   grdcontour("@earth_relief_05m", region=(-66,-60,30,35), proj=:Mercator, figsize=15, cont=250, annot=1000, show=true)

Your plot should look like :ref:`our example 11 below <gmt_tut_11_jl>`

.. _gmt_tut_11_jl:

.. figure:: /_images/GMT_tut_11.*
   :width: 400 px
   :align: center

   Result of GMT Tutorial example 11

Exercises:

#. Add smoothing with **smooth**\ =4.

#. Try tick all highs and lows with **ticks**.

#. Skip small features with **cut**\ =10.

#. Override region using **region**\ =(-70,-60,25,35)

#. Try another region that clips our data domain.

#. Scale data to km and use the km unit in the annotations.

Gridding of arbitrarily spaced data
-----------------------------------

Except in the situation above when a grid file is available, we must convert our data to the right format
readable by GMT before we can make contour plots and color-coded images. We distinguish between two scenarios:

#. The (*x, y, z*) data are available on a regular lattice grid.

#. The (*x, y, z*) data are distributed unevenly in the plane.

The former situation may require a simple reformatting (using :doc:`/xyz2grd`), while the latter must be
interpolated onto a regular lattice; this process is known as gridding. GMT supports three different
approaches to gridding; here, we will briefly discuss the two most common techniques.

All GMT gridding modules have in common the requirement that the
user must specify the grid domain and output filename:

======================================= ======================================================================
Option                                  Purpose
======================================= ======================================================================
**region**\ =\ *(xmin,xmax/ymin,ymax)*  The desired grid extent
**inc**\ =\ *(xinc [,yinc])*            The grid spacing (append **m** or **s** for minutes or seconds of arc)
**outgrid**\ =\ *gridfile*              The output grid filename
======================================= ======================================================================

Nearest neighbor gridding
~~~~~~~~~~~~~~~~~~~~~~~~~

.. figure:: /_images/GMT_nearneighbor.*
   :width: 200 px
   :align: center

   Search geometry for nearneighbor.

The GMT module :doc:`/nearneighbor` implements a simple "nearest neighbor" averaging operation.
It is the preferred way to grid data when the data density is high. :doc:`/nearneighbor` is a
local procedure which means it will only consider the control data that is close to the desired
output grid node. Only data points inside a specified search radius will be used, and we may also
impose the condition that each of the *n* sectors must have at least one data point in order to
assign the nodal value. The nodal value is computed as a weighted average of the nearest data
point per sector inside the search radius, with each point weighted according to its distance
from the node. The most important switches are listed below.

=========================== =====================================================================================
Option                      Purpose
=========================== =====================================================================================
**search_radius**\ =\ *val* Sets search radius.  Append *unit* for radius in that unit [Default is *x*-units]
**empty**\ =*val*           Assign this value to unconstrained nodes [Default is NaN]
**sectors**\ =*n*           Sector search, indicate number of sectors [Default is 4]
**weights**\ =true          Read relative weights from the 4th column of input data
=========================== =====================================================================================

We will grid the data in the file tut_ship.xyz which contains ship observations of bathymetry off
Baja California. We obtain the file via the cache server as before. We desire to make a 5' by 5' grid.
Running gmt info on @tut_ship.xyz yields::

    tut_ship.xyz: N = 82970     <245/254.705>   <20/29.99131>   <-7708/-9>

so we choose the region accordingly, and get a view of the contour map using

   ::

    G = nearneighbor("@tut_ship.xyz", region=(245,255,20,30), inc="5m", search_radius="40k");
    grdcontour(G, proj=:Mercator, figsize=15, cont=250, annot=1000, show=true)

Your plot should look like :ref:`our example 12 below <gmt_tut_12_jl>`

.. _gmt_tut_12_jl:

.. figure:: /_images/GMT_tut_12.*
   :width: 400 px
   :align: center

   Result of GMT Tutorial example 12

Since the grid ship.nc is stored in netCDF format that is supported by a host of other modules,
you can try one of those as well on the same grid.

Exercises:

#. Try using a 100 km search radius and a 10 minute grid spacing.


Gridding with Splines in Tension
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As an alternative, we may use a global procedure to grid our data. This approach, implemented in the module
:doc:`/surface`, represents an improvement over standard minimum curvature algorithms by allowing users to
introduce some tension into the surface. Physically, we are trying to force a thin elastic plate to go
through all our data points; the values of this surface at the grid points become the gridded data.
Mathematically, we want to find the function *z(x, y)* that satisfies the following equation away from data constraints:

.. math::

    (1-t)\nabla ^2 z -  t \nabla z = 0,

where *t* is the "tension" in the 0-1 range. Basically, for zero tension we obtain the minimum curvature
solution, while as tension goes toward unity we approach a harmonic solution (which is linear in cross-section).
The theory behind all this is quite involved and we do not have the time to explain it all here, please see
*Smith and Wessel* [1990] for details. Some of the most important switches for this module are indicated below.

============================ =========================================================
Option                       Purpose
============================ =========================================================
**aspect_ratio**\ =\ *val*   Sets aspect ratio for anisotropic grids.
**convergence**\ =\ *limit*  Sets convergence limit.  Default is 1/1000 of data range.
**tension**\ =\ *val*        Sets the tension [Default is 0]
============================ =========================================================

Preprocessing
-------------

The :doc:`/surface` module assumes that the data have been preprocessed to eliminate aliasing,
hence we must ensure that this step is completed prior to gridding. GMT comes with three preprocessors, called
`blockmean <https://www.generic-mapping-tools.org/GMTjl_doc/documentation/modules/blockmean>`_,
`blockmedian <https://www.generic-mapping-tools.org/GMTjl_doc/documentation/modules/blockmedian/>`_, and
`blockmode <https://www.generic-mapping-tools.org/GMTjl_doc/documentation/modules/blockmode/>`_. The first averages values inside the
grid-spacing boxes, the second returns median values, wile the latter returns modal values. As a rule of thumb,
we use means for most smooth data (such as potential fields) and medians (or modes) for rough, non-Gaussian data
(such as topography). In addition to the required **region** and **inc** switches, these preprocessors all take
the same options shown below:

=========================== ====================================================================
Option                      Purpose
=========================== ====================================================================
**reg**\ =true              Choose pixel node registration [Default is gridline]
=========================== ====================================================================

With respect to our ship data we preprocess it using the median method::

    D = blockmedian("@tut_ship.xyz", region=(245,255,20,30), inc="5m", verbose=true);

The output data can now be used with surface::

    G = surface(D, region=(245,255,20,30), inc="5m", verbose=true);

If you rerun `grdcontour <https://www.generic-mapping-tools.org/GMTjl_doc/documentation/modules/grdcontour/>`_ on the new grid file
(try it!) you will notice a big difference compared to the grid made by :doc:`/nearneighbor`: since surface is a
global method it will evaluate the solution at all nodes, even if there are no data constraints. There are numerous
options available to us at this point:

#. We can reset all nodes too far from a data constraint to the NaN value.

#. We can pour white paint over those regions where contours are unreliable.

#. We can plot the landmass which will cover most (but not all) of the unconstrained areas.

#. We can set up a clip path so that only the contours in the constrained region will show.

Here we have only time to explore the latter approach. The :doc:`/mask` module can read the same preprocessed
data and set up a contour mask based on the data distribution. Once the clip path is activated we can contour
the final grid; we finally deactivate the clipping with a second call to :doc:`/mask`. Note also that since
we are appending layers to the figure, second and on commands use the bang (**!**) form. Here's the recipe:

   ::

    D = blockmedian("@tut_ship.xyz", region=(245,255,20,30), inc="5m", verbose=true);
    G = surface(D, region=(245,255,20,30), inc="5m", verbose=true);
    mask(D, region=(245,255,20,30), inc="5m", figsize=15)
    grdcontour!(G, cont=250, annot=1000)
    mask!(end_clip_path=true, show=true)
    

Your plot should look like :ref:`our example 13 below <gmt_tut_13_jl>`

.. _gmt_tut_13_jl:

.. figure:: /_images/GMT_tut_13.*
   :width: 400 px
   :align: center

   Result of GMT Tutorial example 13

Exercises:

#. Add the continents using any color you want.

#. Color the clip path light gray (use **fill** in the first :doc:`/mask` call).
